
#ifndef _V3CONTROLCOMP_UI_LOADERS_H
#define _V3CONTROLCOMP_UI_LOADERS_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "ui.h"
#include "ui_msgs.h"

    void loadScreen(int screen);
    lv_obj_t *get_screen_obj(int screen);
    void initTopLayer();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif