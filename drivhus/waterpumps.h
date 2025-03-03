#ifndef _WATERPUMPS_H_
#define _WATERPUMPS_H_

#include "component.h"
#include "settings.h"


namespace Drivhus {

class WaterPumps : public Component, public OnConfigChangeListener
{
public:
  WaterPumps(uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin, uint8_t common_pin);
  [[nodiscard]] virtual bool init() override;
  [[nodiscard]] virtual bool postInit() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "CD74HC4067";}

protected:
  virtual void onConfigChanged(OnConfigChangeListener::Type type, uint8_t /*plant_id*/) override;

public:
  [[nodiscard]] bool isBusy() const {return m_previous_watering_time!=0L;}
  [[nodiscard]] bool isWateringPlant(uint8_t id) const {return Drivhus::isValidPlantId(id) && id==m_current_plant_id && isBusy();}

private:
  void activate(uint8_t plant_id);
  void deactivate();
  
private:
  uint8_t m_s0_pin;
  uint8_t m_s1_pin;
  uint8_t m_s2_pin;
  uint8_t m_s3_pin;
  uint8_t m_common_pin;

  unsigned long m_previous_watering_time;
  unsigned long m_previous_watering_duration;
  uint8_t m_current_plant_id;
};

} //namespace

#endif // _WATERPUMPS_H_
