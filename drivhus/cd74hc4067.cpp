#include "cd74hc4067.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "global.h"


Drivhus::CD74HC4067::CD74HC4067(uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin, uint8_t common_pin)
: Drivhus::Component(),
  m_s0_pin(s0_pin),
  m_s1_pin(s1_pin),
  m_s2_pin(s2_pin),
  m_s3_pin(s3_pin),
  m_common_pin(common_pin),
  m_activate_time(0L),
  m_activate_duration(0L)
{
}

bool Drivhus::CD74HC4067::init() {
  pinMode(m_s0_pin, OUTPUT);
  pinMode(m_s1_pin, OUTPUT);
  pinMode(m_s2_pin, OUTPUT);
  pinMode(m_s3_pin, OUTPUT);
  pinMode(m_common_pin, OUTPUT);
  reset();
  return true;
}

void Drivhus::CD74HC4067::loop() {
  if (m_activate_time != 0L) {
    const unsigned long current_time = millis();
    if (current_time < m_activate_time || //Time will wrap around every ~50 days. Abort current activation
        (m_activate_time+m_activate_duration)<current_time) { //activation duration completed
      reset();
    }
  }
}

void Drivhus::CD74HC4067::activate(uint8_t address, unsigned long duration) {
  digitalWrite(m_s0_pin, (address&(1<<0))==0 ? LOW : HIGH);
  digitalWrite(m_s1_pin, (address&(1<<1))==0 ? LOW : HIGH);
  digitalWrite(m_s2_pin, (address&(1<<2))==0 ? LOW : HIGH);
  digitalWrite(m_s3_pin, (address&(1<<3))==0 ? LOW : HIGH);

  if (address != 0) {
    digitalWrite(m_common_pin, HIGH);
    m_activate_time = millis();
    m_activate_duration = duration;
  }
}

void Drivhus::CD74HC4067::reset() {
  digitalWrite(m_common_pin, LOW);
  activate(0, 0L);
  m_activate_time = 0L;
}
