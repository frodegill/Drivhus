#include "ky018.h"

#ifdef TESTING
#else
# include <Arduino.h>
#endif

#include "global.h"
#include "network.h"
#include "webserver.h"


Drivhus::KY018::KY018(uint8_t pin)
: m_pin(pin),
  m_previous_sampling_time(0L),
  m_light_percentage(0.0f) {
}

bool Drivhus::KY018::init() {
  pinMode(m_pin, INPUT);
  return true;
}

void Drivhus::KY018::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_sampling_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_sampling_time = current_time;
  }

  if ((m_previous_sampling_time+POLL_INTERVAL_MS)<current_time) {
    m_previous_sampling_time = current_time;
    m_light_percentage = 100.0f - analogRead(m_pin)/40.95f;
    Drivhus::getSettings()->setLight(m_light_percentage);
  }
}
