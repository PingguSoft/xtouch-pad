#include "../ui.h"

void lv_example_image_1(lv_obj_t* parent) {
    LV_IMG_DECLARE(img_cogwheel_argb);
    lv_obj_t* img1 = lv_img_create(parent);
    lv_obj_align(img1, LV_ALIGN_TOP_MID, 0, 0);
    lv_img_set_src(img1, &img_cogwheel_argb);

    lv_obj_t* img2 = lv_img_create(parent);
    lv_img_set_src(img2, LV_SYMBOL_OK "Accept");
    lv_obj_align_to(img2, img1, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    lv_obj_t* img3 = lv_img_create(parent);
    lv_img_set_src(img3, "S:/1.png");
    lv_obj_align_to(img3, img2, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    // lv_img_set_zoom(img1, 1);
    // lv_img_set_rotation(img1, 10);
}

void ui_browserScreen_screen_init() {
    ui_browserScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_browserScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM); /// Flags
    lv_obj_set_scrollbar_mode(ui_browserScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_browserScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_browserScreen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_browserScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_browserScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_browserScreen, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_browserScreen, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_browserScreen, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_browserScreen, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_browserScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_browserScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_example_image_1(ui_browserScreen);
}
