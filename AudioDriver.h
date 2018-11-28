//
// vs1053b driver for MPEG Layer 3
// @ 12 MHz SCK
//

#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include "GPIODriver.h"
#include "SpiDriver.h"
#include "utilities.h"
#include "printf_lib.h"
#include <cassert>

enum AUDIO_OPCODE {
    WRITE = 0x2,
    READ
};

enum SCI_REGISTER {
    MODE = 0x0,     // 0x4800 on reset
    STATUS,         // 0x000C on reset
    BASS,
    CLOCKF,
    DECODE_TIME,
    AUDATA,
    WRAM,
    WRAMADDR,
    HDAT0,
    HDAT1,
    AIADDR,
    VOL,
    AICTRL0,
    AICTRL1,
    AICTRL2,
    AICTRL3
};

class AudioDriver {
public:
    AudioDriver();
    void resetAudioDecoder();
    bool Init();

    /**
     * Serial Control In read/write
     * @param opcode
     * @param address
     * @param data
     */
    void SCI_RW(AUDIO_OPCODE opcode, SCI_REGISTER address, uint16_t data = 0xA5A5);

    /**
     * Serial Data In write
     * @param buffer
     * @param length
     */
    void SDI_W(uint8_t * buffer, int length);

    bool getDREQ() { return DREQ.getLevel(); };

    /**
     * Sinusoidal frequency test
     * @param sampleRate
     * @param sineSkipSpeed
     */
    void sineTest(uint8_t sampleRate, uint8_t sineSkipSpeed);
private:
    /**
     * @Control_Signals (X indicative of active low)
     *  @output
     *  XCS  - Send Command (Active Low) P0.29
     *  XDCS - Send Data (Active Low)    P0.30
     *  XRST - Reset (Active Low)        P0.0
     *  @input
     *  DREQ - Data Request              P0.1
     */
    GPIODriver x_CS;
    GPIODriver x_DCS;
    GPIODriver x_RST;
    GPIODriver DREQ;

    /**
     * @SPI0_interface
     *  MOSI0 - Master Out Slave In
     *  MISO0 - Master In Slave Out
     *  SCK0  - SPI Clock Signal
     */
    SpiDriver spi;
};

inline void AudioDriver::resetAudioDecoder() {
    x_RST.setLow();
    delay_ms(10);
    x_RST.setHigh();
}

inline void AudioDriver::SCI_RW(AUDIO_OPCODE opcode, SCI_REGISTER address, uint16_t data) {
    uint8_t d[4];
    // wait until DREQ is high
    while (!DREQ.getLevel());
    x_CS.setLow();
    {
        d[0] = spi.transfer_spi0(opcode);
        d[1] = spi.transfer_spi0(address);
        d[2] = spi.transfer_spi0((uint8_t)(data >> 8));
        d[3] = spi.transfer_spi0((uint8_t)(data & 0x00FF));
    }
    x_CS.setHigh();
    // wait until DREQ is high
    while (!DREQ.getLevel());
    u0_dbg_printf("Received: 0x%x\n", (d[2] << 8) | d[3]);
}

inline void AudioDriver::SDI_W(uint8_t * buffer, int length) {
    // wait until DREQ is high
    while (!DREQ.getLevel());
    x_DCS.setLow();
    {
        for (int i = 0; i < length; i++) {
            while (!DREQ.getLevel());
            spi.transfer_spi0(buffer[i]);
        }
    }
    x_DCS.setHigh();
    // wait until DREQ is high
    while (!DREQ.getLevel());
}


#endif //AUDIODRIVER_H
