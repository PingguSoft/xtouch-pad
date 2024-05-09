#ifndef __PUSHBULLET__
#define __PUSHBULLET__

#include <list>
#include <FS.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"

class PushBullet {
public:
    PushBullet(String accesstoken) {
        _accesstoken = accesstoken;
    }
    void notify(String title, String body, String filename="");

private:
    String upload(String filename);
    String filetype(String filename);

    String  _accesstoken;
    WiFiClientSecure _client;
    HTTPClient _https;
};

#endif