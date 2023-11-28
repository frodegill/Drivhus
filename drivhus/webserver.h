#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <ESPAsyncWebSrv.h>


class WebServer
{
public:
  static constexpr unsigned long WARNING_MESSAG_DELAY_MS = 5000L;

public:
  WebServer();
  [[nodiscard]] bool init();
  void loop();

  static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

public:
  void updateSetupMode();
  void updateSensor(uint8_t sensor_id);
  void setSensorScanCompleted();
  size_t wsClientCount() const {return m_ws->count();}

  void addWarningMessage(const std::string& msg);

private:
  static void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);
  static void notifyClients(const std::string& key, const std::string& data);
  static String processor(const String& var);

  void textAll(const std::string& key, const std::string& data);
  void updateNewSensorIdButtons(uint8_t sensor_id);

  [[nodiscard]] std::string getSensorValueAsString(uint8_t sensor_id) const;
  [[nodiscard]] std::string getUnusedSensorIdsAsString() const;
  [[nodiscard]] uint8_t getUnusedSensorId() const; //Returns the ID if one, and only one, sensor is within the range. UNDEFINED_ID if not.
  [[nodiscard]] std::string generateSelectOptions(uint8_t sensor_id) const;

  void checkIfWarningMessageShouldBeShown();
  void showWarningMessage(const std::string& msg);

private:
  std::unique_ptr<AsyncWebServer> m_server;
  std::unique_ptr<AsyncWebSocket> m_ws;

  bool m_is_showing_setup;
  std::set<uint8_t> m_present_sensors;

  std::list<std::string> m_warning_messages;
  std::recursive_mutex m_warning_messages_mutex;
  unsigned long m_warning_message_time;
};

#endif // _WEBSERVER_H_
