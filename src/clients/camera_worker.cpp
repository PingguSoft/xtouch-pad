#include "debug.h"
#include "camera_worker.h"

void taskCamera(void* arg);

int CameraWorker::BufMan::process(WiFiClientSecure *client, Callback *cb) {
    uint8_t ch;
    int     sz = client->available();

    for (int i = 0; i < sz; i++) {
        ch = client->read();

        switch (_state) {
            case STATE_IDLE:
                if (ch == 0xFF) {
                    _state = STATE_HDR_1;
                    _pBuf[_pos++] = ch;
                 } else {
                    _pos = 1;
                    _state = STATE_IDLE;
                 }
                break;

            case STATE_HDR_1:
                if (ch == 0xD8) {
                    _state = STATE_HDR_2;
                    _pBuf[_pos++] = ch;
                } else {
                    _pos = 1;
                    _state = STATE_IDLE;
                }
                break;

            case STATE_HDR_2:
                if (ch == 0xFF) {
                    _state = STATE_HDR_3;
                    _pBuf[_pos++] = ch;
                } else {
                    _pos = 1;
                    _state = STATE_IDLE;
                }
                break;

            case STATE_HDR_3:
                if (ch == 0xE0) {
                    _state = STATE_BODY;
                    _pBuf[_pos++] = ch;
                } else {
                    _pos = 1;
                    _state = STATE_IDLE;
                }
                break;

            case STATE_BODY:
                _pBuf[_pos++] = ch;
                if (ch == 0xFF) {
                    _state = STATE_TAIL_1;
                }
                break;

            case STATE_TAIL_1:
                _pBuf[_pos++] = ch;
                if (ch == 0xD9) {
                    // completed
                    if (cb) {
                        cb->onJpegEvent(_pBuf, _pos);
                    }
                    _pos = 1;
                    _state = STATE_IDLE;
                } else {
                    _state = STATE_BODY;
                }
                break;
        }
    }
    return sz;
}

CameraWorker::CameraWorker(char* ipAddress, uint16_t port, char* user, char* accessCode) {
    _ipAddress = ipAddress;
    _port = port;
    _user = user;
    _accessCode = accessCode;
    _lock = xSemaphoreCreateMutex();
    xTaskCreate(&taskCamera, "taskCamera", 8192, this, 1, NULL);
}

typedef struct {
    uint32_t    id1;
    uint32_t    id2;
    uint32_t    id3;
    uint32_t    id4;
    uint8_t     user[32];
    uint8_t     access_code[32];
} __attribute__((packed)) auth_t;

void CameraWorker::start() {
    _client.setInsecure();
    LOGD("connect to : %s %s\n", _ipAddress, _accessCode);

    if (_client.connect(_ipAddress, _port, _timeout)) {
        LOGD("Camera connected\n");
        auth_t auth = { 0x40, 0x3000, 0, 0, 0, 0};
        strcpy((char*)auth.user, _user);
        strcpy((char*)auth.access_code, _accessCode);
        _client.write((uint8_t*)&auth, sizeof(auth));
        _isConnected = true;
    } else {
        LOGE("connection failed : %s\n", _ipAddress);
    }
}

void CameraWorker::lock() {
    xSemaphoreTake(_lock, portMAX_DELAY);
}

void CameraWorker::unlock() {
    xSemaphoreGive(_lock);
}

void CameraWorker::stop() {
    _isConnected = false;
    _client.stop();
}

int CameraWorker::grabJPEG() {
    int sz = _jpegBuf.process(&_client, _callback);
    return sz;
}

/*
*****************************************************************************************
* taskCamera
*****************************************************************************************
*/
void taskCamera(void* arg) {
    CameraWorker *pWorker = (CameraWorker*)arg;

    LOGD("taskCamera created !\n");
    while (true) {
        if (pWorker->_isConnected) {
            int sz = pWorker->grabJPEG();
            if (sz == 0)
                delay(10);
        } else {
            delay(50);
            // vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    vTaskDelete(NULL);
}
