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
#include "xtouch/ftps_worker.h"

static FTPSWorker *_ftps = NULL;

FTPSWorker *getFTPSWorker() {
    return _ftps;
}

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

    LOGI("setup start !!!\n");
    LOGI("[CPU] speed:%ld\n", getCpuFrequencyMhz());
    LOGI("[ROM]  size:%d, speed:%d\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    LOGI("[RAM]  heap:%d, PSRAM:%d\n", ESP.getFreeHeap(), ESP.getPsramSize());

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

#if !_NO_NETWORK_
    while (!xtouch_wifi_setup());
    xtouch_firmware_checkOnlineFirmwareUpdate();
#endif

    xtouch_screen_setupScreenTimer();
    xtouch_setupGlobalEvents();

#if !_NO_NETWORK_
    xtouch_pair_check();
    xtouch_mqtt_setup();
#endif

    xtouch_chamber_timer_init();
    LOGD("priority : %d\n", uxTaskPriorityGet(NULL));
    if (_ftps == NULL) {
        // ESP32_FTPSClient ftps((char*)"192.168.0.159", 990, (char*)"bblp", (char*)"34801960", 10000, 2);
        _ftps = new FTPSWorker((char*)xTouchConfig.xTouchIP, 990, (char*)"bblp", (char*)xTouchConfig.xTouchAccessCode);
    }
    loadScreen(SCREEN_BROWSER);
    _ftps->startSync();
}

void loop() {
    lv_timer_handler();
    lv_task_handler();
#if !_NO_NETWORK_
    xtouch_mqtt_loop();
#endif
}
