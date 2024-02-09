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
  virtual [[nodiscard]] bool init() override;
  virtual void loop() override;

protected:
  virtual void onPlantMoistureChanged(uint8_t plant_id, float) {registerChange(Drivhus::OnValueChangeListener::Type::PLANT_MOISTURE, 15*1000, plant_id);}
  virtual void onIndoorTempChanged(float) {registerChange(Drivhus::OnValueChangeListener::Type::INDOOR_TEMP, 15*1000);}
  virtual void onIndoorHumidityChanged(float) {registerChange(Drivhus::OnValueChangeListener::Type::INDOOR_HUMIDITY, 15*1000);}
  virtual void onOutdoorTempChanged(float) {registerChange(Drivhus::OnValueChangeListener::Type::OUTDOOR_TEMP, 15*1000);}
  virtual void onOutdoorHumidityChanged(float) {registerChange(Drivhus::OnValueChangeListener::Type::OUTDOOR_HUMIDITY, 15*1000);}
  virtual void onLightChanged(float) {registerChange(Drivhus::OnValueChangeListener::Type::LIGHT, 30*1000);}
  virtual void onVoltChanged(float) {registerChange(Drivhus::OnValueChangeListener::Type::VOLT, 30*1000);}
  virtual void onSunriseChanged(float) {registerChange(Drivhus::OnValueChangeListener::Type::SUNRISE, 1*1000);}
  virtual void onSunsetChanged(float) {registerChange(Drivhus::OnValueChangeListener::Type::SUNSET, 1*1000);}
private:
  void registerChange(Drivhus::OnValueChangeListener::Type type, unsigned long max_cache_time, uint8_t plant_id=0);

public:
  static void globalMQTTCallback(char* topic, byte* payload, unsigned int length);
  void callback(char* topic, byte* payload, unsigned int length);

  void requestMQTTConnection();
  [[nodiscard]] bool isMQTTConnectionRequested() const {return m_reconnect_time!=0L;}

  void log(const std::string& msg);
  
private:
  void subscribe();
  void publishPendingFields();
  void appendField(std::stringstream& stream, bool& first, std::string&& field, float value, uint8_t precision);

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
