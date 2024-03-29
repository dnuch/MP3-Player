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

typedef enum {

    VOL_ZERO             = 0x000000000000,
    VOL_ONE              = 0x000000000100,
    VOL_TWO              = 0x000003000100,
    VOL_THREE            = 0x070003000100,
    VOL_FOUR             = 0x000000000F00,
    VOL_FIVE             = 0x00003F000F00,

} volume_t;

typedef enum {

    TOP = 0x00,
    MID,
    BOT

} position_t;

class OLEDDriver {
private:
    I2C2 * i2c2;

public:
    OLEDDriver();
    bool Init();

    typedef enum {

        PAUSE                = 0x007F7F007F7F,
        PLAY                 = 0x00081C3E7F7F,
        ARROW                = 0x00081c3e0808,
        SPACE                = 0x000000000000,
        COLON                = 0x000066660000,
        PERIOD               = 0x000003030000,

    } symbol_t;

    void toggleDisplay();

    void fillDisplay(uint8_t byte);
    void testDisplay();

    void clearDisplay();
    void resetCursor();
    void moveCursor(uint8_t row, uint8_t column);

    uint64_t charToDisplay(char c); //retrieves look up table value of a char
    void printChar(char c);
    void printLine(const char *s,uint8_t row, uint8_t column);
    void printSymbolAtPosition(symbol_t symbol, uint8_t row, uint8_t column);
    void printSymbol(symbol_t sym);
    void printCurrentSong(const char *s);
    void printPause();
    void printPlay();
    void initVolume(uint8_t vol);
    void printListSong(const char *s, position_t pos);
    void printListArrow(uint8_t pos);
    void eraseSymbolAtPosition(uint8_t row, uint8_t column);
    void setVolumeBars(uint8_t vol);
    void printVolumeBars(volume_t first, volume_t second);

    void setSongList(const char *topSong, const char *midSong, const char *botSong, uint8_t arrowPosition);

    //ALPHABET LOOKUP TABLE
    const uint64_t charHexValues[36] =
    {
            [0]                     = 0x003F4848483F, //A
            [1]                     = 0x00364949497F, //B
            [2]                     = 0x00224141413E, //C
            [3]                     = 0x003E4141417F, //D
            [4]                     = 0x00634149497F, //E
            [5]                     = 0x00604048487F, //F
            [6]                     = 0x00264545413E, //G
            [7]                     = 0x007F0808087F, //H
            [8]                     = 0x0041417F4141, //I
            [9]                     = 0x0040407F4147, //J
            [10]                    = 0x00412214087F, //K
            [11]                    = 0x00030101017F, //L
            [12]                    = 0x007F2010207F, //M
            [13]                    = 0x007F021C207F, //N
            [14]                    = 0x003E4141413E, //O
            [15]                    = 0x00304848483F, //P
            [16]                    = 0x003F4345413E, //Q
            [17]                    = 0x0030494A4C3F, //R
            [18]                    = 0x002649494932, //S
            [19]                    = 0x0040407F4040, //T
            [20]                    = 0x007E0101017E, //U
            [21]                    = 0x007804030478, //V
            [22]                    = 0x007E010E017E, //W
            [23]                    = 0x004136083641, //X
            [24]                    = 0x0040201F2040, //Y
            [25]                    = 0x006151494543, //Z
            [26]                    = 0x003E615D433E, //0
            [27]                    = 0x00007F402000, //1
            [28]                    = 0x007149454321, //2
            [29]                    = 0x003649494122, //3
            [30]                    = 0x00087F281808, //4
            [31]                    = 0x00464949497A, //5
            [32]                    = 0x00264949493E, //6
            [33]                    = 0x007844424160, //7
            [34]                    = 0x003649494936, //8
            [35]                    = 0x003F48484830  //9
    };

};

#endif //SJSU_DEV_OLEDDRIVER_H
