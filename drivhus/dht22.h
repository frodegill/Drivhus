#ifndef _DHT22_H_
#define _DHT22_H_

#ifdef TESTING
# include "testing.h"
#else
# include <DHTesp.h>           // Library: DHT_sensor_library_for_ESPx
#endif

#include "component.h"


namespace Drivhus {

class DHT22 : public Component
{
public:
  DHT22(uint8_t id, uint8_t pin);
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "DHT22";}

  [[nodiscard]] bool isPresent() const {return m_is_present;}

private:
  uint8_t m_id;
  uint8_t m_pin;
  DHTesp m_dht;
  unsigned long m_previous_sampling_time;
  bool m_is_present;
};

} //namespace

#endif // _DHT22_H_
