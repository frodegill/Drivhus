#ifndef _GROWLIGHT_H_
#define _GROWLIGHT_H_

#include <stdint.h>

#include "settings.h"


namespace Drivhus {

class Growlight : public OnChangeListener
{
public:
  Growlight(uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

protected:
  virtual void onSunriseChanged(float value) override;
  virtual void onSunsetChanged(float value) override;

private:
  uint8_t m_pin;

  bool m_is_activated;
  float m_sunrise;
  float m_sunset;
};

} //namespace

#endif // _GROWLIGHT_H_
