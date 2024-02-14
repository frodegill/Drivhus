#ifndef _WATERLEVEL_H_
#define _WATERLEVEL_H_

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "component.h"


namespace Drivhus {

class Waterlevel : public Component
{
public:
  Waterlevel(uint8_t low_pin, uint8_t high_pin, uint8_t valve_pin);
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "Waterlevel";}

private:
  uint8_t m_low_pin;
  uint8_t m_high_pin;
  uint8_t m_valve_pin;

  bool m_water_valve_is_open;
};

} //namespace

#endif // _WATERLEVEL_H_
