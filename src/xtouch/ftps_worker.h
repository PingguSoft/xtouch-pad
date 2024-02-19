#ifndef __FTPS_WORKER__
#define __FTPS_WORKER__

#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "ESP32_FTPSClient.h"

class FTPListParser {
public:
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
        long   ts;
        String file_a;
        String file_b;
        long   size_a;
        long   size_b;

        FilePair() {
            this->ts = 0;
            this->file_a = "";
            this->file_b = "";
        }

        FilePair(long ts, String a, String b) {
            this->ts = ts;
            this->file_a = a;
            this->file_b = b;
        }

        bool operator<(const FilePair &other) {
            return (ts < other.ts);
        };

        static bool comp_asc(FTPListParser::FilePair* first, FTPListParser::FilePair* second) {
            return (first->ts < second->ts);
        }
        static bool comp_des(FTPListParser::FilePair* first, FTPListParser::FilePair* second) {
            return (first->ts > second->ts);
        }
    };

    FTPListParser() {
    }

    void parse(std::vector<String*> logs, std::vector<FileInfo*> &result, int max, String ext="", int sort=0);
    static void matches(std::vector <FileInfo*> infoA, std::vector <FileInfo*> infoB, std::vector <FilePair*> &result, int sort=0);
    static void diff(std::vector <FileInfo*> a, std::vector <FileInfo*> b, std::vector <FileInfo*> &result);

private:
    static std::vector <String> _months;
};

class FTPSWorker {
public:
    typedef enum {
        CMD_SYNC    = 1,
    } cmd_t;

    typedef struct {
        cmd_t       cmd;
        uint8_t     *pData;
        uint16_t    size;
        bool        reqBufDel;
    } cmd_q_t;

    FTPSWorker(char* serverAdress, uint16_t port, char* userName, char* passWord);
    void downloadDir(String srcDir, String dstDir, std::vector<FTPListParser::FileInfo*> &info, String ext="");
    void listDir(String srcDir, std::vector<FTPListParser::FileInfo*> &info, String ext="");
    void startSync();

    friend void task_sync(void* arg);

private:
    void syncImagesModels();
    template<typename T> static void freeList(std::vector<T> &list);

    ESP32_FTPSClient *_ftps;
    FTPListParser    _parser;
    QueueHandle_t    _queue_comm;
    std::vector<FTPListParser::FileInfo*> _modelFiles;
    std::vector<FTPListParser::FileInfo*> _imageFiles;
};

#endif