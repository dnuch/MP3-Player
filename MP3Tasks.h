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
volatile bool setSongPreviousOrIndex = false;
volatile bool setNextSong = false;
volatile bool disableNextAndPrevious = false;

#define MAX_LIST_ENTRY 3
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
extern inline void updateSongList(uint8_t arrowPosition) {
    oled->setSongList(sd->isNextFileFromIndex(MAX_LIST_ENTRY * listMultiplier) ? sd->getNextFileNameFromIndex(MAX_LIST_ENTRY * listMultiplier) : " ",
                      sd->isNextFileFromIndex(MAX_LIST_ENTRY * listMultiplier + 1) ? sd->getNextFileNameFromIndex(MAX_LIST_ENTRY * listMultiplier + 1) : " ",
                      sd->isNextFileFromIndex(MAX_LIST_ENTRY * listMultiplier + 2) ? sd->getNextFileNameFromIndex(MAX_LIST_ENTRY * listMultiplier + 2) : " ",
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
 *  @resumes from playOrPauseFromISR isr
 *  send current mp3 file pointed and
 *  fill vReadMp3File queue,
 *  automatically goes to next song in vector
 */

extern void vSendMp3Files(void *) {
    FRESULT res;
    FIL fil;
    UINT br;

    uint8_t buffer[512];
    uint8_t * const bufferProducer = buffer;

    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* wait on button semaphore signal from isr */
    SET_MP3_STATE(PLAY);
    setSongPreviousOrIndex = false;
    setNextSong = false;
    for (;;) {
        res = f_open(&fil, sd->getCurrentFileName(), FA_OPEN_EXISTING | FA_READ);
        if (res == FR_OK) {
            disableNextAndPrevious = false;
            oled->printCurrentSong(sd->getCurrentFileName());
            oled->printPlay();
            for (;;) {                                              /* playing current song */
                if (uxSemaphoreGetCount(xSemaphore[1][SW_P2_2]) == 1) {    /* if play/pause isr pressed */
                    SET_MP3_STATE(PAUSE);
                    oled->printPause();
                    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* consume entry semaphore */
                    xSemaphoreTake(xSemaphore[1][SW_P2_2], portMAX_DELAY); /* pause then resume after receiving next sem */
                    SET_MP3_STATE(PLAY);
                    oled->printPlay();
                }

                if (setSongPreviousOrIndex || setNextSong) {
                    audio->stopPlayback();
                    break;
                }

                res = f_read(&fil, buffer, sizeof(buffer), &br);    /* read file from sdFile vector */
                if (res || br == 0) break;                          /* error or eof */
                xQueueSend(mp3QueueHandle, &bufferProducer, portMAX_DELAY); /* fill producer queue */
            }
            f_close(&fil);
            if (setSongPreviousOrIndex) {
                setSongPreviousOrIndex = false;
            } else {
                setNextSong = false;
                sd->setNextSong();

                if (2 > listIndex) {
                    if (sd->isNextFileFromIndex(MAX_LIST_ENTRY * listMultiplier + listIndex + 1)) {
                        listIndex++;
                        oled->printListArrow(listIndex);
                    }
                    else {
                        listIndex = TOP;
                        listMultiplier = 0;
                        updateSongList(TOP);
                    }
                }
                else {
                    listMultiplier++;
                    listIndex = TOP;
                    if (!sd->isNextFileFromIndex(MAX_LIST_ENTRY * listMultiplier)) {
                        listMultiplier = 0;
                    }
                    updateSongList(TOP);
                }
            }
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
 *  @resumes from incrVolumeOrListFromISR task
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
                disableNextAndPrevious = true;
                if (2 > listIndex) {
                    if (sd->isNextFileFromIndex(MAX_LIST_ENTRY * listMultiplier + listIndex + 1)) {
                        listIndex++;
                        oled->printListArrow(listIndex);
                    } else {
                        listIndex = TOP;
                        listMultiplier = 0;
                        updateSongList(TOP);
                    }
                } else {
                    listMultiplier++;
                    listIndex = TOP;
                    if (!sd->isNextFileFromIndex(MAX_LIST_ENTRY * listMultiplier)) {
                        listMultiplier = 0;
                    }
                    updateSongList(TOP);
                }
                break;
            default: break;
        }
    }
}

/** vDecrementVolume Task, @Priority = High
 *  @resumes from decrVolumeOrListFromISR task
 *  @state = PLAY; decreases volume level by one
 *  @state = PAUSE; decrement music list
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
                disableNextAndPrevious = true;
                if (listIndex > 0) {
                    listIndex--;
                    oled->printListArrow(listIndex);
                } else {
                    listMultiplier > 0 ? listMultiplier-- :
                        sd->getTotalFileLength() % MAX_LIST_ENTRY == 0 ? listMultiplier = sd->getTotalFileLength() / MAX_LIST_ENTRY - 1 :
                                                                         listMultiplier = sd->getTotalFileLength() / MAX_LIST_ENTRY;

                    listIndex = TOP;
                    updateSongList(TOP);
                }
                break;
            default: break;
        }
    }
}

/** vFastForwardOrSelect Task, @Priority = High
 *  @resumes from fastForwardOrSelectFromISR task
 *  @state = PLAY; set play rate to 1x/4x normal speed
 *  @state = PAUSE; select and play song
 */

extern void vFastForwardOrSelect(void *) {
    bool isFastForward = false;
    for (;;) {
        xSemaphoreTake(xSemaphore[1][SW_P2_3], portMAX_DELAY);
        switch(mp3State) {
            case PLAY:
                isFastForward ? audio->setPlaySpeed(NORMAL) : audio->setPlaySpeed(FAST);
                isFastForward = !isFastForward;
                break;
            case PAUSE:
                sd->setMp3Index(listIndex + (uint16_t)(MAX_LIST_ENTRY * listMultiplier));
                setSongPreviousOrIndex = true;
                xSemaphoreGive(xSemaphore[1][SW_P2_2]);
                break;
            default: break;
        }
    }
}

/** vNextOrPrevious Task, @Priority = High
 *  @resumes from nextOrPreviousFromISR task
 *  @state = PLAY; play next song on list
 *  @state = PAUSE; play previous song on list
 */

extern void vNextOrPrevious(void *) {
    for (;;) {
        xSemaphoreTake(xSemaphore[1][SW_P2_4], portMAX_DELAY);
        if (!disableNextAndPrevious) {
            switch (mp3State) {
                case PLAY:
                    setNextSong = true;
                    break;
                case PAUSE:
                    sd->setPreviousSong();
                    setSongPreviousOrIndex = true;
                    if (listIndex > 0) {
                        listIndex--;
                        oled->printListArrow(listIndex);
                    } else {
                        if (listMultiplier) {
                            listMultiplier--;
                            listIndex = BOT;
                        } else {
                            sd->getTotalFileLength() % MAX_LIST_ENTRY == 0 ? listMultiplier = (sd->getTotalFileLength() / MAX_LIST_ENTRY) - 1 :
                                                                             listMultiplier = (sd->getTotalFileLength() / MAX_LIST_ENTRY);
                            listIndex = TOP;
                            sd->setMp3Index(listMultiplier * MAX_LIST_ENTRY);
                        }
                        updateSongList(listIndex);
                    }
                    xSemaphoreGive(xSemaphore[1][SW_P2_2]);
                    break;
                default: break;
            }
        }
    }
}

#endif //SJSU_DEV_MP3TASKS_H
