//
// Created by drnuc on 9/22/2018.
//

#ifndef SPIDRIVER_H
#define SPIDRIVER_H

#include "GPIODriver.h"
#include "tasks.hpp"

class SpiDriver
{
public:
    enum FrameModes
    {
        SPI = 0,
        TI,
        MICROWIRE
    };

    /**
     * 1) Powers on SPPn peripheral
     * 2) Set peripheral clock
     * 3) Sets pins for specified peripheral to MOSI, MISO, and SCK
     *
     * @param data_size_select transfer size data width; To optimize the code, look for a pattern in the datasheet
     * @param format is the code format for which synchronous serial protocol you want to use.
     * @param divide is the how much to divide the clock for SSP; take care of error cases such as the value of 0, 1, and odd numbers
     *
     * @return true if initialization was successful
     */
    bool initialize(LPC_SSP_TypeDef * sspT, uint8_t data_size_select, FrameModes format, uint8_t divide);

    /**
     * Transfers a byte via SSP to an external device using the SSP data register.
     * This region must be protected by a mutex static to this class.
     *
     * @return received byte from external device via SSP data register.
     */
    uint8_t transfer_spi0(uint8_t send);
    uint8_t transfer_spi1(uint8_t send);

    void setClockDivider(uint8_t divide);

    SpiDriver() = default;
    ~SpiDriver() = default;

    /// read flash signature
//    friend void flashSignatureReading(SpiDriver * s);
    /// read flash status register
//    friend void flashStatusReading(SpiDriver * s);
private:
    LPC_SSP_TypeDef * ssp;

    static SemaphoreHandle_t spi0_bus_lock;
    static SemaphoreHandle_t spi1_bus_lock;
};

/// @mutual_exclusion
inline uint8_t SpiDriver::transfer_spi0(uint8_t send) {
    if (xSemaphoreTake(spi0_bus_lock, 1000)) {
        // Use Guarded Resource
        ssp->DR = (uint32_t)send;
        while (ssp->SR & (1 << 4));
        // Give Semaphore back
        xSemaphoreGive(spi0_bus_lock);
        return (uint8_t)ssp->DR;
    }
    return 0;
}

inline uint8_t SpiDriver::transfer_spi1(uint8_t send) {
    if (xSemaphoreTake(spi1_bus_lock, 1000)) {
        // Use Guarded Resource
        ssp->DR = (uint32_t)send;
        while (ssp->SR & (1 << 4));
        // Give Semaphore back
        xSemaphoreGive(spi0_bus_lock);
        return (uint8_t)ssp->DR;
    }
    return 0;
}

#endif //SPIDRIVER_H
