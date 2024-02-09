#ifndef _VOLT_H_
#define _VOLT_H_

#include <stdint.h>

#include "component.h"


namespace Drivhus {

class Volt : public Component
{
public:
  static constexpr unsigned long POLL_INTERVAL_MS = 30000;
  static constexpr float MAX_VOLT = 14.5f;

public:
  Volt(uint8_t pin);
  virtual [[nodiscard]] bool init() override;
  virtual void loop() override;

private:
  uint8_t m_pin;
  unsigned long m_previous_sampling_time;
};

} //namespace

#endif // _VOLT_H_
