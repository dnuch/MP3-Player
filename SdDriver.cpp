//
// Created by drnuc on 11/15/2018.
//

#include "SdDriver.h"

bool SdDriver::readSDFiles() {
    FRESULT res;
    DIR dir;
    static FILINFO fno;
    // sd card path
    const char path[] = "1:";

    FIL fil;
    char filename[32];
    S sdFile;

    res = f_opendir(&dir, path);                                   /* open sd dir; "1:" */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                           /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;          /* Continue on error or end of dir */
            if (fno.fattrib & AM_DIR) {                            /* It is a directory */
                // do nothing
            } else {                                               /* It is a file. */
                if (strlen(fno.fname) < 4) continue;               /* does not have .mp3/.txt extension */

                if (checkExtension(fno.fname, ".mp3")         /* is mp3/txt file */
//                    || checkExtension(fno.fname, ".txt")
                    ) {
                    strcpy(filename, path);
                    strcat(filename, fno.fname);

                    u0_dbg_printf("%s size: %lu bytes\n", filename, fno.fsize);

                    res = f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ);    /* open found mp3 file */
                    if (res == FR_OK) {
                        strcpy(sdFile.fileName, filename);
                        sdFiles.push_back(sdFile);                               /* fill mp3 vector array */
                        f_close(&fil);
                    }
                }
            }
        }
        f_closedir(&dir);
    }
    return true;
}
