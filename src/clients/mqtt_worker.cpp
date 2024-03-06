#include "debug.h"
#include "mqtt_worker.h"

MQTTWorker::MQTTWorker(char *ip, char *acccessCode, char *serial) {
    _seq_id = 0;
    _ip = ip;
    _access = acccessCode;
    _serial = serial;
}

uint32_t MQTTWorker::seq_id() {
    _seq_id = (_seq_id + 1) % (UINT32_MAX - 1);
    return _seq_id;
}

void MQTTWorker::onMQTT(char *topic, byte *payload, unsigned int length) {
    payload[length] = '\0';
    if (_callback) {
        _callback->onMQTT(topic, payload, length);
    }
}

char *MQTTWorker::generateRandomID(char *key, int len) {
    char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; i++) {
        int randomIndex = random(sizeof(charset) - 1);
        key[i] = charset[randomIndex];
    }
    key[len] = '\0';

    return key;
}

void MQTTWorker::start() {
    _mqtt_client_secure = new WiFiClientSecure();
    _mqtt_client = new PubSubClient(*_mqtt_client_secure);

    _mqtt_client_secure->setInsecure();
    _mqtt_client->setServer(_ip, 8883);
    _mqtt_client->setBufferSize(6 * 1024); // 6KB for mqtt message output
    // _mqtt_client->setStream(stream);

    std::function<void(char *, byte *, unsigned int)> f =
        std::bind(&MQTTWorker::onMQTT, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    _mqtt_client->setCallback(f);
    _mqtt_client->setSocketTimeout(20);
}

void MQTTWorker::stop() {
    if (_mqtt_client->connected()) {
        _mqtt_client->disconnect();
    }
    delete _mqtt_client;
    delete _mqtt_client_secure;
}

void MQTTWorker::publish(JsonDocument json) {
    String result;

    if (_mqtt_client->connected()) {
        serializeJson(json, result);
        _mqtt_client->publish(_topic_request.c_str(), result.c_str());
        _mqtt_client->flush();
    }
}

void MQTTWorker::reqPushAll() {
    JsonDocument json;

    json["pushing"]["command"] = "pushall";
    json["pushing"]["version"] = 1;
    json["pushing"]["push_target"] = 1;
    json["pushing"]["sequence_id"] = seq_id_str();
    json["user_id"] = "123456789";
    publish(json);
}

void MQTTWorker::reqDeviceVersion() {
    JsonDocument json;

    json["info"]["command"] = "get_version";
    publish(json);
}

bool MQTTWorker::loop() {
    bool ret = false;

    if (!_mqtt_client->connected()) {
        char key[17];

        generateRandomID(key, 16);
        if (_mqtt_client->connect(key, "bblp", _access)) {
            LOGI("MQTT connected\n");
            String device = "device/" + String(_serial);
            _topic_request = device + "/request";
            _topic_report  = device + "/report";
            _mqtt_client->subscribe(_topic_report.c_str());

            reqPushAll();
            reqDeviceVersion();
            ret = true;
        } else {
            LOGE("MQTT status : %d\n", _mqtt_client->state());
            ret = false;
        }
    } else {
        ret = _mqtt_client->loop();
    }
    return ret;
}
