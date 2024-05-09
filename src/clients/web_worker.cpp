#include <FS.h>
#include <SD.h>
#include <WiFiClientSecure.h>
#include "debug.h"
#include "web_worker.h"
#include "SPIFFSEditor.h"
#include "pushbullet.h"

//
// increase _async_queue size to 64 in _init_async_event_queue of AsyncTCP.cpp for avoiding WDT
// in AsyncServer::begin, set backlog to 1 not to fetch images concurrently
//
void taskWeb(void* arg);

/*
*****************************************************************************************
* Callbacks
*****************************************************************************************
*/
void WebWorker::onJpegEvent(uint8_t *param, int size) {
    LOGI("JPEG Completed : %d\n", size);
    if (_ws->count() > 0 && _is_cam_view) {
        param[0] = 0x01;
        _ws->binaryAll(param, size);
    }
}

void WebWorker::onMQTTEvent(char *topic, byte *payload, unsigned int length) {
    LOGI("%5d - %s %s\n", length, topic, payload);
    if (_ws->count() > 0) {
        _ws->textAll(payload, length);
    }
}

void WebWorker::onWebSocket(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            LOGD("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            LOGD("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            onWebSocketData(client, arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

int16_t WebWorker::onFTPSEvent(FTPSWorker::cmd_t cmd, void *param, int size) {
    std::list<FTPListParser::FilePair*> *pair = (std::list<FTPListParser::FilePair*>*)param;

    switch (cmd) {
        case FTPSWorker::CMD_SYNC_DONE:
            if (size > 0) {
                for (FTPListParser::FilePair* p : _file_pair)
                    delete p;
                _file_pair.clear();

                for (FTPListParser::FilePair* p : *pair)
                    _file_pair.push_back(new FTPListParser::FilePair(p));

                sendFTPSInfo();
            }
            break;

        case FTPSWorker::CMD_DOWNLOAD_START:
            break;

        case FTPSWorker::CMD_DOWNLOADING:
            LOGD("downloading : %s\n", (char*)param);
            break;
    }
    return 0;
}

/*
*****************************************************************************************
*
*****************************************************************************************
*/
WebWorker::WebWorker(fs::FS *fs, char *root, uint16_t port) {
    _fs = fs;
    _port = port;
    _last_ts = 0;
    _web_root = root;
    if (!_web_root.endsWith("/")) {
        _web_root += "/";
    }

    _server = NULL;
    _ws = NULL;
    _mqtt = NULL;
    _cam = NULL;
    _is_cam_view = false;
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

void WebWorker::setPrinterInfo(char *ip, char *accessCode, char *serial, char *name) {
    _ip = ip;
    _access = accessCode;
    _serial = serial;
    _name = name;
}

void WebWorker::start(Config *cfg) {
    _cfg = cfg;
    _is_running = true;
    xTaskCreate(&taskWeb, "taskWeb", 10*1024, this, 1, NULL);
}

void WebWorker::stop() {
    _is_running = false;
}

void WebWorker::addMount(char *web_dir, fs::FS *fs, char *fs_dir) {
    _list_mnt.push_back({web_dir, fs, fs_dir});
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
    // if (_list_sd.size() > 0) {
    //     String str = _list_sd[_pos];
    //     str = "/image/" + str;
    //     sendFile((char*)str.c_str());
    //     _pos = (_pos + 1) % _list_sd.size();
    // }
}

void WebWorker::_start() {
    // _pos = 0;
    // listDirSD("/image", _list_sd, ".png");

    if (!_server)
        _server = new AsyncWebServer(_port);

    //
    // WebSocket
    //
    if (!_ws)
        _ws = new AsyncWebSocket("/ws");
    std::function<void(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType , void *, uint8_t *, size_t)> f =
        std::bind(&WebWorker::onWebSocket, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
    _ws->onEvent(f);
    _server->addHandler(_ws);

    //
    // WebServer
    //
    _server->serveStatic("/", *_fs, _web_root.c_str()).setDefaultFile("index.html");
    _server->addHandler(new SPIFFSEditor(*_fs, "admin", _cfg->getAdminPassword()));
    for (mount_t m : _list_mnt) {
        _server->serveStatic(m.web_dir, *m.fs, m.fs_dir);
    }
    _server->onNotFound([](AsyncWebServerRequest *request) {
        int headers = request->headers();
        for (int i = 0; i < headers; i++) {
            AsyncWebHeader *h = request->getHeader(i);
            LOGI("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for (int i = 0; i < params; i++) {
            AsyncWebParameter *p = request->getParam(i);
            if (p->isFile()) {
                LOGI("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if (p->isPost()) {
                LOGI("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                LOGI("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }
        request->send(404);
    });
    _server->begin();

    if (_ip && _access && _serial) {
        //
        // Camera worker
        //
        _cam = new CameraWorker(_ip, 6000, (char*)"bblp", _access);
        _cam->setCallback(this);

        //
        // MQTT worker
        //
        _mqtt = new MQTTWorker(_ip, _access, _serial);
        _mqtt->setCallback(this);
        _mqtt->start();

        //
        //
        //
        _ftps = new FTPSWorker(_ip, 990, (char*)"bblp", _access);
        _ftps->setCallback(this);
        // _ftps->startSync();
    }
}

void WebWorker::_stop() {
    if (_cam)
        _cam->stop();

    if (_mqtt)
        _mqtt->stop();

    if (_ws) {
        delete _ws;
        _ws = NULL;
    }
    if (_server) {
        _server->end();
        delete _server;
        _server = NULL;
    }
}

void WebWorker::sendFTPSInfo(AsyncWebSocketClient *client) {
    JsonDocument json_out;
    JsonDocument json_item;
    String json_str;

    JsonArray arr;
    arr = json_out["webui"]["sdcard_list"].to<JsonArray>();
    for (FTPListParser::FilePair* p : _file_pair) {
        json_item["ts"] = p->a.ts;
        json_item["size"] = p->a.size;
        json_item["3mf"] = p->a.name;
        json_item["png"] = p->b.name;
        arr.add(json_item);
    }
    serializeJson(json_out, json_str);
    if (_ws) {
        if (client)
            _ws->text(client->id(), json_str);
        else
            _ws->textAll(json_str);
    }
}

void WebWorker::onWebSocketData(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    data[len] = 0;
    LOGI("%s\n", data);

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        JsonDocument json_in;
        JsonDocument json_out;
        JsonDocument json_item;
        String json_str;

        auto err = deserializeJson(json_in, data, len);
        if (!err) {
            if (json_in.containsKey("command")) {
                String command = json_in["command"].as<String>();

                if (command == "open") {
                    if (_name) {
                        json_out["webui"]["printer_name"] = _name;
                    } else {
                        json_out["webui"]["printer_name"] = "None";
                    }
                    serializeJson(json_out, json_str);
                    if (_ws) {
                        _ws->text(client->id(), json_str);
                    }
                    if (_mqtt) {
                        _mqtt->reqPushAll();
                    }
                } else if (command == "sdcard_list") {
                    _ftps->startSync();
                } else if (command == "camera_view") {
                    _is_cam_view = json_in["data"].as<bool>();
                    if (_is_cam_view) {
                        _cam->start();
                    } else {
                        _cam->stop();
                    }
                } else if (command == "pub" && json_in.containsKey("data")) {
                    if (_mqtt) {
                        json_out = json_in["data"];
                        _mqtt->publish(json_out);
                    }
                } else if (command == "print" && json_in.containsKey("data")) {
                    json_out = json_in["data"];
                    PushBullet _pb(_cfg->getPBToken());
                    _pb.pushFile("BambuBridge", "This message comes from BambuBridge", json_out["png"]);  //"17540064941.png");
                }
            }
        }
    }
}

void WebWorker::_loop() {
    bool ret = false;

    if (!_server || !_ws)
        return;

    long ts = millis();
    if ((ts - _last_ts) > 5000) {
        if (_ws->count() > 0) {
            ret = true;
        }
        _last_ts = ts;
    }
    if (_mqtt)
        ret = _mqtt->loop();

    if (_ws) {
        _ws->cleanupClients();
    }

    if (!ret) {
        delay(20);
    } else {
        delay(1);
    }
}


/*
*****************************************************************************************
* taskWeb
*****************************************************************************************
*/
void taskWeb(void* arg) {
    WebWorker *pWorker = (WebWorker*)arg;

    LOGD("taskWeb created !\n");
    pWorker->_start();

    while (pWorker->_is_running) {
        pWorker->_loop();
    }

    pWorker->_stop();
    vTaskDelete(NULL);
}
