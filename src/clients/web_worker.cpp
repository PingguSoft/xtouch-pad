#include <FS.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "debug.h"
#include "web_worker.h"

void taskWeb(void* arg);

WebWorker::WebWorker(fs::FS *fs, char *root, uint16_t port) {
    _fs = fs;
    _web_root = root;    
    _port = port;
    _server = NULL;
    _ws = NULL;
    _last_ts = 0;

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

void WebWorker::start() {
    _pos = 0;
    listDirSD("/image", _list_sd, ".png");

    cmd_q_t q = { CMD_START, NULL, 0, false };
    xQueueSend(_queue_comm, &q, portMAX_DELAY);
}

void WebWorker::stop() {
    cmd_q_t q = { CMD_STOP, NULL, 0, false };
    xQueueSend(_queue_comm, &q, portMAX_DELAY);
}

void WebWorker::_start() {
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
            String buf(this->_web_root);
            buf = buf + "index.html";
            request->send(*(this->_fs), buf.c_str(), "text/html"); 
        });
    _server->serveStatic("/image/", *_fs, "/image/");
    _server->serveStatic("/", *_fs, "/web/");
    _server->begin();
}

void WebWorker::_stop() {
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

JsonDocument readings;

String WebWorker::getSensorReadings() {
    readings.clear();
    readings["temperature"] = String(random(30));
    readings["humidity"] = String(random(100));
    readings["pressure"] = String(random(20));
    
    JsonArray data = readings["ws_image"].to<JsonArray>();
    if (_list_sd.size() > 0) {
        String str = _list_sd[_pos];

        str = "/image/" + str;
        File file = SD.open(str.c_str(), "rb");
        LOGD("image file : %s %d\n", str.c_str(), file.size());
        for (int i = 0; i < file.size(); i++) {
            data.add(file.read());
        }
        file.close();
        _pos = (_pos + 1) % _list_sd.size();
    }

    String output;
    serializeJson(readings, output);
    return output;
}

void WebWorker::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        if (strcmp((char*)data, "getReadings") == 0) {
            String sensorReadings = getSensorReadings();
            // LOGD("%s\n", sensorReadings.c_str());
            _ws->textAll(sensorReadings);
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
    if (_server && _ws) {
        long ts = millis();
        if ((ts - _last_ts) > 5000) {
            String sensorReadings = getSensorReadings();
            _ws->textAll(sensorReadings);
            _last_ts = ts;
        }
        _ws->cleanupClients();
    }
}


/*
*****************************************************************************************
* taskCamera
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
