#include "ESP32_FTPSClient.h"
#include <WiFiClientSecure.h>
#include "debug.h"


ESP32_FTPSClient::ESP32_FTPSClient(char *_serverAdress, uint16_t _port, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose) {
    userName = _userName;
    passWord = _passWord;
    serverAdress = _serverAdress;
    port = _port;
    timeout = _timeout;
    verbose = _verbose;
}

ESP32_FTPSClient::ESP32_FTPSClient(char *_serverAdress, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose) {
    userName = _userName;
    passWord = _passWord;
    serverAdress = _serverAdress;
    port = 21;
    timeout = _timeout;
    verbose = _verbose;
}

WiFiClientSecure *ESP32_FTPSClient::GetDataClient() {
    return &dclient;
}

bool ESP32_FTPSClient::isConnected() {
    if (!_isConnected) {
        LOGE("FTP error: %s\n", outBuf);
    }

    return _isConnected;
}

void ESP32_FTPSClient::sendCmd(char *format, ...) {
    uint8_t buf[64];

    memset(buf, 0, sizeof(buf));
    va_list arg;
    va_start(arg, format);
    int len = vsnprintf((char*)buf, sizeof(buf), format, arg);
    va_end(arg);

    buf[len] = '\n';
    client.write(buf, len + 1);
    LOGV("%s", buf);
}

void ESP32_FTPSClient::GetLastModifiedTime(const char *fileName, char *result) {
    if (!isConnected()) return;
    sendCmd((char*)"MDTM %s", fileName);
    GetFTPAnswer(result, 4);
}

void ESP32_FTPSClient::WriteClientBuffered(WiFiClientSecure *cli, unsigned char *data, int dataLength) {
    if (!isConnected()) return;

    size_t clientCount = 0;
    for (int i = 0; i < dataLength; i++) {
        clientBuf[clientCount] = data[i];
        clientCount++;
        if (clientCount > bufferSize - 1) {
            cli->write(clientBuf, bufferSize);
            clientCount = 0;
        }
    }
    if (clientCount > 0) {
        cli->write(clientBuf, clientCount);
    }
}

void ESP32_FTPSClient::GetFTPAnswer(char *result, int offsetStart) {
    char thisByte;
    outCount = 0;

    unsigned long _m = millis();
    while (!client.available() && millis() < _m + timeout)
        delay(1);

    if (!client.available()) {
        memset(outBuf, 0, sizeof(outBuf));
        strcpy(outBuf, "Offline");

        _isConnected = false;
        isConnected();
        return;
    }

    while (client.available()) {
        thisByte = client.read();
        if (outCount < sizeof(outBuf)) {
            outBuf[outCount] = thisByte;
            outCount++;
            outBuf[outCount] = 0;
        }
    }

    if (outBuf[0] == '4' || outBuf[0] == '5') {
        LOGE("Error response : %s\n", outBuf);
        _isConnected = false;
        isConnected();
        return;
    } else {
        _isConnected = true;
        LOGV("response : %s\n", outBuf);
    }

    if (result != NULL) {
        // Deprecated
        for (int i = offsetStart; i < sizeof(outBuf); i++) {
            result[i] = outBuf[i - offsetStart];
        }
        LOGV("Result: %s\n", outBuf);
    }
}

void ESP32_FTPSClient::WriteData(unsigned char *data, int dataLength) {
    LOGV("Writing\n");
    if (!isConnected()) return;
    WriteClientBuffered(&dclient, &data[0], dataLength);
}

void ESP32_FTPSClient::CloseFile() {
    LOGV("Close File");
    dclient.stop();

    if (!_isConnected) return;

    GetFTPAnswer();
}

void ESP32_FTPSClient::Write(const char *str) {
    LOGV("Write File");
    if (!isConnected()) return;

    GetDataClient()->print(str);
}

void ESP32_FTPSClient::CloseConnection() {
    sendCmd((char*)"QUIT");
    client.stop();
    LOGV("Connection closed");
}

void ESP32_FTPSClient::OpenConnection(bool secure, bool implicit) {
    LOGV("Connecting to: %s\n", serverAdress);

    _is_secure = secure;
    _is_implicit = implicit;
    if (!_is_secure) {
        client.setInsecure();
    }

    if (client.connect(serverAdress, port, timeout)) {
        LOGV("Command connected\n");
    }
    GetFTPAnswer();

    sendCmd((char*)"USER %s", userName);
    GetFTPAnswer();

    sendCmd((char*)"PASS %s", passWord);
    GetFTPAnswer();

    //   LOGV("Send ACCT");
    //   sendCmd((char*)"ACCT ", "");
    //   client.print("ACCT "));
    //   GetFTPAnswer();

    if (_is_implicit) {
        sendCmd((char*)"PBSZ 0");
        GetFTPAnswer();
        sendCmd((char*)"PROT P");
        GetFTPAnswer();
    } else {
        // sendCmd((char*)"SYST");
        // GetFTPAnswer();
    }
}

void ESP32_FTPSClient::RenameFile(char *from, char *to) {
    if (!isConnected()) return;
    sendCmd((char*)"RNFR %s", from);
    GetFTPAnswer();

    sendCmd((char*)"RNTO %s", to);
    GetFTPAnswer();
}

void ESP32_FTPSClient::NewFile(const char *fileName) {
    if (!isConnected()) return;
    sendCmd((char*)"STOR %s", fileName);
    GetFTPAnswer();
}

void ESP32_FTPSClient::InitFile(const char *type) {
    if (!isConnected()) return;
    sendCmd((char *)type);
    GetFTPAnswer();

    sendCmd((char *)"PASV");
    GetFTPAnswer();

    char *tStr = strtok(outBuf, "(,");
    int array_pasv[6];
    for (int i = 0; i < 6; i++) {
        tStr = strtok(NULL, "(,");
        if (tStr == NULL) {
            LOGE("Bad PASV Answer\n");
            CloseConnection();
            return;
        }
        array_pasv[i] = atoi(tStr);
    }
    unsigned int hiPort, loPort;
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;

    IPAddress pasvServer(array_pasv[0], array_pasv[1], array_pasv[2], array_pasv[3]);

    hiPort = hiPort | loPort;
    LOGV("Data port: %u\n", hiPort);
    if (!_is_secure) {
        dclient.setInsecure();
    }
    if (dclient.connect(pasvServer, hiPort, timeout)) {
        LOGV("Data connection established\n");
    }
}

void ESP32_FTPSClient::AppendFile(char *fileName) {
    if (!isConnected()) return;
    sendCmd((char*)"APPE %s", fileName);
    GetFTPAnswer();
}

void ESP32_FTPSClient::ChangeWorkDir(const char *dir) {
    if (!isConnected()) return;
    sendCmd((char*)"CWD %s", dir);
    GetFTPAnswer();
}

void ESP32_FTPSClient::DeleteFile(const char *file) {
    if (!isConnected()) return;
    sendCmd((char*)"DELE %s", file);
    GetFTPAnswer();
}

void ESP32_FTPSClient::MakeDir(const char *dir) {
    if (!isConnected()) return;
    sendCmd((char*)"MKD %s", dir);
    GetFTPAnswer();
}

void ESP32_FTPSClient::DirShort(const char *dir, std::vector<String*> &list) {
    char _resp[sizeof(outBuf)];
    uint16_t _b = 0;

    if(!isConnected()) return;
    sendCmd((char*)"NLST %s", (char*)dir);
    GetFTPAnswer(_resp);

    // Convert char array to string to manipulate and find response size
    // each server reports it differently, TODO = FEAT
    // String resp_string = _resp;
    // resp_string.substring(resp_string.lastIndexOf('matches')-9);
    // LOGV(resp_string);

    unsigned long _m = millis();
    while (!dclient.available() && millis() < _m + timeout)
        delay(1);

    while (dclient.available()) {
        String *line = new String(dclient.readStringUntil('\n'));
        // String line = dclient.readStringUntil('\n');
        line->trim();
        list.push_back(line);
    }
    GetFTPAnswer();
    dclient.stop();
}

void ESP32_FTPSClient::DirLong(const char *dir, std::vector<String*> &list) {
    char _resp[sizeof(outBuf)];
    uint16_t _b = 0;

    if (!isConnected()) return;
    sendCmd((char*)"LIST %s", (char*)dir);
    GetFTPAnswer(_resp);

    unsigned long _m = millis();
    while (!dclient.available() && millis() < _m + timeout)
        delay(1);

    list.clear();
    while (dclient.available()) {
        // String line = dclient.readStringUntil('\n');
        // line.trim();
        String *line = new String(dclient.readStringUntil('\n'));
        line->trim();
        list.push_back(line);
    }
    GetFTPAnswer();
    dclient.stop();
}

void ESP32_FTPSClient::DownloadString(const char *filename, String &str) {
    if (!isConnected()) return;
    sendCmd((char*)"RETR %s", filename);

    char _resp[sizeof(outBuf)];
    GetFTPAnswer(_resp);

    unsigned long _m = millis();
    while (!GetDataClient()->available() && millis() < _m + timeout)
        delay(1);

    while (GetDataClient()->available()) {
        str += GetDataClient()->readString();
    }

    GetFTPAnswer();
    dclient.stop();
}

void ESP32_FTPSClient::DownloadFile(const char *filename, unsigned char *buf, size_t length, bool printUART) {
    if (!isConnected()) return;
    sendCmd((char*)"RETR %s", filename);

    char _resp[sizeof(outBuf)];
    GetFTPAnswer(_resp);

    unsigned long _m = millis();
    while (!dclient.available() && millis() < _m + timeout)
        delay(1);

    while (dclient.available()) {
        if (!printUART) {
            dclient.readBytes(buf, length);
        } else {
            char _buf[2];

            for (size_t _b = 0; _b < length; _b++) {
                dclient.readBytes(_buf, 1);
                // Serial.print(_buf[0], HEX);
            }
        }
    }
}

void ESP32_FTPSClient::DownloadFile(const char *filename, size_t length, File *dest) {
    if (!isConnected()) return;

    sendCmd((char*)"RETR %s", filename);
    char _resp[sizeof(outBuf)];
    GetFTPAnswer(_resp);

    unsigned long _m = millis();
    while (!dclient.available() && millis() < _m + timeout)
        delay(1);

    uint8_t  buf[256];
    uint16_t sz;

    while (length > 0) {
        sz = min(sizeof(buf), (unsigned int)length);
        int len = dclient.readBytes(buf, sz);
        length -= len;
        if (dest)
            dest->write(buf, len);
    }

    GetFTPAnswer();
    dclient.stop();
}
