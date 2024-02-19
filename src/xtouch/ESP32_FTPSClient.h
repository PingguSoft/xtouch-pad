
#include <WiFiClientSecure.h>
#include <FS.h>
#include <vector>

class ESP32_FTPSClient
{
  private:
  void WriteClientBuffered(WiFiClientSecure* cli, unsigned char * data, int dataLength);
  char outBuf[128];
  unsigned char outCount;
  WiFiClientSecure client;
  WiFiClientSecure dclient;
  uint8_t verbose;

/*
  template<typename T>
  		void FTPdbg(T msg) {
    	if(verbose == 2) Serial.print(msg);
		}

  template<typename T>
  		void FTPdbgn(T msg) {
    	if(verbose == 2) Serial.println(msg);
		}

    template<typename T>
    void FTPerr(T msg) {
    if(verbose == 1 || verbose == 2) Serial.print(msg);
  }
*/

  char* userName;
  char* passWord;
  char* serverAdress;
  uint16_t port;
  bool _isConnected = false;
  bool _is_secure = true;
  bool _is_implicit = false;
  unsigned char clientBuf[1500];
  size_t bufferSize = 1500;
  uint16_t timeout = 10000;
  WiFiClientSecure* GetDataClient();
  void sendCmd(char *format, ...);

  public:
  ESP32_FTPSClient(char* _serverAdress, uint16_t _port, char* _userName, char* _passWord, uint16_t _timeout = 10000, uint8_t _verbose = 1);
  ESP32_FTPSClient(char* _serverAdress, char* _userName, char* _passWord, uint16_t _timeout = 10000, uint8_t _verbose = 1);
  void OpenConnection(bool secure, bool implicit);
  void CloseConnection();
  bool isConnected();
  void NewFile (const char* fileName);
  void AppendFile( char* fileName);
  void WriteData (unsigned char * data, int dataLength);
  void CloseFile ();
  void GetFTPAnswer (char* result = NULL, int offsetStart = 0);
  void GetLastModifiedTime(const char* fileName, char* result);
  void RenameFile(char* from, char* to);
  void Write(const char * str);
  void InitFile(const char* type);
  void ChangeWorkDir(const char * dir);
  void DeleteFile(const char * file);
  void MakeDir(const char * dir);
  void DirShort(const char * dir, std::vector<String*> &list);
  void DirLong(const char * dir, std::vector<String*> &list);
  void DownloadString(const char * filename, String &str);
  void DownloadFile(const char * filename, unsigned char * buf, size_t length, bool printUART = false);

  void DownloadFile(const char *filename, size_t length, File *dest);
};
