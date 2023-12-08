#include "cd74hc4067.h"

#include <Arduino.h>

#include "global.h"
#include "network.h"
#include "webserver.h"


CD74HC4067::CD74HC4067(uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin)
: m_s0_pin(s0_pin),
  m_s1_pin(s1_pin),
  m_s2_pin(s2_pin),
  m_s3_pin(s3_pin) {
}

bool CD74HC4067::init() {
  pinMode(m_s0_pin, OUTPUT);
  pinMode(m_s1_pin, OUTPUT);
  pinMode(m_s2_pin, OUTPUT);
  pinMode(m_s3_pin, OUTPUT);
  return true;
}

void CD74HC4067::loop() {
}
