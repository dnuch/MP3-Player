//
// Created by drnuc on 12/13/2018.
//

#include "InterruptHandler.h"

void incrVolumeOrListFromISR(void) {
    long yield = 0;
    xSemaphoreGiveFromISR(xSemaphore[1][SW_P2_0], &yield);
    portYIELD_FROM_ISR(yield);
}

void decrVolumeOrListFromISR(void) {
    long yield = 0;
    xSemaphoreGiveFromISR(xSemaphore[1][SW_P2_1], &yield);
    portYIELD_FROM_ISR(yield);
}

void playOrPauseFromISR(void) {
    long yield = 0;
    xSemaphoreGiveFromISR(xSemaphore[1][SW_P2_2], &yield);
    portYIELD_FROM_ISR(yield);
}

void fastForwardOrSelectFromISR(void) {
    long yield = 0;
    xSemaphoreGiveFromISR(xSemaphore[1][SW_P2_3], &yield);
    portYIELD_FROM_ISR(yield);
}

bool Init_MP3_GPIO_Interrupts() {
    gpioInterrupts.Initialize();
    gpioInterrupts.AttachInterruptHandler(portTwo, SW_P2_0, incrVolumeOrListFromISR,   kFallingEdge);
    gpioInterrupts.AttachInterruptHandler(portTwo, SW_P2_1, decrVolumeOrListFromISR,   kFallingEdge);
    gpioInterrupts.AttachInterruptHandler(portTwo, SW_P2_2, playOrPauseFromISR,         kFallingEdge);
    gpioInterrupts.AttachInterruptHandler(portTwo, SW_P2_3, fastForwardOrSelectFromISR, kFallingEdge);
    return true;
}
