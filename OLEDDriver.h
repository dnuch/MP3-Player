//
// SSH1106 driver 132x64
//

#ifndef SJSU_DEV_OLEDDRIVER_H
#define SJSU_DEV_OLEDDRIVER_H

#include "i2c2.hpp"
#include "utilities.h"
#include "string.h"

/**
 * Contains specs. of OLED, addresses, and registers
 */
enum {
    OLED_WIDTH   = 132,
    OLED_HEIGHT  = 64,
    PAGE_COUNT   = 8,
    OLED_ADDRESS = 0x78,

    DATA    = 0x40,
    CONTROL = 0x00,

    SET_LOW_COLUMN       = 0x00,    /* 0x00 - 0x0F */
    SET_HIGH_COLUMN      = 0x10,    /* 0x10 - 0x1F */
    SET_PUMP_VOL         = 0x30,    /* 0x30 - 0x33 */
    SET_START_LINE       = 0x40,    /* 0x40 - 0x7F */
    SET_CONTRAST         = 0x81,    /** multi-byte write; send 0x81 address then 0x00 - 0xFF into contrast data register */
    SEG_REMAP            = 0xA0,    /* 0xA0 - 0xA1 */
    DISPLAY_ALLON_RESUME = 0xA4,
    DISPLAY_ALLON        = 0xA5,
    NORMAL_DISPLAY       = 0xA6,
    REVERSE_DISPLAY      = 0xA7,
    SET_MULTIPLEX        = 0xA8,    /** multi-byte write; send 0xA8 then 0x00 - 0x3F */
    SET_DCDC             = 0xAD,    /** multi-byte write; send 0xAD then 0x8A - 0x8B */
    DISPLAY_OFF          = 0xAE,
    DISPLAY_ON           = 0xAF,
    SET_PAGE_ADDR        = 0xB0,    /* 0xB0 - 0xB7, 8 pages in total */
    COM_OUT_SCAN         = 0xC0,    /* 0xC0 - 0xC8 */
    SET_DISPLAY_OFFSET   = 0xD3,    /** multi-byte write; send 0xD3 then 0x00 - 0x3F; for setting COM0-COM63 (height?) */
    SET_DISPLAY_CLOCKDIV = 0xD5,    /** multi-byte write; send 0xD5 then 0x00 - 0xFF */
    SET_PRECHARGE        = 0xD9,    /** multi-byte write; send 0xD9 then 0x00 - 0xFF */
    SET_COM_PINS         = 0xDA,    /** multi-byte write; send 0xDA then 0x02 - 0x12 */
    SET_VCOM_DETECT      = 0xDB,    /** multi-byte write; send 0xDB then 0x00 - 0xFF */
    READ_MODIFY_WRITE    = 0xE0,
    END                  = 0xEE,
    NOP                  = 0xE3
};

class OLEDDriver {
private:
    I2C2 * i2c2;
public:
    OLEDDriver() = default;
    bool Init();

    void toggleDisplay();

    void fillDisplay(uint8_t byte);
    void testDisplay();
    void writeS();
    void clearDisplay();
    void resetCursor();
};

#endif //SJSU_DEV_OLEDDRIVER_H
