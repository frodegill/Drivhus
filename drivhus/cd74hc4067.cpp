#include "cd74hc4067.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "global.h"


Drivhus::CD74HC4067::CD74HC4067(uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin, uint8_t common_pin)
: m_s0_pin(s0_pin),
  m_s1_pin(s1_pin),
  m_s2_pin(s2_pin),
  m_s3_pin(s3_pin),
  m_common_pin(common_pin) {
}

bool Drivhus::CD74HC4067::init() {
  pinMode(m_s0_pin, OUTPUT);
  pinMode(m_s1_pin, OUTPUT);
  pinMode(m_s2_pin, OUTPUT);
  pinMode(m_s3_pin, OUTPUT);
  pinMode(m_common_pin, OUTPUT);
  return true;
}

void Drivhus::CD74HC4067::loop() {
}

void Drivhus::CD74HC4067::setActiveAddress(uint8_t address) {
  digitalWrite(m_common_pin, LOW);

  digitalWrite(m_s0_pin, (address&(1<<0))==0 ? LOW : HIGH);
  digitalWrite(m_s1_pin, (address&(1<<1))==0 ? LOW : HIGH);
  digitalWrite(m_s2_pin, (address&(1<<2))==0 ? LOW : HIGH);
  digitalWrite(m_s3_pin, (address&(1<<3))==0 ? LOW : HIGH);

  if (address != 0) {
    digitalWrite(m_common_pin, HIGH);
  }
}

void Drivhus::CD74HC4067::deactivate() {
  setActiveAddress(0);
}
