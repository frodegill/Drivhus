#ifdef TESTING

#include "testing.h"


EEPROMClass EEPROM;

SerialClass Serial;
SerialClass Serial1;
SerialClass Serial2;

ESP8266WiFiClass WiFi;


void pinMode(uint8_t, uint8_t) {}
void analogReadResolution(uint8_t) {}
uint16_t analogRead(uint8_t) {return 0;}
int digitalRead(uint8_t) {return 0;}
void digitalWrite(uint8_t, uint8_t) {}
unsigned long millis() {return 0L;}
time_t now() {return 0L;}
void setTime(time_t) {}
void delay(uint32_t) {}

String::String(const char*) {}
String::String(float, unsigned int) {}
const char* String::c_str() const {return "";}
bool String::equals(const char*) const {return true;}
bool String::operator ==(const char* cstr) const {return equals(cstr);}
bool String::startsWith(const String&) const {return true;}
String String::substring(unsigned int) const {return String();}
String String::substring(unsigned int, unsigned int) const {return String();}

void SerialClass::begin(unsigned long, uint32_t, int8_t, int8_t, bool, unsigned long, uint8_t) {}
uint32_t SerialClass::baudRate() {return 0;}
size_t SerialClass::println(const char[]) {return 0L;}

bool EEPROMClass::begin(size_t) {return true;}
uint8_t EEPROMClass::read(int) {return 0;}
void EEPROMClass::write(int, uint8_t) {}
bool EEPROMClass::commit() {return true;}

void DHTesp::setup(uint8_t, DHT_MODEL_t) {}
int DHTesp::getMinimumSamplingPeriod() {return 0;}
TempAndHumidity DHTesp::getTempAndHumidity() {return temp_and_humidity;}
DHTesp::DHT_ERROR_t DHTesp::getStatus() {return ERROR_NONE;}

Timezone::Timezone(TimeChangeRule, TimeChangeRule) {}
time_t Timezone::toLocal(time_t) {return 0L;}

timeStatus_t timeStatus() {return timeNotSet;}
void setSyncProvider(getExternalTime) {}
void setSyncInterval(time_t) {}

uint8_t ESP8266WiFiClass::begin(uint16_t) {return 0;}
wl_status_t ESP8266WiFiClass::begin(const char*, const char[], int32_t, const uint8_t[], bool) {return WL_IDLE_STATUS;}
int ESP8266WiFiClass::beginPacket(const char*, uint16_t) {return 0;}
int ESP8266WiFiClass::parsePacket() {return 0;}
int ESP8266WiFiClass::endPacket() {return 0;}
size_t ESP8266WiFiClass::write(const uint8_t*, size_t) {return 0L;}
int ESP8266WiFiClass::read(unsigned char*, size_t) {return 0;}
bool ESP8266WiFiClass::softAP(const char*, const char[], int, int, int, bool) {return true;}
bool ESP8266WiFiClass::enableAP(bool) {return true;}
bool ESP8266WiFiClass::isConnected() {return true;}
bool ESP8266WiFiClass::disconnect(bool, bool) {return true;}
bool ESP8266WiFiClass::mode(wifi_mode_t) {return true;}

IPAddress::IPAddress(uint8_t, uint8_t, uint8_t, uint8_t) {}

void DNSServer::processNextRequest() {}
bool DNSServer::start(const uint16_t&, const String&, const IPAddress&) {return true;}
void DNSServer::stop() {}

void AsyncWebServerRequest::send_P(int, const String&, PGM_P, AwsTemplateProcessor) {}

AsyncWebServer::AsyncWebServer(uint16_t) {}
AsyncWebHandler& AsyncWebServer::addHandler(AsyncWebHandler* handler) {return *handler;}
AsyncCallbackWebHandler& AsyncWebServer::on(const char*, WebRequestMethodComposite, ArRequestHandlerFunction) {return asyncCallbackWebHandler;}
void AsyncWebServer::begin() {}

AsyncWebSocket::AsyncWebSocket(const String&) {}
size_t AsyncWebSocket::count() const {return 0L;}
void AsyncWebSocket::onEvent(AwsEventHandler) {}
void AsyncWebSocket::cleanupClients(uint16_t) {}
void AsyncWebSocket::textAll(const char*) {}

ModbusRTUMaster::ModbusRTUMaster(SerialClass&, uint8_t) {}
void ModbusRTUMaster::begin(unsigned long, uint32_t, int8_t, int8_t, bool) {}
bool ModbusRTUMaster::readHoldingRegisters(uint8_t, uint16_t, uint16_t*, uint16_t) {return true;}
bool ModbusRTUMaster::writeSingleHoldingRegister(uint8_t, uint16_t, uint16_t) {return true;}
void ModbusRTUMaster::setTimeout(unsigned long) {}
void ModbusRTUMaster::clearTimeoutFlag() {}
bool ModbusRTUMaster::getTimeoutFlag() {return true;}

int main() {return 0;}

#endif // TESTING
