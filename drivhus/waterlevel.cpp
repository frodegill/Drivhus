#include "waterlevel.h"


Drivhus::Waterlevel::Waterlevel(uint8_t low_pin, uint8_t high_pin, uint8_t valve_pin)
: m_low_pin(low_pin),
  m_high_pin(high_pin),
  m_valve_pin(valve_pin),
  m_water_valve_is_open(false) {
}

bool Drivhus::Waterlevel::init() {
  pinMode(m_low_pin, INPUT);
  pinMode(m_high_pin, INPUT);
  pinMode(m_valve_pin, OUTPUT);
  return true;
}

void Drivhus::Waterlevel::loop() {
  if (m_water_valve_is_open && digitalRead(m_high_pin)==HIGH) {
    digitalWrite(m_valve_pin, LOW);
    m_water_valve_is_open = false;
  } else if (!m_water_valve_is_open && digitalRead(m_low_pin)==LOW) {
    digitalWrite(m_valve_pin, HIGH);
    m_water_valve_is_open = true;
  }
}
