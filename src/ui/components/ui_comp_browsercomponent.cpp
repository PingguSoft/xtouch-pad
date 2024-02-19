#include <list>
#include "../ui.h"
#include "../../xtouch/debug.h"
#include "../../xtouch/ftps_worker.h"

typedef struct {
    char   *pngName;
    char   *modelName;
} file_info_t;


static FTPSWorker *_ftps = NULL;
static lv_obj_t *_cui_browserComponent = NULL;
static lv_obj_t *_tile_view = NULL;
static int       _last_tile = 0;
static std::list <file_info_t*> _file_list;

void rebuild_tiles(int move_to);

void free_node(file_info_t *info) {
    if (info->pngName)
        lv_mem_free(info->pngName);
    if (info->modelName)
        lv_mem_free(info->modelName);
}

void onDeleteFileConfirm(void *user_data) {
    file_info_t *info = (file_info_t *)user_data;
    LOGI("DELETED : %s\n", (char*)info->pngName);

    free_node(info);
    _file_list.remove(info);
    rebuild_tiles(_last_tile);
}

void onPrintConfirm(void *user_data) {
    file_info_t *info = (file_info_t *)user_data;
    LOGI("PRINTING : %s\n", (char*)info->pngName);
}

void ui_event_comp_png(lv_event_t *e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    file_info_t *info = (file_info_t *)lv_event_get_user_data(e);
    static bool is_long = false;

    switch (event_code) {
        case LV_EVENT_CLICKED:
            if (!is_long && info != NULL) {
                LOGI("PRINT? : %s\n", (char*)info->pngName);
                ui_confirmPanel_show(LV_SYMBOL_WARNING " Print ?", onPrintConfirm, info);
            }
            is_long = false;
            break;

        case LV_EVENT_LONG_PRESSED:
            is_long = true;
            break;

        case LV_EVENT_RELEASED:
            if (is_long && info != NULL) {
                LOGI("DELETE? : %s\n", (char*)info->pngName);
                _last_tile = lv_obj_get_index(lv_obj_get_parent(target));
                ui_confirmPanel_show(LV_SYMBOL_WARNING " Delete ?", onDeleteFileConfirm, info);
            }
            break;
    }
}

void add_file(lv_obj_t *tile, file_info_t *info) {
    lv_obj_t *cui_png = lv_img_create(tile);
    lv_obj_set_width(cui_png, 128);
    lv_obj_set_height(cui_png, 128);
    lv_img_set_src(cui_png, info->pngName);
    lv_obj_set_align(cui_png, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(cui_png, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(cui_png, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_border_color(cui_png, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_width(cui_png, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_pad(cui_png, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(cui_png, ui_event_comp_png, LV_EVENT_ALL, info);

    lv_obj_t *cui_labelFileName = lv_label_create(cui_png);
    lv_obj_set_width(cui_labelFileName, 128);
    lv_obj_set_height(cui_labelFileName, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_align(cui_labelFileName, LV_ALIGN_BOTTOM_MID);
    lv_label_set_long_mode(cui_labelFileName, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_clear_flag(cui_labelFileName, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
            LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_style_text_color(cui_labelFileName, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_labelFileName, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    char *fn = strrchr(info->pngName, '/');
    if (fn) {
        fn++;
        lv_label_set_text(cui_labelFileName, fn);
    }
}

lv_obj_t *add_tile(lv_obj_t *parent, int row) {
    lv_obj_t *tile = lv_tileview_add_tile(parent, 0, row, LV_DIR_VER);
    lv_obj_set_align(tile, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    return tile;
}

void rebuild_tiles(int move_to) {
    lv_obj_t *tile;
    int cnt = 0;
    const int imgs_per_tile = 6;

    if (_tile_view != NULL) {
        lv_obj_clean(_tile_view);
        lv_obj_del(_tile_view);
        _tile_view = NULL;
    }
    _tile_view = lv_tileview_create(_cui_browserComponent);
    lv_obj_set_width(_tile_view, lv_pct(100));
    lv_obj_set_height(_tile_view, lv_pct(93));
    lv_obj_set_align(_tile_view, LV_ALIGN_BOTTOM_MID);
    lv_obj_clear_flag(_tile_view, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_style_bg_color(_tile_view, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    for (file_info_t *info : _file_list) {
        if (cnt % imgs_per_tile == 0) {
            tile = add_tile(_tile_view, cnt / imgs_per_tile);
        }
        cnt++;
        add_file(tile, info);
    }

    if (move_to > 0) {
        int cnt = lv_obj_get_child_cnt(_tile_view);
        int max_tile_id = (cnt > 0) ? (cnt - 1) : 0;

        move_to = min(move_to, max_tile_id);
        lv_obj_set_tile_id(_tile_view, 0, move_to, LV_ANIM_OFF);
    }
}

void build_file_list(char *path) {
    lv_fs_dir_t dir;
    lv_fs_res_t res;

    for (file_info_t *info : _file_list) {
        free_node(info);
    }
    _file_list.clear();
    res = lv_fs_dir_open(&dir, path);
    if (res != LV_FS_RES_OK) {
        LOGE("Failed to open dir !!!\n");
        return;
    }

    char fn[256];

    while (1) {
        res = lv_fs_dir_read(&dir, fn);
        if (res != LV_FS_RES_OK) {
            LOGE("Failed to read dir !!!\n");
            break;
        }
        /*fn is empty, if not more files to read*/
        int len = strlen(fn);
        if (len == 0) {
            break;
        } else if (len > 4) {
            char *ext = &fn[len - 4];
            LOGV("%s\n", fn);
            if (!strcasecmp(ext, ".png")) {
                file_info_t *info = (file_info_t *)lv_mem_alloc(sizeof(file_info_t));
                lv_memset_00(info, sizeof(file_info_t));
                info->pngName = (char*)lv_mem_alloc(strlen(path) + strlen(fn) + 2);
                strcpy(info->pngName, path);
                strcat(info->pngName, "/");
                strcat(info->pngName, fn);
                _file_list.push_back(info);
            }
        }
    }
    lv_fs_dir_close(&dir);
}

lv_obj_t *ui_browserComponent_create(lv_obj_t *comp_parent) {
    _cui_browserComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(_cui_browserComponent, lv_pct(85));
    lv_obj_set_height(_cui_browserComponent, lv_pct(100));
    lv_obj_clear_flag(_cui_browserComponent, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    lv_obj_set_style_bg_color(_cui_browserComponent, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_cui_browserComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(_cui_browserComponent, LV_ALIGN_RIGHT_MID);

    lv_obj_t *cui_sd_browser = lv_label_create(_cui_browserComponent);
    lv_obj_set_width(cui_sd_browser, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(cui_sd_browser, lv_pct(7));
    lv_obj_set_align(cui_sd_browser, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(cui_sd_browser, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_sd_browser, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_label_set_text(cui_sd_browser, "SD Card Browser");
    lv_obj_set_style_text_color(cui_sd_browser, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sd_browser, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_sd_browser, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sd_browser, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sd_browser, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sd_browser, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sd_browser, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    _tile_view = NULL;

    if (_ftps == NULL) {
        // ESP32_FTPSClient ftps((char*)"192.168.0.159", 990, (char*)"bblp", (char*)"34801960", 10000, 2);
        _ftps = new FTPSWorker((char*)xTouchConfig.xTouchIP, 990, (char*)"bblp", (char*)xTouchConfig.xTouchAccessCode);
    }
    _ftps->startSync();

    build_file_list("S:/ftps/image");
    rebuild_tiles(0);

    return _cui_browserComponent;
}
