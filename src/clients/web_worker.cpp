#include <FS.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "debug.h"
#include "web_worker.h"


void taskWeb(void* arg);

WebWorker::WebWorker(fs::FS *fs, char *root, uint16_t port) {
    _fs = fs;
    _port = port;
    _server = NULL;
    _ws = NULL;
    _last_ts = 0;
    _web_root = root;
    if (!_web_root.endsWith("/")) {
        _web_root += "/";
    }
    _queue_comm  = xQueueCreate(5, sizeof(cmd_q_t));
    xTaskCreate(&taskWeb, "taskWeb", 8192, this, 4, NULL);
}

void WebWorker::listDirSD(char *path, std::vector<String> &info, String ext) {
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
            info.push_back(String(name));
        }
        entry.close();
    }
    delete new_path;
}

void WebWorker::start(char *ip, char *acccessCode) {
    _ip = ip;
    _access = acccessCode;
    cmd_q_t q = { CMD_START, NULL, 0, false };
    xQueueSend(_queue_comm, &q, portMAX_DELAY);
}

void WebWorker::stop() {
    cmd_q_t q = { CMD_STOP, NULL, 0, false };
    xQueueSend(_queue_comm, &q, portMAX_DELAY);
}

void WebWorker::addMount(char *web_dir, fs::FS *fs, char *fs_dir) {
    _list_mnt.push_back({web_dir, fs, fs_dir});
}

void WebWorker::onJpeg(uint8_t *param, int size) {
    if (_ws->count() > 0) {
        _ws->binaryAll(param, size);
    }
}

void WebWorker::sendFile(char *path) {
    File file = SD.open(path, "rb");
    LOGD("image file : %s %d\n", path, file.size());

    uint8_t *buf = (uint8_t*)malloc(file.size());
    file.read(buf, file.size());
    if (_ws->count() > 0) {
        _ws->binaryAll(buf, file.size());
    }
    file.close();
    free(buf);
}

void WebWorker::sendPNG() {
    if (_list_sd.size() > 0) {
        String str = _list_sd[_pos];
        str = "/image/" + str;
        sendFile((char*)str.c_str());
        _pos = (_pos + 1) % _list_sd.size();
    }
}

void WebWorker::_start() {
    _pos = 0;
    listDirSD("/image", _list_sd, ".png");

    if (!_server)
        _server = new AsyncWebServer(_port);
    if (!_ws)
        _ws = new AsyncWebSocket("/ws");

    std::function<void(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType , void *, uint8_t *, size_t)> f =
        std::bind(&WebWorker::onEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);

    _ws->onEvent(f);
    _server->addHandler(_ws);
    _server->on("/", HTTP_GET,
        [this](AsyncWebServerRequest *request) {
            String buf(_web_root + "index.html");
            request->send(*(this->_fs), buf.c_str(), "text/html");
        });
    _server->serveStatic("/", *_fs, _web_root.c_str());
    for (mount_t m : _list_mnt) {
        _server->serveStatic(m.web_dir, *m.fs, m.fs_dir);
    }
    _server->begin();

    _cam = new CameraWorker(_ip, 6000, (char*)"bblp", _access);
    _cam->setCallback(this);
    _cam->start();
}

void WebWorker::_stop() {
    _cam->stop();
    _server->end();

    if (_ws) {
        delete _ws;
        _ws = NULL;
    }
    if (_server) {
        delete _server;
        _server = NULL;
    }
}

String WebWorker::getSensorReadings() {
    JsonDocument json;

    json["temperature"] = String(random(30));
    json["humidity"] = String(random(100));
    json["pressure"] = String(random(20));

    String output;
    serializeJson(json, output);
    return output;
}

void WebWorker::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        if (strcmp((char*)data, "getReadings") == 0) {
            String sensorReadings = getSensorReadings();
            // LOGD("%s\n", sensorReadings.c_str());
            _ws->textAll(sensorReadings);
            sendPNG();
        }
    }
}

void WebWorker::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
    case WS_EVT_CONNECT:
        LOGD("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        LOGD("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void WebWorker::loop() {
    if (!_server || !_ws)
        return;

    long ts = millis();
    if ((ts - _last_ts) > 5000) {
        if (_ws->count() > 0) {
            String sensorReadings = getSensorReadings();
            _ws->textAll(sensorReadings);
            sendPNG();
        }
        _last_ts = ts;
    }
    _ws->cleanupClients();
}


/*
*****************************************************************************************
* taskWeb
*****************************************************************************************
*/
void taskWeb(void* arg) {
    WebWorker *pWorker = (WebWorker*)arg;
    WebWorker::cmd_q_t *q = new WebWorker::cmd_q_t;

    LOGD("taskWeb created !\n");
    while (true) {
        if (xQueueReceive(pWorker->_queue_comm, q, pdMS_TO_TICKS(10)) == pdTRUE) {
            switch (q->cmd) {
                case WebWorker::CMD_START:
                    pWorker->_start();
                    LOGD("started\n");
                    break;

                case WebWorker::CMD_STOP:
                    pWorker->_stop();
                    LOGD("stopped\n");
                    break;
            }
            if (q->reqBufDel)
                delete q->pData;
        }
        pWorker->loop();
    }

    delete q;
    vTaskDelete(NULL);
}
