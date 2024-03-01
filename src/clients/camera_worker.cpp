#include "debug.h"
#include "camera_worker.h"

void taskCamera(void* arg);

CameraWorker::CameraWorker(char* ipAddress, uint16_t port, char* user, char* accessCode) {
    this->_ipAddress = ipAddress;
    this->_port = port;
    this->_user = user;
    this->_accessCode = accessCode;

    _lock = xSemaphoreCreateMutex();
    xTaskCreate(&taskCamera, "taskCamera", 8192, this, 3, NULL);
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
        _pJpegBuf = (uint8_t*)malloc(kMaxJPEGSize);
        _pChunk = (uint8_t*)malloc(kChunkSize);
        _pos = 0;
        _pos_scan = 0;
        _pos_jpeg_beg = -1;
        _pos_jpeg_end = -1;
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
    free(_pChunk);
    free(_pJpegBuf);
}

void CameraWorker::grabJPEG() {
    int sz = _client.readBytes(_pChunk, kChunkSize);

    memcpy(_pJpegBuf + _pos, _pChunk, sz);

    if (_pos_jpeg_beg < 0) {
        const uint32_t jpeg_sof = 0xe0ffd8ff;
        for (int i = _pos_scan; i < _pos + sz - 3; i++) {
            if (memcmp(_pJpegBuf + i, &jpeg_sof, 4) == 0) {
                _pos_jpeg_beg = i;
                _pos_scan = i + 4;
                LOGD("JPEG START : %d\n", _pos_jpeg_beg);
                break;
            }
        }
        if (_pos_jpeg_beg < 0) {
            _pos_scan = _pos + sz - 3;
        }
    }

    if (_pos_jpeg_beg >= 0) {
        const uint16_t jpeg_eof = 0xd9ff;
        for (int i = _pos_scan; i < _pos + sz - 1; i++) {
            if (memcmp(_pJpegBuf + i, &jpeg_eof, 2) == 0) {
                _pos_jpeg_end = i + 1;
                break;
            }
        }
        if (_pos_jpeg_end < 0) {
            _pos_scan = _pos + sz - 1;
        } else {
            uint32_t jpg_size = _pos_jpeg_end - _pos_jpeg_beg + 1;
            LOGD("JPEG END : %d, size:%d\n", _pos_jpeg_end, jpg_size);
            if (_callback) {
                _callback->onJpeg(&_pJpegBuf[_pos_jpeg_beg], jpg_size);
            }

            // copy remained buffer to front back
            int remain = (_pos + sz) - (_pos_jpeg_end + 1);
            memcpy(_pJpegBuf, _pJpegBuf + _pos_jpeg_end + 1, remain);
            _pos_jpeg_beg = -1;
            _pos_jpeg_end = -1;
            _pos_scan = 0;
            _pos = 0;
            sz = 0;
        }
    }
    _pos = (_pos + sz) % kMaxJPEGSize;
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
            pWorker->grabJPEG();
        } else {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    vTaskDelete(NULL);
}
