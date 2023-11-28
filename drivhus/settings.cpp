#include "settings.h"

#include <sstream>
#include <vector>

#include "global.h"
#include "mqtt.h"
#include "network.h"


Settings::Settings()
: m_mqtt_serverport_param(Mqtt::MQTT_DEFAULT_PORT),
  m_settings_changed(false),
  m_previous_setup_pin_poll_time(0L),
  m_in_setup_mode(false),
  m_should_flush_settings(false) {
}

bool Settings::init() {
  pinMode(SETUP_MODE_ENABLE_PIN, INPUT_PULLUP);
  analogReadResolution(12);

  EEPROM.begin(1 + 
               MAX_SSID_LENGTH+1 + MAX_SSID_PASSWORD_LENGTH+1 +
               MAX_MQTT_SERVERNAME_LENGTH+1 + MAX_MQTT_SERVERPORT_LENGTH+1 + MAX_MQTT_SERVERID_LENGTH+1 + MAX_MQTT_USERNAME_LENGTH+1 + MAX_MQTT_PASSWORD_LENGTH+1);

  readPersistentParams();
  m_in_setup_mode = isInSetupMode();
  return true;
}

void Settings::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_setup_pin_poll_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_setup_pin_poll_time = current_time;
  }
  
  bool previous_setup_mode = m_in_setup_mode;
  if (isInSetupMode() != previous_setup_mode) {
    ::getNetwork()->getWebServer()->updateSetupMode();
  }

  checkIfSettingsShouldBeFlushed();
}

bool Settings::isInSetupMode() {
  const unsigned long current_time = millis();
  if ((m_previous_setup_pin_poll_time+SETUP_PIN_POLL_INTERVAL_MS)<current_time) {
    m_previous_setup_pin_poll_time = current_time;
    m_in_setup_mode = digitalRead(SETUP_MODE_ENABLE_PIN)==LOW;
  }
  return m_in_setup_mode;
}

void Settings::setShouldFlushSettings() {
  const std::lock_guard<std::recursive_mutex> lock(m_should_flush_settings_mutex);
  m_should_flush_settings = true;
}

void Settings::readPersistentString(std::string& s, int max_length, int& adr) {
  std::vector<uint8_t> d;
  int i = 0;
  uint8_t c;
  do
  {
    c = EEPROM.read(adr++);
    if (i<max_length)
    {
      d.emplace_back(c);
    }
  } while (++i<max_length && c!=0);
  s = std::string(d.begin(), d.end());
}

void Settings::readPersistentByte(uint8_t& b, int& adr) {
  b = EEPROM.read(adr++);
}

void Settings::readPersistentParams() {
  int adr = 0;
  if (EEPROM_INITIALIZED_MARKER != EEPROM.read(adr++))
  {
    m_ssid_param[0] = 0;
    m_ssid_password_param[0] = 0;
    m_mqtt_servername_param[0] = 0;
    m_mqtt_serverport_param = Mqtt::MQTT_DEFAULT_PORT;
    m_mqtt_serverid_param = DEFAULT_SERVERID;
    m_mqtt_username_param[0] = 0;
    m_mqtt_password_param[0] = 0;
  }
  else
  {
    readPersistentString(m_ssid_param, MAX_SSID_LENGTH, adr);
    readPersistentString(m_ssid_password_param, MAX_SSID_PASSWORD_LENGTH, adr);
    readPersistentString(m_mqtt_servername_param, MAX_MQTT_SERVERNAME_LENGTH, adr);

    std::string port;
    readPersistentString(port, MAX_MQTT_SERVERPORT_LENGTH, adr);
    m_mqtt_serverport_param = std::stoi(port)&0xFFFF;

    readPersistentString(m_mqtt_serverid_param, MAX_MQTT_SERVERID_LENGTH, adr);
    readPersistentString(m_mqtt_username_param, MAX_MQTT_USERNAME_LENGTH, adr);
    readPersistentString(m_mqtt_password_param, MAX_MQTT_PASSWORD_LENGTH, adr);
  }
}

void Settings::writePersistentString(const std::string& s, size_t max_length, int& adr) {
  for (size_t i=0; i<utf8ByteArrayLength(s, max_length); i++)
  {
    EEPROM.write(adr++, s.at(i));
  }
  EEPROM.write(adr++, 0);
}

void Settings::writePersistentByte(uint8_t b, int& adr) {
  EEPROM.write(adr++, b);
}

bool Settings::writePersistentParams() {
  int adr = 0;
  EEPROM.write(adr++, EEPROM_INITIALIZED_MARKER);
  writePersistentString(m_ssid_param, MAX_SSID_LENGTH, adr);
  writePersistentString(m_ssid_password_param, MAX_SSID_PASSWORD_LENGTH, adr);
  writePersistentString(m_mqtt_servername_param, MAX_MQTT_SERVERNAME_LENGTH, adr);

  char port[MAX_MQTT_SERVERPORT_LENGTH+1];
  sprintf(port, "%hu", m_mqtt_serverport_param);
  port[MAX_MQTT_SERVERPORT_LENGTH] = 0;
  writePersistentString(port, MAX_MQTT_SERVERPORT_LENGTH, adr);

  writePersistentString(m_mqtt_serverid_param, MAX_MQTT_SERVERID_LENGTH, adr);
  writePersistentString(m_mqtt_username_param, MAX_MQTT_USERNAME_LENGTH, adr);
  writePersistentString(m_mqtt_password_param, MAX_MQTT_PASSWORD_LENGTH, adr);
  return EEPROM.commit();
}

size_t Settings::utf8ByteArrayLength(const std::string& s, size_t max_length) {
  size_t len = 0;
  max_length = std::min(max_length, s.length());

  char c;
  uint8_t inc = 0;
  while (len < max_length) {
    c = s.at(len);
    if (c == 0)
      break;
    else if ((c&0x80)==0x00)
      inc = 1;
    else if ((c&0xE0)==0xC0)
      inc = 2;
    else if ((c&0xF0)==0xE0)
      inc = 3;
    else if ((c&0xF8)==0xF0)
      inc = 4;
    else
      break;

    if (len+inc <= max_length)
      len += inc;
    else
      break;
  };

  return len;
}

void Settings::checkIfSettingsShouldBeFlushed() {
  const std::lock_guard<std::recursive_mutex> lock(m_should_flush_settings_mutex);
  if (m_should_flush_settings) {
    flushSettings();
  }
}

void Settings::flushSettings() {
  const std::lock_guard<std::recursive_mutex> lock(m_should_flush_settings_mutex);
  m_should_flush_settings = false;

  if (isInSetupMode() && m_settings_changed) {
    bool result = writePersistentParams();
    if (result) {
      m_settings_changed = false;
    }

    std::stringstream ss;
    ss << "Flushing settings " << (result ? "SUCCEEDED" : "FAILED");
    ::getNetwork()->getWebServer()->addWarningMessage(ss.str());
  }
}