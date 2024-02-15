#include "../ui.h"

void onSidebarComponentButtonClicked(lv_event_t *e) {
    lv_obj_t *target = lv_event_get_target(e);
    // lv_obj_t *parent = lv_obj_get_parent(target);

    // if (parent) {
    //     int i;
    //     // clear checked
    //     for (i = 0; i < lv_obj_get_child_cnt(parent); i++) {
    //         lv_obj_t *child = lv_obj_get_child(parent, i);
    //         lv_obj_clear_state(child, LV_STATE_CHECKED);
    //     }
    // }
    // // set checked
    // lv_obj_add_state(target, LV_STATE_CHECKED);
    int screen = lv_obj_get_index(target);
    loadScreen(screen);
}

void ui_sidebarComponent_set_active(int index) {
    lv_obj_t *target;
    int i;

    for (i = 0; i < lv_obj_get_child_cnt(ui_sidebarComponent); i++) {
        lv_obj_t *child = lv_obj_get_child(ui_sidebarComponent, i);
        lv_obj_clear_state(child, LV_STATE_CHECKED);

        if (index == i) {
            lv_obj_add_state(child, LV_STATE_CHECKED);
        }
    }
}

lv_obj_t *ui_sidebarComponent_create(lv_obj_t *comp_parent)
{
    lv_obj_t *cui_sidebarComponent;
    cui_sidebarComponent = lv_obj_create(comp_parent);
    // lv_obj_set_width(cui_sidebarComponent, 48);
    lv_obj_set_width(cui_sidebarComponent, lv_pct(15));
    lv_obj_set_height(cui_sidebarComponent, lv_pct(100));
    lv_obj_set_x(cui_sidebarComponent, 387);
    lv_obj_set_y(cui_sidebarComponent, 178);
    lv_obj_set_flex_flow(cui_sidebarComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_sidebarComponent, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarComponent, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarComponent, lv_color_hex(0xDDDDDD), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_sidebarHomeButton;
    cui_sidebarHomeButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarHomeButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarHomeButton, 1);
    lv_obj_set_x(cui_sidebarHomeButton, 386);
    lv_obj_set_y(cui_sidebarHomeButton, 178);
    lv_obj_set_flex_flow(cui_sidebarHomeButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarHomeButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarHomeButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarHomeButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarHomeButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarHomeButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarHomeButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarHomeButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarHomeButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarHomeButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarHomeButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarHomeButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarHomeButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarHomeButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarHomeButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarHomeButtonIcon;
    cui_sidebarHomeButtonIcon = lv_label_create(cui_sidebarHomeButton);
    lv_obj_set_width(cui_sidebarHomeButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarHomeButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarHomeButtonIcon, "a");
    lv_obj_clear_flag(cui_sidebarHomeButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarHomeButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_sidebarHomeButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_sidebarTempButton;
    cui_sidebarTempButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarTempButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarTempButton, 1);
    lv_obj_set_x(cui_sidebarTempButton, 386);
    lv_obj_set_y(cui_sidebarTempButton, 178);
    lv_obj_set_flex_flow(cui_sidebarTempButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarTempButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarTempButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarTempButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarTempButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarTempButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarTempButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarTempButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarTempButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarTempButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarTempButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarTempButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarTempButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarTempButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarTempButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarTempButtonIcon;
    cui_sidebarTempButtonIcon = lv_label_create(cui_sidebarTempButton);
    lv_obj_set_width(cui_sidebarTempButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarTempButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarTempButtonIcon, "b");
    lv_obj_clear_flag(cui_sidebarTempButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarTempButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_sidebarTempButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_sidebarControlButton;
    cui_sidebarControlButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarControlButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarControlButton, 1);
    lv_obj_set_x(cui_sidebarControlButton, 386);
    lv_obj_set_y(cui_sidebarControlButton, 178);
    lv_obj_set_flex_flow(cui_sidebarControlButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarControlButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);                                                                                                                                                     /// States
    lv_obj_clear_flag(cui_sidebarControlButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarControlButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarControlButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarControlButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarControlButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarControlButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarControlButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarControlButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarControlButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarControlButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarControlButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarControlButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarControlButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarControlButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarControlButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarControlButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarControlButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarControlButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarControlButtonIcon;
    cui_sidebarControlButtonIcon = lv_label_create(cui_sidebarControlButton);
    lv_obj_set_width(cui_sidebarControlButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarControlButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarControlButtonIcon, "c");
    lv_obj_clear_flag(cui_sidebarControlButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarControlButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_sidebarControlButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_sidebarNozzleButton;
    cui_sidebarNozzleButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarNozzleButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarNozzleButton, 1);
    lv_obj_set_x(cui_sidebarNozzleButton, 386);
    lv_obj_set_y(cui_sidebarNozzleButton, 178);
    lv_obj_set_flex_flow(cui_sidebarNozzleButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarNozzleButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarNozzleButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarNozzleButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarNozzleButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarNozzleButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarNozzleButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarNozzleButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarNozzleButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarNozzleButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarNozzleButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarNozzleButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarNozzleButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarNozzleButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarNozzleButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarNozzleButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarNozzleButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarNozzleButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarNozzleButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarNozzleButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarNozzleButtonIcon;
    cui_sidebarNozzleButtonIcon = lv_label_create(cui_sidebarNozzleButton);
    lv_obj_set_width(cui_sidebarNozzleButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarNozzleButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarNozzleButtonIcon, "n");
    lv_obj_clear_flag(cui_sidebarNozzleButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarNozzleButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_sidebarNozzleButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_sidebarSettingsButton;
    cui_sidebarSettingsButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarSettingsButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarSettingsButton, 1);
    lv_obj_set_x(cui_sidebarSettingsButton, 386);
    lv_obj_set_y(cui_sidebarSettingsButton, 178);
    lv_obj_set_flex_flow(cui_sidebarSettingsButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarSettingsButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarSettingsButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarSettingsButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarSettingsButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarSettingsButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarSettingsButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarSettingsButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarSettingsButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarSettingsButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarSettingsButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarSettingsButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarSettingsButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarSettingsButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarSettingsButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarSettingsButtonIcon;
    cui_sidebarSettingsButtonIcon = lv_label_create(cui_sidebarSettingsButton);
    lv_obj_set_width(cui_sidebarSettingsButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarSettingsButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarSettingsButtonIcon, "d");
    lv_obj_clear_flag(cui_sidebarSettingsButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarSettingsButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_sidebarSettingsButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);


    //
    //
    lv_obj_t *cui_sidebarBrowserButton;
    cui_sidebarBrowserButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarBrowserButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarBrowserButton, 1);
    lv_obj_set_x(cui_sidebarBrowserButton, 386);
    lv_obj_set_y(cui_sidebarBrowserButton, 178);
    lv_obj_set_flex_flow(cui_sidebarBrowserButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarBrowserButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarBrowserButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarBrowserButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarBrowserButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarBrowserButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarBrowserButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarBrowserButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarBrowserButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarBrowserButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarBrowserButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarBrowserButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarBrowserButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarBrowserButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarBrowserButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarBrowserButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarBrowserButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarBrowserButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarBrowserButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarBrowserButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);


    lv_obj_t *cui_sidebarBrowserButtonIcon;
    cui_sidebarBrowserButtonIcon = lv_label_create(cui_sidebarBrowserButton);
    lv_obj_set_width(cui_sidebarBrowserButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarBrowserButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarBrowserButtonIcon, LV_SYMBOL_SD_CARD);
    lv_obj_clear_flag(cui_sidebarBrowserButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarBrowserButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_sidebarBrowserButtonIcon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(cui_sidebarBrowserButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(cui_sidebarHomeButton, onSidebarComponentButtonClicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cui_sidebarTempButton, onSidebarComponentButtonClicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cui_sidebarControlButton, onSidebarComponentButtonClicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cui_sidebarNozzleButton, onSidebarComponentButtonClicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cui_sidebarSettingsButton, onSidebarComponentButtonClicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cui_sidebarBrowserButton, onSidebarComponentButtonClicked, LV_EVENT_CLICKED, NULL);

    ui_comp_sidebarComponent_create_hook(cui_sidebarComponent);
    return cui_sidebarComponent;
}
