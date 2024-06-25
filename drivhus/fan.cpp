#include "fan.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include <cmath>

#include "global.h"
#include "log.h"


Drivhus::Fan::Fan(uint8_t pin)
: Drivhus::Component(),
  Drivhus::OnValueChangeListener(),
  m_pin(pin),
  m_previous_event_time(0L),
  m_fan_activate_temp_value(0.0f),
  m_fan_activate_humid_value(0.0f),
  m_indoor_temp(NAN),
  m_indoor_humidity(NAN),
  m_outdoor_temp(NAN),
  m_outdoor_humidity_indoor(NAN),
  m_activated(false)
{
  Drivhus::getSettings()->addValueChangeListener(this);
  Drivhus::getSettings()->addConfigChangeListener(this);
}
 
bool Drivhus::Fan::init() {
  pinMode(m_pin, OUTPUT);

  m_fan_activate_temp_value = Drivhus::getSettings()->getFanActivateTemp();
  m_fan_activate_humid_value = Drivhus::getSettings()->getFanActivateHumidity();

  m_indoor_temp = Drivhus::getSettings()->getIndoorTemp();
  m_indoor_humidity = Drivhus::getSettings()->getIndoorHumidity();
  m_outdoor_temp = Drivhus::getSettings()->getOutdoorTemp();
  m_outdoor_humidity_indoor = Drivhus::getSettings()->getOutdoorAsIndoorHumidity();

  toggle(Drivhus::OFF);
  return true;
}

void Drivhus::Fan::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_event_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_event_time = current_time;
  }

  if ((m_previous_event_time+ON_OFF_INTERVAL_MS)<current_time) {
    bool temp_above_treshold = !isnan(m_indoor_temp) && !isnan(m_outdoor_temp) &&
                               m_indoor_temp > m_fan_activate_temp_value && //Can we get cooler air into the greenhouse?
                               (m_indoor_temp-TEMP_DIFF_TRESHOLD) > m_outdoor_temp;
    bool humidity_above_treshold = !isnan(m_indoor_humidity) && !isnan(m_outdoor_humidity_indoor) &&
                                   m_indoor_humidity > m_fan_activate_humid_value && //Can we get dryer air into the greenhouse?
                                   (m_indoor_humidity-HUMIDITY_DIFF_TRESHOLD) > m_outdoor_humidity_indoor;
    
    if (m_activated && !temp_above_treshold && !humidity_above_treshold) {
      Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, "Toggling fan OFF");
      toggle(Drivhus::OFF);
    } else if (!m_activated && (temp_above_treshold || humidity_above_treshold)) {
      Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, "Toggling fan ON");
      toggle(Drivhus::ON);
    }
  }
}

void Drivhus::Fan::onValueChanged(Drivhus::OnValueChangeListener::Type type, uint8_t /*plant_id*/) {
  if (type==INDOOR_TEMP || type==INDOOR_HUMIDITY || type==OUTDOOR_TEMP || type==OUTDOOR_HUMIDITY) {
    switch(type) {
      case INDOOR_TEMP:
        m_indoor_temp = Drivhus::getSettings()->getIndoorTemp();
        break;
      case INDOOR_HUMIDITY:
        m_indoor_humidity = Drivhus::getSettings()->getIndoorTemp();
        break;
      case OUTDOOR_TEMP:
        m_outdoor_temp = Drivhus::getSettings()->getIndoorTemp();
        break;
      default: break;
    }; 
    m_outdoor_humidity_indoor = Drivhus::getSettings()->getOutdoorAsIndoorHumidity();
  }
}

void Drivhus::Fan::onConfigChanged(Drivhus::OnConfigChangeListener::Type type, uint8_t /*plant_id*/) {
  switch(type) {
    case FAN_ACTIVATE_TEMP:
      m_fan_activate_temp_value = Drivhus::getSettings()->getFanActivateTemp();
      break;
    case FAN_ACTIVATE_HUMIDITY:
      m_fan_activate_humid_value = Drivhus::getSettings()->getFanActivateHumidity();
      break;
    default: break;
  };
}

void Drivhus::Fan::toggle(bool on) {
  m_activated = on;
  m_previous_event_time = millis();
  digitalWrite(m_pin, on ? HIGH : LOW);
  Drivhus::getSettings()->setFanActive(on);
}
