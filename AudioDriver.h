//
// vs1053b driver for MPEG Layer 3
// @ 12 MHz SCK
//

#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include "GPIODriver.h"
#include "SpiDriver.h"
#include "utilities.h"
//#include "printf_lib.h"
#include <cassert>
#define MAX_VOLUME_LEVEL 5

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
    VOL,            // 0x0000 (max); 0xFEFE (silence)
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
     * @return 16 byte message
     */
    uint16_t SCI_RW(AUDIO_OPCODE opcode, SCI_REGISTER address, uint16_t data = 0xA5A5);

    /**
     * Serial Data In write
     * @param buffer
     * @param length
     */
    void SDI_W(const uint8_t * buffer, int length);

    /**
     * When DREQ is low, that means audio co-processing unit 2048 FIFO is full
     * @return
     */
    bool getDREQ() { return DREQ.getLevel(); };

    /**
     * Sinusoidal frequency test
     * @param sampleRate
     * @param sineSkipSpeed
     */
    void sineTest(uint8_t sampleRate, uint8_t sineSkipSpeed);

    void incrementVolume();
    void decrementVolume();

    uint8_t getVolumeLevel() { return volumeLevel; }

    void stopPlayback();
private:
    /**
     * @Control_Signals (X indicative of active low)
     *  @output
     *  XCS  - Send Command (Active Low) P0.29
     *  XDCS - Send Data    (Active Low) P0.30
     *  XRST - Reset        (Active Low) P0.0
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

    SemaphoreHandle_t audioControlDataLock;

    enum VOLUME_LEVEL {
        VOL_FIVE  = 0x2020,   // max
        VOL_FOUR  = 0x3030,   // init
        VOL_THREE = 0x4040,
        VOL_TWO   = 0x5050,
        VOL_ONE   = 0x6060,
        VOL_ZERO  = 0xFEFE    // silence
    };

    uint8_t volumeLevel;

};

inline void AudioDriver::resetAudioDecoder() {
    x_RST.setLow();
    delay_ms(10);
    x_RST.setHigh();
}

inline uint16_t AudioDriver::SCI_RW(AUDIO_OPCODE opcode, SCI_REGISTER address, uint16_t data) {
    uint8_t d[4];
    // wait until DREQ is high
    xSemaphoreTake(audioControlDataLock, portMAX_DELAY);
    while (!getDREQ());
    x_CS.setLow();
    {
        d[0] = spi.transfer_spi0(opcode);
        d[1] = spi.transfer_spi0(address);
        d[2] = spi.transfer_spi0((uint8_t)(data >> 8));
        d[3] = spi.transfer_spi0((uint8_t)(data & 0x00FF));
    }
    x_CS.setHigh();
    // wait until DREQ is high
    while (!getDREQ());
    xSemaphoreGive(audioControlDataLock);

//    u0_dbg_printf("SPI0 Received: 0x%x\n", (d[2] << 8) | d[3]);
    return (d[2] << 8) | d[3];
}

inline void AudioDriver::SDI_W(const uint8_t * buffer, int length) {
    // wait until DREQ is high
    xSemaphoreTake(audioControlDataLock, portMAX_DELAY);
    while (!getDREQ());
    x_DCS.setLow();
    {
        for (int i = 0; i < length; i++) {
            while (!getDREQ());
            spi.transfer_spi0(buffer[i]);
        }
    }
    x_DCS.setHigh();
    // wait until DREQ is high
    while (!getDREQ());
    xSemaphoreGive(audioControlDataLock);
}

inline void AudioDriver::incrementVolume() {
    uint16_t vol = 0x0000;

    switch (volumeLevel) {
        case 0:  vol = VOL_ONE;   break;
        case 1:  vol = VOL_TWO;   break;
        case 2:  vol = VOL_THREE; break;
        case 3:  vol = VOL_FOUR;  break;
        case 4:  vol = VOL_FIVE;  break;
        default: vol = VOL_FOUR;
    }
    volumeLevel++;
    SCI_RW(WRITE, VOL, vol);
}

inline void AudioDriver::decrementVolume() {
    uint16_t vol = 0x0000;

    if (volumeLevel != 0) {
        switch (volumeLevel) {
            case 5:  vol = VOL_FOUR;  break;
            case 4:  vol = VOL_THREE; break;
            case 3:  vol = VOL_TWO;   break;
            case 2:  vol = VOL_ONE;   break;
            case 1:  vol = VOL_ZERO;  break;
            default: vol = VOL_FOUR;
        }
        volumeLevel--;
        SCI_RW(WRITE, VOL, vol);
    }
}

inline void AudioDriver::stopPlayback() {
    SCI_RW(WRITE, MODE, 0x4800 | (1 << 3));  /* set SM_CANCEL bit */
    while ((SCI_RW(READ, MODE) & (1 << 3))); /* wait until SM_CANCEL bit cleared */
}

#endif //AUDIODRIVER_H
