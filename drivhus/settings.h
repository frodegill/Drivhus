#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#ifdef TESTING
# include "testing.h"
#else
# include <EEPROM.h>
#endif

#include <mutex>
#include <string>
#include <vector>

#include "volt.h"

namespace Drivhus {

class OnChangeListener {
public:
  enum FloatType {
    INDOOR_TEMP,
    INDOOR_HUMIDITY,
    OUTDOOR_TEMP,
    OUTDOOR_HUMIDITY,
    LIGHT,
    VOLT,
    SUNRISE,
    SUNSET
  };

  virtual void onIndoorTempChanged(float /*value*/) {}
  virtual void onIndoorHumidityChanged(float /*value*/) {}
  virtual void onOutdoorTempChanged(float /*value*/) {}
  virtual void onOutdoorHumidityChanged(float /*value*/) {}
  virtual void onLightChanged(float /*value*/) {}
  virtual void onVoltChanged(float /*value*/) {}
  virtual void onSunriseChanged(float /*value*/) {}
  virtual void onSunsetChanged(float /*value*/) {}
};

class Settings
{
public:
  static constexpr uint8_t  EEPROM_INITIALIZED_MARKER = 0xF1; //Just a magic number. CHange when EEPROM data format is incompatibly changed
  static constexpr const char* SETUP_SSID = "drivhus-setup";
  static constexpr const char*  DEFAULT_SERVERID = "/Drivhus/1/";
  static constexpr unsigned long SETUP_PIN_POLL_INTERVAL_MS = 2000; //If system has been disconnected this long, switch to AccessPoint mode

  static constexpr uint8_t MAX_SSID_LENGTH            = 32;
  static constexpr uint8_t MAX_SSID_PASSWORD_LENGTH   = 64;
  static constexpr uint8_t MAX_MQTT_SERVERNAME_LENGTH = 64;
  static constexpr uint8_t MAX_MQTT_SERVERPORT_LENGTH =  5; //5 digits, actually "0"-"65535"
  static constexpr uint8_t MAX_MQTT_SERVERID_LENGTH   = 32;
  static constexpr uint8_t MAX_MQTT_USERNAME_LENGTH   = 32;
  static constexpr uint8_t MAX_MQTT_PASSWORD_LENGTH   = 32;
  static constexpr uint8_t VOLT_MULTIPLIER_LENGTH     =  5; //5 digits, actually "0"-"65535". float value is found by dividing by 256 (so 8bit.8bit)
  static constexpr uint8_t MAX_TIMEZONE_LENGTH        =  3;
  static constexpr uint8_t MAX_EMULATE_LATLONG_LENGTH =  3;

public:
  Settings(uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

  [[nodiscard]] bool isInSetupMode(bool force_read = false);

  void addChangeListener(OnChangeListener* listener);
private:
  void notifyFloatChangeListeners(OnChangeListener::FloatType type, float value);

public:
  [[nodiscard]] const std::string& getSSID() const {return m_ssid_param;}
  [[nodiscard]] const std::string& getSSIDPassword() const {return m_ssid_password_param;}
  [[nodiscard]] const std::string& getMQTTServername() const {return m_mqtt_servername_param;}
  [[nodiscard]] uint16_t getMQTTPort() const {return m_mqtt_serverport_param;}
  [[nodiscard]] const std::string& getMQTTServerId() const {return m_mqtt_serverid_param;}
  [[nodiscard]] const std::string& getMQTTUsername() const {return m_mqtt_username_param;}
  [[nodiscard]] const std::string& getMQTTPassword() const {return m_mqtt_password_param;}
  [[nodiscard]] float getVoltMultiplier() const {return m_volt_multiplier_param;}
  [[nodiscard]] const std::string& getTimezone() const {return m_timezone_param;}
  [[nodiscard]] int8_t getEmulateLatitude() const {return m_emulate_latitude_param;}
  [[nodiscard]] int16_t getEmulateLongitude() const {return m_emulate_longitude_param;}

  [[nodiscard]] float getCurrentFanActivateTemp() const {return 40.0f;} //TODO

  void setSSID(const std::string& value) {if (isInSetupMode() && m_ssid_param!=value){m_ssid_param=value; m_settings_changed=true;}}
  void setSSIDPassword(const std::string& value) {if (isInSetupMode() && m_ssid_password_param!=value){m_ssid_password_param=value; m_settings_changed=true;}}
  void setMQTTServername(const std::string& value) {if (isInSetupMode() && m_mqtt_servername_param!=value){m_mqtt_servername_param=value; m_settings_changed=true;}}
  void setMQTTPort(uint16_t value) {if (isInSetupMode() && m_mqtt_serverport_param!=value){m_mqtt_serverport_param=value; m_settings_changed=true;}}
  void setMQTTServerId(const std::string& value) {if (isInSetupMode() && m_mqtt_serverid_param!=value){m_mqtt_serverid_param=value; m_settings_changed=true;}}
  void setMQTTUsername(const std::string& value) {if (isInSetupMode() && m_mqtt_username_param!=value){m_mqtt_username_param=value; m_settings_changed=true;}}
  void setMQTTPassword(const std::string& value) {if (isInSetupMode() && m_mqtt_password_param!=value){m_mqtt_password_param=value; m_settings_changed=true;}}
  void setVoltMultiplier(float value) {if (isInSetupMode() && value>=0.0f && value<=Volt::MAX_VOLT && m_volt_multiplier_param!=value){m_volt_multiplier_param=value; m_settings_changed=true;}}
  void setTimezone(const std::string& value) {if (isInSetupMode() && m_timezone_param!=value){m_timezone_param=value; m_settings_changed=true;}}
  void setEmulateLatitude(int8_t value) {if (isInSetupMode() && value>=-90 && value<=90 && m_emulate_latitude_param!=value){m_emulate_latitude_param=value; m_settings_changed=true;}}
  void setEmulateLongitude(int16_t value) {if (isInSetupMode() && value>=-180 && value<=180 && m_emulate_longitude_param!=value){m_emulate_longitude_param=value; m_settings_changed=true;}}
  void setShouldFlushSettings();

private:
  void readPersistentString(std::string& s, int max_length, int& adr);
  void readPersistentByte(uint8_t& b, int& adr);
  void readPersistentParams();
  void writePersistentString(const std::string& s, size_t max_length, int& adr);
  void writePersistentByte(uint8_t b, int& adr);
  [[nodiscard]] bool writePersistentParams();

  [[nodiscard]] size_t utf8ByteArrayLength(const std::string& s, size_t max_length);

  void checkIfSettingsShouldBeFlushed();
  void flushSettings();

public:
  void setIndoorTemp(float value) {m_indoor_temp=value; notifyFloatChangeListeners(OnChangeListener::FloatType::INDOOR_TEMP, value);}
  void setIndoorHumidity(float value) {m_indoor_humidity=value; notifyFloatChangeListeners(OnChangeListener::FloatType::INDOOR_HUMIDITY, value);}
  void setOutdoorTemp(float value) {m_outdoor_temp=value; notifyFloatChangeListeners(OnChangeListener::FloatType::OUTDOOR_TEMP, value);}
  void setOutdoorHumidity(float value) {m_outdoor_humidity=value; notifyFloatChangeListeners(OnChangeListener::FloatType::OUTDOOR_HUMIDITY, value);}
  void setLight(float value) {m_light=value; notifyFloatChangeListeners(OnChangeListener::FloatType::LIGHT, value);}
  void setVolt(float value) {m_volt=value; notifyFloatChangeListeners(OnChangeListener::FloatType::VOLT, value);}
  void setSunrise(float value) {m_sunrise=value; Serial.println("Setting sunrise"); notifyFloatChangeListeners(OnChangeListener::FloatType::SUNRISE, value);}
  void setSunset(float value) {m_sunset=value; notifyFloatChangeListeners(OnChangeListener::FloatType::SUNSET, value);}
  [[nodiscard]] float getIndoorTemp() const {return m_indoor_temp;}
  [[nodiscard]] float getIndoorHumidity() const {return m_indoor_humidity;}
  [[nodiscard]] float getOutdoorTemp() const {return m_outdoor_temp;}
  [[nodiscard]] float getOutdoorHumidity() const {return m_outdoor_humidity;}
  [[nodiscard]] float getLight() const {return m_light;}
  [[nodiscard]] float getVolt() const {return m_volt;}
  [[nodiscard]] float getSunrise() const {return m_sunrise;}
  [[nodiscard]] float getSunset() const {return m_sunset;}

private:
  uint8_t m_pin;

public:
  std::vector<OnChangeListener*> m_change_listeners;

  std::string m_ssid_param;
  std::string m_ssid_password_param;
  std::string m_mqtt_servername_param;
  uint16_t m_mqtt_serverport_param;
  std::string m_mqtt_serverid_param;
  std::string m_mqtt_username_param;
  std::string m_mqtt_password_param;
  float m_volt_multiplier_param;
  std::string m_timezone_param;
  int8_t m_emulate_latitude_param; //-90 (South Pole) to 90 (North Pole)
  int16_t m_emulate_longitude_param; //-180 (West) to 180 (East)

  bool m_settings_changed;

  unsigned long m_previous_setup_pin_poll_time;
  bool m_in_setup_mode;

  bool m_should_flush_settings;
  std::recursive_mutex m_should_flush_settings_mutex;

  float m_indoor_temp;
  float m_indoor_humidity;
  float m_outdoor_temp;
  float m_outdoor_humidity;
  float m_light;
  float m_volt;
  float m_sunrise;
  float m_sunset;
};

} //namespace

#endif // _SETTINGS_H_
