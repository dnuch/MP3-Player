//
// Created by drnuc on 11/30/2018.
//

#ifndef SJSU_DEV_INTERRUPTHANDLER_H
#define SJSU_DEV_INTERRUPTHANDLER_H

#include "GPIOInterrupts.h"

extern void incrementVolumeFromISR(void) {
    long yield = 0;
    xSemaphoreGiveFromISR(xSemaphore[1][SW_P2_0], &yield);
    portYIELD_FROM_ISR(yield);
}

extern void decrementVolumeFromISR(void) {
    long yield = 0;
    xSemaphoreGiveFromISR(xSemaphore[1][SW_P2_1], &yield);
    portYIELD_FROM_ISR(yield);
}

extern void playOrPauseFromISR(void) {
    long yield = 0;
    xSemaphoreGiveFromISR(xSemaphore[1][SW_P2_2], &yield);
    portYIELD_FROM_ISR(yield);
}

extern bool Init_MP3_GPIO_Interrupts() {
    gpioInterrupts.Initialize();
    gpioInterrupts.AttachInterruptHandler(portTwo, SW_P2_0, incrementVolumeFromISR, kFallingEdge);
    gpioInterrupts.AttachInterruptHandler(portTwo, SW_P2_1, decrementVolumeFromISR, kFallingEdge);
    gpioInterrupts.AttachInterruptHandler(portTwo, SW_P2_2, playOrPauseFromISR,     kFallingEdge);
    return true;
}

#endif //SJSU_DEV_INTERRUPTHANDLER_H
