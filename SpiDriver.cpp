//
// Created by drnuc on 9/22/2018.
//

#include "SpiDriver.h"

SemaphoreHandle_t SpiDriver::spi0_bus_lock = xSemaphoreCreateMutex();
SemaphoreHandle_t SpiDriver::spi1_bus_lock = xSemaphoreCreateMutex();

bool SpiDriver::initialize(LPC_SSP_TypeDef * sspT, uint8_t data_size_select, SpiDriver::FrameModes format, uint8_t divide) {
    // base case
    if (((sspT != LPC_SSP1 ) && (sspT != LPC_SSP0)) || data_size_select < 4 || data_size_select > 16
            || format > 3 || (divide & 0x01) || divide == 0) {
        return false;
    }

    ssp = sspT;

    // power on SSP1/0
    LPC_SC->PCONP |= (1 << 10) | (1 << 21);

    // clear FRF bits
    ssp->CR0 &= ~(3 << 4);
    // set FRF bits
    ssp->CR0 |= (format << 4);

    if (ssp == LPC_SSP1) {
        // clear SSP1/0 scale clock bits
        LPC_SC->PCLKSEL0 &= ~(3 << 20);
        // divide by 2
        LPC_SC->PCLKSEL0 |= (2 << 20);
    }
    else if (ssp == LPC_SSP0) {
        // clear ssp0 scale clock bits
        LPC_SC->PCLKSEL1 &= ~(3 << 10);
        // divide by 2
        LPC_SC->PCLKSEL1 |= (2 << 10);
        // clear clock pre-scale bits
    }

    // set clock prescale bits
    setClockDivider(divide);

    // clear Data Size Select bits
    ssp->CR0 &= ~15;
    // Data Size Select; set bit frame transfer_spi0 over wire
    ssp->CR0 |= (data_size_select - 1);

    return true;
}

void SpiDriver::setClockDivider(uint8_t divide) {
    if ((divide & 0x01) || divide == 0) return;
    // clear clock pre-scale bits
    ssp->CPSR &= ~15;
    // set clock prescale bits
    ssp->CPSR = divide;
}
