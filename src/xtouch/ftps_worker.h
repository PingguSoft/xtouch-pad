#ifndef __FTPS_WORKER__
#define __FTPS_WORKER__

#include <list>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "ESP32_FTPSClient.h"

class FTPListParser {
public:
    enum {
        SORT_DESC = -1,
        SORT_NONE = 0,
        SORT_ASC  = 1,
    };

    enum {
        BY_TS = 0,
        BY_NAME = 1,
    };

    class FileInfo {
    public:
        long   ts;
        long   size;
        String name;

        void set(long ts, long size, String name) {
            this->ts = ts;
            this->size = size;
            this->name = name;
        }

        FileInfo() {
            set(0, 0, "");
        }

        FileInfo(FileInfo *p) {
            set(p->ts, p->size, p->name);
        }

        FileInfo(long ts, long size, String name) {
            set(ts, size, name);
        }

        bool operator<(const FileInfo &other) {
            return (ts < other.ts);
        };

        static bool comp_asc(FTPListParser::FileInfo* first, FTPListParser::FileInfo* second) {
            return (first->ts < second->ts);
        }
        static bool comp_des(FTPListParser::FileInfo* first, FTPListParser::FileInfo* second) {
            return (first->ts > second->ts);
        }
    };

    class FilePair {
    public:
        long        ts;
        FileInfo    a;
        FileInfo    b;

        void set(long ts, FileInfo a, FileInfo b) {
            this->ts = ts;
            this->a  = a;
            this->b  = b;
        }

        FilePair() {
            this->ts = 0;
        }

        FilePair(FilePair *p) {
            set(p->ts, p->a, p->b);
        }

        FilePair(long ts, FileInfo a, FileInfo b) {
            set(ts, a, b);
        }

        // for offline simulation
        FilePair(long ts, long a_size, String a_name, long b_size, String b_name) {
            this->ts = ts;
            this->a = FileInfo(ts, a_size, a_name);
            this->b = FileInfo(ts, b_size, b_name);
        }

        void invalid() {
            this->ts = 0;
        }
        bool isValid() {
            return (this->ts > 0);
        }
        static bool comp_asc(FTPListParser::FilePair* first, FTPListParser::FilePair* second) {
            return (first->ts < second->ts);
        }
        static bool comp_des(FTPListParser::FilePair* first, FTPListParser::FilePair* second) {
            return (first->ts > second->ts);
        }
    };

    FTPListParser() {
    }

    void parse(std::list<String*> logs, std::list<FileInfo*> &result, int max, String ext="", int sort=SORT_ASC);
    static void matches(std::list<FileInfo*> infoA, std::list<FileInfo*> infoB, std::list<FilePair*> &result, int sort=SORT_ASC);
    static void diff(std::list<FileInfo*> a, std::list<FileInfo*> b, std::list<FileInfo*> &result, int by=BY_TS);
    void exportA(std::list<FilePair*> pair, std::list <FileInfo*> &result);
    void exportB(std::list<FilePair*> pair, std::list <FileInfo*> &result);

private:
    static std::vector<String> _months;
};

class FTPSWorker {
public:
    typedef enum {
        CMD_SYNC    = 1,
        CMD_DOWNLOADING,
    } cmd_t;

    class Callback {
        public:
            virtual ~Callback() { }
            virtual int16_t onCallback(FTPSWorker::cmd_t cmd, void *param, int size) = 0;
    };

    typedef struct {
        cmd_t       cmd;
        uint8_t     *pData;
        uint16_t    size;
        bool        reqBufDel;
    } cmd_q_t;

    FTPSWorker(char* serverAdress, uint16_t port, char* userName, char* passWord);
    void downloadDir(String srcDir, String dstDir, std::list<FTPListParser::FileInfo*> info, String ext="");
    void listDir(String srcDir, std::list<FTPListParser::FileInfo*> &info, String ext="", int max=30);
    void startSync();
    void listDirSD(char *path, std::list<FTPListParser::FileInfo*> &info, String ext="");
    void invalidate();

    void setCallback(Callback *cb) { _callback = cb; }
    static char *getImagePath(bool lv=false) { return (lv ? (char*)"S:/ftps/image/" : (char*)"/ftps/image/"); }
    friend void taskFTPS(void* arg);

private:
    void syncImagesModels();
    template<typename T> static void freeList(std::list<T> &list);

    Callback         *_callback;
    ESP32_FTPSClient *_ftps;
    FTPListParser    _parser;
    QueueHandle_t    _queue_comm;
    std::list<FTPListParser::FilePair*> _pairList;

#if _NO_NETWORK_
    static std::list<String> _testPair;
#endif
};

#endif