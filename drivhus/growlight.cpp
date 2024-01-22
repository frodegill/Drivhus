#include "growlight.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "global.h"


Drivhus::Growlight::Growlight(uint8_t pin)
: Drivhus::OnChangeListener(),
  m_pin(pin),
  m_sunrise(0.0f),
  m_sunset(0.0f),
  m_activated(false)
{
  Drivhus::getSettings()->addChangeListener(this);
}

bool Drivhus::Growlight::init() {
  pinMode(m_pin, OUTPUT);
  toggle(OFF);
  return true;
}

void Drivhus::Growlight::loop() {
  const struct tm* local_tm = Drivhus::getNTP()->getLocalTm();
  if (local_tm != nullptr) {
    bool within_growlight_period;
    if (m_sunrise == m_sunset) {
      within_growlight_period = false;
    } else {
      float day_pos = local_tm->tm_hour*60 + local_tm->tm_min + local_tm->tm_sec/60.0;
      bool within_growlight_period = (day_pos>=m_sunrise && day_pos<=m_sunset);
      if (m_sunrise > m_sunset) {
        within_growlight_period = !within_growlight_period;
      }
    }

    if (within_growlight_period != m_activated) {
      toggle(within_growlight_period);
    }
  }
}

void Drivhus::Growlight::toggle(bool on) {
  m_activated = on;
  digitalWrite(m_pin, on ? HIGH : LOW);
}

