#ifndef _KY018_H_
#define _KY018_H_

#include <stdint.h>

namespace Drivhus {

class KY018
{
public:
  static constexpr unsigned long POLL_INTERVAL_MS = 30000;

public:
  KY018(uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

  [[nodiscard]] float getLightPercentage() const {return m_light_percentage;}

private:
  uint8_t m_pin;
  unsigned long m_previous_sampling_time;
  float m_light_percentage;
};

} //namespace

#endif // _KY018_H_
