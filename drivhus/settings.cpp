#include "settings.h"

#include <vector>

#include "global.h"
#include "mqtt.h"


Settings::Settings()
: m_mqtt_serverport_param(Mqtt::MQTT_DEFAULT_PORT),
  m_previous_setup_pin_poll_time(0L),
  m_in_setup_mode(false) {
}

bool Settings::init() {
  pinMode(SETUP_MODE_ENABLE_PIN, INPUT_PULLUP);

  EEPROM.begin(1 + 
               MAX_SSID_LENGTH+1 + MAX_SSID_PASSWORD_LENGTH+1 +
               MAX_MQTT_SERVERNAME_LENGTH+1 + MAX_MQTT_SERVERPORT_LENGTH+1 + MAX_MQTT_SERVERID_LENGTH+1 + MAX_MQTT_USERNAME_LENGTH+1 + MAX_MQTT_PASSWORD_LENGTH+1);
  return true;
}

bool Settings::loop(const unsigned long& current_time) {
  if (current_time < m_previous_setup_pin_poll_time) { //Time will wrap around every ~50 days. DOn't consider this an error
    m_previous_setup_pin_poll_time = current_time;
  } else if ((m_previous_setup_pin_poll_time+SETUP_PIN_POLL_INTERVAL_MS)<current_time) {
    m_previous_setup_pin_poll_time = current_time;
    m_in_setup_mode = digitalRead(SETUP_MODE_ENABLE_PIN)==LOW;
  }

  return true;
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

void Settings::writePersistentParams() {
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
  EEPROM.commit();
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
