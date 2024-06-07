#include <string>
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

void Drivhus::Log::onConfigChanged(OnConfigChangeListener::Type type, uint8_t id) {
  if (!Drivhus::getSettings()->getIsSystemReady() ||
      m_log_level!=LogLevel::LEVEL_DEBUG) {
    return;
  }
  
  switch(type) {
    case SETUP_MODE: print(LogLevel::LEVEL_DEBUG, std::string("Setup Mode set to ") + (Drivhus::getSettings()->getIsInSetupMode() ? "ENABLED" : "DISABLED")); break;
    case MS_BETWEEN_READING: print(LogLevel::LEVEL_DEBUG, std::string("Ms Between Reading set to ") + std::to_string(Drivhus::getSettings()->getMsBetweenReading())); break;
    case FAN_ACTIVATE_TEMP: print(LogLevel::LEVEL_DEBUG, std::string("Fan Activate Temp set to ") + Drivhus::floatToString(Drivhus::getSettings()->getFanActivateTemp(), 2)); break;
    case FAN_ACTIVATE_HUMIDITY: print(LogLevel::LEVEL_DEBUG, std::string("Fan Activate Humidity set to ") + Drivhus::floatToString(Drivhus::getSettings()->getFanActivateHumidity(), 2)); break;
    case PLANT_REQUEST_WATERING: print(LogLevel::LEVEL_DEBUG, std::string("Plant ") + std::to_string(id) + " requested watering"); break;
    case PLANT_ENABLED:  print(LogLevel::LEVEL_DEBUG, std::string("Plant ") + std::to_string(id) + (Drivhus::getSettings()->getEnabled(id) ? " ENABLED" : " DISABLED")); break;
    case PLANT_WET_VALUE: print(LogLevel::LEVEL_DEBUG, std::string("Plant ") + std::to_string(id) + " Wet Value set to " + Drivhus::floatToString(Drivhus::getSettings()->getWetValue(id), 2)); break;
    case PLANT_DRY_VALUE: print(LogLevel::LEVEL_DEBUG, std::string("Plant ") + std::to_string(id) + " Dry Value set to " + Drivhus::floatToString(Drivhus::getSettings()->getDryValue(id), 2)); break;
    case PLANT_WATERING_DURATION: print(LogLevel::LEVEL_DEBUG, std::string("Plant ") + std::to_string(id) + " Watering Duration set to " + std::to_string(Drivhus::getSettings()->getWateringDuration(id))); break;
    case PLANT_WATERING_GRACE_VALUE: print(LogLevel::LEVEL_DEBUG, std::string("Plant ") + std::to_string(id) + " Watering Grace Period set to " + std::to_string(Drivhus::getSettings()->getWateringGracePeriod(id))); break;
  };
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
