//
// Created by drnuc on 12/4/2018.
//

#include "OLEDDriver.h"

bool OLEDDriver::Init() {
    i2c2 = &i2c2->getInstance();

    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_OFF);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_DISPLAY_CLOCKDIV);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, 0x80);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_MULTIPLEX);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, 0x3F);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_DISPLAY_OFFSET);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, 0x00);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_START_LINE);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SEG_REMAP | 0x01);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_COM_PINS);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, 0x12);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_CONTRAST);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, 0xCF);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_PRECHARGE);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, 0xF1);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_VCOM_DETECT);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, 0x40);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_ALLON_RESUME);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, NORMAL_DISPLAY);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_ON);

    testDisplay();

    return true;
}

void OLEDDriver::toggleDisplay() {
    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_OFF);
    delay_ms(1000);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_ON);
}

void OLEDDriver::fillDisplay(uint8_t byte) {
    uint8_t row, column, buffer[8];
    memset(buffer, byte, sizeof(buffer));
    for (row = 0; row < OLED_HEIGHT / 8; row++) { // row = page
        i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | row));
        for (column = 0; column < OLED_WIDTH; column += 8) {
            /* Clear the display - 8x8 pixels at the time */
            for (unsigned char b : buffer) {
                i2c2->writeReg(OLED_ADDRESS, DATA, b);
            }
        }
    }
}

void OLEDDriver::testDisplay() {
    fillDisplay(0xFF);
    delay_ms(1000);
    fillDisplay(0x00);
    delay_ms(1000);
    fillDisplay(0x02);
    delay_ms(1000);
    fillDisplay(0xFF);
}
