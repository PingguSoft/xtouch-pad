#include "pushbullet.h"
#include "debug.h"
#include <FS.h>
#include <SD.h>
#include <ArduinoJson.h>

#define PBFINGERPRINT "bbfc9f1bc13cd996f268a2e34129d1478fb933be"
#define HTTPPORT      443
#define PBHOST        "api.pushbullet.com"

String PushBullet::filetype(String filename) {
    if (filename.endsWith(".jpg")) {
        return ("image/jpeg");
    } else if (filename.endsWith(".png")) {
        return ("image/png");
    }
    return "plain/text";
}

void PushBullet::connect() {
    LOGD("connecting to %s\n", PBHOST);
    // code for connecting to pushbullet
    _client.setInsecure();
    if (!_client.connect(PBHOST, HTTPPORT)) {
        LOGE("connection failed\n");
        return;
    }
    // checks if the certification matches
    if (_client.verify(PBFINGERPRINT, PBHOST)) {
        // LOGD("certificate matches");
        return;
    } else {
        LOGE("certificate doesn't match\n");
    }
}

void PushBullet::notify(String title, String body) {
    // String url = "/v2/pushes";
    // String data = {"{\"body\":\"" + body + "\",\"title\":\"" + title + "\",\"type\":\"note\"}"};

    JsonDocument json_out;
    String json_str;

    json_out["body"] = body;
    json_out["title"] = title;
    json_out["type"] = "note";
    serializeJson(json_out, json_str);
    LOGD("notify:%s\n", json_str.c_str());

    _client.setInsecure();
    _https.begin(_client, "https://api.pushbullet.com/v2/pushes");
    _https.addHeader("Content-Type", "application/json");
    _https.addHeader("Access-Token", _accesstoken);
    int httpResponseCode = _https.POST(json_str);
    if (httpResponseCode > 0) {
        LOGD("HTTP response code: %d\n", httpResponseCode);
        String response = _https.getString();
        LOGD("Response:%s\n", response.c_str());
    } else {
        LOGE("HTTP response code: %d\n", httpResponseCode);
    }
    _https.end();
}

String PushBullet::upload(String filename) {
    JsonDocument json_out;
    JsonDocument json_in;
    String json_str;
    String upload_url;
    String file_url;

    json_out["file_name"] = filename;
    json_out["file_type"] = filetype(filename);
    serializeJson(json_out, json_str);
    LOGD("upload:%s\n", json_str.c_str());

    _client.setInsecure();
    _https.begin(_client, "https://api.pushbullet.com/v2/upload-request");
    _https.addHeader("Content-Type", "application/json");
    _https.addHeader("Access-Token", _accesstoken);
    int httpResponseCode = _https.POST(json_str);
    if (httpResponseCode > 0) {
        LOGD("HTTP response code: %d\n", httpResponseCode);
        String response = _https.getString();
        LOGD("Response:%s\n", response.c_str());

        auto err = deserializeJson(json_in, response.c_str(), response.length());
        if (!err) {
            if (json_in.containsKey("file_url")) {
                file_url = json_in["file_url"].as<String>();
                LOGD("file_url:%s\n", file_url.c_str());
            }
            if (json_in.containsKey("upload_url")) {
                upload_url = json_in["upload_url"].as<String>();
                LOGD("upload_url:%s\n", upload_url.c_str());
            }
        }
    } else {
        LOGE("HTTP response code: %d\n", httpResponseCode);
    }
    _https.end();

    if (!upload_url.isEmpty()) {
        _client.setInsecure();

        String data = String("--710ff0c6cf2d4c73b12db64cab12e58c\r\n") +
                String("Content-Disposition:  form-data; name=\"file\"; filename=\"" + filename + "\"\r\n") +
                String("Content-Type: " + filetype(filename) + "\r\n\r\n");

        const char *tail = "\r\n--710ff0c6cf2d4c73b12db64cab12e58c--\r\n";


        String fname = "/image/" + filename;
        if (SD.exists(fname)) {
            File imgfile = SD.open((fname), "r");
            _https.begin(_client, upload_url);
            _https.addHeader("Access-Token", _accesstoken);
            _https.addHeader("Accept-Encoding", "gzip, deflate");
            // _https.addHeader("Accept-Charset", "utf-8;");
            _https.addHeader("Accept", "*/*");
            _https.addHeader("Connection", "keep-alive");
            _https.addHeader("Content-Length", String(imgfile.size() + strlen(tail) + data.length()));
            _https.addHeader("Content-Type", "multipart/form-data; boundary=710ff0c6cf2d4c73b12db64cab12e58c");

            int httpResponseCode = _https.POST(data);
            if (httpResponseCode > 0) {
                LOGD("HTTP response code: %d\n", httpResponseCode);
                String response = _https.getString();
                LOGD("Response:%s\n", response.c_str());

                if (httpResponseCode == 200) {
                    const int bufSize = 512;
                    char buf[bufSize];
                    int sz = 0;
                    int rem = imgfile.size();

                    while (rem > 0) {
                        sz = imgfile.readBytes(buf, min(bufSize, rem));
                        if (sz > 0) {
                            _client.write((const uint8_t *)buf, sz);
                            rem -= sz;
                            LOGD("TRANSFER: %d/%d\n", sz, rem);
                            delay(100);
                        }
                    }
                    imgfile.close();


                    _client.write((const uint8_t *)tail, strlen(tail));
                    _client.flush();
                    delay(500);
                }
                _https.end();

                LOGD("transfer completed..\n");
            } else {
                LOGE("file not found : %s\n", fname.c_str());
            }
        }
    }

    return file_url;
}

void PushBullet::pushFile(String title, String body, String filename) {
    JsonDocument json_out;
    String json_str;

    String url = upload(filename);
    if (url.isEmpty())
        return;

    json_out["body"] = body;
    json_out["title"] = title;
    json_out["type"] = "file";
    json_out["file_type"] = filetype(filename);
    json_out["file_name"] = filename;
    json_out["file_url"] = url;

    serializeJson(json_out, json_str);
    LOGD("notify:%s\n", json_str.c_str());

    _client.setInsecure();
    _https.begin(_client, "https://api.pushbullet.com/v2/pushes");
    _https.addHeader("Content-Type", "application/json");
    _https.addHeader("Access-Token", _accesstoken);
    int httpResponseCode = _https.POST(json_str);
    if (httpResponseCode > 0) {
        LOGD("HTTP response code: %d\n", httpResponseCode);
        String response = _https.getString();
        LOGD("Response:%s\n", response.c_str());
    } else {
        LOGE("HTTP response code: %d\n", httpResponseCode);
    }
    _https.end();
}