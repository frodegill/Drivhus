#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <EEPROM.h>
#include <string>


class Settings
{
public:
  static constexpr uint8_t  EEPROM_INITIALIZED_MARKER = 0xF1; //Just a magic number. CHange when EEPROM data format is incompatibly changed
  static constexpr const char* SETUP_SSID = "drivhus-setup";
  static constexpr const char*  DEFAULT_SERVERID = "/MiniDrivhus/1/";
  static constexpr unsigned long SETUP_PIN_POLL_INTERVAL_MS = 2000; //If system has been disconnected this long, switch to AccessPoint mode

  static constexpr uint8_t MAX_SSID_LENGTH            = 32;
  static constexpr uint8_t MAX_SSID_PASSWORD_LENGTH   = 64;
  static constexpr uint8_t MAX_MQTT_SERVERNAME_LENGTH = 64;
  static constexpr uint8_t MAX_MQTT_SERVERPORT_LENGTH =  5;
  static constexpr uint8_t MAX_MQTT_SERVERID_LENGTH   = 32;
  static constexpr uint8_t MAX_MQTT_USERNAME_LENGTH   = 32;
  static constexpr uint8_t MAX_MQTT_PASSWORD_LENGTH   = 32;

public:
  Settings();
  [[nodiscard]] bool init();
  [[nodiscard]] bool loop(const unsigned long& current_time);

  [[nodiscard]] bool isInSetupMode() const {return m_in_setup_mode;}

  [[nodiscard]] const std::string& getSSID() const {return m_ssid_param;}
  [[nodiscard]] const std::string& getSSIDPassword() const {return m_ssid_password_param;}
  [[nodiscard]] const std::string& getMQTTServername() const {return m_mqtt_servername_param;}
  [[nodiscard]] uint16_t getMQTTPort() const {return m_mqtt_serverport_param;}
  [[nodiscard]] const std::string& getMQTTServerId() const {return m_mqtt_serverid_param;}
  [[nodiscard]] const std::string& getMQTTUsername() const {return m_mqtt_username_param;}
  [[nodiscard]] const std::string& getMQTTPassword() const {return m_mqtt_password_param;}

private:
  void readPersistentString(std::string& s, int max_length, int& adr);
  void readPersistentByte(uint8_t& b, int& adr);
  void readPersistentParams();
  void writePersistentString(const std::string& s, size_t max_length, int& adr);
  void writePersistentByte(uint8_t b, int& adr);
  void writePersistentParams();

  [[nodiscard]] size_t utf8ByteArrayLength(const std::string& s, size_t max_length);

public:
  std::string m_ssid_param;
  std::string m_ssid_password_param;
  std::string m_mqtt_servername_param;
  uint16_t m_mqtt_serverport_param;
  std::string m_mqtt_serverid_param;
  std::string m_mqtt_username_param;
  std::string m_mqtt_password_param;

  unsigned long m_previous_setup_pin_poll_time;
  bool m_in_setup_mode;
};

#endif // _SETTINGS_H_
