#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"
#include "io.hpp"
#include "gpio.hpp"
#include "i2c2_device.hpp"
#include "OLEDDriver.h"
#include "tasks.hpp"

int main()
{
    //scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    OLEDDriver oled;
    oled.Init();
    //oled.printLine("hello world", 7, 5);
    //oled.printCurrentSong("songName.mp3");


    //scheduler_start();


    while(1)
    {
        oled.printPlay();
        oled.printCurrentSong("songName.mp3");
        delay_ms(2000);
        oled.printPause();
        oled.printCurrentSong("longSongName.mp3");
        delay_ms(2000);
    }
}

