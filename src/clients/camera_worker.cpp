#include "debug.h"
#include "camera_worker.h"

void taskCamera(void* arg);

CameraWorker::CameraWorker(char* serverAddress, uint16_t port, char* userName, char* passWord) {
    this->_serverAddress = serverAddress;
    this->_port = port;
    this->_userName = userName;
    this->_passWord = passWord;

    _queue_comm  = xQueueCreate(5, sizeof(cmd_q_t));
    xTaskCreate(&taskCamera, "taskCamera", 8192, this, 4, NULL);
}


typedef struct {
    uint32_t    id1;
    uint32_t    id2;
    uint32_t    id3;
    uint32_t    id4;
    uint8_t     user[32];
    uint8_t     access_code[32];
} __attribute__((packed)) auth_t;

void CameraWorker::connect() {
    _client.setInsecure();
    if (_client.connect((char*)_serverAddress, _port, _timeout)) {
        LOGD("Camera connected\n");

        // _sclient = new WiFiClientSecure(_client.fd());

        auth_t auth = { 0x40, 0x3000, 0, 0, 0, 0};
        strcpy((char*)auth.user, _userName);
        strcpy((char*)auth.access_code, _passWord);
        _client.write((uint8_t*)&auth, sizeof(auth));

        _isConnected = true;
        _pJPEGData = (uint8_t*)malloc(kMaxJPEGSize);
        _pChunk = (uint8_t*)malloc(kChunkSize);
        _pos = 0;
        _pos_scan = 0;
        _pos_jpeg_st = -1;
        _pos_jpeg_end = -1;
    } else {
        LOGE("connection failed : %s\n", _serverAddress);
    }
}

void CameraWorker::disconnect() {
    _isConnected = false;
    free(_pChunk);
    free(_pJPEGData);
}

void CameraWorker::grabJPEG() {
    int sz = _client.readBytes(_pChunk, kChunkSize);

    memcpy(_pJPEGData + _pos, _pChunk, sz);

    if (_pos_jpeg_st < 0) {
        const uint32_t jpeg_sof = 0xe0ffd8ff;
        for (int i = _pos_scan; i < _pos + sz - 3; i++) {
            if (memcmp(_pJPEGData + i, &jpeg_sof, 4) == 0) {
                _pos_jpeg_st = i;
                _pos_scan = i + 4;
                LOGD("JPEG START : %d\n", _pos_jpeg_st);
                break;
            }
        }
        if (_pos_jpeg_st < 0) {
            _pos_scan = _pos + sz - 3;
        }
    }

    if (_pos_jpeg_st >= 0) {
        const uint16_t jpeg_eof = 0xd9ff;
        for (int i = _pos_scan; i < _pos + sz - 1; i++) {
            if (memcmp(_pJPEGData + i, &jpeg_eof, 2) == 0) {
                _pos_jpeg_end = i + 1;
                break;
            }
        }
        if (_pos_jpeg_end < 0) {
            _pos_scan = _pos + sz - 1;
        } else {
            LOGD("JPEG END : %d, size:%d\n", _pos_jpeg_end, _pos_jpeg_end - _pos_jpeg_st);

            // copy remained buffer to front back
            int remain = (_pos + sz) - (_pos_jpeg_end + 1);
            memcpy(_pJPEGData, _pJPEGData + _pos_jpeg_end + 1, remain);
            _pos_jpeg_st = -1;
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
    CameraWorker::cmd_q_t *q = new CameraWorker::cmd_q_t;

    LOGD("taskCamera created !\n");
    while (true) {
        if (pWorker->_isConnected) {
            pWorker->grabJPEG();
        } else {
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        // if (xQueueReceive(pWorker->_queue_comm, q, pdMS_TO_TICKS(10)) == pdTRUE) {
        //     switch (q->cmd) {
        //         case CameraWorker::CMD_START:
        //             pWorker->connect();
        //             break;

        //         case CameraWorker::CMD_STOP:
        //             pWorker->disconnect();
        //             break;
        //     }
        //     if (q->reqBufDel)
        //         delete q->pData;
        // }
    }

    delete q;
    vTaskDelete(NULL);
}
