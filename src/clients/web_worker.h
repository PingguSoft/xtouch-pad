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
    typedef enum {
        CMD_START = 1,
        CMD_STOP
    } cmd_t;

    class Callback {
        public:
            virtual ~Callback() { }
            virtual int16_t onCallback(WebWorker::cmd_t cmd, void *param, int size) = 0;
    };

    typedef struct {
        cmd_t       cmd;
        uint8_t     *pData;
        uint16_t    size;
        bool        reqBufDel;
    } cmd_q_t;

    typedef struct {
        char    *web_dir;
        fs::FS  *fs;
        char    *fs_dir;
    } mount_t;

    WebWorker(fs::FS *fs, char *root, uint16_t port);
    void start(char *ip, char *accessCode, char *serial);
    void stop();
    void addMount(char *web_dir, fs::FS *fs, char *fs_dir);
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    void listDirSD(char *path, std::vector<String> &info, String ext);
    void setCallback(Callback *cb) { _callback = cb; }
    void sendFile(char *path);
    void sendPNG();

    String getSensorReadings();
    void loop();

    virtual void onJpeg(uint8_t *param, int size);
    virtual void onMQTT(char *topic, byte *payload, unsigned int length);
    friend void taskWeb(void* arg);

private:
    void _start();
    void _stop();

    Callback         *_callback;
    QueueHandle_t    _queue_comm;
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

    MQTTWorker      *_mqtt;
    CameraWorker    *_cam;
    std::vector<String> _list_sd;
    std::list<mount_t> _list_mnt;
};

#endif