#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include <memory>
#include <set>
#include <ESPAsyncWebSrv.h>


class WebServer
{
public:
  [[nodiscard]] bool init();
  [[nodiscard]] bool loop(const unsigned long& current_time);

  static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

public:
  void setSetupMode(bool in_setup_mode);
  void updateSensor(uint8_t sensor_id);
  void setSensorScanCompleted();

private:
  static void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);
  static void notifyClients(const std::string& key, const std::string& data);
  static String processor(const String& var);

  void textAll(const std::string& key, const std::string& data);

private:
  std::unique_ptr<AsyncWebServer> m_server;
  std::unique_ptr<AsyncWebSocket> m_ws;
  std::set<uint8_t> m_present_sensors;
};

#endif // _WEBSERVER_H_
