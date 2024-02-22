#ifndef _XLCD_SCREEN
#define _XLCD_SCREEN

#include <esp32_smartdisplay.h>

static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 480;


#include "ui/ui.h"
#include "touch.h"
#include "xtouch/globals.h"

bool xtouch_screen_touchFromPowerOff = false;

void xtouch_screen_setBrightness(byte brightness)
{
    smartdisplay_lcd_set_backlight((float)(brightness / 255.0f));
}

void xtouch_screen_setBackLedOff()
{
}

void xtouch_screen_wakeUp()
{
    LOGI("[XTouch][SCREEN] xtouch_screen_wakeUp\n");
    lv_timer_reset(xtouch_screen_onScreenOffTimer);
    xtouch_screen_touchFromPowerOff = false;
    // loadScreen(SCREEN_HOME);
    xtouch_screen_setBrightness(xTouchConfig.xTouchBacklightLevel);
}

void xtouch_screen_onScreenOff(lv_timer_t *timer)
{
    // if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING)
    // {
    //     return;
    // }

    if (xTouchConfig.xTouchTFTOFFValue < XTOUCH_LCD_MIN_SLEEP_TIME)
    {
        return;
    }

    LOGI("[XTouch][SCREEN] Screen Off\n");
    xtouch_screen_setBrightness(0);
    xtouch_screen_touchFromPowerOff = true;
}

void xtouch_screen_setupScreenTimer()
{
    xtouch_screen_onScreenOffTimer = lv_timer_create(xtouch_screen_onScreenOff, xTouchConfig.xTouchTFTOFFValue * 1000 * 60, NULL);
    lv_timer_pause(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_startScreenTimer()
{
    lv_timer_resume(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_setScreenTimer(uint32_t period)
{
    lv_timer_set_period(xtouch_screen_onScreenOffTimer, period);
}

void xtouch_screen_invertColors()
{
    // tft.invertDisplay(xTouchConfig.xTouchTFTInvert);
}

byte xtouch_screen_getTFTFlip()
{
    byte val = xtouch_eeprom_read(XTOUCH_EEPROM_POS_TFTFLIP);
    xTouchConfig.xTouchTFTFlip = val;
    return val;
}

void xtouch_screen_setTFTFlip(byte mode)
{
    xTouchConfig.xTouchTFTFlip = mode;
    xtouch_eeprom_write(XTOUCH_EEPROM_POS_TFTFLIP, mode);
}

void xtouch_screen_toggleTFTFlip()
{
    xtouch_screen_setTFTFlip(!xtouch_screen_getTFTFlip());
    // xtouch_resetTouchConfig();

    lv_disp_t *dispp = lv_disp_get_default();
    lv_disp_set_rotation(dispp, xTouchConfig.xTouchTFTFlip ? LV_DISP_ROT_180 : LV_DISP_ROT_NONE);
}

void xtouch_screen_dispFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    // lv_disp_flush_ready(disp);
}

void xtouch_screen_touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    lv_timer_reset(xtouch_screen_onScreenOffTimer);
    // dont pass first touch after power on
    if (xtouch_screen_touchFromPowerOff)
    {
        xtouch_screen_wakeUp();
    }
}

void xtouch_screen_setup()
{
    xtouch_screen_setBackLedOff();
    smartdisplay_init();
#if LV_IMG_CACHE_DEF_SIZE
    lv_img_cache_set_size(40);
#endif
    smartdisplay_set_touch_cb(xtouch_screen_touchRead);
    xtouch_screen_setBrightness(255);

    /*Initialize the graphics library */
    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t *dispp = lv_disp_get_default();
    lv_disp_set_rotation(dispp, xtouch_screen_getTFTFlip() ? LV_DISP_ROT_180 : LV_DISP_ROT_NONE);

    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    initTopLayer();
    xTouchConfig.currentScreenIndex = -1;
}

#endif
