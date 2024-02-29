#include <Arduino.h>
#include <ArduinoJson.h>
#include "debug.h"
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
#include "main.h"

#include "clients/camera_worker.h"
#include "clients/web_worker.h"
#include <LittleFS.h>

/*
*****************************************************************************************
* VARIABLES
*****************************************************************************************
*/
static SemaphoreHandle_t   _lock;


/*
*****************************************************************************************
* UTILITY FUNCTIONS
*****************************************************************************************
*/
void lv_lock() {
    while(xSemaphoreTake(_lock, portMAX_DELAY) != pdPASS);
}

void lv_unlock() {
    xSemaphoreGive(_lock);
}

void print_ram_info() {
    LOGI("[RAM FREE] PSRAM:%d, HEAP:%d\n", ESP.getFreePsram(), ESP.getFreeHeap());
}


/*
*****************************************************************************************
* FUNCTION DEFINITION
*****************************************************************************************
*/
void xtouch_intro_show(void) {
    loadScreen(SCREEN_INTRO);
    lv_timer_handler();
}


CameraWorker *_cam;
WebWorker    *_web;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    LOGD("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        LOGE("- failed to open directory\n");
        return;
    }
    if(!root.isDirectory()){
        LOGE(" - not a directory\n");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            LOGD("%4d-%02d-%02d %02d:%02d:%02d <DIR> %8s %s\n",(tmstruct->tm_year) + 1900,( tmstruct->tm_mon) + 1, 
                tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec,
                "", file.name());
            if(levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            LOGD("%4d-%02d-%02d %02d:%02d:%02d       %8ld %s\n",(tmstruct->tm_year) + 1900,( tmstruct->tm_mon) + 1, 
                tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec,
                file.size(), file.name());
        }
        file = root.openNextFile();
    }
}

void setup() {
#if XTOUCH_USE_SERIAL == true || XTOUCH_DEBUG_ERROR == true || XTOUCH_DEBUG_DEBUG == true || XTOUCH_DEBUG_INFO == true
    Serial.begin(115200);
#endif
    LOGI("setup start !!!\n");
    LOGI("[CPU] speed:%ld\n", getCpuFrequencyMhz());
    LOGI("[ROM]  size:%d, speed:%d\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    LOGI("[RAM]  heap:%d, PSRAM:%d\n", ESP.getHeapSize(), ESP.getPsramSize());
    heap_caps_malloc_extmem_enable(4096);

    // if(!LittleFS.begin(true)){
    //     LOGE("LittleFS Mount Failed\n");
    //     return;
    // } else {
    //     LOGD("LittleFS mounted : %d\n", LittleFS.totalBytes());
    // }
    // listDir(LittleFS, "/", 2);

    _lock = xSemaphoreCreateMutex();

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

// #if !_NO_NETWORK_
    while (!xtouch_wifi_setup());
    // xtouch_firmware_checkOnlineFirmwareUpdate();
// #endif

    xtouch_screen_setupScreenTimer();
    xtouch_setupGlobalEvents();

#if !_NO_NETWORK_
    xtouch_pair_check();
    xtouch_mqtt_setup();
#endif

    xtouch_chamber_timer_init();
    LOGD("priority : before %d\n", uxTaskPriorityGet(NULL));
    vTaskPrioritySet(NULL, 5);
    LOGD("priority : before %d\n", uxTaskPriorityGet(NULL));

#if _NO_NETWORK_
    loadScreen(SCREEN_BROWSER);
#endif

    uint32_t fh = ESP.getFreeHeap();
    uint32_t fp = ESP.getFreePsram();
    LOGI("[FREE]  heap:%d, PSRAM:%d, Total:%d\n", fh, fp, fh + fp);

    // _cam = new CameraWorker((char*)xTouchConfig.xTouchIP, 6000, (char*)"bblp", (char*)xTouchConfig.xTouchAccessCode);
    // _cam->connect();

    _web = new WebWorker(&SD, "/web/", 80);
    _web->start();
}

void loop() {
    lv_lock();
    lv_task_handler();
    lv_unlock();
#if !_NO_NETWORK_
    xtouch_mqtt_loop();
#endif
}
