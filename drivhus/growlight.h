#ifndef _GROWLIGHT_H_
#define _GROWLIGHT_H_

#include <stdint.h>

#include "component.h"
#include "settings.h"


namespace Drivhus {

class Growlight : public Component, public OnValueChangeListener
{
public:
  Growlight(uint8_t pin);
  virtual [[nodiscard]] bool init() override;
  virtual void loop() override;
  virtual std::string&& getName() {return "GROWLIGHT";}

protected:
  virtual void onSunriseChanged(float value) override {m_sunrise = value;}
  virtual void onSunsetChanged(float value) override {m_sunset = value;}

private:
  void toggle(bool on);

private:
  uint8_t m_pin;
  bool m_activated;
  float m_sunrise;
  float m_sunset;
};

} //namespace

#endif // _GROWLIGHT_H_
