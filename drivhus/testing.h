#ifndef _TESTING_H_
#define _TESTING_H_

#ifdef TESTING

#include <functional>


typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;
typedef long int time_t;
typedef long unsigned int size_t;

static constexpr uint8_t INPUT = 0;
static constexpr uint8_t OUTPUT = 1;
static constexpr uint8_t INPUT_PULLUP = 2;

static constexpr uint8_t LOW = 0;
static constexpr uint8_t HIGH = 1;

#define PROGMEM


void pinMode(uint8_t, uint8_t);
void analogReadResolution(uint8_t);
uint16_t analogRead(uint8_t);
int digitalRead(uint8_t);
void digitalWrite(uint8_t, uint8_t);
unsigned long millis();
time_t now();
void setTime(time_t);
void delay(uint32_t);

class String {
public:
  String(const char* = "");
  explicit String(float, unsigned int=2);
  const char* c_str() const;
  bool equals(const char*) const;
  bool operator ==(const char* cstr) const;
  bool startsWith(const String&) const;
  String substring(unsigned int) const;
  String substring(unsigned int, unsigned int) const;
};

#define SERIAL_8N1 0x800001c
#define SERIAL_8E1 0x800001e
class SerialClass {
public:
  void begin(unsigned long, uint32_t=0, int8_t=0, int8_t=0, bool=false, unsigned long=0UL, uint8_t=0);
  uint32_t baudRate();
  size_t println(const char[]);
};
extern SerialClass Serial;
extern SerialClass Serial1;
extern SerialClass Serial2;

class EEPROMClass {
public:
  bool begin(size_t);
  uint8_t read(int);
  void write(int, uint8_t);
  bool commit();
};
extern EEPROMClass EEPROM;

class TempAndHumidity {
public:
  float temperature;
  float humidity;
};
class DHTesp {
public:
  typedef enum {AUTO_DETECT, DHT11, DHT22, AM2302, RHT03 } DHT_MODEL_t;
  typedef enum {ERROR_NONE = 0, ERROR_TIMEOUT, ERROR_CHECKSUM} DHT_ERROR_t;
  void setup(uint8_t, DHT_MODEL_t);
  int getMinimumSamplingPeriod();
  TempAndHumidity getTempAndHumidity();
  DHT_ERROR_t getStatus();
private:
  TempAndHumidity temp_and_humidity;
};

enum week_t {Last, First, Second, Third, Fourth};
enum dow_t {Sun=1, Mon, Tue, Wed, Thu, Fri, Sat};
enum month_t {Jan=1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec};
struct TimeChangeRule
{
  char abbrev[6];
  uint8_t week;
  uint8_t dow;
  uint8_t month;
  uint8_t hour;
  int offset;
};
class Timezone {
public:
  Timezone(TimeChangeRule, TimeChangeRule);
  time_t toLocal(time_t);
};

typedef time_t(*getExternalTime)();
typedef enum {timeNotSet, timeNeedsSync, timeSet} timeStatus_t;
timeStatus_t timeStatus();
void setSyncProvider(getExternalTime);
void setSyncInterval(time_t);

typedef enum {WIFI_MODE_NULL=0, WIFI_MODE_STA, WIFI_MODE_AP, WIFI_MODE_APSTA, WIFI_MODE_MAX} wifi_mode_t;
typedef enum {WL_NO_SHIELD=255, WL_IDLE_STATUS=0, WL_NO_SSID_AVAIL=1, WL_SCAN_COMPLETED=2, WL_CONNECTED=3, WL_CONNECT_FAILED=4, WL_CONNECTION_LOST=5, WL_DISCONNECTED=6} wl_status_t;
#define WiFiMode_t   wifi_mode_t
#define WIFI_OFF     WIFI_MODE_NULL
#define WIFI_STA     WIFI_MODE_STA
#define WIFI_AP      WIFI_MODE_AP
#define WIFI_AP_STA  WIFI_MODE_APSTA
class ESP8266WiFiClass {
public:
  uint8_t begin(uint16_t);
  wl_status_t begin(const char*, const char[]=nullptr, int32_t=0, const uint8_t[]=nullptr, bool=true);
  int beginPacket(const char*, uint16_t);
  int parsePacket();
  int endPacket();
  size_t write(const uint8_t*, size_t);
  int read(unsigned char*, size_t);
  bool softAP(const char*, const char[]=nullptr, int=1, int=0, int=4, bool=false);
  bool enableAP(bool);
  bool isConnected();
  bool disconnect(bool=false, bool=false);
  static bool mode(wifi_mode_t);
};
extern ESP8266WiFiClass WiFi;
#define WiFiUDP ESP8266WiFiClass

class IPAddress {
public:
  IPAddress(uint8_t, uint8_t, uint8_t, uint8_t);
};

class DNSServer {
public:
  void processNextRequest();
  bool start(const uint16_t&, const String&, const IPAddress&);
  void stop();
};

class AsyncWebSocketClient {
};

class AsyncWebHandler {
};

class AsyncCallbackWebHandler: public AsyncWebHandler {
};

typedef uint8_t WebRequestMethodComposite;
#define PGM_P         const char *
typedef std::function<String(const String&)> AwsTemplateProcessor;
class AsyncWebServerRequest {
public:
  void send_P(int, const String&, PGM_P, AwsTemplateProcessor=nullptr);
};
typedef std::function<void(AsyncWebServerRequest *request)> ArRequestHandlerFunction;
typedef enum {
  HTTP_GET     = 0b00000001,
  HTTP_POST    = 0b00000010,
  HTTP_DELETE  = 0b00000100,
  HTTP_PUT     = 0b00001000,
  HTTP_PATCH   = 0b00010000,
  HTTP_HEAD    = 0b00100000,
  HTTP_OPTIONS = 0b01000000,
  HTTP_ANY     = 0b01111111,
} WebRequestMethod;
class AsyncWebServer {
public:
  AsyncWebServer(uint16_t);
  AsyncWebHandler& addHandler(AsyncWebHandler* handler);
  AsyncCallbackWebHandler& on(const char*, WebRequestMethodComposite, ArRequestHandlerFunction);
  void begin();
private:
  AsyncCallbackWebHandler asyncCallbackWebHandler;
};

typedef struct {
    uint8_t message_opcode;
    uint32_t num;
    uint8_t final;
    uint8_t masked;
    uint8_t opcode;
    uint64_t len;
    uint8_t mask[4];
    uint64_t index;
} AwsFrameInfo;

typedef enum { WS_CONTINUATION, WS_TEXT, WS_BINARY, WS_DISCONNECT = 0x08, WS_PING, WS_PONG } AwsFrameType;

class AsyncWebSocket;
typedef enum { WS_EVT_CONNECT, WS_EVT_DISCONNECT, WS_EVT_PONG, WS_EVT_ERROR, WS_EVT_DATA } AwsEventType;
typedef std::function<void(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType, void*, uint8_t*, size_t)> AwsEventHandler;
#define DEFAULT_MAX_WS_CLIENTS 8
class AsyncWebSocket : public AsyncWebHandler {
public:
  AsyncWebSocket(const String&);
  size_t count() const;
  void onEvent(AwsEventHandler);
  void cleanupClients(uint16_t=DEFAULT_MAX_WS_CLIENTS);
  void textAll(const char*);
};

#define NO_DE_PIN 255
class ModbusRTUMaster {
public:
  ModbusRTUMaster(SerialClass&, uint8_t=NO_DE_PIN);
  void begin(unsigned long, uint32_t=SERIAL_8N1, int8_t=-1, int8_t=-1, bool=false);
  bool readHoldingRegisters(uint8_t, uint16_t, uint16_t*, uint16_t);
  bool writeSingleHoldingRegister(uint8_t, uint16_t, uint16_t);
  void setTimeout(unsigned long);
  void clearTimeoutFlag();
  bool getTimeoutFlag();
};

#endif // TESTING

#endif // _TESTING_H_
