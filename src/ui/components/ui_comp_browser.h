#ifndef _UI_COMP_BROWSER_H
#define _UI_COMP_BROWSER_H

#include "../ui.h"

#ifdef __cplusplus

extern "C"
{
#endif

#include "../ui_msgs.h"

    lv_obj_t *ui_browserComponent_create(lv_obj_t *comp_parent);

    lv_obj_t *ui_download_popupscreen_init(lv_obj_t *comp_parent);
    lv_obj_t *getPopupTitle();
    lv_obj_t *getPopupFileName();
    lv_obj_t *getPopupProgress();
    lv_obj_t *getPopupSpinner();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
