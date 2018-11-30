//
// Created by drnuc on 9/9/2018.
//

#include "GPIOInterrupts.h"

GPIOInterrupts & gpioInterrupts = GPIOInterrupts::getInstance();

SemaphoreHandle_t xSemaphore[2][32];

void GPIOInterrupts::Initialize() {
    // enable interrupts on port 0/2
    NVIC_EnableIRQ(EINT3_IRQn);
    // Register C function which delegates interrupt handling to your C++ class function
    isr_register(EINT3_IRQn, eint3Handler);
}

bool GPIOInterrupts::AttachInterruptHandler(uint8_t port,
                                            uint8_t pin,
                                            IsrPointer pin_isr,
                                            InterruptCondition condition) {
    // map switch to LED
    switch (port) {
        case portZero:
            switch (condition) {
                case kBothEdges:
                    LPC_GPIOINT->IO0IntEnR |= (1 << pin);
                case kFallingEdge:
                    LPC_GPIOINT->IO0IntEnF |= (1 << pin);
                    break;
                case kRisingEdge:
                    LPC_GPIOINT->IO0IntEnR |= (1 << pin);
                    break;
                default:
                    break;
            }
            break;
        case portTwo:
            switch (condition) {
                case kBothEdges:
                    LPC_GPIOINT->IO2IntEnR |= (1 << pin);
                case kFallingEdge:
                    LPC_GPIOINT->IO2IntEnF |= (1 << pin);
                    break;
                case kRisingEdge:
                    LPC_GPIOINT->IO2IntEnR |= (1 << pin);
                    break;
                default:
                    break;
            }
            break;
        default:
            // wrong port init
            return false;
    }

    xSemaphore[port == 2 ? 1 : 0][pin] = xSemaphoreCreateBinary();
    // init switch GPIO
    GPIODriver sw(port, pin);
    // set as input
    sw.setDirection(false);
    pin_isr_map[port == 2 ? 1 : 0][pin] = pin_isr;
    return true;
}


void GPIOInterrupts::HandleInterrupt() {
    // iterate through all pins and check interrupt status
    for (uint8_t port = 0; port < 2; port++) {
        for (uint8_t pin = 0; pin < 32; pin++) {
            if ((port == 0 && (LPC_GPIOINT->IO0IntStatF & (1 << pin) || LPC_GPIOINT->IO0IntStatR & (1 << pin))) ||
                (port == 1 && (LPC_GPIOINT->IO2IntStatF & (1 << pin) || LPC_GPIOINT->IO2IntStatR & (1 << pin)))) {
                port == 1 ? LPC_GPIOINT->IO2IntClr = (uint32_t)(1 << pin) : LPC_GPIOINT->IO0IntClr = (uint32_t)(1 << pin);
                pin_isr_map[port][pin]();
                port = 2;
                pin = 32;
            };
        }
    }
}

void eint3Handler(void) {
    gpioInterrupts.HandleInterrupt();
}
