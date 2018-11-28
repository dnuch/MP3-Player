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

SemaphoreHandle_t xSemaphore[2][32];

/*
void vControlLED_2(void * pvParameters)
{
    auto * gpio = (gpioInit_t *)pvParameters;
    GPIODriver led(gpio->port, gpio->pin);

    // set as output & active high
    led.setDirection(true);
    led.set(true);
    for (;;)
    {
//        uart0_puts("task");
        // block task forever until given semaphore from respective SW interrupt
        xSemaphoreTake(xSemaphore[gpio->port == 2 ? 1 : 0][gpio->pin], portMAX_DELAY);
        // toggle LED for a second
//        led.set(false);
//        vTaskDelayMs(1000);
//        led.set(true);
        // toggle LED
        led.getLevel() ? led.set(false) : led.set(true);
//        vTaskDelayMs(100);
    }
}
*/
