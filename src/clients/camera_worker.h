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
    class Callback {
        public:
            virtual ~Callback() { }
            virtual void onJpeg(uint8_t *param, int size) = 0;
    };

    CameraWorker(char* ipAddress, uint16_t port, char* user, char* accessCode);
    void setCallback(Callback *cb) { _callback = cb; }

    void start();
    void stop();
    void lock();
    void unlock();

    friend void taskCamera(void* arg);

private:
    int grabJPEG();

    WiFiClientSecure  _client;
    Callback         *_callback;

    char* _user;
    char* _accessCode;
    char* _ipAddress;
    uint16_t _port;
    uint16_t _timeout = 10000;
    bool _isConnected = false;
    bool _is_secure = true;

    int _pos;
    int _pos_scan;
    int _pos_jpeg_beg;
    int _pos_jpeg_end;

    uint8_t *_pJpegBuf;
    uint8_t *_pChunk;
    SemaphoreHandle_t _lock;

    static const uint16_t kChunkSize = 1024;
    static const uint32_t kMaxJPEGSize = (256 * 1024U);
};

#endif