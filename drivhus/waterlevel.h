#ifndef _WATERLEVEL_H_
#define _WATERLEVEL_H_

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "component.h"
#include "settings.h"


namespace Drivhus {

class Waterlevel : public Component, public OnValueChangeListener
{
private:
  static constexpr unsigned long MAXIMUM_VALVE_OPEN_MS = 60*1000L;
  static constexpr unsigned long VALVE_FORCED_CLOSE_TIMEOUT_MS = 15*60*1000L;

public:
  Waterlevel(uint8_t low_pin, uint8_t high_pin, uint8_t valve_pin);
  [[nodiscard]] virtual bool init() override;
  [[nodiscard]] virtual bool postInit() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "Waterlevel";}

protected:
  virtual void onValueChanged(OnValueChangeListener::Type type, uint8_t /*plant_id*/) override;

private:
  uint8_t m_low_pin;
  uint8_t m_high_pin;
  uint8_t m_valve_pin;

  unsigned long m_previous_valve_event_time;
};

} //namespace

#endif // _WATERLEVEL_H_
