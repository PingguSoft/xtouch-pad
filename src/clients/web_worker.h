#ifndef __WEB_WORKER__
#define __WEB_WORKER__

#include <list>
#include <vector>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "camera_worker.h"
#include "mqtt_worker.h"

class WebWorker : public CameraWorker::Callback, MQTTWorker::Callback {
public:
    typedef struct {
        char    *web_dir;
        fs::FS  *fs;
        char    *fs_dir;
    } mount_t;

    WebWorker(fs::FS *fs, char *root, uint16_t port);
    void setPrinterInfo(char *ip, char *accessCode, char *serial, char *name=NULL);
    void addMount(char *web_dir, fs::FS *fs, char *fs_dir);
    void sendFile(char *path);
    void start();
    void stop();

    virtual void onJpeg(uint8_t *param, int size);
    virtual void onMQTT(char *topic, byte *payload, unsigned int length);
    friend  void taskWeb(void* arg);

private:
    void onWebSocket(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void onWebSocketData(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len);
    void sendPNG();
    void listDirSD(char *path, std::vector<String> &info, String ext);
    void _start();
    void _stop();
    void _loop();

    String           _web_root;
    AsyncWebServer  *_server;
    AsyncWebSocket  *_ws;
    fs::FS          *_fs;
    uint16_t        _port;
    long            _last_ts;
    int             _pos;
    char            *_ip;
    char            *_access;
    char            *_serial;
    char            *_name;
    bool            _is_running;

    MQTTWorker      *_mqtt;
    CameraWorker    *_cam;
    std::vector<String> _list_sd;
    std::list<mount_t> _list_mnt;
};

#endif