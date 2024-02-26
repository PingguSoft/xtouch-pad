#include "ui.h"
#include "ui_msgs.h"
#include "main.h"
#include "../xtouch/debug.h"

typedef struct {
    int         id;
    lv_obj_t*   (*init)(void);
    bool        en_sidebar;
} loader_t;

static loader_t _tbl_loader[] = {
    { SCREEN_HOME,       ui_homeScreen_screen_init,         true },
    { SCREEN_TEMP,       ui_temperatureScreen_screen_init,  true },
    { SCREEN_CONTROL,    ui_controlScreen_screen_init,      true },
    { SCREEN_FILAMENT,   ui_filamentScreen_screen_init,     true },
    { SCREEN_SETTING,    ui_settingsScreen_screen_init,     true },
    { SCREEN_BROWSER,    ui_browserScreen_screen_init,      true },

    { SCREEN_INTRO,      ui_introScreen_screen_init,        false },
    { SCREEN_PAIRING,    ui_printerPairScreen_screen_init,  false },
    { SCREEN_ACCESSCODE, ui_accessCodeScreen_screen_init,   false },
};

// lv_obj_t *get_screen_obj(int screen) {
//     if (SCREEN_MIN <= screen && screen < SCREEN_MAX) {
//         return _tbl_loader[screen].screen_obj;
//     }
//     return NULL;
// }

void sendMqttMsg(int message, uint32_t data) {
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = data;
    lv_msg_send(message, &eventData);
}

void fillScreenData(int screen) {
    switch (screen) {
        case SCREEN_HOME:
            sendMqttMsg(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper);
            sendMqttMsg(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper);
            sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
            sendMqttMsg(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper);
            sendMqttMsg(XTOUCH_ON_LIGHT_REPORT, bambuStatus.chamberLed);
            sendMqttMsg(XTOUCH_ON_AMS, bambuStatus.ams);
            sendMqttMsg(XTOUCH_ON_PRINT_STATUS, 0);
            sendMqttMsg(XTOUCH_ON_CHAMBER_TEMP, bambuStatus.chamber_temper);
            break;
        case SCREEN_TEMP:
            sendMqttMsg(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper);
            sendMqttMsg(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper);
            sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
            sendMqttMsg(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper);
            sendMqttMsg(XTOUCH_ON_PART_FAN_SPEED, bambuStatus.cooling_fan_speed);
            sendMqttMsg(XTOUCH_ON_PART_AUX_SPEED, bambuStatus.big_fan1_speed);
            sendMqttMsg(XTOUCH_ON_PART_CHAMBER_SPEED, bambuStatus.big_fan2_speed);
            break;
        case SCREEN_CONTROL:
            sendMqttMsg(XTOUCH_CONTROL_INC_SWITCH, controlMode.inc);
            break;
        case SCREEN_FILAMENT:
            sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
            break;
    }
}

void loadScreen(int screen) {
    if (xTouchConfig.currentScreenIndex == screen)
        return;

    LOGD("screen load : %d\n", screen);
    print_ram_info();
    if (SCREEN_MIN <= screen && screen < SCREEN_MAX) {
        lv_obj_t *scr = _tbl_loader[screen].init();
        lv_disp_load_scr(scr);
        fillScreenData(screen);
        if (_tbl_loader[screen].en_sidebar) {
            ui_sidebarComponent_set_active(screen);
        }
    }
    xTouchConfig.currentScreenIndex = screen;
}

void initTopLayer() {
    ui_confirmComponent = ui_confirmPanel_create(lv_layer_top());
    lv_obj_add_flag(ui_confirmComponent, LV_OBJ_FLAG_HIDDEN);
    ui_hmsComponent = ui_hmsPanel_create(lv_layer_top());
    lv_obj_add_flag(ui_hmsComponent, LV_OBJ_FLAG_HIDDEN);
}
