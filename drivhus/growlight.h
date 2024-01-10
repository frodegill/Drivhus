#ifndef _GROWLIGHT_H_
#define _GROWLIGHT_H_

#include <stdint.h>

namespace Drivhus {

class Growlight
{
public:
  Growlight(uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

private:
  uint8_t m_pin;
};

} //namespace

#endif // _GROWLIGHT_H_
