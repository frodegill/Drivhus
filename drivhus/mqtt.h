#ifndef _MQTT_H_
#define _MQTT_H_

#ifdef TESTING
# include "testing.h"
#else
# include <WiFi.h>
# include <PubSubClient.h>
#endif

#include "component.h"
#include "settings.h"


namespace Drivhus {

class MQTT : public Component, public OnValueChangeListener
{
public:
  static constexpr uint16_t MQTT_DEFAULT_PORT = 1883;
  static constexpr unsigned long MQTT_CONNECTION_TIMEOUT_MS = 10000L;
  static constexpr unsigned long MINIMUM_PACKET_INTERVAL_MS = 10000L;

public:
  MQTT();
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "MQTT";}

private:
  unsigned long changeTypeToCacheTime(OnValueChangeListener::Type type);
  std::string errorStateMessage();

protected:
  virtual void onValueChanged(OnValueChangeListener::Type type, uint8_t plant_id) override;

public:
  static void globalMQTTCallback(char* topic, uint8_t* payload, unsigned int length);
  void callback(char* topic, uint8_t* payload, unsigned int length);

  void requestMQTTConnection();
  [[nodiscard]] bool isMQTTConnectionRequested() const {return m_reconnect_time!=0L;}

  void log(const std::string& msg);
  
private:
  void subscribe();
  void publishPendingFields();
  void appendField(std::stringstream& stream, bool& first, std::string&& field, float value, uint8_t precision);
  void appendField(std::stringstream& stream, bool& first, std::string&& field, int value);

public:
  WiFiClient m_esp_client;
  PubSubClient m_mqtt_client;

  unsigned long m_reconnect_time;

  unsigned long m_cached_packet_time;
  unsigned long m_max_cache_time;
  uint32_t m_changed_fields;
};

} //namespace

#endif // _MQTT_H_
