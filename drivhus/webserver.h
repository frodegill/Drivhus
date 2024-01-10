#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#ifdef TESTING
# include "testing.h"
#else
# include <ESPAsyncWebSrv.h>
#endif

#include <list>
#include <memory>
#include <mutex>
#include <set>

namespace Drivhus {

class WebServer
{
public:
  static constexpr unsigned long WARNING_MESSAG_DELAY_MS = 5000L;
  static constexpr unsigned long RELAY_TEST_DELAY_MS = 200L;
  static constexpr unsigned long RELAY_TEST_ON_MS = WebServer::WARNING_MESSAG_DELAY_MS;

public:
  WebServer();
  [[nodiscard]] bool init();
  void loop();

  static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

public:
  void updateSetupMode();
  void updateSensor(uint8_t sensor_id);
  void setSensorScanCompleted();
  void updateTempHumid(uint8_t id, float temp, float humid);
  void updateLight(float light);
  void updateVolt(float volt);

  size_t wsClientCount() const {return m_ws->count();}

  void addWarningMessage(const std::string& msg);

private:
  static void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);
  static void notifyClients(const std::string& key, const std::string& data);
  static String processor(const String& var);

  [[nodiscard]] float getIndoorTemp() const {return m_temp[0];}
  [[nodiscard]] float getIndoorHumid() const {return m_humid[0];}
  [[nodiscard]] float getLight() const {return m_light;}
  [[nodiscard]] float getOutdoorTemp() const {return m_temp[1];}
  [[nodiscard]] float getOutdoorHumid() const {return m_humid[1];}
  [[nodiscard]] float getVolt() const {return m_volt;}

  void textAll(const std::string& key, const std::string& data);
  void updateNewSensorIdButtons(uint8_t sensor_id);

  [[nodiscard]] std::string getSensorValueAsString(uint8_t sensor_id) const;
  [[nodiscard]] std::string getUnusedSensorIdsAsString() const;
  [[nodiscard]] uint8_t getUnusedSensorId() const; //Returns the ID if one, and only one, sensor is within the range. UNDEFINED_ID if not.
  [[nodiscard]] std::string generateSelectOptions(uint8_t sensor_id) const;
  [[nodiscard]] std::string generateVoltMultiplierCalibration() const;

  void checkIfWarningMessageShouldBeShown();
  void showWarningMessage(const std::string& msg);

  void activateRelayTests();
  void updateRelayTest();
  bool activateTestRelay(bool turn_on);

private:
  std::unique_ptr<AsyncWebServer> m_server;
  std::unique_ptr<AsyncWebSocket> m_ws;

  bool m_is_showing_setup;
  std::set<uint8_t> m_present_sensors;

  float m_temp[2];
  float m_humid[2];
  float m_light;
  float m_volt;

  std::list<std::string> m_warning_messages;
  std::recursive_mutex m_warning_messages_mutex;
  unsigned long m_warning_message_time;

  bool m_is_testing_relays;
  unsigned long m_relay_test_event_time;
  uint8_t m_relay_test_index;
  bool m_relay_test_on;
};

} //namespace

#endif // _WEBSERVER_H_
