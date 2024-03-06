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

    class BufMan {
    public:
        typedef enum {
            STATE_IDLE = 0,
            STATE_HDR_1,
            STATE_HDR_2,
            STATE_HDR_3,
            STATE_HDR_4,
            STATE_BODY,
            STATE_TAIL_1,
            STATE_COMPLETED
        } state_t;
        BufMan() {
            _pos = 0;
            _len = 0;
            _state = STATE_IDLE;
            _pBuf = (uint8_t*)malloc(kMaxJPEGSize);
        }

        ~BufMan() {
            free(_pBuf);
        }

        void process(uint8_t *buf, int len, Callback *cb);

    private:
        int     _pos;
        int     _len;
        state_t _state;
        uint8_t *_pBuf;
        static const uint32_t kMaxJPEGSize = (140 * 1024U);
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
    BufMan            _jpegBuf;

    char* _user;
    char* _accessCode;
    char* _ipAddress;
    uint16_t _port;
    uint16_t _timeout = 10000;
    bool _isConnected = false;
    bool _is_secure = true;

    SemaphoreHandle_t _lock;
};

#endif