#include <list>
#include "ui_comp_browser.h"
#include "debug.h"
#include "clients/ftps_worker.h"
#include "main.h"

/*
*****************************************************************************************
* FUNCTION DEFINITION
*****************************************************************************************
*/
void addTextItem2ScrollView(lv_obj_t *scroll_view, char *dir, FTPListParser::FilePair *info);
void addImageItem2ScrollView(lv_obj_t *scroll_view, char *dir, FTPListParser::FilePair *info);

/*
*****************************************************************************************
* CLASS
*****************************************************************************************
*/
class FTPSCallback : public FTPSWorker::Callback {
public:
    FTPSCallback() {
        _scroll_view = NULL;
        _popup = NULL;
    }

    void setTilePopup(lv_obj_t *tile, lv_obj_t *popup) {
        _scroll_view = tile;
        _popup = popup;
    }

    void quit() {
        lv_obj_clean(_popup);
        lv_obj_del(_popup);
        _scroll_view = NULL;
        _popup = NULL;
    }

    std::list<FTPListParser::FilePair*> getFilePair() {
        return _file_pair;
    }

    virtual int16_t onCallback(FTPSWorker::cmd_t cmd, void *param, int size) {
        std::list<FTPListParser::FilePair*> *pair = (std::list<FTPListParser::FilePair*>*)param;

        lv_lock();
        switch (cmd) {
            case FTPSWorker::CMD_SYNC_DONE:
                if (size > 0)
                    deepCopy(*pair);
                if (xTouchConfig.currentScreenIndex == SCREEN_BROWSER) {
                    if (_popup)
                        lv_obj_add_flag(_popup, LV_OBJ_FLAG_HIDDEN);
                    if (_scroll_view)
                        lv_event_send(_scroll_view, LV_EVENT_REFRESH, NULL);
                }
                break;

            case FTPSWorker::CMD_DOWNLOAD_START:
                _download_cnt = size;
                _download_idx = 0;
                lv_obj_add_flag(getPopupSpinner(), LV_OBJ_FLAG_HIDDEN);
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
        lv_unlock();

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
    lv_obj_t *_scroll_view;
    lv_obj_t *_popup;
    std::list<FTPListParser::FilePair*> _file_pair;
};

/*
*****************************************************************************************
* VARIABLES
*****************************************************************************************
*/
static lv_obj_t     *_scroll_view = NULL;
static int           _sel_index = 0;
static FTPSCallback *_callback = new FTPSCallback();
static FTPSWorker   *_ftps = NULL;
static bool          _is_text = true;
/*
*****************************************************************************************
* EVENT FUNCTIONS
*****************************************************************************************
*/
void onDeleteFileConfirm(void *user_data) {
    FTPListParser::FilePair *info = (FTPListParser::FilePair *)user_data;
    LOGI("DELETED : %s\n", (char*)info->a.name.c_str());

    info->invalid();
    lv_event_send(_scroll_view, LV_EVENT_REFRESH, (void*)_sel_index);
    _ftps->removeFile(info);
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
                _sel_index = lv_obj_get_index(target);
                LOGI("DELETE? : %s %d\n", (char*)info->a.name.c_str(), _sel_index);
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

void onEventScrollViewRefresh(lv_event_t *e) {
    int move_to = (int)lv_event_get_param(e);

    LOGV("refresh children : %d\n", lv_obj_get_child_cnt(_scroll_view));
    lv_obj_clean(_scroll_view);

    char *dir = _ftps->getImagePath(true);
    for (FTPListParser::FilePair* p:_callback->getFilePair()) {
        if (p->isValid()) {
            if (_is_text)
                addTextItem2ScrollView(_scroll_view, dir, p);
            else
                addImageItem2ScrollView(_scroll_view, dir, p);
        }
    }

    if (move_to > 0) {
        int cnt = lv_obj_get_child_cnt(_scroll_view);
        int max_tile_id = (cnt > 0) ? (cnt - 1) : 0;

        move_to = min(move_to, max_tile_id);
        LOGV("move to %d / %d\n", move_to, max_tile_id);

        lv_obj_t *here = lv_obj_get_child(_scroll_view, move_to);
        lv_obj_scroll_to_view(here, LV_ANIM_OFF);
    }
    LOGV("refresh done, children:%d\n", lv_obj_get_child_cnt(_scroll_view));
}


/*
*****************************************************************************************
* Image Scroll View
*****************************************************************************************
*/
void addImageItem2ScrollView(lv_obj_t *scroll_view, char *dir, FTPListParser::FilePair *info) {
    char pathPNG[100];

    lv_obj_t *ui_containerItem = lv_obj_create(scroll_view);
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

lv_obj_t *ui_browserComponent_create_image(lv_obj_t *comp_parent) {
    lv_obj_t *ui_browserComponent = lv_obj_create(comp_parent);
    lv_obj_remove_style_all(ui_browserComponent);
    lv_obj_set_width(ui_browserComponent, lv_pct(85));
    lv_obj_set_height(ui_browserComponent, lv_pct(100));
    lv_obj_set_align(ui_browserComponent, LV_ALIGN_RIGHT_MID);
    lv_obj_set_flex_flow(ui_browserComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_browserComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_browserComponent, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_browserComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_browserComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_browserComponent, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_browserComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_browserComponent, onEventBrowserDeleted, LV_EVENT_DELETE, NULL);

    lv_obj_t *ui_sd_browser_title = lv_label_create(ui_browserComponent);
    lv_obj_set_height(ui_sd_browser_title, LV_SIZE_CONTENT);
    lv_obj_set_width(ui_sd_browser_title, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align(ui_sd_browser_title, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_sd_browser_title, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_sd_browser_title, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_label_set_text(ui_sd_browser_title, "REMOTE SD CARD BROWSER");
    lv_obj_set_style_text_color(ui_sd_browser_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_sd_browser_title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sd_browser_title, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_sd_browser_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_sd_browser_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_sd_browser_title, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_sd_browser_title, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    _scroll_view = lv_obj_create(ui_browserComponent);
    lv_obj_set_width(_scroll_view, lv_pct(100));
    lv_obj_set_height(_scroll_view, lv_pct(91));
    lv_obj_set_align(_scroll_view, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_set_flex_flow(_scroll_view, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(_scroll_view, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(_scroll_view, LV_OBJ_FLAG_SCROLL_ELASTIC);      /// Flags
    lv_obj_set_style_bg_color(_scroll_view, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_scroll_view, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(_scroll_view, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_scroll_view, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(_scroll_view, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(_scroll_view, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_scroll_view, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_scroll_view, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_scroll_view, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(_scroll_view, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(_scroll_view, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(_scroll_view, onEventScrollViewRefresh, LV_EVENT_REFRESH, NULL);

    lv_obj_t *popup = ui_download_popupscreen_init(lv_layer_top());
    _callback->setTilePopup(_scroll_view, popup);

    if (_ftps == NULL) {
        // ESP32_FTPSClient ftps((char*)"192.168.0.159", 990, (char*)"bblp", (char*)"34801960", 10000, 2);
        _ftps = new FTPSWorker((char*)xTouchConfig.xTouchIP, 990, (char*)"bblp", (char*)xTouchConfig.xTouchAccessCode);
    }
    _ftps->setCallback(_callback);
    _ftps->startSync(_is_text);
    lv_event_send(_scroll_view, LV_EVENT_REFRESH, NULL);

    return ui_browserComponent;
}

/*
*****************************************************************************************
* Text Scroll View
*****************************************************************************************
*/
void parseTS(long ts, int *d) {
    long div;
    int tbl[] = { 60, 24, 31, 365 };

    // 2024-02-12 22:45
    // long ts  = min + (hour * 60L) + (day * 60 * 24L) + (mon * 60 * 24 * 31L) + (year * 60 * 24 * 31 * 365L);
    for (int i = 4; i >= 0; i--) {
        div = 1;
        for (int j = i - 1; j >= 0; j--) {
            div *= tbl[j];
        }
        d[i] = ts / div;
        ts -= (d[i] * div);
    }
}

char *parseSize(long size, char *buf, int len) {
    float sz;
    char  *fmt;

    if (size > (1024 * 1024L)) {
        sz = size / (1024 * 1024.0);
        fmt = (char*)"%.1fMB";
    } else if (size > 1024) {
        sz = size / 1024.0;
        fmt = (char*)"%.1fKB";
    } else {
        sz = size;
        fmt = (char*)"%.0f";
    }
    snprintf(buf, len, fmt, sz);

    return buf;
}

void addTextItem2ScrollView(lv_obj_t *scroll_view, char *dir, FTPListParser::FilePair *info) {
    lv_obj_t *ui_container_item_list = lv_obj_create(scroll_view);
    lv_obj_set_width(ui_container_item_list, lv_pct(100));
    lv_obj_set_height(ui_container_item_list, lv_pct(12));
    lv_obj_set_flex_flow(ui_container_item_list, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_container_item_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_container_item_list, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);      /// Flags
    lv_obj_set_style_radius(ui_container_item_list, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_container_item_list, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_container_item_list, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_container_item_list, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_container_item_list, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_container_item_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_container_item_list, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_container_item_list, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_container_item_list, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_container_item_list, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_container_item_list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_container_item_list, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_container_item_list, onEventItem, LV_EVENT_ALL, info);

    lv_obj_t *ui_name = lv_label_create(ui_container_item_list);
    lv_obj_set_width(ui_name, lv_pct(50));
    lv_obj_set_height(ui_name, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_name, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_name, LV_LABEL_LONG_SCROLL_CIRCULAR);
    String title = info->a.name;
    title.replace(".gcode.3mf", "");
    lv_label_set_text(ui_name, title.c_str());
    lv_obj_clear_flag(ui_name, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);      /// Flags
    lv_obj_set_style_text_color(ui_name, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    int  d[5];
    char buf[40];
    lv_obj_t *ui_date = lv_label_create(ui_container_item_list);
    lv_obj_set_width(ui_date, lv_pct(30));
    lv_obj_set_height(ui_date, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_date, LV_ALIGN_CENTER);
    parseTS(info->a.ts, d);
    if (d[4] >= 120) {
        snprintf(buf, sizeof(buf), "----/%02d/%02d %02d:%02d", d[3], d[2], d[1], d[0]);
    } else {
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d", 1980 + d[4], d[3], d[2], d[1], d[0]);
    }
    lv_label_set_text(ui_date, buf);
    lv_obj_clear_flag(ui_date, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);      /// Flags
    lv_obj_set_style_text_color(ui_date, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_date, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_size = lv_label_create(ui_container_item_list);
    lv_obj_set_width(ui_size, lv_pct(18));
    lv_obj_set_height(ui_size, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_size, LV_ALIGN_CENTER);
    lv_label_set_long_mode(ui_name, LV_LABEL_LONG_SCROLL_CIRCULAR);
    parseSize(info->a.size, buf, sizeof(buf));
    lv_label_set_text(ui_size, buf);
    lv_obj_add_flag(ui_size, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_EVENT_BUBBLE);     /// Flags
    lv_obj_set_style_text_color(ui_size, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_size, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}

lv_obj_t *ui_browserComponent_create_text(lv_obj_t *comp_parent) {
    lv_obj_t *ui_browserComponent = lv_obj_create(comp_parent);
    lv_obj_remove_style_all(ui_browserComponent);
    lv_obj_set_width(ui_browserComponent, lv_pct(85));
    lv_obj_set_height(ui_browserComponent, lv_pct(100));
    lv_obj_set_align(ui_browserComponent, LV_ALIGN_RIGHT_MID);
    lv_obj_set_flex_flow(ui_browserComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_browserComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_browserComponent, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_browserComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_browserComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_browserComponent, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_browserComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_browserComponent, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_browserComponent, onEventBrowserDeleted, LV_EVENT_DELETE, NULL);

    lv_obj_t *ui_sd_browser_title = lv_label_create(ui_browserComponent);
    lv_obj_set_height(ui_sd_browser_title, LV_SIZE_CONTENT);
    lv_obj_set_width(ui_sd_browser_title, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align(ui_sd_browser_title, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(ui_sd_browser_title, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_sd_browser_title, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_label_set_text(ui_sd_browser_title, "REMOTE SD CARD BROWSER");
    lv_obj_set_style_text_color(ui_sd_browser_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_sd_browser_title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sd_browser_title, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_sd_browser_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_sd_browser_title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_sd_browser_title, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_sd_browser_title, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_container_noname1 = lv_obj_create(ui_browserComponent);
    lv_obj_remove_style_all(ui_container_noname1);
    lv_obj_set_width(ui_container_noname1, lv_pct(100));
    lv_obj_set_height(ui_container_noname1, lv_pct(91));
    lv_obj_set_align(ui_container_noname1, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_container_noname1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_container_noname1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(ui_container_noname1,
                      LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_set_style_pad_row(ui_container_noname1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_container_noname1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_container_header = lv_obj_create(ui_container_noname1);
    lv_obj_remove_style_all(ui_container_header);
    lv_obj_set_width(ui_container_header, lv_pct(100));
    lv_obj_set_height(ui_container_header, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_flex_flow(ui_container_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_container_header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_container_header,
                      LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_set_style_pad_left(ui_container_header, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_container_header, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_container_header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_container_header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_container_header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_container_header, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_header_name = lv_label_create(ui_container_header);
    lv_obj_set_width(ui_header_name, lv_pct(50));
    lv_obj_set_height(ui_header_name, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_header_name, LV_ALIGN_CENTER);
    lv_label_set_text(ui_header_name, "Name");
    lv_obj_clear_flag(ui_header_name, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_set_style_text_color(ui_header_name, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_header_name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_header_date = lv_label_create(ui_container_header);
    lv_obj_set_width(ui_header_date, lv_pct(30));
    lv_obj_set_height(ui_header_date, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_header_date, LV_ALIGN_CENTER);
    lv_label_set_text(ui_header_date, "Date");
    lv_obj_clear_flag(ui_header_date, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_set_style_text_color(ui_header_date, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_header_date, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_header_size = lv_label_create(ui_container_header);
    lv_obj_set_width(ui_header_size, lv_pct(18));
    lv_obj_set_height(ui_header_size, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_header_size, LV_ALIGN_CENTER);
    lv_label_set_text(ui_header_size, "Size");
    lv_obj_clear_flag(ui_header_size, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                      LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC |
                      LV_OBJ_FLAG_SCROLL_MOMENTUM);     /// Flags
    lv_obj_set_style_text_color(ui_header_size, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_header_size, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    _scroll_view = lv_obj_create(ui_container_noname1);
    lv_obj_set_width(_scroll_view, lv_pct(100));
    lv_obj_set_height(_scroll_view, lv_pct(94));
    lv_obj_set_align(_scroll_view, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_set_flex_flow(_scroll_view, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_scroll_view, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(_scroll_view, LV_OBJ_FLAG_SCROLL_ELASTIC);      /// Flags
    lv_obj_set_style_radius(_scroll_view, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(_scroll_view, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_scroll_view, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(_scroll_view, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_scroll_view, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(_scroll_view, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_scroll_view, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_scroll_view, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_scroll_view, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(_scroll_view, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(_scroll_view, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(_scroll_view, onEventScrollViewRefresh, LV_EVENT_REFRESH, NULL);

    lv_obj_t *popup = ui_download_popupscreen_init(lv_layer_top());
    _callback->setTilePopup(_scroll_view, popup);

    if (_ftps == NULL) {
        // ESP32_FTPSClient ftps((char*)"192.168.0.159", 990, (char*)"bblp", (char*)"34801960", 10000, 2);
        _ftps = new FTPSWorker((char*)xTouchConfig.xTouchIP, 990, (char*)"bblp", (char*)xTouchConfig.xTouchAccessCode);
    }
    _ftps->setCallback(_callback);
    _ftps->startSync(_is_text);
    lv_event_send(_scroll_view, LV_EVENT_REFRESH, NULL);

    return ui_browserComponent;
}

lv_obj_t *ui_browserComponent_create(lv_obj_t *comp_parent) {
    _is_text = xTouchConfig.xTouchBrowserText;
    return _is_text ? ui_browserComponent_create_text(comp_parent) : ui_browserComponent_create_image(comp_parent);
}
