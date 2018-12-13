//#define NDEBUG
#include <cassert>
#include <utilities.h>

#include "SdDriver.h"
#include "AudioDriver.h"
#include "OLEDDriver.h"

#include "InterruptHandler.h"


// TODO shared global vars - find way to package
QueueHandle_t mp3QueueHandle;
QueueHandle_t mp3CmdTaskHandle;
QueueHandle_t txtCmdTaskHandle;

/** vSendFilesFromCmd Task, @Priority = Low
 *  @resumes from terminal task 'play' command
 *  retrieves mp3/txt name param and
 *  fill vPlayMp3FilesFromCmd/vPlayTxtFilesFromCmd queue
 */

void vSendFilesFromCmd(void * pvParameter) {
    auto * audio = (AudioDriver *)pvParameter;
    FRESULT res;
    FIL fil;
    UINT br;

    char * fileNamePtr;
    uint8_t buffer[512]; // init as 0
    uint8_t * const bufferProducer = buffer;
    QueueHandle_t toCmdTask;

    for (;;) {
        xQueueReceive(sdFileCmdTaskHandle, &fileNamePtr, portMAX_DELAY);        /* wait on 'play' command */
        res = f_open(&fil, fileNamePtr, FA_OPEN_EXISTING | FA_READ);
        if (res == FR_OK) {
            checkExtension(fileNamePtr, ".mp3") ? toCmdTask = mp3CmdTaskHandle  /* check .mp3/.txt */
                                                : toCmdTask = txtCmdTaskHandle;

            for (;;) {   /* playing current song */

                // stop song and play next song on another 'play' command received
                if (uxQueueSpacesAvailable(sdFileCmdTaskHandle) == 0) {  /* received another item from queue */
                    memset(buffer, 0, sizeof(buffer));                   /* empty buffer */
                    audio->stopPlayback();
                    break;
                }

                res = f_read(&fil, buffer, sizeof(buffer), &br);    /* read file from queue */
                if (res || br == 0) break;                          /* error or eof */
                xQueueSend(toCmdTask, &bufferProducer, portMAX_DELAY);
            }
            f_close(&fil);
            u0_dbg_put("\nfile done sending\n");
        } else {
            u0_dbg_printf("\nERROR: %s not found\n", &fileNamePtr[2]);
        }
    }
}

/** vPlayMp3FilesFromCmd Task, @Priority = High
 *  @resumes from vSendFilesFromCmd task
 *  audio decodes 512 byte mp3 blocks
 *  and encodes as music from sd card
 */

void vPlayMp3FilesFromCmd(void * pvParameter) {
    auto * audio = (AudioDriver *)pvParameter;
    const uint8_t * bufferConsumer;
    const uint16_t BUFFER_SIZE = 512;
    for (;;) {
        xQueueReceive(mp3CmdTaskHandle, &bufferConsumer, portMAX_DELAY);
        audio->SDI_W(bufferConsumer, BUFFER_SIZE);
    }
}

/** vPlayTxtFilesFromCmd Task, @Priority = High
 *  @resumes from vSendFilesFromCmd task
 *  prints content of text file from sd card
 */

void vPlayTxtFilesFromCmd(void *) {
    const char * bufferConsumer;
    for (;;) {
        xQueueReceive(txtCmdTaskHandle, &bufferConsumer, portMAX_DELAY);
        u0_dbg_printf("%s", bufferConsumer);
    }
}

/** vSendMp3Files Task, @Priority = Low
 *  @resumes from button isr
 *  send current mp3 file pointed and
 *  fill vReadMp3File queue,
 *  automatically goes to next song in vector
 */

void vSendMp3Files(void * pvParameter) {
    auto * const sd = (SdDriver *)pvParameter;
    FRESULT res;
    FIL fil;
    UINT br;

    uint8_t buffer[512];
    uint8_t * const bufferProducer = buffer;

    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* wait on button semaphore signal from isr */
    for (;;) {
        res = f_open(&fil, sd->getCurrentFileName(), FA_OPEN_EXISTING | FA_READ);
        if (res == FR_OK) {
            for (;;) {                                              /* playing current song */
                if (uxSemaphoreGetCount(xSemaphore[1][SW_P2_2]) == 1) {    /* if play/pause isr pressed */
                    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* consume entry semaphore */
                    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* pause then resume after receiving next sem */
                }

                res = f_read(&fil, buffer, sizeof(buffer), &br);    /* read file from sdFile vector */
                if (res || br == 0) break;                          /* error or eof */
                xQueueSend(mp3QueueHandle, &bufferProducer, portMAX_DELAY); /* fill producer queue */
            }
            f_close(&fil);
            sd->setNextSong();
            u0_dbg_printf("finished song\n");
        }
    }
}

/** vPlayMp3Files Task, @Priority = High
 *  @resumes from vSendMp3Files task
 *  audio decodes 512 byte mp3 blocks
 *  and encodes as music
 */

void vPlayMp3Files(void * pvParameter) {
    auto * const audio = (AudioDriver *)pvParameter;
    uint8_t * bufferConsumer;
    const uint16_t bufferSize = 512;
    for (;;) {
        xQueueReceive(mp3QueueHandle, &bufferConsumer, portMAX_DELAY);
        audio->SDI_W(bufferConsumer, bufferSize);
    }
}

/** vIncrementVolume Task, @Priority = High
 *  @resumes from incrementVolumeFromISR task
 *  increases volume level by one
 */

void vIncrementVolume(void * pvParameter) {
    auto * audio = (AudioDriver *)pvParameter;
    for (;;) {
        xSemaphoreTake(xSemaphore[1][SW_P2_0], portMAX_DELAY);
        audio->incrementVolume();
    }
}

/** vDecrementVolume Task, @Priority = High
 *  @resumes from decrementVolumeFromISR task
 *  decreases volume level by one
 */

void vDecrementVolume(void * pvParameter) {
    auto * audio = (AudioDriver *)pvParameter;
    for (;;) {
        xSemaphoreTake(xSemaphore[1][SW_P2_1], portMAX_DELAY);
        audio->decrementVolume();
    }
}

int main(void) {
    /// This "stack" memory is enough for each task to run properly (512 * 32-bit (4 bytes)) = 2Kbytes stack
    const uint32_t STACK_SIZE_WORDS = 0x00000200;

    auto * const sd = new SdDriver();
    auto * const audio = new AudioDriver();
    auto * const oled = new OLEDDriver();

    assert(sd->Init() && audio->Init() && Init_MP3_GPIO_Interrupts() && oled->Init());

    /// command line queue handles
    sdFileCmdTaskHandle = xQueueCreate(1, sizeof(uint8_t *));
    mp3CmdTaskHandle = xQueueCreate(2, sizeof(uint8_t *));
    txtCmdTaskHandle = xQueueCreate(2, sizeof(uint8_t *));

    /// isr queue handles
    mp3QueueHandle = xQueueCreate(2, sizeof(uint8_t *));

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    /// command line handled tasks
    xTaskCreate(vPlayMp3FilesFromCmd, "cmp3cmd", STACK_SIZE_WORDS, audio,   PRIORITY_HIGH, nullptr);
    xTaskCreate(vPlayTxtFilesFromCmd, "ctxtcmd", STACK_SIZE_WORDS, nullptr, PRIORITY_HIGH, nullptr);
    xTaskCreate(vSendFilesFromCmd,    "pmp3cmd", STACK_SIZE_WORDS, audio,   PRIORITY_LOW,  nullptr);

    /// isr handled tasks
    xTaskCreate(vPlayMp3Files,        "cmp3",    STACK_SIZE_WORDS, audio,   PRIORITY_HIGH, nullptr);
    xTaskCreate(vSendMp3Files,        "pmp3",    STACK_SIZE_WORDS, sd,      PRIORITY_LOW,  nullptr);

    xTaskCreate(vIncrementVolume,     "vinc",    STACK_SIZE_WORDS, audio,   PRIORITY_HIGH, nullptr);
    xTaskCreate(vDecrementVolume,     "vdec",    STACK_SIZE_WORDS, audio,   PRIORITY_HIGH, nullptr);

    scheduler_start();

    for (;;); /* endless loop - retain code execution */
}

