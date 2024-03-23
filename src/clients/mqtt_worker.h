#ifndef __MQTT_WORKER__
#define __MQTT_WORKER__

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

class MQTTWorker {
public:
    class Callback {
        public:
            virtual ~Callback() { }
            virtual void onMQTT(char *topic, byte *payload, unsigned int length) = 0;
    };

    MQTTWorker(char *ip, char *acccessCode, char *serial);
    void start();
    void stop();
    void onMQTT(char *topic, byte *payload, unsigned int length);
    bool loop();
    void publish(JsonDocument json, char *user=NULL);
    void setCallback(Callback *cb) { _callback = cb; }
    void reqPushAll();
    void reqDeviceVersion();


private:
    char *generateRandomID(char *key, int len);
    uint32_t seq_id();
    String seq_id_str() { return String(seq_id()); }

    WiFiClientSecure *_mqtt_client_secure;
    PubSubClient     *_mqtt_client;
    Callback         *_callback;
    char *_ip;
    char *_access;
    char *_serial;
    uint32_t    _seq_id;

    String _topic_request;
    String _topic_report;
};

#endif