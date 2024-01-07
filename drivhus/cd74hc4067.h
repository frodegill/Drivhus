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

  void setActiveAddress(uint8_t address);
  void deactivate();
  
private:
  uint8_t m_s0_pin;
  uint8_t m_s1_pin;
  uint8_t m_s2_pin;
  uint8_t m_s3_pin;
  uint8_t m_common_pin;
};

} //namespace

#endif // _CD74HC4067_H_
