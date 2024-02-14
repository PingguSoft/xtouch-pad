#ifndef _XLCD_TOUCH
#define _XLCD_TOUCH

XTouchPanelConfig x_touch_touchConfig;

class ScreenPoint
{
public:
    int16_t x;
    int16_t y;

    // default constructor
    ScreenPoint()
    {
    }

    ScreenPoint(int16_t xIn, int16_t yIn)
    {
        x = xIn;
        y = yIn;
    }
};

ScreenPoint getScreenCoords(int16_t x, int16_t y)
{
    int16_t xCoord = round((x * x_touch_touchConfig.xCalM) + x_touch_touchConfig.xCalC);
    int16_t yCoord = round((y * x_touch_touchConfig.yCalM) + x_touch_touchConfig.yCalC);
    if (xCoord < 0)
        xCoord = 0;
    if (xCoord >= 320)
        xCoord = 320 - 1;
    if (yCoord < 0)
        yCoord = 0;
    if (yCoord >= 240)
        yCoord = 240 - 1;
    return (ScreenPoint(xCoord, yCoord));
}

void xtouch_loadTouchConfig(XTouchPanelConfig &config)
{
    // Open file for reading
    File file = xtouch_filesystem_open(SD, xtouch_paths_touch);

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<512> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        LOGE("[XTouch][Touch] Failed to read touch config\n");

    config.xCalM = doc["xCalM"].as<float>();
    config.yCalM = doc["yCalM"].as<float>();
    config.xCalC = doc["xCalC"].as<float>();
    config.yCalC = doc["yCalC"].as<float>();

    file.close();
}

void xtouch_saveTouchConfig(XTouchPanelConfig &config)
{
    // StaticJsonDocument<512> doc;
    DynamicJsonDocument doc(512);
    doc["xCalM"] = config.xCalM;
    doc["yCalM"] = config.yCalM;
    doc["xCalC"] = config.xCalC;
    doc["yCalC"] = config.yCalC;
    xtouch_filesystem_writeJson(SD, xtouch_paths_touch, doc);
}

void xtouch_resetTouchConfig()
{
    LOGI("[XTouch][FS] Resetting touch config\n");
    xtouch_filesystem_deleteFile(SD, xtouch_paths_touch);
    delay(500);
    ESP.restart();
}

bool hasTouchConfig()
{
    LOGI("[XTouch][FS] Checking for touch config\n");
    return xtouch_filesystem_exist(SD, xtouch_paths_touch);
}

void xtouch_touch_setup()
{
    if (hasTouchConfig())
    {
        LOGI("[XTouch][TOUCH] Load from disk\n");
        xtouch_loadTouchConfig(x_touch_touchConfig);
    }
    else
    {
        LOGI("[XTouch][TOUCH] Touch Setup\n");

        lv_label_set_text(introScreenCaption, "Touch the  " LV_SYMBOL_PLUS "  with the stylus");
        lv_timer_handler();
        xtouch_saveTouchConfig(x_touch_touchConfig);
        loadScreen(SCREEN_INTRO);
    }
}

#endif
