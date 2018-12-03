//
// Created by drnuc on 11/14/2018.
//

#include "AudioDriver.h"

AudioDriver::AudioDriver() : x_CS(portZero, 29), x_DCS(portZero, 30), 
                             x_RST(portZero, 0), DREQ(portZero, 1) {
}

bool AudioDriver::Init() {
    /// Init control signals
    x_CS.setAsOutput();
    x_DCS.setAsOutput();
    x_RST.setAsOutput();
    DREQ.setAsInput();

    // disable control signals
    x_CS.setHigh();
    x_DCS.setHigh();
    x_RST.setHigh();

    audioControlDataLock = xSemaphoreCreateMutex();

    resetAudioDecoder();
    while (!getDREQ());

    /// init SSP0 : SCLK @ (48 MHz / 48) 1 MHz @ 8 bit data frame
    assert(spi.initialize(LPC_SSP0, 8, SpiDriver::SPI, 48));

    /** set clock frequency
     *  12.288 * 2 = 24.576 MHz
     */
    SCI_RW(WRITE, CLOCKF, 0x2000);

    // set spi clock back to (48 MHz / 4) 12 MHz
    spi.setClockDivider(4);

    // set bass   @ 15 db; below 60 Hz @ 0xF6
    // set treble @ 15 db; at and above 10 kHz @ 0xFA
    SCI_RW(WRITE, BASS, (0xFA << 8) | 0xF6);

    // init volume
    volumeLevel = 4;
    SCI_RW(WRITE, VOL, VOL_FOUR);

    // stereo 44.1 KHz
//    SCI_RW(WRITE, AUDATA, 0xAC45);

    // 5 seconds sine test
//    sineTest(1, 0);

    return true;
}

void AudioDriver::sineTest(uint8_t sampleRate, uint8_t sineSkipSpeed) {
    // set SDI tests
    SCI_RW(WRITE, MODE, 0x4820);

    uint8_t sineStartTest[8] = { 0x53, 0xEF, 0x6E, (uint8_t)((sampleRate << 5) | sineSkipSpeed), 0, 0, 0, 0 };
    uint8_t sineEndTest[8]   = { 0x45, 0x78, 0x69, 0x74, 0, 0, 0, 0 };

    SDI_W(sineStartTest, sizeof(sineStartTest));
    delay_ms(5000);
    SDI_W(sineEndTest, sizeof(sineEndTest));

    // unset SDI test bit
    SCI_RW(WRITE, MODE, 0x4800);
}
