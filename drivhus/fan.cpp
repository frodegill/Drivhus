#include "fan.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "global.h"


Drivhus::Fan::Fan(uint8_t pin)
: Drivhus::OnChangeListener(),
  m_pin(pin),
  m_previous_event_time(0L),
  m_temp(0.0f),
  m_activated(false)
{
  Drivhus::getSettings()->addChangeListener(this);
}

bool Drivhus::Fan::init() {
  pinMode(m_pin, OUTPUT);
  toggle(OFF);
  return true;
}

void Drivhus::Fan::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_event_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_event_time = current_time;
  }

  if ((m_previous_event_time+ON_OFF_INTERVAL_MS)<current_time) {
    if (m_activated && m_temp<Drivhus::getSettings()->getCurrentFanActivateTemp()) {
      toggle(OFF);
    } else if (!m_activated && m_temp>=Drivhus::getSettings()->getCurrentFanActivateTemp()) {
      toggle(ON);
    }
  }
}

void Drivhus::Fan::toggle(bool on) {
  m_activated = on;
  m_previous_event_time = millis();
  digitalWrite(m_pin, on ? HIGH : LOW);
}
