#ifndef _SOILSENSORS_H_
#define _SOILSENSORS_H_

#ifdef TESTING
# include "testing.h"
#else
# include <HardwareSerial.h>
#endif

#include "component.h"


namespace Drivhus {

class SoilSensors : public Component
{
public:
  static constexpr uint8_t UNDEFINED_ID = 0;
  static constexpr uint8_t MIN_ID = 1;
  static constexpr uint8_t MAX_ID = 15;

  static constexpr uint8_t SENSOR_COUNT = MAX_ID - MIN_ID + 1;

private:
  static constexpr unsigned long SCAN_INTERVAL_MS = 15000L;
  static constexpr unsigned long SENSOR_ACTIVATE_DELAY_MS = 100L;

public:
  SoilSensors(uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin, uint8_t common_pin);
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "SoilSensors";}

private:
  void activateNextSensor();
  void activate(uint8_t plant_id);
  [[nodiscard]] float getCurrentSensorValue() const;

private:
  uint8_t m_s0_pin;
  uint8_t m_s1_pin;
  uint8_t m_s2_pin;
  uint8_t m_s3_pin;
  uint8_t m_common_pin;

  unsigned long m_previous_sensor_activate_time;
  unsigned long m_previous_complete_scan_time;
  uint8_t m_active_sensor_id;
};

} //namespace

#endif // _SOILSENSORS_H_
