#include "volt.h"

#include <Arduino.h>

#include "global.h"
#include "network.h"
#include "settings.h"
#include "webserver.h"


Volt::Volt(uint8_t pin)
: m_pin(pin),
  m_previous_sampling_time(0L),
  m_volt(0.0f) {
}

bool Volt::init() {
  pinMode(m_pin, INPUT);
  return true;
}

void Volt::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_sampling_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_sampling_time = current_time;
  }

  if ((m_previous_sampling_time+POLL_INTERVAL_MS)<current_time) {
    m_previous_sampling_time = current_time;
    m_volt = analogRead(m_pin)/4095.0f * MAX_VOLT * ::getSettings()->getVoltMultiplier();
    ::getNetwork()->getWebServer()->updateVolt(m_volt);
  }
}
