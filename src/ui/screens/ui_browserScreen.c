#include "../ui.h"

lv_obj_t* ui_browserScreen_screen_init(void)
{
    lv_obj_t* ui_browserScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_browserScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM); /// Flags
    lv_obj_set_scrollbar_mode(ui_browserScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_browserScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_browserScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    // lv_obj_set_style_pad_left(ui_browserScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_right(ui_browserScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_top(ui_browserScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_bottom(ui_browserScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_row(ui_browserScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_column(ui_browserScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_browserScreen);

    lv_obj_t *ui_browserComponent = ui_browserComponent_create(ui_browserScreen);

    return ui_browserScreen;
}
