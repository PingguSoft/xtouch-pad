#include "ESP32_SFTPClient.h"
#include <WiFiClientSecure.h>


ESP32_SFTPClient::ESP32_SFTPClient(char *_serverAdress, uint16_t _port, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose) {
    userName = _userName;
    passWord = _passWord;
    serverAdress = _serverAdress;
    port = _port;
    timeout = _timeout;
    verbose = _verbose;
}

ESP32_SFTPClient::ESP32_SFTPClient(char *_serverAdress, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose) {
    userName = _userName;
    passWord = _passWord;
    serverAdress = _serverAdress;
    port = 21;
    timeout = _timeout;
    verbose = _verbose;
}

WiFiClientSecure *ESP32_SFTPClient::GetDataClient() {
    return &dclient;
}

bool ESP32_SFTPClient::isConnected() {
    if (!_isConnected) {
        FTPerr("FTP error: ");
        FTPerr(outBuf);
        FTPerr("\n");
    }

    return _isConnected;
}

void ESP32_SFTPClient::sendCmd(char *format, ...) {
    uint8_t buf[64];

    memset(buf, 0, sizeof(buf));
    va_list arg;
    va_start(arg, format);
    int len = vsnprintf((char*)buf, sizeof(buf), format, arg);
    va_end(arg);

    client.write(buf, strlen((char*)buf));
    FTPdbgn((char*)buf);
}

void ESP32_SFTPClient::GetLastModifiedTime(const char *fileName, char *result) {
    if (!isConnected()) return;
    sendCmd((char*)"MDTM %s", fileName);
    GetFTPAnswer(result, 4);
}

void ESP32_SFTPClient::WriteClientBuffered(WiFiClientSecure *cli, unsigned char *data, int dataLength) {
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

void ESP32_SFTPClient::GetFTPAnswer(char *result, int offsetStart) {
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

    printf(outBuf);
    if (outBuf[0] == '4' || outBuf[0] == '5') {
        _isConnected = false;
        isConnected();
        return;
    } else {
        _isConnected = true;
    }

    if (result != NULL) {
        FTPdbgn("Result start");
        // Deprecated
        for (int i = offsetStart; i < sizeof(outBuf); i++) {
            result[i] = outBuf[i - offsetStart];
        }
        FTPdbg("Result: ");
        // Serial.write(result);
        FTPdbg(outBuf);
        FTPdbgn("Result end");
    }
}

void ESP32_SFTPClient::WriteData(unsigned char *data, int dataLength) {
    FTPdbgn(F("Writing"));
    if (!isConnected()) return;
    WriteClientBuffered(&dclient, &data[0], dataLength);
}

void ESP32_SFTPClient::CloseFile() {
    FTPdbgn(F("Close File"));
    dclient.stop();

    if (!_isConnected) return;

    GetFTPAnswer();
}

void ESP32_SFTPClient::Write(const char *str) {
    FTPdbgn(F("Write File"));
    if (!isConnected()) return;

    GetDataClient()->print(str);
}

void ESP32_SFTPClient::CloseConnection() {
    sendCmd((char*)"QUIT");
    client.stop();
    FTPdbgn(F("Connection closed"));
}

void ESP32_SFTPClient::OpenConnection(bool secure, bool implicit) {
    FTPdbg(F("Connecting to: "));
    FTPdbgn(serverAdress);

    _is_secure = secure;
    _is_implicit = implicit;
    if (!_is_secure) {
        client.setInsecure();
    }

    if (client.connect(serverAdress, port, timeout)) {
        FTPdbgn(F("Command connected"));
    }
    GetFTPAnswer();

    sendCmd((char*)"USER %s", userName);
    GetFTPAnswer();

    sendCmd((char*)"PASS %s", passWord);
    GetFTPAnswer();

    //   FTPdbgn("Send ACCT");
    //   sendCmd((char*)"ACCT ", "");
    //   client.print(F("ACCT "));
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

void ESP32_SFTPClient::RenameFile(char *from, char *to) {
    if (!isConnected()) return;
    sendCmd((char*)"RNFR %s", from);
    GetFTPAnswer();

    sendCmd((char*)"RNTO %s", to);
    GetFTPAnswer();
}

void ESP32_SFTPClient::NewFile(const char *fileName) {
    if (!isConnected()) return;
    sendCmd((char*)"STOR %s", fileName);
    GetFTPAnswer();
}

void ESP32_SFTPClient::InitFile(const char *type) {
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
            FTPdbgn(F("Bad PASV Answer"));
            CloseConnection();
            return;
        }
        array_pasv[i] = atoi(tStr);
    }
    unsigned int hiPort, loPort;
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;

    IPAddress pasvServer(array_pasv[0], array_pasv[1], array_pasv[2], array_pasv[3]);

    FTPdbg(F("Data port: "));
    hiPort = hiPort | loPort;
    FTPdbgn(hiPort);
    if (!_is_secure) {
        dclient.setInsecure();
    }
    if (dclient.connect(pasvServer, hiPort, timeout)) {
        FTPdbgn(F("Data connection established"));
    }
}

void ESP32_SFTPClient::AppendFile(char *fileName) {
    if (!isConnected()) return;
    sendCmd((char*)"APPE %s", fileName);
    GetFTPAnswer();
}

void ESP32_SFTPClient::ChangeWorkDir(const char *dir) {
    if (!isConnected()) return;
    sendCmd((char*)"CWD %s", dir);
    GetFTPAnswer();
}

void ESP32_SFTPClient::DeleteFile(const char *file) {
    if (!isConnected()) return;
    sendCmd((char*)"DELE %s", file);
    GetFTPAnswer();
}

void ESP32_SFTPClient::MakeDir(const char *dir) {
    if (!isConnected()) return;
    sendCmd((char*)"MKD %s", dir);
    GetFTPAnswer();
}

void ESP32_SFTPClient::DirShort(const char *dir, String *list) {
    char _resp[sizeof(outBuf)];
    uint16_t _b = 0;

    if(!isConnected()) return;
    sendCmd((char*)"NLST %s", (char*)dir);
    GetFTPAnswer(_resp);

    // Convert char array to string to manipulate and find response size
    // each server reports it differently, TODO = FEAT
    // String resp_string = _resp;
    // resp_string.substring(resp_string.lastIndexOf('matches')-9);
    // FTPdbgn(resp_string);

    unsigned long _m = millis();
    while (!dclient.available() && millis() < _m + timeout)
        delay(1);

    while (dclient.available()) {
        if (_b < 128) {
            list[_b++] = dclient.readStringUntil('\n');
            // String tmp = dclient.readStringUntil('\n');
            // list[_b++] = tmp.substring(tmp.lastIndexOf(" ") + 1, tmp.length());
        }
    }
    GetFTPAnswer();
}

void ESP32_SFTPClient::DirLong(const char *dir, String *list) {
    char _resp[sizeof(outBuf)];
    uint16_t _b = 0;

    if (!isConnected()) return;
    sendCmd((char*)"LIST %s", (char*)dir);
    GetFTPAnswer(_resp);

    // Convert char array to string to manipulate and find response size
    // each server reports it differently, TODO = FEAT
    // String resp_string = _resp;
    // resp_string.substring(resp_string.lastIndexOf('matches')-9);
    // FTPdbgn(resp_string);

    unsigned long _m = millis();
    while (!dclient.available() && millis() < _m + timeout)
        delay(1);

    while (dclient.available()) {
        if (_b < 128) {
            list[_b++] = dclient.readStringUntil('\n');
        }
    }
    GetFTPAnswer();
}

void ESP32_SFTPClient::DownloadString(const char *filename, String &str) {
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
}

void ESP32_SFTPClient::DownloadFile(const char *filename, unsigned char *buf, size_t length, bool printUART) {
    if (!isConnected()) return;
    sendCmd((char*)"RETR %s", filename);

    char _resp[sizeof(outBuf)];
    GetFTPAnswer(_resp);

    char _buf[2];

    unsigned long _m = millis();
    while (!dclient.available() && millis() < _m + timeout)
        delay(1);

    while (dclient.available()) {
        if (!printUART)
            dclient.readBytes(buf, length);

        else {
            for (size_t _b = 0; _b < length; _b++) {
                dclient.readBytes(_buf, 1);
                // Serial.print(_buf[0], HEX);
            }
        }
    }
}

void ESP32_SFTPClient::DownloadFile(const char *filename, size_t length, File *dest) {
    if (!isConnected()) return;

    sendCmd((char*)"RETR %s", filename);
    char _resp[sizeof(outBuf)];
    GetFTPAnswer(_resp);

    unsigned long _m = millis();
    while (!dclient.available() && millis() < _m + timeout)
        delay(1);

    uint8_t buf[1024];

    if (dest) {
        while (length > 0) {
            int len = dclient.readBytes(buf, 1024);
            length -= len;
            dest->write(buf, len);
        }
    }
}
