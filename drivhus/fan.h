#ifndef _FAN_H_
#define _FAN_H_

#include <stdint.h>

#include "component.h"
#include "settings.h"


namespace Drivhus {

class Fan : public Component, public OnValueChangeListener
{
private:
  static constexpr unsigned long ON_OFF_INTERVAL_MS = 15000L;
  static constexpr float TEMP_DIFF_TRESHOLD = 1.0f; //C
  static constexpr float HUMIDITY_DIFF_TRESHOLD = 1.0f; //%RH

public:
  Fan(uint8_t pin);
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "Fan";}

protected:
  virtual void onValueChanged(Type type, uint8_t /*plant_id*/) override;

private:
  void toggle(bool on);
  void calculateOutdoorHumidityIndoor();
  
private:
  uint8_t m_pin;
  unsigned long m_previous_event_time;
  float m_indoor_temp;
  float m_indoor_humidity;
  float m_outdoor_temp;
  float m_outdoor_humidity_indoor;
  bool m_activated;
};

} //namespace

#endif // _FAN_H_
