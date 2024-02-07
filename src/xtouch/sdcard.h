#ifndef _XLCD_SDCARD
#define _XLCD_SDCARD

#include "FS.h"
#include "SD.h"
#include <ArduinoJson.h>
#include <Arduino.h>

static SPIClass _spi_sd(HSPI);

#if 0
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

esp_err_t mount_sd_card() {
    esp_err_t ret;

    sdmmc_card_t* card;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 10,
        .allocation_unit_size = 16 * 1024,
    };
    ESP_LOGI(TAG, "Initializing sd Card");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = HSPI_HOST;

    // spi总线配置
    spi_bus_config_t bus_config = {
        .mosi_io_num = SD_MOSI,
        .miso_io_num = SD_MISO,
        .sclk_io_num = SD_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4 * 1024 * sizeof(uint8_t)
    };

    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_config, host.slot);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "failed to initialize bus");
        return ret;
    }
    ESP_LOGI(TAG, "host slot (SPI%d)", host.slot);

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)SD_CS;
    // slot_config.gpio_cd = ;
    slot_config.host_id = (spi_host_device_t)host.slot;
    ret = esp_vfs_fat_sdspi_mount("/", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGI(TAG, "failed to mount filesystem."
                "if you want to card to be formatted,"
                "set the examole_format_if_mount_failerd menuconfig option");
        } else {
            ESP_LOGI(TAG, "failed to initialize the card (%s)"
                "make sure sd card line have pull-up resistors in place",
                esp_ - esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}
#endif

bool xtouch_sdcard_setup() {
#if 1
    _spi_sd.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
    if (!SD.begin(SD_CS, _spi_sd)) {
        ConsoleError.println("[XTouch][SD] Card Mount Failed");
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        ConsoleError.println("[XTouch][SD] No SD card attached");
        return false;
    }

    ConsoleInfo.print("XTouch][SD] SD Card Type: ");
    if (cardType == CARD_MMC) {
        ConsoleInfo.println("[XTouch][SD] MMC");
    } else if (cardType == CARD_SD) {
        ConsoleInfo.println("[XTouch][SD] SDSC");
    } else if (cardType == CARD_SDHC) {
        ConsoleInfo.println("[XTouch][SD] SDHC");
    } else {
        ConsoleInfo.println("[XTouch][SD] UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    ConsoleInfo.printf("[XTouch][SD] SD Card Size: %lluMB\n", cardSize);
    xtouch_filesystem_mkdir(SD, xtouch_paths_root);

#else
    mount_sd_card();
#endif

    return true;
}

#endif
