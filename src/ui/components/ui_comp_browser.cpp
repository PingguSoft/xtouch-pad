#include <list>
#include "ui_comp_browser.h"
#include "../../xtouch/debug.h"
#include "../../xtouch/ftps_worker.h"


/*
*****************************************************************************************
* FUNCTION DEFINITION
*****************************************************************************************
*/
extern FTPSWorker *getFTPSWorker();
void rebuildTiles(int move_to);

/*
*****************************************************************************************
* CLASS
*****************************************************************************************
*/
class FTPSCallback : public FTPSWorker::Callback {
    virtual int16_t onCallback(FTPSWorker::cmd_t cmd, void *pParam, uint16_t size) {
        switch (cmd) {
            case FTPSWorker::CMD_SYNC:
                if (xTouchConfig.currentScreenIndex == SCREEN_BROWSER)
                    rebuildTiles(0);
                break;

            case FTPSWorker::CMD_DOWNLOADING:
                LOGD("downloading : %s\n", (char*)pParam);
                break;
        }
        return 0;
    }
};

/*
*****************************************************************************************
* VARIABLES
*****************************************************************************************
*/
static lv_obj_t *_cui_browserComponent = NULL;
static lv_obj_t *_tile_view = NULL;
static int       _last_tile = 0;
static FTPSCallback *_callback = new FTPSCallback();


/*
*****************************************************************************************
* FUNCTIONS
*****************************************************************************************
*/
void onDeleteFileConfirm(void *user_data) {
    FTPListParser::FilePair *info = (FTPListParser::FilePair *)user_data;
    LOGI("DELETED : %s\n", (char*)info->a->name.c_str());

    info->invalid();
    rebuildTiles(_last_tile);
}

void onPrintConfirm(void *user_data) {
    FTPListParser::FilePair *info = (FTPListParser::FilePair *)user_data;
    LOGI("PRINTING : %s\n", (char*)info->a->name.c_str());
}

void onEventPNG(lv_event_t *e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    FTPListParser::FilePair *info = (FTPListParser::FilePair *)lv_event_get_user_data(e);
    static bool is_long = false;

    switch (event_code) {
        case LV_EVENT_CLICKED:
            if (!is_long && info != NULL) {
                LOGI("PRINT? : %s\n", (char*)info->a->name.c_str());
                ui_confirmPanel_show(LV_SYMBOL_WARNING " Print ?", onPrintConfirm, info);
            }
            is_long = false;
            break;

        case LV_EVENT_LONG_PRESSED:
            is_long = true;
            break;

        case LV_EVENT_RELEASED:
            if (is_long && info != NULL) {
                LOGI("DELETE? : %s\n", (char*)info->a->name.c_str());
                _last_tile = lv_obj_get_index(lv_obj_get_parent(target));
                ui_confirmPanel_show(LV_SYMBOL_WARNING " Delete ?", onDeleteFileConfirm, info);
            }
            break;
    }
}

void addPNGToTile(lv_obj_t *tile, char *dir, FTPListParser::FilePair *info) {
    char pathPNG[40];

    lv_obj_t *cui_png = lv_img_create(tile);
    lv_obj_set_width(cui_png, 128);
    lv_obj_set_height(cui_png, 128);

    strcpy(pathPNG, dir);
    strcat(pathPNG, info->b->name.c_str());
    lv_img_set_src(cui_png, pathPNG);
    lv_obj_set_align(cui_png, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(cui_png, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(cui_png, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    // lv_obj_set_style_border_color(cui_png, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_width(cui_png, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_pad(cui_png, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(cui_png, onEventPNG, LV_EVENT_ALL, info);

    lv_obj_t *cui_labelFileName = lv_label_create(cui_png);
    lv_obj_set_width(cui_labelFileName, 128);
    lv_obj_set_height(cui_labelFileName, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_align(cui_labelFileName, LV_ALIGN_BOTTOM_MID);
    lv_label_set_long_mode(cui_labelFileName, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_clear_flag(cui_labelFileName, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
            LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_style_text_color(cui_labelFileName, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_labelFileName, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_labelFileName, info->a->name.c_str());
}

lv_obj_t *addTile(lv_obj_t *parent, int row) {
    lv_obj_t *tile = lv_tileview_add_tile(parent, 0, row, LV_DIR_VER);
    lv_obj_set_align(tile, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    return tile;
}

void rebuildTiles(int move_to) {
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

    std::vector<FTPListParser::FilePair*> pair = getFTPSWorker()->getModelImagePair();
    for (FTPListParser::FilePair* p:pair) {
        if (p->isValid()) {
            if (cnt % imgs_per_tile == 0) {
                tile = addTile(_tile_view, cnt / imgs_per_tile);
            }
            cnt++;
            char *dir = getFTPSWorker()->getImagePath(true);
            addPNGToTile(tile, dir, p);
        }
    }
    if (move_to > 0) {
        int cnt = lv_obj_get_child_cnt(_tile_view);
        int max_tile_id = (cnt > 0) ? (cnt - 1) : 0;

        move_to = min(move_to, max_tile_id);
        lv_obj_set_tile_id(_tile_view, 0, move_to, LV_ANIM_OFF);
    }
}

void onEventBrowserDeleted(lv_event_t *e) {
    LOGD("browser is deleted...\n");
    getFTPSWorker()->invalidate();
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
    lv_obj_add_event_cb(_cui_browserComponent, onEventBrowserDeleted, LV_EVENT_DELETE, NULL);
    rebuildTiles(0);
    getFTPSWorker()->setCallback(_callback);

    return _cui_browserComponent;
}
