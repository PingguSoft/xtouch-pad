#ifndef _UI_COMP_SIDEBARCOMPONENT_H
#define _UI_COMP_SIDEBARCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void ui_sidebarComponent_set_active(int index);
    lv_obj_t *ui_sidebarComponent_create(lv_obj_t *comp_parent);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
