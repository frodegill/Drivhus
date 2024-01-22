#include "growlight.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "global.h"


Drivhus::Growlight::Growlight(uint8_t pin)
: m_pin(pin),
  m_is_activated(false)
{
}

bool Drivhus::Growlight::init() {
  pinMode(m_pin, OUTPUT);
  digitalWrite(m_pin, m_is_activated ? HIGH : LOW);
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

    if (within_growlight_period != m_is_activated) {
      digitalWrite(m_pin, within_growlight_period ? HIGH : LOW);
      m_is_activated = within_growlight_period;
    }
  }
}

void Drivhus::Growlight::onSunriseChanged(float value) {
  m_sunrise = value;
}

void Drivhus::Growlight::onSunsetChanged(float value) {
  m_sunset = value;
}
