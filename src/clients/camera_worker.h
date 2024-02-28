#ifndef __CAMERA_WORKER__
#define __CAMERA_WORKER__

#include <list>
#include <vector>
#include <WiFiClientSecure.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"

class CameraWorker {
public:
    typedef enum {
        CMD_START = 1,
        CMD_STOP
    } cmd_t;

    class Callback {
        public:
            virtual ~Callback() { }
            virtual int16_t onCallback(CameraWorker::cmd_t cmd, void *param, int size) = 0;
    };

    typedef struct {
        cmd_t       cmd;
        uint8_t     *pData;
        uint16_t    size;
        bool        reqBufDel;
    } cmd_q_t;

    CameraWorker(char* serverAddress, uint16_t port, char* userName, char* passWord);
    void setCallback(Callback *cb) { _callback = cb; }

    void connect();
    void disconnect();
    void grabJPEG();
    friend void taskCamera(void* arg);

private:
    WiFiClientSecure  _client;
    Callback         *_callback;
    QueueHandle_t    _queue_comm;

    char* _userName;
    char* _passWord;
    char* _serverAddress;
    uint16_t _port;
    uint16_t _timeout = 10000;
    bool _isConnected = false;
    bool _is_secure = true;

    int _pos;
    int _pos_scan;
    int _pos_jpeg_st;
    int _pos_jpeg_end;

    uint8_t *_pJPEGData;
    uint8_t *_pChunk;

    static const uint16_t kChunkSize = 1024;
    static const uint32_t kMaxJPEGSize = (256 * 1024U);
};

#endif