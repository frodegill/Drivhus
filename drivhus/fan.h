#ifndef _FAN_H_
#define _FAN_H_

#include <stdint.h>

#include "settings.h"


namespace Drivhus {

class Fan : public OnValueChangeListener
{
private:
  static constexpr unsigned long ON_OFF_INTERVAL_MS = 15000L;

public:
  Fan(uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

protected:
  void onIndoorTempChanged(float value) override {m_temp=value;}

private:
  void toggle(bool on);
  
private:
  uint8_t m_pin;
  unsigned long m_previous_event_time;
  float m_temp;
  bool m_activated;
};

} //namespace

#endif // _FAN_H_
