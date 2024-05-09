#include <SPIFFS.h>
#include "config.h"
#include "debug.h"

Config::Config(char *fname) {
    _fname = fname ? fname : (char*)"/config.json";

    _json["admin"] = "admin";
    _json["pushbullet"]["token"] = "";

    _json["wifi"]["ssid"] = "";
    _json["wifi"]["password"] = "";
}

bool Config::setup() {
    if (SPIFFS.exists(_fname)) {
        File cfgFile = SPIFFS.open(_fname);
        if (cfgFile) {
            DeserializationError error = deserializeJson(_json, cfgFile);
            cfgFile.close();
            if (error) {
                LOGE("error json file in %s\n", _fname);
                return false;
            }
        } else {
            LOGE("file open error:%s\n", _fname);
            return false;
        }
        // print all key:value pairs
        for (JsonPair kv : _json.as<JsonObject>()) {
            LOGD("%s:%s\n", kv.key().c_str(), kv.value().as<String>().c_str());
        }
    } else {
        LOGE("file not found:%s\n", _fname);
        return save();
    }
    return true;
}

bool Config::save() {
    File cfgFile = SPIFFS.open(_fname, FILE_WRITE);
    if (cfgFile) {
        serializeJson(_json, cfgFile);
        cfgFile.close();
    } else {
        LOGE("file write error:%s\n", _fname);
        return false;
    }
    return true;
}
