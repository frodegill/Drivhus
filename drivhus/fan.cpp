#include "fan.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "global.h"


Drivhus::Fan::Fan(uint8_t pin)
: m_pin(pin)
{
}

bool Drivhus::Fan::init() {
  pinMode(m_pin, OUTPUT);
  return true;
}

void Drivhus::Fan::loop() {
}
