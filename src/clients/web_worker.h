#ifndef __WEB_WORKER__
#define __WEB_WORKER__

#include <list>
#include <vector>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"

class WebWorker {
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

    WebWorker(fs::FS *fs, char *root, uint16_t port);
    void start();
    void stop();
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    void listDirSD(char *path, std::vector<String> &info, String ext);
    void setCallback(Callback *cb) { _callback = cb; }
    String getSensorReadings();

    void connect();
    void disconnect();
    void loop();
    friend void taskWeb(void* arg);

private:
    void _start();
    void _stop();

    Callback         *_callback;
    QueueHandle_t    _queue_comm;
    char            *_web_root;
    AsyncWebServer  *_server;
    AsyncWebSocket  *_ws;
    fs::FS          *_fs;
    uint16_t        _port;
    long            _last_ts;
    int             _pos;

    std::vector<String> _list_sd;
};

#endif