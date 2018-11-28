#include <stdint.h>
#include <stdio.h>
//#define NDEBUG
#include <cassert>
#include <utilities.h>
//#include "printf_lib.h"

#include "SdDriver.h"
#include "AudioDriver.h"

// TODO shared global vars - find way to package
QueueHandle_t mp3QueueHandle;
QueueHandle_t mp3CmdTaskHandle;
QueueHandle_t txtCmdTaskHandle;
SemaphoreHandle_t startMp3Handle;

/** vSendFilesFromCmd Task, @Priority = Low
 *  @resumes from terminal task 'play' command
 *  retrieves mp3/txt name param and
 *  fill vPlayMp3FilesFromCmd/vPlayTxtFilesFromCmd queue
 */

void vSendFilesFromCmd(void * pvParameter) {
    auto * sd = (SdDriver *)pvParameter;
    FRESULT res;
    FIL fil;
    UINT br;

    char * fileNamePtr;
    uint8_t buffer[512];
    uint8_t * const bufferProducer = buffer;
    QueueHandle_t toCmdTask;

    for (;;) {
        xQueueReceive(sdFileCmdTaskHandle, &fileNamePtr, portMAX_DELAY); /* wait on 'play' command */

        res = f_open(&fil, fileNamePtr, FA_OPEN_EXISTING | FA_READ);
        if (res == FR_OK) {
            checkExtension(sd->getCurrentFileName(), ".mp3") ? toCmdTask = mp3CmdTaskHandle /* check .mp3/.txt */
                                                             : toCmdTask = txtCmdTaskHandle;
            for (;;) {                                              /* playing current song */
                res = f_read(&fil, buffer, sizeof(buffer), &br);    /* read file from mp3File vector */
                if (res || br == 0) break;                          /* error or eof */
                xQueueSendToBack(toCmdTask, &bufferProducer, portMAX_DELAY);
            }
            u0_dbg_put("finish song\n");
        } else {
            u0_dbg_put("ERROR: file not found\n");
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
    uint8_t * bufferConsumer;
    const uint16_t bufferSize = 512;
    for (;;) {
        xQueueReceive(mp3CmdTaskHandle, &bufferConsumer, portMAX_DELAY);
        audio->SDI_W(bufferConsumer, bufferSize);
    }
}

/** vPlayTxtFilesFromCmd Task, @Priority = High
 *  @resumes from vSendFilesFromCmd task
 *  prints content of text file from sd card
 */

void vPlayTxtFilesFromCmd(void *) {
    uint8_t * bufferConsumer;
    const uint16_t bufferSize = 512;
    for (;;) {
        xQueueReceive(txtCmdTaskHandle, &bufferConsumer, portMAX_DELAY);
        for (uint16_t i = 0; i < bufferSize; i++) {
            u0_dbg_printf("%x", bufferConsumer[i]);
        }
    }
}

/** vSendMp3Files Task, @Priority = Low
 *  @resumes from button semaphore
 *  send current mp3 file and
 *  fill vReadMp3File queue,
 *  automatically goes to next song in vector
 */

void vSendMp3Files(void * pvParameter) {
    auto * sd = (SdDriver *)pvParameter;
    FRESULT res;
    FIL fil;
    UINT br;

    uint8_t buffer[512];
    uint8_t * const bufferProducer = buffer;

    for (;;) {
        xSemaphoreTake(startMp3Handle, portMAX_DELAY); /* wait on button semaphore signal */

        res = f_open(&fil, sd->getCurrentFileName(), FA_OPEN_EXISTING | FA_READ);
        if (res == FR_OK) {
            for (;;) {                                              /* playing current song */
                res = f_read(&fil, buffer, sizeof(buffer), &br);    /* read file from mp3File vector */
                if (res || br == 0) break;                          /* error or eof */
                xQueueSendToBack(mp3QueueHandle, &bufferProducer, portMAX_DELAY); /* fill producer queue */
            }
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
    auto * audio = (AudioDriver *)pvParameter;
    uint8_t * bufferConsumer;
    const uint16_t bufferSize = 512;
    for (;;) {
        xQueueReceive(mp3QueueHandle, &bufferConsumer, portMAX_DELAY);
        audio->SDI_W(bufferConsumer, bufferSize);
    }
}

int main(void) {
    /// This "stack" memory is enough for each task to run properly (512 * 32-bit (4 bytes)) = 2Kbytes stack
    const uint32_t STACK_SIZE_WORDS = 0x00000200;
    auto * sd = new SdDriver();
    auto * audio = new AudioDriver();
    assert(sd->Init() && audio->Init());

    sdFileCmdTaskHandle = xQueueCreate(1, sizeof(uint8_t *));
    mp3CmdTaskHandle = xQueueCreate(2, sizeof(uint8_t *));
    txtCmdTaskHandle = xQueueCreate(2, sizeof(uint8_t *));

    startMp3Handle = xSemaphoreCreateBinary();

    mp3QueueHandle = xQueueCreate(2, sizeof(uint8_t *));


    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    /// command line handled tasks
    xTaskCreate(vPlayMp3FilesFromCmd, "cmp3task", STACK_SIZE_WORDS, audio,   PRIORITY_HIGH, nullptr);
    xTaskCreate(vPlayTxtFilesFromCmd, "ctxttask", STACK_SIZE_WORDS, nullptr, PRIORITY_HIGH, nullptr);
    xTaskCreate(vSendFilesFromCmd,    "pmp3task", STACK_SIZE_WORDS, sd,      PRIORITY_LOW,  nullptr);

    /// isr handled tasks
    xTaskCreate(vPlayMp3Files,        "cmp3",     STACK_SIZE_WORDS, audio,   PRIORITY_HIGH, nullptr);
    xTaskCreate(vSendMp3Files,        "pmp3",     STACK_SIZE_WORDS, sd,      PRIORITY_LOW,  nullptr);

    scheduler_start();

    for (;;); /* endless loop - retain code execution */
}
