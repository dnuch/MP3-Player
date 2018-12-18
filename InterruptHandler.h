//
// Created by drnuc on 11/30/2018.
//

#ifndef SJSU_DEV_INTERRUPTHANDLER_H
#define SJSU_DEV_INTERRUPTHANDLER_H

#include "GPIOInterrupts.h"

extern void incrVolumeOrListFromISR(void);
extern void decrVolumeOrListFromISR(void);
extern void playOrPauseFromISR(void);
extern void fastForwardOrSelectFromISR(void);
extern void nextOrPreviousFromISR(void);

extern bool Init_MP3_GPIO_Interrupts();

#endif //SJSU_DEV_INTERRUPTHANDLER_H
