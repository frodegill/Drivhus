#ifndef _WATERLEVEL_H_
#define _WATERLEVEL_H_

#include <Arduino.h>


class Waterlevel
{
public:
  Waterlevel(uint8_t low_pin, uint8_t high_pin, uint8_t valve_pin);
  [[nodiscard]] bool init();
  void loop();

private:
  uint8_t m_low_pin;
  uint8_t m_high_pin;
  uint8_t m_valve_pin;

  bool m_water_valve_is_open;
};

#endif // _WATERLEVEL_H_
