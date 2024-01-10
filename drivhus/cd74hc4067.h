#ifndef _CD74HC4067_H_
#define _CD74HC4067_H_

#include <stdint.h>

namespace Drivhus {

class CD74HC4067
{
public:
  static constexpr unsigned long POLL_INTERVAL_MS = 30000;

public:
  CD74HC4067(uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin, uint8_t common_pin);
  [[nodiscard]] bool init();
  void loop();

  void activate(uint8_t address, unsigned long duration);
  [[nodiscard]] bool isActive() const {return m_activate_time!=0L;}

private:
  void reset();
  
private:
  uint8_t m_s0_pin;
  uint8_t m_s1_pin;
  uint8_t m_s2_pin;
  uint8_t m_s3_pin;
  uint8_t m_common_pin;
  unsigned long m_activate_time;
  unsigned long m_activate_duration;
};

} //namespace

#endif // _CD74HC4067_H_
