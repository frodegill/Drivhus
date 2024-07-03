#include "growlight.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "global.h"
#include "ntp.h"


Drivhus::Growlight::Growlight(uint8_t pin)
: Drivhus::Component(),
  Drivhus::OnValueChangeListener(),
  m_pin(pin),
  m_activated(false),
  m_sunrise(0.0f),
  m_sunset(0.0f)
{
  Drivhus::getSettings()->addValueChangeListener(this);
}

bool Drivhus::Growlight::init() {
  pinMode(m_pin, OUTPUT);
  toggle(Drivhus::OFF);
  return true;
}

void Drivhus::Growlight::loop() {
  const struct tm* local_tm = Drivhus::getNTP()->getLocalTm();
  if (local_tm != nullptr) {
    bool within_growlight_period;
    if (m_sunrise == m_sunset) {
      within_growlight_period = false;
    } else {
      float day_pos = static_cast<float>(local_tm->tm_hour)*60.0f + static_cast<float>(local_tm->tm_min) + static_cast<float>(local_tm->tm_sec)/60.0f;
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

void Drivhus::Growlight::onValueChanged(Drivhus::OnValueChangeListener::Type type, uint8_t /*plant_id*/) {
  switch(type) {
    case SUNRISE:
      m_sunrise = Drivhus::getSettings()->getSunrise();
      break;
    case SUNSET:
      m_sunset = Drivhus::getSettings()->getSunset();
      break;
    default: break;
  };
}

void Drivhus::Growlight::toggle(bool on) {
  m_activated = on;
  digitalWrite(m_pin, on ? HIGH : LOW);
}

