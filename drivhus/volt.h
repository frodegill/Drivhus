#ifndef _VOLT_H_
#define _VOLT_H_

#include <stdint.h>


class Volt
{
public:
  static constexpr unsigned long POLL_INTERVAL_MS = 30000;
  static constexpr float MAX_VOLT = 14.5f;

public:
  Volt(uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

  [[nodiscard]] float getVolt() const {return m_volt;}

private:
  uint8_t m_pin;
  unsigned long m_previous_sampling_time;
  float m_volt;
};

#endif // _VOLT_H_
