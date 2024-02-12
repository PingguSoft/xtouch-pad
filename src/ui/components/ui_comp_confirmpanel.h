#ifndef _UI_COMP_CONFIRMPANEL_H
#define _UI_COMP_CONFIRMPANEL_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

// COMPONENT confirmPanel
#define UI_COMP_CONFIRMPANEL_CONFIRMPANEL 0
#define UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER 1
#define UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELCAPTION 2
#define UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELNO 3
#define UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELNO1_CONFIRMPANELNOLABEL 4
#define UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELYES 5
#define UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELYES_CONFIRMPANELYESLABEL 6
#define _UI_COMP_CONFIRMPANEL_NUM 7
    lv_obj_t *ui_confirmPanel_create(lv_obj_t *comp_parent);
    void ui_confirmPanel_show(const char *title, void (*onYES)(void *user_data), void *user_data);
    void ui_confirmPanel_hide();
    void ui_confirmPanel_NOOP();
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
