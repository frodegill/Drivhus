#include "volt.h"

#ifdef TESTING
#else
# include <Arduino.h>
#endif

#include "global.h"
#include "network.h"
#include "settings.h"
#include "webserver.h"


Drivhus::Volt::Volt(uint8_t pin)
: Drivhus::Component(),
  m_pin(pin),
  m_previous_sampling_time(0L) {
}

bool Drivhus::Volt::init() {
  pinMode(m_pin, INPUT);
  return true;
}

void Drivhus::Volt::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_sampling_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_sampling_time = current_time;
  }

  if ((m_previous_sampling_time+POLL_INTERVAL_MS)<current_time) {
    m_previous_sampling_time = current_time;
    float volt = analogRead(m_pin)/4095.0f * MAX_VOLT * Drivhus::getSettings()->getVoltMultiplier();
    Drivhus::getSettings()->setVolt(volt);
  }
}
