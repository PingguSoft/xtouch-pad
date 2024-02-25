#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <algorithm>
#include "debug.h"
#include "ftps_worker.h"

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

/*
*****************************************************************************************
* FTPListParser
*****************************************************************************************
*/
std::vector<String> FTPListParser::_months = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Sep", "Nov", "Dec" };


void FTPListParser::parse(std::list<String*> logs, std::list<FileInfo*> &result, int max, String ext, int sort) {
    std::vector <char*> tokens;
    String fname;
    int cnt = 1;

    for (String *line: logs) {
        char *token = strtok((char*)line->c_str(), " ");
        while (token != NULL) {
            tokens.push_back(token);
            token = strtok(NULL, " ");
        }

        if (tokens.size() > 0) {
            long sz  = atoi(tokens[4]);
            int mon  = std::find(_months.begin(), _months.end(), tokens[5]) - _months.begin() + 1;
            int day  = atoi(tokens[6]);

            int hour = 0;
            int min  = 0;
            if (tokens[7][2] == ':') {
                tokens[7][2] = '\0';
                hour = atoi(tokens[7]);
                min  = atoi(&tokens[7][3]);
            }
            long ts  = min + (hour * 60) + (day * 60 * 24) + (mon * 60 * 24 * 31);
            fname = tokens[8];
            for (int i = 9; i < tokens.size(); i++)
                fname = fname + ' ' + tokens[i];

            if (ext.length() == 0 || fname.endsWith(ext)) {
                result.push_back(new FileInfo(ts, sz, fname));
                // LOGD("%5d, %10lu [%10lu] %20s %d\n", cnt++, ts, sz, fname.c_str(), ESP.getFreeHeap());
            }
            tokens.clear();
        }
    }
    if (sort == SORT_ASC)
        result.sort(FTPListParser::FileInfo::comp_asc);
    else if (sort == SORT_DESC)
        result.sort(FTPListParser::FileInfo::comp_des);

    if (result.size() > max) {
        for (int i = 0; i < result.size() - max; i++) {
            FileInfo* t = result.back();
            result.pop_back();
            delete t;
        }
    }
}

void FTPListParser::matches(std::list <FTPListParser::FileInfo*> infoA, std::list <FTPListParser::FileInfo*> infoB, std::list <FTPListParser::FilePair*> &result, int sort) {
    FTPListParser::FilePair r;
    bool is_found;

    for (FTPListParser::FileInfo *ia : infoA) {
        is_found = false;
        for (FTPListParser::FileInfo *ib : infoB) {
            if (std::abs(ia->ts - ib->ts) < 3) {
                r.set(ia->ts, *ia, *ib);
                is_found = true;
                if (ia->ts == ib->ts)
                    break;
            }
        }
        if (is_found)
            result.push_back(new FilePair(r));
    }
    if (sort == SORT_ASC)
        result.sort(FTPListParser::FilePair::comp_asc);
    else if (sort == SORT_DESC)
        result.sort(FTPListParser::FilePair::comp_des);
}

void FTPListParser::sub(std::list <FTPListParser::FileInfo*> a, std::list <FTPListParser::FileInfo*> b, std::list <FTPListParser::FileInfo*> &result, int by) {
    FTPListParser::FileInfo r;
    bool is_found;

    for (FTPListParser::FileInfo *info_a : a) {
        is_found = false;
        for (FTPListParser::FileInfo *info_b : b) {
            if (by == BY_TS && info_a->ts == info_b->ts) {
                is_found = true;
                break;
            } else if (by == BY_NAME && info_a->name.equals(info_b->name)) {
                is_found = true;
                break;
            }
        }
        if (!is_found)
            result.push_back(new FTPListParser::FileInfo(*info_a));
    }
}

void FTPListParser::exportA(std::list<FilePair*> pair, std::list <FileInfo*> &result) {
    for (FilePair *p : pair) {
        result.push_back(&p->a);
    }
}

void FTPListParser::exportB(std::list<FilePair*> pair, std::list <FileInfo*> &result) {
    for (FilePair *p : pair) {
        result.push_back(&p->b);
    }
}


/*
*****************************************************************************************
* FTPSWorker
*****************************************************************************************
*/
void tokenize(String &line, String token, std::vector<String> &tokens) {
    String tmp(line);
    char *tkn = strtok((char*)tmp.c_str(), token.c_str());

    while (tkn != NULL) {
        tokens.push_back(String(tkn));
        tkn = strtok(NULL, token.c_str());
    }
}

void taskFTPS(void* arg);

FTPSWorker::FTPSWorker(char* serverAdress, uint16_t port, char* userName, char* passWord) {
    _ftps = new ESP32_FTPSClient(serverAdress, port, userName, passWord);
    _queue_comm  = xQueueCreate(5, sizeof(cmd_q_t));
    xTaskCreate(&taskFTPS, "taskFTPS", 8192, this, 8, NULL);
    _is_first = true;

    char bufs[60];
    char drv;
    drv = lv_fs_get_letters(bufs)[0];

    sprintf(bufs, "%C:%s", drv, PATH_IMG);
    _img_path = String(bufs);

    sprintf(bufs, "%C:%s", drv, PATH_MODEL);
    _model_path = String(bufs);

    // ESP32_FTPSClient ftps((char*)"192.168.137.1", 990, (char*)"bblp", (char*)"34801960", 10000, 2);
}

template<typename T> void FTPSWorker::freeList(std::list<T> &list) {
    for (T i : list) {
        if (i)
            delete i;
    }
    list.clear();
}

void FTPSWorker::downloadDirRemote(String srcDir, String dstDir, std::list<FTPListParser::FileInfo*> info, String ext) {
    std::list<String*> list;
    std::vector<String> tokens;
    String src;
    String dst;
    String path;

    tokenize(dstDir, "/", tokens);
    for (int i = 0; i < tokens.size() - 1; i++) {
        path = path + "/" + tokens[i];
        LOGD("%s\n", path.c_str());
        if (!SD.exists(path)) {
            LOGE("Dest Dir doesn't exist : %s\n", path.c_str());
            SD.mkdir(path);
        }
    }

    if (!srcDir.endsWith("/"))
        srcDir += '/';
    if (!dstDir.endsWith("/"))
        dstDir += '/';

    int cnt = 1;
    LOGV("Start downloading...\n");
    for (FTPListParser::FileInfo* i : info) {
        src = srcDir + i->name;
        dst = dstDir + i->name;
        LOGD("%4d, download [%10ld] %s to %s\n", cnt++, i->size, src.c_str(), dst.c_str());
        if (_callback)
            _callback->onCallback(CMD_DOWNLOADING, (char*)i->name.c_str(), 0);

        File file = SD.open(dst.c_str(), "wb", true);
        _ftps->InitFile("Type I");
        _ftps->DownloadFile(src.c_str(), i->size, &file);
        file.close();
    }
    LOGV("Completed...\n");
}

void FTPSWorker::listDirRemote(String srcDir, std::list<FTPListParser::FileInfo*> &info, String ext, int max) {
    FTPListParser parser;
    std::list<String*> list;

    _ftps->InitFile("Type A");
    _ftps->DirLong(srcDir.c_str(), list);
    parser.parse(list, info, max, ext, FTPListParser::SORT_DESC);
    freeList(list);

    int cnt = 1;
    LOGV("list count:%d\n", info.size());
    for (FTPListParser::FileInfo* i : info) {
        LOGV("%4d, %10ld, [%10ld] %s\n", cnt++, i->ts, i->size, i->name.c_str());
    }
}

void FTPSWorker::startSync() {
    cmd_q_t q = { CMD_SYNC, NULL, 0, false };
    xQueueSend(_queue_comm, &q, portMAX_DELAY);
}

void FTPSWorker::removeFile(FTPListParser::FilePair* pair) {
    cmd_q_t q = { CMD_REMOVE_FILE, (uint8_t*)pair, 0, false };
    xQueueSend(_queue_comm, &q, portMAX_DELAY);
}

void FTPSWorker::removeFileRemote(FTPListParser::FilePair* pair) {
    String path;

    _ftps->OpenConnection(false, true);
    path = getModelPath() + pair->a.name;
    LOGD("delete : %s\n", path.c_str());
    _ftps->DeleteFile(path.c_str());

    path = getImagePath() + pair->b.name;
    LOGD("delete : %s\n", path.c_str());
    _ftps->DeleteFile(path.c_str());
    _ftps->CloseConnection();

    path = getImagePath() + pair->b.name;
    LOGD("delete : %s\n\n", path.c_str());
    SD.remove(path);
}

void FTPSWorker::listDirSD(char *path, std::list<FTPListParser::FileInfo*> &info, String ext) {
    FTPListParser::FileInfo *item;
    int  len = strlen(path);
    char *new_path = new char[len + 1];

    strcpy(new_path, path);
    if (new_path[len - 1] == '/')
        new_path[len - 1] = 0;

    File dir = SD.open(new_path);
    while (true) {
        File entry = dir.openNextFile();
        if (!entry)
            break;

        String name = String(entry.name());
        name.trim();
        if (ext.length() == 0 || name.endsWith(ext)) {
            item = new FTPListParser::FileInfo((long)entry.getLastWrite(), entry.size(), entry.name());
            info.push_back(item);
            // LOGD("%20s %10lu\n", entry.name(), entry.size());
        }
        entry.close();
    }
    delete new_path;
}

#if _NO_NETWORK_
std::list<String> FTPSWorker::_testPair = {
    "107925,       4442,      15137327011.png,     838822, galaxy watch stand.gcode.3mf",
    "101717,       4684,      28260101861.png,     667533, 11111111.gcode.3mf",
    " 97888,       4684,      42885271611.png,     667533, rpi3p_case.gcode.3mf",
    " 97874,       2540,       9240571371.png,     810058, plate_holder.gcode.3mf",
    " 94551,       4505,      38297837321.png,    2108337, controller-left.gcode.3mf",
    " 89469,       5285,      14229151451.png,     953086, 1.gcode.3mf",
    " 85813,       4857,      26698210481.png,     970885, swerve-large-gear.gcode.3mf",
    " 85661,       1863,      17930814601.png,     458648, swerve-axle-gearx3.gcode.3mf",
    " 85235,       4081,          2064281.png,    1548586, rc_con_left_bottom.gcode.3mf",
    " 84326,       1182,      19799488481.png,     767304, swerve-gear-motorx6.gcode.3mf",
    " 84322,       2171,       4678907721.png,    1029925, hex_bits_holder_7x3.gcode.3mf",
    " 83511,       2755,      10363259711.png,     710512, hex_bits_holder_7 x 2.gcode.3mf",
    " 82040,       3609,       2613376571.png,     413301, No-Catch Y-Splitter Self-Tap (long slot) (9.4mm-M3x6 mount).gcode.3mf",
    " 81969,       3804,       4380517671.png,     176374, AMS_disconnect_tool_with_magnet_seperate_letters.gcode.3mf"
};
#endif

void FTPSWorker::syncImagesModels() {
    int  cnt;

    std::list<FTPListParser::FileInfo*> modelFilesRemote;
    std::list<FTPListParser::FileInfo*> imageFilesRemote;
    std::list<FTPListParser::FileInfo*> imageFilesSD;
    std::list<FTPListParser::FilePair*> pairList;

    LOGV("--------------- SD CARD ---------------\n");
    listDirSD(getImagePath(), imageFilesSD, ".png");
    cnt = 1;
    for (FTPListParser::FileInfo *p:imageFilesSD) {
        LOGV("%3d %10ld, [%10ld] %s\n", cnt++, p->ts, p->size, p->name.c_str());
    }

#if !_NO_NETWORK_
    _ftps->OpenConnection(false, true);
    listDirRemote(getModelPath(), modelFilesRemote, ".3mf", 30);     // model files
    listDirRemote(getImagePath(), imageFilesRemote, ".png", 30);     // image files

    // make pairList after matching timestamp
    FTPListParser::matches(modelFilesRemote, imageFilesRemote, pairList, FTPListParser::SORT_DESC);
    cnt = 1;
    LOGV("--------------- FILE MODEL & PNG ---------------\n");
    for (FTPListParser::FilePair *p:pairList) {
        LOGV("%3d %10ld, [%10ld] %20s, [%10ld] %s\n", cnt++, p->ts, p->b.size, p->b.name.c_str(), p->a.size, p->a.name.c_str());
    }

    //
    // download files not in SD card
    //
    // get image files from pairList
    std::list<FTPListParser::FileInfo*> imageFilesRemoteFinal;
    _parser.exportB(pairList, imageFilesRemoteFinal);
    LOGV("--------------- imageFilesRemoteFinal ---------------\n");
    for (FTPListParser::FileInfo *p:imageFilesRemoteFinal) {
        LOGV("%3d %10ld, [%10ld] %s\n", cnt++, p->ts, p->size, p->name.c_str());
    }

    // make  TBU(to be updated) files after comparing files in SD card and ftps server lists
    std::list<FTPListParser::FileInfo*> imageFilesDownload;
    _parser.sub(imageFilesRemoteFinal, imageFilesSD, imageFilesDownload, FTPListParser::BY_NAME);
    imageFilesRemoteFinal.clear();

    cnt = 1;
    LOGD("--------------- TBU ---------------\n");
    for (FTPListParser::FileInfo *p:imageFilesDownload) {
        LOGD("%3d %10ld, [%10ld] %s\n", cnt++, p->ts, p->size, p->name.c_str());
    }
    if (imageFilesDownload.size() > 0) {
        if (_callback)
            _callback->onCallback(CMD_DOWNLOAD_START, NULL, imageFilesDownload.size());
        downloadDirRemote(getImagePath(), getImagePath(), imageFilesDownload, ".png");
    }
    cnt = _is_first ? pairList.size() : imageFilesDownload.size();
    _ftps->CloseConnection();

    freeList(imageFilesDownload);
    freeList(imageFilesRemote);
    freeList(modelFilesRemote);
#else
    std::vector<String> tokens;
    for (String s : _testPair) {
        tokenize(s, ",", tokens);
        tokens[2].trim();
        tokens[4].trim();
        // _testpair : ts, <png size> <png name> <3mf size> <3mf name>
        // pair : ts, <3mf> <png>
        pairList.push_back(new FTPListParser::FilePair(tokens[0].toInt(), tokens[3].toInt(), tokens[4], tokens[1].toInt(), tokens[2]));
        tokens.clear();
    }

    LOGI("--------------- FILE MODEL & PNG ---------------\n");
    for (FTPListParser::FilePair *p:pairList) {
        LOGD("%3d %10ld, [%10ld] %20s, [%10ld] %s\n", cnt++, p->ts, p->b.size, p->b.name.c_str(), p->a.size, p->a.name.c_str());
    }
    cnt = _is_first ? _testPair.size() : 0;
#endif
    if (_callback)
        _callback->onCallback(CMD_SYNC_DONE, &pairList, cnt);

    freeList(pairList);
    freeList(imageFilesSD);

    if (_is_first)
        _is_first = false;
}

/*
*****************************************************************************************
* taskFTPS
*****************************************************************************************
*/
void taskFTPS(void* arg) {
    FTPSWorker *pWorker = (FTPSWorker*)arg;
    FTPSWorker::cmd_q_t *q = new FTPSWorker::cmd_q_t;

    LOGD("taskFTPS created !\n");
    while (true) {
        if (xQueueReceive(pWorker->_queue_comm, q, pdMS_TO_TICKS(10)) == pdTRUE) {
            switch (q->cmd) {
                case FTPSWorker::CMD_SYNC:
                    pWorker->syncImagesModels();
                    break;

                case FTPSWorker::CMD_REMOVE_FILE: {
                    FTPListParser::FilePair* pair = (FTPListParser::FilePair*)q->pData;
                    pWorker->removeFileRemote(pair);
                }
                break;
            }
            if (q->reqBufDel)
                delete q->pData;
        }
    }

    delete q;
    vTaskDelete(NULL);
}
