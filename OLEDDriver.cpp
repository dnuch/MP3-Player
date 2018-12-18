//
// SH1106 implementation
//

#include "OLEDDriver.h"

OLEDDriver::OLEDDriver()
{
    i2c2 = &i2c2->getInstance();
}

bool OLEDDriver::Init()
{
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

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN);

    fillDisplay(0x00);

    return true;
}

void OLEDDriver::toggleDisplay()
{
    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_OFF);
    delay_ms(1000);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_ON);
}

void OLEDDriver::fillDisplay(uint8_t byte)
{
    uint8_t row, column, buffer[6];
    memset(buffer, byte, sizeof(buffer));
    for (row = 0; row < OLED_HEIGHT / PAGE_COUNT; row++) { // row = page
        i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | row));
        for (column = 0; column < OLED_WIDTH; column += 6) {
            /* Clear the display - 8x6 pixels at the time */
            for (unsigned char b : buffer) {
                i2c2->writeReg(OLED_ADDRESS, DATA, b);
            }
        }
    }
}

void OLEDDriver::testDisplay()
{
    fillDisplay(0xFF);
    delay_ms(1000);
    fillDisplay(0x00);
    delay_ms(1000);
    fillDisplay(0x02);
    delay_ms(1000);
    fillDisplay(0xFF);
}

void OLEDDriver::clearDisplay()
{
    uint8_t row, column, buffer[6];
    memset(buffer, 0xFF, sizeof(buffer));
    for (row = 0; row < OLED_HEIGHT / PAGE_COUNT; row++)
    { // row = page
        i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | row));
        for (column = 0; column < OLED_WIDTH; column += 6)
        {
            /* Clear the display - 8x6 pixels at the time */
            for (unsigned char b : buffer)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, b);
            }
        }
    }
}

void OLEDDriver::resetCursor()
{
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | 0x02);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN);
}

void OLEDDriver::moveCursor(uint8_t row, uint8_t column)
{
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_PAGE_ADDR | row);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | (((column*6 + 0x02) & 0x0F ) >> 0));
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | (((column*6 + 0x02) & 0xF0) >> 4));
}

uint64_t OLEDDriver::charToDisplay(char c)
{
    if(c > 90) {
        //Letter is lower case
        c = c - 32;
    }
    else if(c > 47 && c < 58){
        //c is a number
        return charHexValues[c - 22];
    }
    return charHexValues[c - 65]; //to make the range between 0 and 25
}

void OLEDDriver::printChar(char c)
{
    uint64_t character = charToDisplay(c);
    for(int i = 0; i < 6; i++){
        i2c2->writeReg(OLED_ADDRESS, DATA, character >> 8*i);
    }
}
void OLEDDriver::printLine(const char *s, uint8_t row, uint8_t column)
{
    /*use s to point at every index of the string and at that
     * index print out the char*/

    moveCursor(row, column);

    while(*s != '\0')
    {
        if(*s == ' ')
        {
            printSymbol(SPACE);
        } else if(*s == ':')
        {
            printSymbol(COLON);
        }
        else if(*s == '.')
        {
            printSymbol(PERIOD);
        }
        else
        {
            printChar(*s);

        }
        s++;
    }

}

void OLEDDriver::printSymbolAtPosition(symbol_t symbol, uint8_t row, uint8_t column)
{
    moveCursor(row, column);
    for(uint8_t i = 0; i < 6; i++)
    {
        i2c2->writeReg(OLED_ADDRESS, DATA, (symbol >> 8 * i));
    }

}

void OLEDDriver::printCurrentSong(const char *s) //PRINT TO PAGE 6, STARTING AT COLUMN 2
{
    moveCursor(6, 2);
    //clear line, columns 2 - 21
    for(uint8_t i = 0; i <= 18; i++)
    {
        printSymbol(SPACE);
    }
    printLine(s, 6, 2);
}

void OLEDDriver::printPause()
{
    printSymbolAtPosition(SPACE, 6, 1);
    printSymbolAtPosition(PAUSE, 6, 1);
}

void OLEDDriver::printPlay()
{
    printSymbolAtPosition(SPACE, 6, 1);
    printSymbolAtPosition(PLAY, 6, 1);
}

void OLEDDriver::setVolumeBars(uint8_t vol) {
    if(vol == 0) {
        printVolumeBars(VOL_ZERO, VOL_ZERO);
    } else if(vol == 1) {
        printVolumeBars(VOL_ONE, VOL_ZERO);
    } else if(vol == 2) {
        printVolumeBars(VOL_TWO, VOL_ZERO);
    } else if(vol == 3) {
        printVolumeBars(VOL_THREE, VOL_ZERO);
    } else if(vol == 4) {
        printVolumeBars(VOL_THREE, VOL_FOUR);
    } else {
        printVolumeBars(VOL_THREE, VOL_FIVE);
    }
}

void OLEDDriver::initVolume(uint8_t vol)
{
    moveCursor(5, 9);
    for(uint8_t i = 0; i < 9; i++)
    {
        printSymbol(SPACE);
    }
    printLine("Volume", 5, 9);
    printSymbol(COLON);
    setVolumeBars(vol);
}

void OLEDDriver::printListSong(const char *s, position_t pos)
{
    uint8_t row = 0;
    switch (pos)
    {
        case TOP: row = 3; break;
        case MID: row = 2; break;
        case BOT: row = 1;
    }

    moveCursor(row, 2);
    for(uint8_t i = 0; i <= 18; i++)
    {
        printSymbol(SPACE);
    }

    printLine(s, row, 2);
}

void OLEDDriver::printSymbol(symbol_t sym)
{
    for(uint8_t i = 0; i < 6; i ++)
    {
        i2c2->writeReg(OLED_ADDRESS, DATA, (sym >> 8 * i));
    }
}

void OLEDDriver::printListArrow(uint8_t pos)
{
    uint8_t row = 0;
    switch (pos)
    {
        case TOP: row = 3; break;
        case MID: row = 2; break;
        case BOT: row = 1;
    }

    if (pos == TOP || pos == MID) eraseSymbolAtPosition(1,1);
    if (pos == TOP || pos == BOT) eraseSymbolAtPosition(2,1);
    if (pos == MID || pos == BOT) eraseSymbolAtPosition(3,1);
    printSymbolAtPosition(ARROW, row, 1);
}

void OLEDDriver::eraseSymbolAtPosition(uint8_t row, uint8_t column)
{
    printSymbolAtPosition(SPACE, row, column);
}

void OLEDDriver::printVolumeBars(volume_t first, volume_t second)
{
    printSymbolAtPosition(SPACE, 5, 16);
    printSymbolAtPosition(SPACE, 5, 17);
    moveCursor(5, 16);
    for(uint8_t i = 0; i < 6; i++)
    {
        i2c2->writeReg(OLED_ADDRESS, DATA, (first >> 8 * i));
    }
    for(uint8_t j = 0; j < 6; j++)
    {
        i2c2->writeReg(OLED_ADDRESS, DATA, (second >> 8 * j));
    }
}

void OLEDDriver::setSongList(const char *topSong, const char *midSong, const char *botSong, position_t arrowPosition)
{
    printListSong(topSong, TOP);
    printListSong(midSong, MID);
    printListSong(botSong, BOT);
    printListArrow(arrowPosition);
}
