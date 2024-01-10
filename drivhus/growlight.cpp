#include "growlight.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "global.h"


Drivhus::Growlight::Growlight(uint8_t pin)
: m_pin(pin)
{
}

bool Drivhus::Growlight::init() {
  pinMode(m_pin, OUTPUT);
  return true;
}

void Drivhus::Growlight::loop() {
}
