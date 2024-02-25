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
public:
    FTPSCallback() {
        _tile = NULL;
        _popup = NULL;
    }

    void setTilePopup(lv_obj_t *tile, lv_obj_t *popup) {
        _tile = tile;
        _popup = popup;
    }

    void quit() {
        lv_obj_clean(_popup);
        lv_obj_del(_popup);
        _tile = NULL;
        _popup = NULL;
    }

    std::list<FTPListParser::FilePair*> getFilePair() {
        return _file_pair;
    }

    virtual int16_t onCallback(FTPSWorker::cmd_t cmd, void *param, int size) {
        std::list<FTPListParser::FilePair*> *pair = (std::list<FTPListParser::FilePair*>*)param;

        switch (cmd) {
            case FTPSWorker::CMD_SYNC_DONE:
                if (size > 0)
                    deepCopy(*pair);
                if (xTouchConfig.currentScreenIndex == SCREEN_BROWSER) {
                    if (_tile)
                        lv_event_send(_tile, LV_EVENT_REFRESH, NULL);
                    if (_popup)
                        lv_obj_add_flag(_popup, LV_OBJ_FLAG_HIDDEN);
                }
                break;

            case FTPSWorker::CMD_DOWNLOAD_START:
                _download_cnt = size;
                _download_idx = 0;
                lv_bar_set_range(getPopupProgress(), 0, size);
                lv_label_set_text(getPopupTitle(), (char*)"Downloading");
                lv_obj_clear_flag(getPopupProgress(), LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(getPopupFileName(), LV_OBJ_FLAG_HIDDEN);
                break;

            case FTPSWorker::CMD_DOWNLOADING:
                lv_bar_set_value(getPopupProgress(), ++_download_idx, LV_ANIM_OFF);
                lv_label_set_text(getPopupFileName(), (char*)param);
                LOGD("downloading : %s\n", (char*)param);
                break;
        }
        return 0;
    }

private:
    void deepCopy(std::list<FTPListParser::FilePair*> pair) {
        for (FTPListParser::FilePair* p : _file_pair)
            delete p;
        _file_pair.clear();

        for (FTPListParser::FilePair* p : pair)
            _file_pair.push_back(new FTPListParser::FilePair(p));
    }

    int _download_cnt;
    int _download_idx;
    lv_obj_t *_tile;
    lv_obj_t *_popup;
    std::list<FTPListParser::FilePair*> _file_pair;
};

/*
*****************************************************************************************
* VARIABLES
*****************************************************************************************
*/
static lv_obj_t *_tile_view = NULL;
static int       _last_tile = 0;
static FTPSCallback *_callback = new FTPSCallback();


/*
*****************************************************************************************
* EVENT FUNCTIONS
*****************************************************************************************
*/
void onDeleteFileConfirm(void *user_data) {
    FTPListParser::FilePair *info = (FTPListParser::FilePair *)user_data;
    LOGI("DELETED : %s\n", (char*)info->a.name.c_str());

    info->invalid();
    lv_event_send(_tile_view, LV_EVENT_REFRESH, &_last_tile);
    getFTPSWorker()->removeFile(info);
}

void onPrintConfirm(void *user_data) {
    FTPListParser::FilePair *info = (FTPListParser::FilePair *)user_data;
    LOGI("PRINTING : %s\n", (char*)info->a.name.c_str());
}

void onEventItem(lv_event_t *e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    FTPListParser::FilePair *info = (FTPListParser::FilePair *)lv_event_get_user_data(e);
    static bool is_long = false;

    switch (event_code) {
        case LV_EVENT_CLICKED:
            if (!is_long && info != NULL) {
                LOGI("PRINT? : %s\n", (char*)info->a.name.c_str());
                ui_confirmPanel_show(LV_SYMBOL_WARNING " Print ?", onPrintConfirm, info);
            }
            is_long = false;
            break;

        case LV_EVENT_LONG_PRESSED:
            is_long = true;
            break;

        case LV_EVENT_RELEASED:
            if (is_long && info != NULL) {
                LOGI("DELETE? : %s\n", (char*)info->a.name.c_str());
                _last_tile = lv_obj_get_index(lv_obj_get_parent(target));
                ui_confirmPanel_show(LV_SYMBOL_WARNING " Delete ?", onDeleteFileConfirm, info);
            }
            break;

        case LV_EVENT_SCROLL_BEGIN:
        case LV_EVENT_SCROLL_END:
        case LV_EVENT_SCROLL:
        case LV_EVENT_GESTURE:
            is_long = false;
            break;
    }
}

void onEventBrowserDeleted(lv_event_t *e) {
    LOGD("browser is deleted...\n");
    if (_callback)
        _callback->quit();
}

void onEventTileViewRefresh(lv_event_t *e) {
    // lv_event_code_t event_code = lv_event_get_code(e);
    // lv_obj_t *target = lv_event_get_target(e);
    int *p = (int*)lv_event_get_user_data(e);
    rebuildTiles((p == NULL) ? 0 : *p);
}


/*
*****************************************************************************************
* FUNCTIONS
*****************************************************************************************
*/
void addPNGToTile(lv_obj_t *tile, char *dir, FTPListParser::FilePair *info) {
    char pathPNG[100];

    lv_obj_t *ui_containerItem = lv_obj_create(tile);
    lv_obj_remove_style_all(ui_containerItem);
    lv_obj_set_width(ui_containerItem, 160);
    lv_obj_set_height(ui_containerItem, 135);
    lv_obj_set_align(ui_containerItem, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_containerItem, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_containerItem, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_containerItem, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_containerItem, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_containerItem, onEventItem, LV_EVENT_ALL, info);

    lv_obj_t *ui_png = lv_img_create(ui_containerItem);
    lv_obj_set_width(ui_png, 128);
    lv_obj_set_height(ui_png, 128);
    lv_obj_set_align(ui_png, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_png, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_png, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_SNAPPABLE |
                      LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);     /// Flags
    strcpy(pathPNG, dir);
    strcat(pathPNG, info->b.name.c_str());
    lv_img_set_src(ui_png, pathPNG);

    lv_obj_t *ui_labelFileName = lv_label_create(ui_containerItem);
    lv_obj_set_width(ui_labelFileName, lv_pct(100));
    lv_obj_set_height(ui_labelFileName, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_labelFileName, LV_ALIGN_BOTTOM_MID);
    lv_obj_clear_flag(ui_labelFileName, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_set_style_text_color(ui_labelFileName, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_labelFileName, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_labelFileName, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    String title = info->a.name;
    title.replace(".gcode.3mf", "");
    lv_label_set_text(ui_labelFileName, title.c_str());
}

lv_obj_t *addTile(lv_obj_t *parent, int row) {
    lv_obj_t *tile = lv_tileview_add_tile(parent, 0, row, LV_DIR_VER);
    lv_obj_set_width(tile, lv_pct(100));
    lv_obj_set_height(tile, lv_pct(100));
    lv_obj_set_align(tile, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLL_ELASTIC);      /// Flags
    lv_obj_set_style_bg_color(tile, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(tile, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(tile, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(tile, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(tile, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(tile, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(tile, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(tile, 30, LV_PART_MAIN | LV_STATE_DEFAULT);

    return tile;
}

void rebuildTiles(int move_to) {
    lv_obj_t *tile;
    int cnt = 0;
    const int imgs_per_tile = 6;

    LOGD("refresh tileview : %d\n", lv_obj_get_child_cnt(_tile_view));
    for (int i = 0; i < lv_obj_get_child_cnt(_tile_view); i++) {
        lv_obj_t *child = lv_obj_get_child(_tile_view, i);
        lv_obj_clean(child);
        lv_obj_del(child);
    }

    for (FTPListParser::FilePair* p:_callback->getFilePair()) {
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
        LOGI("move to %d / %d\n", move_to, max_tile_id);
        lv_obj_set_tile_id(_tile_view, 0, move_to, LV_ANIM_OFF);
    }
    LOGD("refresh done, tiles:%d\n", lv_obj_get_child_cnt(_tile_view));
}

lv_obj_t *ui_browserComponent_create(lv_obj_t *comp_parent) {
    lv_obj_t *ui_browserComponent = lv_obj_create(comp_parent);
    lv_obj_remove_style_all(ui_browserComponent);
    lv_obj_set_width(ui_browserComponent, lv_pct(85));
    lv_obj_set_height(ui_browserComponent, lv_pct(100));
    lv_obj_set_align(ui_browserComponent, LV_ALIGN_RIGHT_MID);
    lv_obj_clear_flag(ui_browserComponent, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_browserComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_browserComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_pad_left(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_browserComponent, onEventBrowserDeleted, LV_EVENT_DELETE, NULL);

    lv_obj_t *ui_sd_browser = lv_label_create(ui_browserComponent);
    lv_obj_set_height(ui_sd_browser, lv_pct(7));
    lv_obj_set_width(ui_sd_browser, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align(ui_sd_browser, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_sd_browser, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_sd_browser, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_label_set_text(ui_sd_browser, "REMOTE SD CARD BROWSER");
    lv_obj_set_style_text_color(ui_sd_browser, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_sd_browser, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sd_browser, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_sd_browser, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_sd_browser, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_sd_browser, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_sd_browser, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    _tile_view = lv_tileview_create(ui_browserComponent);
    lv_obj_set_width(_tile_view, lv_pct(100));
    lv_obj_set_height(_tile_view, lv_pct(93));
    lv_obj_set_align(_tile_view, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_clear_flag(_tile_view, LV_OBJ_FLAG_SCROLL_ELASTIC);      /// Flags
    lv_obj_set_style_bg_color(_tile_view, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(_tile_view, onEventTileViewRefresh, LV_EVENT_REFRESH, NULL);

    lv_obj_t *popup = ui_download_popupscreen_init(lv_layer_top());
    _callback->setTilePopup(_tile_view, popup);

    getFTPSWorker()->setCallback(_callback);
    getFTPSWorker()->startSync();
    lv_event_send(_tile_view, LV_EVENT_REFRESH, NULL);

    return ui_browserComponent;
}
