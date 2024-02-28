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

    String line;
    for (String *l: logs) {
        line = *l;
        char *token = strtok((char*)line.c_str(), " ");
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
            int year = 2100;
            if (tokens[7][2] == ':') {
                tokens[7][2] = '\0';
                hour = atoi(tokens[7]);
                min  = atoi(&tokens[7][3]);
                year = year - 1980;
            } else {
                year = atoi(tokens[7]) - 1980;
            }
            long ts  = min + (hour * 60L) + (day * 60 * 24L) + (mon * 60 * 24 * 31L) + (year * 60 * 24 * 31 * 365L);
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

FTPSWorker::FTPSWorker(char* serverAddress, uint16_t port, char* userName, char* passWord) {
    _ftps = new ESP32_FTPSClient(serverAddress, port, userName, passWord);
    _queue_comm  = xQueueCreate(5, sizeof(cmd_q_t));
    xTaskCreate(&taskFTPS, "taskFTPS", 8192, this, 4, NULL);

    char bufs[60];
    char drv;
    drv = lv_fs_get_letters(bufs)[0];

    sprintf(bufs, "%C:%s", drv, PATH_IMG);
    _img_path = String(bufs);

    sprintf(bufs, "%C:%s", drv, PATH_MODEL);
    _model_path = String(bufs);
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
    LOGD("Start downloading...\n");
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
    LOGD("Completed...\n");
}

void FTPSWorker::listDirRemote(String srcDir, std::list<FTPListParser::FileInfo*> &info, String ext, int max) {
    std::list<String*> list;

    _ftps->InitFile("Type A");
    _ftps->DirLong(srcDir.c_str(), list);
    _parser.parse(list, info, max, ext, FTPListParser::SORT_DESC);
    freeList(list);

    int cnt = 1;
    LOGV("list count:%d\n", info.size());
    for (FTPListParser::FileInfo* i : info) {
        LOGV("%4d, %10ld, [%10ld] %s\n", cnt++, i->ts, i->size, i->name.c_str());
    }
}

void FTPSWorker::startSync(bool textonly) {
    cmd_q_t q = { CMD_SYNC, NULL, (uint16_t)(textonly ? 1 : 0), false };
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
std::list<String*> FTPSWorker::_testModels = {
    new String("drw-rw-rw-   1 root  root      1234 Jan 01 1980 time_test.3mf"),
    new String("-rw-rw-rw-   1 root  root    326780 Jan 23 22:57 HKR -AMS Side mount spool holder.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    595232 Jan 24 09:26 X1_Touch.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    413301 Jan 25 23:20 No-Catch Y-Splitter Self-Tap (long slot) (9.4mm-M3x6 mount).gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    698414 Jan 24 21:58 Bambu Trash_plate_1.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    301823 Jan 25 00:08 Bambu Trash_plate_3.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    176374 Jan 25 22:09 AMS_disconnect_tool_with_magnet_seperate_letters.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    710512 Jan 26 23:51 hex_bits_holder_7 x 2.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root   1029925 Jan 27 13:22 hex_bits_holder_7x3.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    767304 Jan 27 13:26 swerve-gear-motorx6.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root   1548586 Jan 28 04:35 rc_con_left_bottom.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    458648 Jan 28 11:41 swerve-axle-gearx3.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    970885 Jan 28 14:13 swerve-large-gear.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root   2108337 Feb 03 15:51 controller-left.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    810058 Feb 05 23:14 plate_holder.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    667533 Feb 05 23:28 rpi3p_case.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    838822 Feb 12 22:45 galaxy watch stand.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root   1573271 Feb 22 06:17 Pikachu.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root   1196949 Feb 22 06:26 hello-kitty-6-Dom.gcode.3mf"),
    new String("-rw-rw-rw-   1 root  root    766263 Feb 25 01:52 Cute Schnauzer keychain.gcode.3mf")
};

std::list<String*> FTPSWorker::_testImages = {
    new String("drw-rw-rw-   1 root  root      1234 Jan 01 1980 time_test.png"),
    new String("-rw-rw-rw-   1 root  root      4796 Jan 23 22:57 39585176581.png"),
    new String("-rw-rw-rw-   1 root  root      5034 Feb 25 01:52 17540064941.png"),
    new String("-rw-rw-rw-   1 root  root      4043 Jan 24 09:26 22181592811.png"),
    new String("-rw-rw-rw-   1 root  root      3609 Jan 25 23:20 2613376571.png"),
    new String("-rw-rw-rw-   1 root  root      2157 Jan 24 21:58 7143709141.png"),
    new String("-rw-rw-rw-   1 root  root      3186 Jan 25 00:08 3553883803.png"),
    new String("-rw-rw-rw-   1 root  root      3804 Jan 25 22:09 4380517671.png"),
    new String("-rw-rw-rw-   1 root  root      3471 Jan 26 23:16 15642952511.png"),
    new String("-rw-rw-rw-   1 root  root      2755 Jan 26 23:51 10363259711.png"),
    new String("-rw-rw-rw-   1 root  root      2171 Jan 27 13:22 4678907721.png"),
    new String("-rw-rw-rw-   1 root  root      1182 Jan 27 13:26 19799488481.png"),
    new String("-rw-rw-rw-   1 root  root      4081 Jan 28 04:35 2064281.png"),
    new String("-rw-rw-rw-   1 root  root      1863 Jan 28 11:41 17930814601.png"),
    new String("-rw-rw-rw-   1 root  root      4857 Jan 28 14:13 26698210481.png"),
    new String("-rw-rw-rw-   1 root  root      4796 Jan 24 01:06 12718038631.png"),
    new String("-rw-rw-rw-   1 root  root      4505 Feb 03 15:51 38297837321.png"),
    new String("-rw-rw-rw-   1 root  root      2540 Feb 05 23:13 9240571371.png"),
    new String("-rw-rw-rw-   1 root  root      4684 Feb 05 23:28 42885271611.png"),
    new String("-rw-rw-rw-   1 root  root      4442 Feb 12 22:44 15137327011.png"),
    new String("-rw-rw-rw-   1 root  root      2082 Feb 22 06:16 1270915251.png"),
    new String("-rw-rw-rw-   1 root  root      5206 Feb 22 06:26 29877475381.png")
};
#endif

void FTPSWorker::syncImagesModels(bool textonly) {
    int  cnt;
    int  max_items = textonly ? 100 : 30;

    std::list<FTPListParser::FileInfo*> imageFilesSD;
    std::list<FTPListParser::FileInfo*> modelFilesRemote;
    std::list<FTPListParser::FileInfo*> imageFilesRemote;
    std::list<FTPListParser::FilePair*> pairList;

    if (!textonly) {
        LOGV("--------------- SD CARD ---------------\n");
        listDirSD(getImagePath(), imageFilesSD, ".png");
        cnt = 1;
        for (FTPListParser::FileInfo *p:imageFilesSD) {
            LOGV("%3d %10ld, [%10ld] %s\n", cnt++, p->ts, p->size, p->name.c_str());
        }
    }

#if !_NO_NETWORK_
    _ftps->OpenConnection(false, true);
    listDirRemote(getModelPath(), modelFilesRemote, ".3mf", max_items);     // model files
#else
    _parser.parse(_testModels, modelFilesRemote, max_items, ".3mf", FTPListParser::SORT_DESC);
#endif

    if (!textonly) {
#if _NO_NETWORK_
        _parser.parse(_testImages, imageFilesRemote, max_items, ".png", FTPListParser::SORT_DESC);
#else
        listDirRemote(getImagePath(), imageFilesRemote, ".png", max_items);     // image files
#endif
        // make pairList after matching timestamp
        _parser.matches(modelFilesRemote, imageFilesRemote, pairList, FTPListParser::SORT_DESC);
        cnt = 1;
        LOGV("--------------- FILE MODEL & PNG ---------------\n");
        for (FTPListParser::FilePair *p:pairList) {
            LOGV("%3d %10ld, [%10ld] %20s, [%10ld] %s\n", cnt++, p->ts, p->b.size, p->b.name.c_str(), p->a.size, p->a.name.c_str());
        }

#if !_NO_NETWORK_
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
        LOGV("--------------- TBU ---------------\n");
        for (FTPListParser::FileInfo *p:imageFilesDownload) {
            LOGV("%3d %10ld, [%10ld] %s\n", cnt++, p->ts, p->size, p->name.c_str());
        }
        if (imageFilesDownload.size() > 0) {
            if (_callback)
                _callback->onCallback(CMD_DOWNLOAD_START, NULL, imageFilesDownload.size());
            downloadDirRemote(getImagePath(), getImagePath(), imageFilesDownload, ".png");
        }
        cnt = pairList.size();
        freeList(imageFilesDownload);
#endif
    } else {
        for (FTPListParser::FileInfo* info : modelFilesRemote) {
            pairList.push_back(new FTPListParser::FilePair(info->ts, FTPListParser::FileInfo(info), FTPListParser::FileInfo()));
        }
        cnt = pairList.size();
    }

#if !_NO_NETWORK_
    _ftps->CloseConnection();
#endif

    freeList(imageFilesSD);
    freeList(modelFilesRemote);
    freeList(imageFilesRemote);

    if (_callback) {
        _callback->onCallback(CMD_SYNC_DONE, &pairList, cnt);
    }
    freeList(pairList);
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
                    pWorker->syncImagesModels(q->size == 1);
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
