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
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "GROWLIGHT";}

protected:
  virtual void onValueChanged(OnValueChangeListener::Type type, uint8_t /*plant_id*/) override;

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
