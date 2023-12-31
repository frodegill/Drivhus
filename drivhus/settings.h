#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#ifdef TESTING
# include "testing.h"
#else
# include <EEPROM.h>
#endif

#include <mutex>
#include <string>

#include "volt.h"

namespace Drivhus {

class Settings
{
public:
  static constexpr uint8_t  EEPROM_INITIALIZED_MARKER = 0xF2; //Just a magic number. CHange when EEPROM data format is incompatibly changed
  static constexpr const char* SETUP_SSID = "drivhus-setup";
  static constexpr const char*  DEFAULT_SERVERID = "/MiniDrivhus/1/";
  static constexpr unsigned long SETUP_PIN_POLL_INTERVAL_MS = 2000; //If system has been disconnected this long, switch to AccessPoint mode

  static constexpr uint8_t MAX_SSID_LENGTH            = 32;
  static constexpr uint8_t MAX_SSID_PASSWORD_LENGTH   = 64;
  static constexpr uint8_t MAX_MQTT_SERVERNAME_LENGTH = 64;
  static constexpr uint8_t MAX_MQTT_SERVERPORT_LENGTH =  5; //5 digits, actually "0"-"65535"
  static constexpr uint8_t MAX_MQTT_SERVERID_LENGTH   = 32;
  static constexpr uint8_t MAX_MQTT_USERNAME_LENGTH   = 32;
  static constexpr uint8_t MAX_MQTT_PASSWORD_LENGTH   = 32;
  static constexpr uint8_t VOLT_MULTIPLIER_LENGTH     =  5; //5 digits, actually "0"-"65535". float value is found by dividing by 256 (so 8bit.8bit)

public:
  Settings(uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

  [[nodiscard]] bool isInSetupMode();

  [[nodiscard]] const std::string& getSSID() const {return m_ssid_param;}
  [[nodiscard]] const std::string& getSSIDPassword() const {return m_ssid_password_param;}
  [[nodiscard]] const std::string& getMQTTServername() const {return m_mqtt_servername_param;}
  [[nodiscard]] uint16_t getMQTTPort() const {return m_mqtt_serverport_param;}
  [[nodiscard]] const std::string& getMQTTServerId() const {return m_mqtt_serverid_param;}
  [[nodiscard]] const std::string& getMQTTUsername() const {return m_mqtt_username_param;}
  [[nodiscard]] const std::string& getMQTTPassword() const {return m_mqtt_password_param;}
  [[nodiscard]] float getVoltMultiplier() const {return m_volt_multiplier_param;}

  void setSSID(const std::string& value) {if (isInSetupMode() && m_ssid_param!=value){m_ssid_param=value; m_settings_changed=true;}}
  void setSSIDPassword(const std::string& value) {if (isInSetupMode() && m_ssid_password_param!=value){m_ssid_password_param=value; m_settings_changed=true;}}
  void setMQTTServername(const std::string& value) {if (isInSetupMode() && m_mqtt_servername_param!=value){m_mqtt_servername_param=value; m_settings_changed=true;}}
  void setMQTTPort(uint16_t value) {if (isInSetupMode() && m_mqtt_serverport_param!=value){m_mqtt_serverport_param=value; m_settings_changed=true;}}
  void setMQTTServerId(const std::string& value) {if (isInSetupMode() && m_mqtt_serverid_param!=value){m_mqtt_serverid_param=value; m_settings_changed=true;}}
  void setMQTTUsername(const std::string& value) {if (isInSetupMode() && m_mqtt_username_param!=value){m_mqtt_username_param=value; m_settings_changed=true;}}
  void setMQTTPassword(const std::string& value) {if (isInSetupMode() && m_mqtt_password_param!=value){m_mqtt_password_param=value; m_settings_changed=true;}}
  void setVoltMultiplier(float value) {if (isInSetupMode() && value>=0.0f && value<=Volt::MAX_VOLT && m_volt_multiplier_param!=value){m_volt_multiplier_param=value; m_settings_changed=true;}}
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

private:
  uint8_t m_pin;

public:
  std::string m_ssid_param;
  std::string m_ssid_password_param;
  std::string m_mqtt_servername_param;
  uint16_t m_mqtt_serverport_param;
  std::string m_mqtt_serverid_param;
  std::string m_mqtt_username_param;
  std::string m_mqtt_password_param;
  float m_volt_multiplier_param;
  bool m_settings_changed;

  unsigned long m_previous_setup_pin_poll_time;
  bool m_in_setup_mode;

  bool m_should_flush_settings;
  std::recursive_mutex m_should_flush_settings_mutex;
};

} //namespace

#endif // _SETTINGS_H_
