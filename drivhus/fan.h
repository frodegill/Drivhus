#ifndef _FAN_H_
#define _FAN_H_

#include <stdint.h>

namespace Drivhus {

class Fan
{
public:
  Fan(uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

private:
  uint8_t m_pin;
};

} //namespace

#endif // _FAN_H_
