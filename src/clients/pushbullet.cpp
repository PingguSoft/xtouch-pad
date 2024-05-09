#include "pushbullet.h"
#include "debug.h"
#include <FS.h>
#include <SD.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

String PushBullet::filetype(String filename) {
    if (filename.endsWith(".jpg")) {
        return ("image/jpeg");
    } else if (filename.endsWith(".png")) {
        return ("image/png");
    }
    return "text/plain";
}

String PushBullet::upload(String filename) {
    JsonDocument json_out;
    JsonDocument json_resp;
    String json_str;
    String upload_url;
    String file_url="";

    json_out["file_name"] = filename;
    json_out["file_type"] = filetype(filename);
    serializeJson(json_out, json_str);
    LOGD("upload:%s\n", json_str.c_str());

    _client.setInsecure();
    _https.begin(_client, "https://api.pushbullet.com/v2/upload-request");
    _https.addHeader("Content-Type", "application/json");
    _https.addHeader("Access-Token", _accesstoken);
    int ret = _https.POST(json_str);
    if (ret > 0) {
        LOGD("HTTP response code: %d\n", ret);
        String response = _https.getString();
        LOGD("Response:%s\n", response.c_str());

        auto err = deserializeJson(json_resp, response.c_str(), response.length());
        // if (!err) {
        //     if (json_resp.containsKey("data")) {
        //         json_resp = json_resp["data"];
        //     }
        // }
    } else {
        LOGE("HTTP response code: %d\n", ret);
    }
    _https.end();
    delay(300);

    if (json_resp.containsKey("file_url")) {
        file_url = json_resp["file_url"].as<String>();
        LOGD("file_url:%s\n", file_url.c_str());
    }
    if (json_resp.containsKey("upload_url")) {
        upload_url = json_resp["upload_url"].as<String>();
        LOGD("upload_url:%s\n", upload_url.c_str());
    }

    if (!upload_url.isEmpty()) {
        _client.setInsecure();

        String fname = "/image/" + filename;
        if (SD.exists(fname)) {
            File imgfile = SD.open((fname), "r");
            String boundary = "------------------------" + String(random(0xFFFFFF), HEX);
            String requestBody = "";

            // object
            requestBody += "--" + boundary + "\r\n";
            requestBody += "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n";
            requestBody += "Content-Type: " + filetype(filename) + "\r\n\r\n";
            // requestBody += "Content-Transfer-Encoding: binary\r\n";
            requestBody += imgfile.readString();
            requestBody += "\r\n";
            requestBody += "--" + boundary + "--\r\n";
            LOGD("BODY:\n%s\n", requestBody.c_str());

            // send
            _https.begin(_client, upload_url);
            // _https.addHeader("Accept-Encoding", "gzip, deflate");
            _https.addHeader("Accept", "*/*");
            _https.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
            int ret = _https.sendRequest("POST", requestBody);
            if (ret > 0) {
                LOGD("HTTP response code: %d\n", ret);
                if (ret != HTTP_CODE_NO_CONTENT) {
                    file_url = "";
                    String response = _https.getString();
                    LOGD("Response:%s\n", response.c_str());
                }
            }
            imgfile.close();
            _https.end();
            LOGD("upload completed\n");
            delay(300);

        } else {
            LOGE("file not found : %s\n", fname.c_str());
        }
    }

    return file_url;
}

void PushBullet::notify(String title, String body, String filename) {
    JsonDocument json_out;
    String json_str;
    bool is_file = !filename.isEmpty();

    json_out["body"] = body;
    json_out["title"] = title;
    json_out["type"] = "note";

    if (is_file) {
        String url = upload(filename);
        if (!url.isEmpty()) {
            json_out["type"] = "file";
            json_out["file_type"] = filetype(filename);
            json_out["file_name"] = filename;
            json_out["file_url"] = url;
        } else {
            LOGE("file upload fails:%s\n", filename.c_str());
        }
    }

    serializeJson(json_out, json_str);
    LOGD("notify:%s\n", json_str.c_str());

    _client.setInsecure();
    _https.begin(_client, "https://api.pushbullet.com/v2/pushes");
    _https.addHeader("Content-Type", "application/json");
    _https.addHeader("Access-Token", _accesstoken);
    int ret = _https.POST(json_str);
    if (ret > 0) {
        LOGD("HTTP response code: %d\n", ret);
        String response = _https.getString();
        LOGD("Response:%s\n", response.c_str());
    } else {
        LOGE("HTTP response code: %d\n", ret);
    }
    _https.end();
}
