#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <ArduinoJson.h>

class Config {
public:
    Config(char *fname=NULL);
    bool setup();
    bool save();

    JsonDocument json()             { return _json; }

    String getAdminPassword()       { return _json["admin"]; }

    String getWifiSSID()            { return _json["wifi"]["ssid"].as<String>(); }
    String getWifiPassword()        { return _json["wifi"]["password"].as<String>(); }

    String getPBToken()             { return _json["pushbullet"].as<String>(); }
    void   setPBToken(String token) { _json["pushbullet"] = token; }

private:
    JsonDocument _json;
    char *_fname;
};

#endif