#ifndef _XLCD_SDCARD
#define _XLCD_SDCARD

#include "FS.h"
#include "SD.h"
#include <ArduinoJson.h>
#include <Arduino.h>

static SPIClass _spi_sd(HSPI);

bool xtouch_sdcard_setup() {
    _spi_sd.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
    if (!SD.begin(SD_CS, _spi_sd)) {
        LOGE("[XTouch][SD] Card Mount Failed\n");
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        LOGE("[XTouch][SD] No SD card attached\n");
        return false;
    }

    char *str_card = "UNKNOWN";

    if (cardType == CARD_MMC) {
        str_card = "MMC";
    } else if (cardType == CARD_SD) {
        str_card = "SDSC";
    } else if (cardType == CARD_SDHC) {
        str_card = "SDHC";
    }
    LOGI("[XTouch][SD] SD Card Type: %s\n", str_card);

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    LOGI("[XTouch][SD] SD Card Size: %lluMB\n", cardSize);
    xtouch_filesystem_mkdir(SD, xtouch_paths_root);

    return true;
}

#endif
