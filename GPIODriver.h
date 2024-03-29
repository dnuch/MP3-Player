/*
 * GPIODriver.hpp
 *
 *  Created on: Aug 31, 2018
 *      Author: drnuc
 */

#ifndef GPIODRIVER_H
#define GPIODRIVER_H

#include "LPC17xx.h"
#include "stdint.h"

#include "tasks.hpp"

// gpio initializer
typedef struct {
    uint8_t port;
    uint8_t pin;
} gpioInit_t;

// define port one LED and SWITCH and custom LED/SWITCH pins
enum {
    portZero = 0,
    portOne,
    portTwo,

    LED0 = 0,
    LED1,
    LED2 = 4,
    LED3 = 8,
    SW0,
    SW1,
    SW2 = 14,
    SW3,

    // custom define LED/SWITCH on external GPIO ports 0 & 2
    SW_P2_0 = 0,
    SW_P2_1 = 1,
    SW_P2_2 = 2,
    SW_P2_3 = 3,
    SW_P2_4 = 4
};

class GPIODriver
{
private:
    /**
     * port, pin and any other variables should be placed here.
     * NOTE: that the state of the pin should not be recorded here. The true
     *      source of that information is embedded in the hardware registers
     */
    volatile LPC_GPIO_TypeDef * LPC_GPIO;
    const uint8_t pinByte;

public:
    /**
     * You should not modify any hardware registers at this point
     * You should store the port and pin using the constructor.
     *
     * @param {uint8_t} pin  - pin number between 0 and 32
     */
    GPIODriver(uint8_t port, uint8_t pin);
    ~GPIODriver() = default;

    /**
     * Should alter the hardware registers to set the pin as an input
     */
    void setAsInput() { LPC_GPIO->FIODIR &= ~(1 << pinByte); };
    /**
     * Should alter the hardware registers to set the pin as an input
     */
    void setAsOutput() { LPC_GPIO->FIODIR |= (1 << pinByte); };
    /**
     * Should alter the set the direction output or input depending on the input.
     *
     * @param {bool} output - true => output, false => set pin to input
     */
    void setDirection(bool output) { output ? setAsOutput() : setAsInput(); };
    /**
     * Should alter the hardware registers to set the pin as high
     */
    void setHigh() { LPC_GPIO->FIOSET = (uint32_t)(1 << pinByte); };
    /**
     * Should alter the hardware registers to set the pin as low
     */
    void setLow() { LPC_GPIO->FIOCLR = (uint32_t)(1 << pinByte); };
    /**
     * Should alter the hardware registers to set the pin as low
     *
     * @param {bool} high - true => pin high, false => pin low
     */
    void set(bool high) { high ? setHigh() : setLow(); };
    /**
     * Should return the state of the pin (input or output, doesn't matter)
     *
     * @return {bool} level of pin high => true, low => false
     */
    bool getLevel() { return (LPC_GPIO->FIOPIN & (1 << pinByte)) != 0; };
};

#endif //GPIODRIVER_H
