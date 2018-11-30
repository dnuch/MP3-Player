//
// Created by drnuc on 9/2/2018.
//

#include "GPIODriver.h"

GPIODriver::GPIODriver(uint8_t port, uint8_t pin): pinByte(pin) {
    switch (port)
    {
        case portZero:
            LPC_GPIO = LPC_GPIO0;
            pin < 16 ? LPC_PINCON->PINSEL0 &= ~(3 << (pin << 1)) : LPC_PINCON->PINSEL1 &= ~(3 << ((pin & ~(1 << 5)) << 1));
            break;
        case portOne:
            LPC_GPIO = LPC_GPIO1;
            pin < 16 ? LPC_PINCON->PINSEL2 &= ~(3 << (pin << 1)) : LPC_PINCON->PINSEL3 &= ~(3 << ((pin & ~(1 << 5)) << 1));
            break;
        case portTwo:
            LPC_GPIO = LPC_GPIO2;
            if (pin < 16) LPC_PINCON->PINSEL4 &= ~(3 << (pin << 1));
            break;
        default:
            break;
    }
}
