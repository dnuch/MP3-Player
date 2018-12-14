//
// Created by drnuc on 12/13/2018.
//

#ifndef SJSU_DEV_MP3TASKS_H
#define SJSU_DEV_MP3TASKS_H

#include "SdDriver.h"
#include "AudioDriver.h"
#include "OLEDDriver.h"

#include "InterruptHandler.h"

enum {
    IDLE = 0x00,
    PAUSE,
    PLAY
};

volatile uint8_t mp3State = PAUSE;
volatile uint8_t listIndex = 0;
volatile uint16_t listMultiplier = 0;
#define SET_MP3_STATE(state) (mp3State = state)

auto * const sd = new SdDriver();
auto * const audio = new AudioDriver();
auto * const oled = new OLEDDriver();

QueueHandle_t mp3QueueHandle;
QueueHandle_t mp3CmdTaskHandle;
QueueHandle_t txtCmdTaskHandle;

/**
 * update song list based on list index and list multiplier
 * @param arrowPosition
 */
static inline void updateSongList(position_t arrowPosition) {
    oled->setSongList(sd->isNextFileFromIndex(3 * listMultiplier) ? sd->getNextFileNameFromIndex(3 * listMultiplier) : " ",
                      sd->isNextFileFromIndex(3 * listMultiplier + 1) ? sd->getNextFileNameFromIndex(3 * listMultiplier + 1) : " ",
                      sd->isNextFileFromIndex(3 * listMultiplier + 2) ? sd->getNextFileNameFromIndex(3 * listMultiplier + 2) : " ",
                      arrowPosition);
}

/** vSendFilesFromCmd Task, @Priority = Low
 *  @resumes from terminal task 'play' command
 *  retrieves mp3/txt name param and
 *  fill vPlayMp3FilesFromCmd/vPlayTxtFilesFromCmd queue
 */

extern void vSendFilesFromCmd(void *) {
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

extern void vPlayMp3FilesFromCmd(void *) {
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

extern void vPlayTxtFilesFromCmd(void *) {
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
// TODO needs work with vFastForwardOrSelect
extern void vSendMp3Files(void *) {
    FRESULT res;
    FIL fil;
    UINT br;

    uint8_t buffer[512];
    uint8_t * const bufferProducer = buffer;

    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* wait on button semaphore signal from isr */
    SET_MP3_STATE(PLAY);
    for (;;) {
        res = f_open(&fil, sd->getCurrentFileName(), FA_OPEN_EXISTING | FA_READ);
        if (res == FR_OK) {
            for (;;) {                                              /* playing current song */
                if (uxSemaphoreGetCount(xSemaphore[1][SW_P2_2]) == 1) {    /* if play/pause isr pressed */
                    SET_MP3_STATE(PAUSE);
                    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* consume entry semaphore */
                    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* pause then resume after receiving next sem */
                    SET_MP3_STATE(PLAY);
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

extern void vPlayMp3Files(void *) {
    uint8_t * bufferConsumer;
    const uint16_t bufferSize = 512;
    for (;;) {
        xQueueReceive(mp3QueueHandle, &bufferConsumer, portMAX_DELAY);
        audio->SDI_W(bufferConsumer, bufferSize);
    }
}

/** vIncrVolumeOrList Task, @Priority = High
 *  @resumes from incrementVolumeFromISR task
 *  @state = PLAY, increases volume level by one
 *  @state = PAUSE, increase list index
 */

extern void vIncrVolumeOrList(void *) {
    for (;;) {
        xSemaphoreTake(xSemaphore[1][SW_P2_0], portMAX_DELAY);
        switch (mp3State) {
            case PLAY:
                if (audio->getVolumeLevel() != MAX_VOLUME_LEVEL) {
                    audio->incrementVolume();
                    oled->setVolumeBars(audio->getVolumeLevel());
                }
                break;
            case PAUSE:
                if (2 > listIndex) {
                    if (sd->isNextFileFromIndex(3 * listMultiplier + listIndex + 1)) {
                        listIndex++;
                        oled->printListArrow(listIndex);
                    }
                } else {
                    if (sd->isNextFileFromIndex(3 * listMultiplier + 1)) {
                        listIndex = TOP;
                        listMultiplier++;
                        updateSongList(TOP);
                    }
                }
                break;
            default: break;
        }
    }
}

/** vDecrementVolume Task, @Priority = High
 *  @resumes from decrementVolumeFromISR task
 *  decreases volume level by one
 */

extern void vDecrVolumeOrList(void *) {
    for (;;) {
        xSemaphoreTake(xSemaphore[1][SW_P2_1], portMAX_DELAY);
        switch (mp3State) {
            case PLAY:
                if (audio->getVolumeLevel() > 0) {
                    audio->decrementVolume();
                    oled->setVolumeBars(audio->getVolumeLevel());
                }
                break;
            case PAUSE:
                if (listIndex > 0) {
                    listIndex--;
                    oled->printListArrow(listIndex);
                } else {
                    listIndex = TOP;
                    if (listMultiplier) {
                        listMultiplier--;
                        oled->printListArrow(listIndex);
                    } else {
                        listMultiplier = sd->getTotalFileLength() / 3;
                    }
                    updateSongList(TOP);
                }
                break;
            default: break;
        }
    }
}

// TODO needs work
extern void vFastForwardOrSelect(void *) {
    for (;;) {
        xSemaphoreTake(xSemaphore[1][SW_P2_3], portMAX_DELAY);
        switch(mp3State) {
            case PLAY:
                break;
            case PAUSE:
                sd->setMp3Index(listIndex + (uint16_t)(3 * listMultiplier));
                oled->printListArrow(listIndex);
                break;
            default: break;
        }
    }
}

#endif //SJSU_DEV_MP3TASKS_H
