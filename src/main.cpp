#include <Arduino.h>
#include <ArduinoJson.h>
#include "xtouch/debug.h"
#include "xtouch/paths.h"
#include "xtouch/eeprom.h"
#include "xtouch/types.h"
#include "xtouch/bblp.h"
#include "xtouch/globals.h"
#include "xtouch/filesystem.h"
#include "ui/ui.h"
#include "xtouch/sdcard.h"
#include "xtouch/hms.h"
#include "xtouch/ssdp.h"

#if defined(ESP32_2432S028R)
#include "devices/2.8/screen.h"
#elif defined(ESP32_4848S040CIY1)
#include "devices/4.0/screen.h"
#endif

#include "xtouch/pair.h"
#include "xtouch/settings.h"
#include "xtouch/net.h"
#include "xtouch/firmware.h"
#include "xtouch/mqtt.h"
#include "xtouch/sensors/chamber.h"
#include "xtouch/events.h"
#include "xtouch/connection.h"
#include "xtouch/coldboot.h"
#include "lv_fs_if.h"

extern "C" {
    void print_sram_info() {
        LOGI("[PSRAM] free:%d, heap:%d\n", ESP.getFreePsram(), ESP.getFreeHeap());
    }
}

void xtouch_intro_show(void) {
    loadScreen(SCREEN_INTRO);
    lv_timer_handler();
}

void setup() {
#if XTOUCH_USE_SERIAL == true || XTOUCH_DEBUG_ERROR == true || XTOUCH_DEBUG_DEBUG == true || XTOUCH_DEBUG_INFO == true
    Serial.begin(115200);
#endif

    xTouchConfig.currentScreenIndex = -1;
    LOGI("setup start !!!\n");
    LOGI("[CPU]   speed:%ld\n", getCpuFrequencyMhz());
    LOGI("[ROM]   size:%d, speed:%d\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    LOGI("[PSRAM] size:%d, heap:%d\n", ESP.getPsramSize(), ESP.getFreeHeap());

    xtouch_eeprom_setup();
    xtouch_globals_init();
    xtouch_screen_setup();
    xtouch_intro_show();
    while (!xtouch_sdcard_setup()) {
        lv_label_set_text(introScreenCaption, LV_SYMBOL_SD_CARD " INSERT SD CARD");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
    }
    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_port_sd_fs_init();

    xtouch_coldboot_check();

    xtouch_settings_loadSettings();

    xtouch_firmware_checkFirmwareUpdate();

    xtouch_touch_setup();

    while (!xtouch_wifi_setup());

    xtouch_firmware_checkOnlineFirmwareUpdate();

    xtouch_screen_setupScreenTimer();
    xtouch_setupGlobalEvents();
    xtouch_pair_check();

    xtouch_mqtt_setup();
    xtouch_chamber_timer_init();

    // loadScreen(SCREEN_BROWSER);
}

void loop() {
    lv_timer_handler();
    lv_task_handler();
    xtouch_mqtt_loop();
}


/*
    ESP32_SFTPClient test codes

    ESP32_SFTPClient sftp((char*)"192.168.0.159", 990, (char*)"bblp", (char*)"34801960", 10000, 2);

    sftp.OpenConnection(false, true);
    String list[128];

    sftp.InitFile("TYPE A");
    sftp.DirLong("/image", list);
    for (uint8_t i = 0; i < sizeof(list); i++) {
        uint8_t indexSize = 0;
        uint8_t indexMod = 0;

        if (list[i].length() > 0) {
            printf("%s\n", list[i].c_str());
            list[i].clear();
        } else {
            break;
        }
    }

    File file = SD.open("/image/down.png", "wb", true);
    sftp.InitFile("Type I");
    sftp.DownloadFile("/image/15137327011.png", 4442, &file);
    file.close();

    sftp.CloseConnection();
*/
