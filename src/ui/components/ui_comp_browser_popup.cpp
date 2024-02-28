#include "debug.h"
#include "ui_comp_browser.h"

static lv_obj_t *ui_popup_title;
static lv_obj_t *ui_filename;
static lv_obj_t *ui_progress;
static lv_obj_t *ui_spinner;

lv_obj_t *ui_download_popupscreen_init(lv_obj_t *comp_parent) {
    lv_obj_t *ui_browser_popup = lv_obj_create(comp_parent);
    lv_obj_remove_style_all(ui_browser_popup);
    lv_obj_set_width(ui_browser_popup, lv_pct(100));
    lv_obj_set_height(ui_browser_popup, lv_pct(100));
    lv_obj_set_align(ui_browser_popup, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_browser_popup, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_browser_popup, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_browser_popup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_browser_popup, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_browser_popup, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_browser_popup, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_browser_popup, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_browser_popup, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *ui_container = lv_obj_create(ui_browser_popup);
    lv_obj_remove_style_all(ui_container);
    lv_obj_set_width(ui_container, lv_pct(90));
    lv_obj_set_height(ui_container, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_container, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_obj_clear_flag(ui_container, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_container, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_container, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_container, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_container, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_popup_title = lv_label_create(ui_container);
    lv_obj_set_width(ui_popup_title, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_popup_title, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_popup_title, LV_ALIGN_CENTER);
    lv_label_set_text(ui_popup_title, "Synchronizing with Printer");
    lv_obj_clear_flag(ui_popup_title, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_set_style_text_color(ui_popup_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_popup_title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_popup_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_popup_title, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_popup_title, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_popup_title, lv_color_hex(0x05AB18), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_popup_title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_popup_title, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_popup_title, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_popup_title, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_popup_title, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_spinner = lv_spinner_create(ui_container, 1000, 90);
    lv_obj_set_width(ui_spinner, 70);
    lv_obj_set_height(ui_spinner, 70);
    lv_obj_set_align(ui_spinner, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_spinner, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags

    ui_filename = lv_label_create(ui_container);
    lv_obj_set_width(ui_filename, lv_pct(90));
    lv_obj_set_height(ui_filename, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_filename, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_filename, LV_LABEL_LONG_SCROLL);
    lv_label_set_text(ui_filename, "image file name");
    lv_obj_clear_flag(ui_filename, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_set_style_text_color(ui_filename, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_filename, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_filename, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_filename, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_filename, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_filename, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_filename, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_filename, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(ui_filename, LV_OBJ_FLAG_HIDDEN);

    ui_progress = lv_bar_create(ui_container);
    lv_bar_set_value(ui_progress, 10, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_progress, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_progress, lv_pct(90));
    lv_obj_set_height(ui_progress, lv_pct(15));
    lv_obj_set_align(ui_progress, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_progress, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE);     /// Flags
    lv_obj_set_style_pad_left(ui_progress, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_progress, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_progress, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_progress, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(ui_progress, LV_OBJ_FLAG_HIDDEN);

    return ui_browser_popup;
}

lv_obj_t *getPopupTitle() {
    return ui_popup_title;
}

lv_obj_t *getPopupFileName() {
    return ui_filename;
}

lv_obj_t *getPopupProgress() {
    return ui_progress;
}

lv_obj_t *getPopupSpinner() {
    return NULL; //ui_spinner;
}
