#include "settings.h"

#include <sstream>
#include <vector>

#include "mqtt.h"
#include "network.h"


Drivhus::Settings::Settings(uint8_t pin)
: Drivhus::Component(),
  m_pin(pin),
  m_mqtt_serverport_param(Drivhus::MQTT::MQTT_DEFAULT_PORT),
  m_volt_multiplier_param(1.0f),
  m_settings_changed(false),
  m_previous_setup_pin_poll_time(0L),
  m_in_setup_mode(false),
  m_should_flush_settings(false),
  m_ms_between_reading(0L),
  m_fan_activate_temp_value(0.0f),
  m_fan_activate_humid_value(0.0f),
  m_indoor_temp(0.0f),
  m_indoor_humidity(0.0f),
  m_outdoor_temp(0.0f),
  m_outdoor_humidity(0.0f),
  m_outdoor_as_indoor_humidity(0.0f),
  m_light(0.0f),
  m_volt(0.0f),
  m_sunrise(0.0f),
  m_sunset(0.0f) {
}

bool Drivhus::Settings::init() {
  pinMode(m_pin, INPUT_PULLUP);
  analogReadResolution(12);

  EEPROM.begin(1 + 
               MAX_SSID_LENGTH+1 + MAX_SSID_PASSWORD_LENGTH+1 +
               MAX_MQTT_SERVERNAME_LENGTH+1 + MAX_MQTT_SERVERPORT_LENGTH+1 + MAX_MQTT_SERVERID_LENGTH+1 + MAX_MQTT_USERNAME_LENGTH+1 + MAX_MQTT_PASSWORD_LENGTH+1 +
               VOLT_MULTIPLIER_LENGTH+1 + MAX_TIMEZONE_LENGTH+1 + MAX_EMULATE_LATLONG_LENGTH+1 + MAX_EMULATE_LATLONG_LENGTH+1);

  readPersistentParams();
  m_in_setup_mode = isInSetupMode(true);
  return true;
}

void Drivhus::Settings::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_setup_pin_poll_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_setup_pin_poll_time = current_time;
  }
  
  bool previous_setup_mode = m_in_setup_mode;
  if (isInSetupMode() != previous_setup_mode) {
    Drivhus::getNetwork()->getWebServer()->updateSetupMode();
  }

  checkIfSettingsShouldBeFlushed();
}

bool Drivhus::Settings::isInSetupMode(bool force_read) {
  const unsigned long current_time = millis();
  if (force_read ||
      (m_previous_setup_pin_poll_time+SETUP_PIN_POLL_INTERVAL_MS)<current_time) {
    m_previous_setup_pin_poll_time = current_time;
    m_in_setup_mode = digitalRead(m_pin)==LOW;
  }
  return m_in_setup_mode;
}

void Drivhus::Settings::addValueChangeListener(Drivhus::OnValueChangeListener* listener) {
  m_value_change_listeners.push_back(listener);
}

void Drivhus::Settings::addConfigChangeListener(Drivhus::OnConfigChangeListener* listener) {
  m_config_change_listeners.push_back(listener);
}

void Drivhus::Settings::notifyValueChangeListeners(Drivhus::OnValueChangeListener::Type type, uint8_t plant_id) {
  for (auto listener : m_value_change_listeners) {
    listener->onValueChanged(type, plant_id);
  }
}

void Drivhus::Settings::notifyConfigChangeListeners(Drivhus::OnConfigChangeListener::Type type, uint8_t plant_id) {
  for (auto listener : m_config_change_listeners) {
    listener->onConfigChanged(type, plant_id);
  }
}

void Drivhus::Settings::setShouldFlushSettings() {
  const std::lock_guard<std::recursive_mutex> lock(m_should_flush_settings_mutex);
  m_should_flush_settings = true;
}

void Drivhus::Settings::readPersistentString(std::string& s, int max_length, int& adr) {
  std::vector<uint8_t> d;
  int i = 0;
  uint8_t c;
  do
  {
    c = EEPROM.read(adr++);
    if (c == 0) {
      break;
    }
    d.emplace_back(c);
  } while (++i<max_length);
  s = d.empty() ? "" : std::string(d.begin(), d.end());
}

void Drivhus::Settings::readPersistentByte(uint8_t& b, int& adr) {
  b = EEPROM.read(adr++);
}

void Drivhus::Settings::readPersistentParams() {
  int adr = 0;
  if (EEPROM_INITIALIZED_MARKER != EEPROM.read(adr++))
  {
    m_ssid_param[0] = 0;
    m_ssid_password_param[0] = 0;
    m_mqtt_servername_param[0] = 0;
    m_mqtt_serverport_param = Drivhus::MQTT::MQTT_DEFAULT_PORT;
    m_mqtt_serverid_param = DEFAULT_SERVERID;
    m_mqtt_username_param[0] = 0;
    m_mqtt_password_param[0] = 0;
    m_volt_multiplier_param = 1.0f;
    m_timezone_param[0] = 0;
    m_emulate_latitude_param = 0;
    m_emulate_longitude_param = 0;
  }
  else
  {
    std::string temp;

    readPersistentString(m_ssid_param, MAX_SSID_LENGTH, adr);
    readPersistentString(m_ssid_password_param, MAX_SSID_PASSWORD_LENGTH, adr);
    readPersistentString(m_mqtt_servername_param, MAX_MQTT_SERVERNAME_LENGTH, adr);

    readPersistentString(temp, MAX_MQTT_SERVERPORT_LENGTH, adr);
    m_mqtt_serverport_param = static_cast<uint16_t>(std::stoi(temp)&0xFFFF);

    readPersistentString(m_mqtt_serverid_param, MAX_MQTT_SERVERID_LENGTH, adr);
    readPersistentString(m_mqtt_username_param, MAX_MQTT_USERNAME_LENGTH, adr);
    readPersistentString(m_mqtt_password_param, MAX_MQTT_PASSWORD_LENGTH, adr);

    readPersistentString(temp, VOLT_MULTIPLIER_LENGTH, adr);
    m_volt_multiplier_param = static_cast<uint16_t>(std::stoi(temp)&0xFFFF) / 256.0f;

    readPersistentString(m_timezone_param, MAX_TIMEZONE_LENGTH, adr);

    readPersistentString(temp, MAX_EMULATE_LATLONG_LENGTH, adr);
    m_emulate_latitude_param = static_cast<int8_t>(temp.length()==0 ? 0 : std::stoi(temp) - 90);
    readPersistentString(temp, MAX_EMULATE_LATLONG_LENGTH, adr);
    m_emulate_longitude_param = static_cast<int16_t>(temp.length()==0 ? 0 : std::stoi(temp) - 180);
  }
}

void Drivhus::Settings::writePersistentString(const std::string& s, size_t max_length, int& adr) {
  size_t length = utf8ByteArrayLength(s, max_length);
  for (size_t i=0; i<length; i++)
  {
    EEPROM.write(adr++, s.at(i));
  }
  if (length < max_length) {
    EEPROM.write(adr++, 0);
  }
}

void Drivhus::Settings::writePersistentByte(uint8_t b, int& adr) {
  EEPROM.write(adr++, b);
}

bool Drivhus::Settings::writePersistentParams() {
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

  char volt_multiplier[VOLT_MULTIPLIER_LENGTH+1];
  uint16_t tmp_volt_multiplier = static_cast<uint16_t>(m_volt_multiplier_param*256.0f);
  sprintf(volt_multiplier, "%hu", tmp_volt_multiplier);
  volt_multiplier[VOLT_MULTIPLIER_LENGTH] = 0;
  writePersistentString(volt_multiplier, VOLT_MULTIPLIER_LENGTH, adr);

  writePersistentString(m_timezone_param, MAX_TIMEZONE_LENGTH, adr);

  char tmp_pos[MAX_EMULATE_LATLONG_LENGTH+1];
  snprintf(tmp_pos, MAX_EMULATE_LATLONG_LENGTH+1, "%hu", m_emulate_latitude_param+90);
  writePersistentString(tmp_pos, MAX_EMULATE_LATLONG_LENGTH, adr);
  snprintf(tmp_pos, MAX_EMULATE_LATLONG_LENGTH+1, "%hu", m_emulate_longitude_param+180);
  writePersistentString(tmp_pos, MAX_EMULATE_LATLONG_LENGTH, adr);

  return EEPROM.commit();
}

size_t Drivhus::Settings::utf8ByteArrayLength(const std::string& s, size_t max_length) {
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

void Drivhus::Settings::checkIfSettingsShouldBeFlushed() {
  const std::lock_guard<std::recursive_mutex> lock(m_should_flush_settings_mutex);
  if (m_should_flush_settings) {
    flushSettings();
  }
}

void Drivhus::Settings::flushSettings() {
  const std::lock_guard<std::recursive_mutex> lock(m_should_flush_settings_mutex);
  m_should_flush_settings = false;

  if (isInSetupMode() && m_settings_changed) {
    bool result = writePersistentParams();
    if (result) {
      m_settings_changed = false;
    }

    std::stringstream ss;
    ss << "Flushing settings " << (result ? "SUCCEEDED" : "FAILED");
    Drivhus::getNetwork()->getWebServer()->addWarningMessage(ss.str());
  }
}

void Drivhus::Settings::calculateOutdoorHumidityIndoor() {
  if (isnan(m_indoor_temp) || isnan(m_outdoor_temp) || isnan(m_outdoor_humidity)) {
    m_outdoor_as_indoor_humidity = NAN;
  } else {
    float const_a = 17.625f;
    float const_b = 243.04f;
    float dew = const_b*(std::log(m_outdoor_humidity/100.0f)+((const_a*m_outdoor_temp)/(const_b+m_outdoor_temp))) / (const_a-std::log(m_outdoor_humidity/100.0f)-((const_a*m_outdoor_temp)/(const_b+m_outdoor_temp)));
    m_outdoor_as_indoor_humidity = 100.0f*std::exp((const_a*dew)/(const_b+dew)) / std::exp((const_a*m_indoor_temp)/(const_b+m_indoor_temp));
  }
}
