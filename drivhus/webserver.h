#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#ifdef TESTING
# include "testing.h"
#else
# include <AsyncTCP.h>
# include <WiFi.h>
# include <ESPAsyncWebServer.h>
#endif

#include <list>
#include <memory>
#include <mutex>
#include <set>

#include "component.h"
#include "settings.h"


namespace Drivhus {

class WebServer : public Component, public OnValueChangeListener, public OnConfigChangeListener
{
public:
  static constexpr unsigned long WARNING_MESSAG_DELAY_MS = 5000L;
  static constexpr unsigned long WATERPUMPS_TEST_DELAY_MS = 200L;
  static constexpr unsigned long WATERPUMPS_TEST_ON_MS = WebServer::WARNING_MESSAG_DELAY_MS;

public:
  WebServer();
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "WebServer";}

  static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

protected:
  virtual void onValueChanged(OnValueChangeListener::Type type, uint8_t plant_id) override;
  virtual void onConfigChanged(OnConfigChangeListener::Type type, uint8_t id) override;

public:
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
  [[nodiscard]] float getOutdoorAsIndoorHumid() const {return m_humid[2];}
  [[nodiscard]] float getVolt() const {return m_volt;}
  [[nodiscard]] std::string getGrowlightTime() const {return m_growlight_time;}

  void textAll(const std::string& key, const std::string& data);
  void updateSensor(uint8_t sensor_id);
  void updateGrowlightTime();

  [[nodiscard]] std::string getSensorValueAsString(uint8_t sensor_id) const;
  [[nodiscard]] std::string generateSensorControl(uint8_t sensor_id) const;
  [[nodiscard]] std::string generateOutdoorHumidityAsString() const;
  [[nodiscard]] std::string generateVoltMultiplierCalibration() const;
  [[nodiscard]] std::string generateTimezoneSelectOptions() const;

  void checkIfWarningMessageShouldBeShown();
  void showWarningMessage(const std::string& msg);

  void activateWaterpumpsTests();
  void updateWaterpumpsTest();
  bool activateTestWaterpumps(bool turn_on);

private:
  std::unique_ptr<AsyncWebServer> m_server;
  std::unique_ptr<AsyncWebSocket> m_ws;

  bool m_is_showing_setup;
  std::set<uint8_t> m_present_sensors;

  float m_temp[2]; //0==indoor, 1=outdoor
  float m_humid[3]; //0=indoor, 1=outdoor, 2=outdoor as indoor
  float m_light;
  float m_volt;
  float m_sunrise;
  float m_sunset;
  std::string m_growlight_time;

  std::list<std::string> m_warning_messages;
  std::recursive_mutex m_warning_messages_mutex;
  unsigned long m_warning_message_time;

  bool m_is_testing_waterpumps;
  unsigned long m_waterpumps_test_event_time;
  uint8_t m_waterpumps_test_index;
  bool m_waterpumps_test_on;
};

} //namespace

#endif // _WEBSERVER_H_
