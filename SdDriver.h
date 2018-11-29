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

    /**
     * Reads all file names from sd card and stores in sdFiles vector
     * @return
     */
    bool readSDFiles();
    void setNextSong();

    void setPreviousSong() { if (currentMp3Index != 0) currentMp3Index--; }

    char * getCurrentFileName() { return sdFiles[currentMp3Index].fileName; }

    /**
     * Set file index based on filename in parameter
     * @param fileName
     */
    void setFileIndex(const char * fileName);

    /**
     * Print all mp3 names
     */
    void printMp3Names();

    /**
     * friend function for checking extensions of given filename compared with literal extension
     * @param fileName
     * @param ext
     * @return
     */
    friend bool checkExtension(const char * fileName, const char * ext);
};

inline bool checkExtension(const char * fileName, const char * ext) {
    char extension[8];
    strcpy(extension, &fileName[strlen(fileName)-4]);

    if (isupper(extension[1])) {
        for (unsigned int i = 1; i < strlen(extension); i++) {
            extension[i] = (char)tolower(extension[i]);
        };
    }

    return strcmp(ext, extension) == 0;
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

inline void SdDriver::setFileIndex(const char * fileName) {
    for (uint8_t i = 0; i < sdFiles.size(); i++) {
        if (strcmp(fileName, sdFiles[i].fileName) == 0) {
            currentMp3Index = i;
        }
    }
}

#endif //SDDRIVER_H
