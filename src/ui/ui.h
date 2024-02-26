#ifndef _V3CONTROLCOMP_UI_H
#define _V3CONTROLCOMP_UI_H
#include <Arduino.h>
#include "xtouch/errors.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "xtouch/types.h"

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

#include "ui_msgs.h"
#include "ui_helpers.h"
#include "ui_loaders.h"
#include "components/ui_comp.h"
#include "components/ui_comp_hook.h"
#include "ui_events.h"

enum {
    SCREEN_MIN = 0,
    SCREEN_HOME = SCREEN_MIN,
    SCREEN_TEMP,
    SCREEN_CONTROL,
    SCREEN_FILAMENT,
    SCREEN_SETTING,
    SCREEN_BROWSER,

    SCREEN_INTRO,
    SCREEN_PAIRING,
    SCREEN_ACCESSCODE,
    SCREEN_MAX
};

// SCREEN
lv_obj_t *ui_introScreen_screen_init(void);
lv_obj_t *ui_homeScreen_screen_init(void);
lv_obj_t *ui_temperatureScreen_screen_init(void);
lv_obj_t *ui_controlScreen_screen_init(void);
lv_obj_t *ui_filamentScreen_screen_init(void);
lv_obj_t *ui_settingsScreen_screen_init(void);
lv_obj_t *ui_browserScreen_screen_init(void);
lv_obj_t *ui_printerPairScreen_screen_init(void);
lv_obj_t *ui_accessCodeScreen_screen_init(void);

void ui_event____initial_actions0(lv_event_t *e);
void ui_init(void);
bool xtouch_bblp_is_p1p();
bool xtouch_bblp_is_p1Series();
bool xtouch_bblp_is_x1();
bool xtouch_bblp_is_x1s();


extern lv_timer_t *xtouch_screen_onScreenOffTimer;
extern lv_timer_t *xtouch_ssdp_onButtonTimerTimer;
extern lv_obj_t *introScreenCaption;
extern lv_obj_t *ui_sidebarComponent;
extern lv_obj_t *ui_homeComponent;
extern lv_obj_t *ui_accessCodeScreenKeyboard;
extern lv_obj_t *ui_accessCodeInput;
extern lv_obj_t *ui_temperatureComponent;
extern lv_obj_t *ui_controlComponent;
extern lv_obj_t *ui_settingsComponent;
extern lv_obj_t *ui_settingsBackLightPanelSlider;
extern lv_obj_t *ui_settingsTFTOFFSlider;
extern lv_obj_t *ui_settingsTFTOFFValue;
extern lv_obj_t *ui_settingsTFTInvertSwitch;
extern lv_obj_t *ui_settingsWOPSwitch;
extern lv_obj_t *ui_settingsTFTFlipSwitch;
extern lv_obj_t *ui_settingsBrowserTextSwitch;
extern lv_obj_t *ui_settings_auxFanSwitch;
extern lv_obj_t *ui_settings_chamberFanSwitch;
extern lv_obj_t *ui_settings_chamberSensorSwitch;
extern lv_obj_t *ui_settings_otaSwitch;
extern lv_obj_t *ui_confirmComponent; // layertop
extern lv_obj_t *ui_hmsComponent;     // layertop
extern lv_obj_t *ui_mainStatusComponent;
extern lv_obj_t *ui_filamentComponent;
extern lv_obj_t *ui_printerPairScreenRoller;
extern lv_obj_t *ui_printerPairScreenSubmitButton;
extern lv_obj_t *ui_mainScreenChamberTempValue;
extern lv_obj_t *ui____initial_actions0;

LV_FONT_DECLARE(ui_font_xlcd);
LV_FONT_DECLARE(ui_font_xlcdmin);
LV_FONT_DECLARE(ui_font_xperiments);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
