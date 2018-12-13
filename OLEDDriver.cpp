//
// Created by drnuc on 12/4/2018.
//

#include "OLEDDriver.h"

OLEDDriver::OLEDDriver() {
    i2c2 = &i2c2->getInstance();
}

bool OLEDDriver::Init() {
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

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | 0x02);

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN);

    fillDisplay(0x00);


    return true;
}

void OLEDDriver::toggleDisplay() {
    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_OFF);
    delay_ms(1000);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, DISPLAY_ON);
}

void OLEDDriver::fillDisplay(uint8_t byte) {
    uint8_t row, column, buffer[6];
    memset(buffer, byte, sizeof(buffer));
    for (row = 0; row < OLED_HEIGHT / 8; row++) { // row = page
        i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | row));
        for (column = 0; column < OLED_WIDTH; column += 6) {
            /* Clear the display - 8x6 pixels at the time */
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

void OLEDDriver::writeS()
{
    clearDisplay();

    uint8_t row, column;
    //, buffer[8];
    //memset(buffer, byte, sizeof(buffer));
    for (row = 0; row < OLED_HEIGHT / 8; row++)
    { // row = page
        i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | row));
        for (column = 0; column < 12; column++)
        {
            //D7 - D0
            //i2c2->writeReg(OLED_ADDRESS, DATA, 0x0F);
            if(column%6 == 0)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, 0x89);
            }
            if(column%6 ==1)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, 0x6E);
            }
            if(column%6 ==2)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, 0x6E);
            }
            if(column%6 ==3)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, 0x6E);
            }
            if(column%6 ==4)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, 0xB1);
            }
            if(column%6 ==5)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, 0xFF);
            }
            if(column%6 ==6)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, 0xFF);
            }
            if(column%6 ==7)
            {
                i2c2->writeReg(OLED_ADDRESS, DATA, 0xFF);
            }


        }
        resetCursor();
    }
}

void OLEDDriver::clearDisplay()
{
    uint8_t row, column, buffer[6];
    memset(buffer, 0xFF, sizeof(buffer));
    for (row = 0; row < OLED_HEIGHT / 8; row++)
    { // row = page
        i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | row));
        for (column = 0; column < OLED_WIDTH; column += 6)
        {
            /* Clear the display - 8x8 pixels at the time */
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

uint64_t OLEDDriver::charToDisplay(char c){
    if(c > 90) {
        //Letter is lower case
        c = c -32;
    }
    else if( c > 47 && c < 58){
        //c is a number
        return charHexValues[c -22];
    }
    return charHexValues[c - 65]; //to make the range between 0 and 25
}

void OLEDDriver::printChar(char c){
    uint64_t character = charToDisplay(c);


    //i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | 2));
    for(int i = 0; i < 6; i++){
        i2c2->writeReg(OLED_ADDRESS, DATA, character >> 8*i);
    }
}
void OLEDDriver::printLine(const char *s, uint8_t row, uint8_t column)
{
    /*use s to point at every index of the string and at that
     * index print out the char*/

    i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | row));
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | (((column*6 + 0x02) & 0x0F ) >> 0));

    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | (((column*6 + 0x02) & 0xF0) >> 4));


    while(*s != '\0')
    {
        if(*s == ' ')
        {
            printSymbol(SPACE);
        } else if(*s == ':')
        {
            printSymbol(COLON);
        }
        else
        {
            printChar(*s);

        }
        s++;
    }

}
void OLEDDriver::printSymbol(symbol_t sym)
{
    for(uint8_t i = 0; i < 6; i ++)
    {
        i2c2->writeReg(OLED_ADDRESS, DATA, (sym >> 8 * i));
    }
}

void OLEDDriver::printSymbolAtPosition(symbol_t symbol, uint8_t row, uint8_t column)
{
    i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | row));
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | (((column*6 + 0x02) & 0x0F) >> 0)); //lower hex + 2 to low column
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | (((column*6 +0x02) & 0xF0) >> 4)); //high hex value to high column

    for(uint8_t i = 0; i < 6; i++)
    {
        i2c2->writeReg(OLED_ADDRESS, DATA, (symbol >> 8 * i));
    }

}

void OLEDDriver::printCurrentSong(const char *s) //PRINT TO PAGE 6, STARTING AT COLUMN 2
{
    //set page and move cursor
    i2c2->writeReg(OLED_ADDRESS, CONTROL, (uint8_t)(SET_PAGE_ADDR | 6));
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | ((0x16 & 0x0F) >> 0)); //22 dec -> 16 hex
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | ((0x16 & 0xF0) >> 4));
    //clear line, columns 2 - 21
    for(uint8_t i = 0; i <= 18; i++){
        printSymbol(SPACE);
    }
    //reset cursor
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | ((0x16 & 0x0F) >> 0)); //22 dec -> 16 hex
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | ((0x16 & 0xF0) >> 4));
    while(*s != '\0')
    {
        if(*s == ' ')
        {
            printSymbol(SPACE);
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

void OLEDDriver::printVolume(uint8_t vol)
{
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_PAGE_ADDR | 5);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | ((0X66 & 0x0F) >> 0)); //102 DEC -> 66 HEX
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | ((0x66 & 0xF0) >> 4));
    for(uint8_t i = 0; i < 9; i++)
    {
        //clears the volume if it exists
        printSymbol(SPACE);
    }
    //reset cursor
//    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | ((0X66 & 0x0F) >> 0)); //102 DEC -> 66 HEX
//    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | ((0x66 & 0xF0) >> 4));
    printLine("Volume", 6, 10); //prints "VOLUME"
    printSymbol(COLON); //prints ":"
    printChar(vol); //prints "#"
}

void OLEDDriver::printTopSong(const char *s)
{
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_PAGE_ADDR | 3);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | ((0x16 & 0x0F) >> 0)); //22 dec ->16 hex
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | ((0x16 & 0xF0) >> 4));
    //clear the line
    for(uint8_t i = 0; i <= 18; i++)
    {
        printSymbol(SPACE);
    }

    printLine(s, 3, 2);
}

void OLEDDriver::printMidSong(const char *s)
{
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_PAGE_ADDR | 2);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | ((0x16 & 0x0F) >> 0)); //22 dec ->16 hex
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | ((0x16 & 0xF0) >> 4));
    //clear the line
    for(uint8_t i = 0; i <= 18; i++)
    {
        printSymbol(SPACE);
    }

    printLine(s, 2, 2);
}

void OLEDDriver::printBotSong(const char *s)
{
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_PAGE_ADDR | 1);
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_LOW_COLUMN | ((0x16 & 0x0F) >> 0)); //22 dec ->16 hex
    i2c2->writeReg(OLED_ADDRESS, CONTROL, SET_HIGH_COLUMN | ((0x16 & 0xF0) >> 4));
    //clear the line
    for(uint8_t i = 0; i <= 18; i++)
    {
        printSymbol(SPACE);
    }

    printLine(s, 1, 2);
}

void OLEDDriver::moveArrowTop()
{
    printSymbolAtPosition(ARROW, 3, 1);
}

void OLEDDriver::moveArrowMid()
{
    printSymbolAtPosition(ARROW, 2, 1);
}

void OLEDDriver::moveArrowBot()
{
    printSymbolAtPosition(ARROW, 1, 1);
}

