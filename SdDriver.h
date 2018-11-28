//
// Created by drnuc on 11/15/2018.
//

#ifndef SDDRIVER_H
#define SDDRIVER_H

#include "ff.h"
#include "printf_lib.h"
#include <string.h>
#include <vector>
#include <ctype.h>

class SdDriver {
private:
    struct S {
       char fileName[32];
    };
    /**
     * vector of sd file File Names
     */
    std::vector<S> sdFiles;
    /**
     * points to current file in sdFiles vector
     */
    uint16_t currentMp3Index;

public:
    SdDriver() : currentMp3Index(0) {}
    bool Init() { return readSDFiles(); }

    bool readSDFiles();
    void setNextSong();

    void setPreviousSong() { if (currentMp3Index != 0) currentMp3Index--; }

    char * getCurrentFileName() { return sdFiles[currentMp3Index].fileName; }

    bool findFile(const char * fileName);

    void printMp3Names();

    friend bool checkExtension(const char * fileName, const char * ext);
};

inline bool checkExtension(const char * fileName, const char * ext) {
    char extension[8];
    strcpy(extension, &fileName[strlen(fileName)-4]);

    if (isupper(extension[1])) {
        for (unsigned int i = 1; i < strlen(extension); i++) {
            extension[i] = (char) tolower(extension[i]);
        };
    }

    return strncmp(ext, extension, strlen(extension)) == 0;
}

inline void SdDriver::setNextSong() {
    if (sdFiles.size() != (unsigned int)(currentMp3Index + 1)) {
        currentMp3Index++;
    }

    while (!checkExtension(getCurrentFileName(), ".mp3")) {
        if (sdFiles.size() != (unsigned int)(currentMp3Index + 1)) {
            currentMp3Index++; /* skip non .mp3 files */
        }
    }
}

inline void SdDriver::printMp3Names() {
    for (const auto &file : sdFiles)
        if (checkExtension(file.fileName, ".mp3")) u0_dbg_printf("%s\n", file.fileName);
}

inline bool SdDriver::findFile(const char * fileName) {
    for (uint8_t i = 0; i < sdFiles.size(); i++) {
        if (strncmp(fileName, sdFiles[i].fileName, strlen(sdFiles[i].fileName)) == 0) {
            currentMp3Index = i;
            return true;
        }
    }
    return false;
}

#endif //SDDRIVER_H
