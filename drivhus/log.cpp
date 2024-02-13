#include "log.h"

#ifndef TESTING
# include <HardwareSerial.h>
#endif

#include "global.h"
#include "mqtt.h"
#include "settings.h"


Drivhus::Log::Log(LogLevel log_level, LogMode log_mode)
: Drivhus::Component(),
  m_log_level(LogLevel::LEVEL_NONE) {
  setLogLevel(log_level);
  setLogMode(log_mode);
}

bool Drivhus::Log::init() {
  return true;
}

void Drivhus::Log::loop() {
}

void Drivhus::Log::setLogLevel(LogLevel log_level)
{
  if (m_log_level!=LogLevel::LEVEL_NONE && log_level==LogLevel::LEVEL_NONE) {
    Serial.flush();
    Serial.end();
  } else if (m_log_level==LogLevel::LEVEL_NONE && log_level!=LogLevel::LEVEL_NONE) {
    Serial.begin(115200);
  }

  m_log_level = log_level;
}

void Drivhus::Log::setLogMode(LogMode log_mode) {
  m_log_mode = log_mode;
}

void Drivhus::Log::print(LogLevel level, const std::string& msg)
{
  std::string level_string;
  switch(level) {
    case LogLevel::LEVEL_ERROR: level_string="ERROR: "; break;
    case LogLevel::LEVEL_INFO: level_string="INFO: "; break;
    case LogLevel::LEVEL_DEBUG: level_string="DEBUG: "; break;
  };

  if (level<=Drivhus::Log::LOG_LEVEL && !msg.empty())
  {
    if (m_log_mode==LogMode::MODE_SERIAL || m_log_mode==LogMode::MODE_SERIAL_AND_MQTT) {
      Serial.println((level_string+msg).c_str());
    }

    if (m_log_mode==LogMode::MODE_MQTT || m_log_mode==LogMode::MODE_SERIAL_AND_MQTT) {
      Drivhus::getMQTT()->log(level_string+msg);
    }
  }
}
