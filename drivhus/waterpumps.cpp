#include "waterpumps.h"

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
#endif

#include "log.h"
#include "plant.h"
#include "settings.h"


Drivhus::WaterPumps::WaterPumps(uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin, uint8_t common_pin)
: Drivhus::Component(),
  Drivhus::OnConfigChangeListener(),
  m_s0_pin(s0_pin),
  m_s1_pin(s1_pin),
  m_s2_pin(s2_pin),
  m_s3_pin(s3_pin),
  m_common_pin(common_pin),
  m_previous_watering_time(0L),
  m_previous_watering_duration(0L),
  m_current_plant_id(0) {
  Drivhus::getSettings()->addConfigChangeListener(this);
}

bool Drivhus::WaterPumps::init() {
  pinMode(m_s0_pin, OUTPUT);
  pinMode(m_s1_pin, OUTPUT);
  pinMode(m_s2_pin, OUTPUT);
  pinMode(m_s3_pin, OUTPUT);
  pinMode(m_common_pin, OUTPUT);
  return true;
}

bool Drivhus::WaterPumps::postInit() {
  deactivate();
  return true;
}

void Drivhus::WaterPumps::loop() {
  const unsigned long current_time = millis();

  if (m_previous_watering_time != 0L) { //If currently watering, and enough time has passed, stop watering
    if (current_time < m_previous_watering_time || //Time will wrap around every ~50 days. Abort current event
        (m_previous_watering_time+m_previous_watering_duration) < current_time) { //Current event completed
      deactivate();
    }
  } else {
    if (++m_current_plant_id > Drivhus::SoilSensors::MAX_ID) {
      m_current_plant_id = Drivhus::SoilSensors::MIN_ID;
    }

    auto settings = Drivhus::getSettings();
    if (!settings->getEnabled(m_current_plant_id) && !settings->getRequestWatering(m_current_plant_id)) {
      if (settings->getIsInWateringCycle(m_current_plant_id)) {
        deactivate();
      }
    } else {
      if (current_time < settings->getPreviousWateringTimeMs(m_current_plant_id)) { //Time will wrap around every ~50 days.
        settings->setForceUpdateWateringTimeMs(m_current_plant_id, current_time);
      }

      if (settings->getIsInWateringCycle(m_current_plant_id) && settings->getPlantMoisture(m_current_plant_id) >= settings->getWetValue(m_current_plant_id)) {
        settings->setIsInWateringCycle(m_current_plant_id, false);
      }

      bool activate_watering = false;
      if (settings->getRequestWatering(m_current_plant_id)) { //Manual request should overrule anything
        activate_watering = true;
      }
      if (settings->getEnabled(m_current_plant_id) && //If plant is enabled and we are not in grace period
          (settings->getPreviousWateringTimeMs(m_current_plant_id)+settings->getWateringGracePeriodMs(m_current_plant_id)) < current_time)
      {
        if (settings->getPlantMoisture(m_current_plant_id) < settings->getDryValue(m_current_plant_id)) { //If plant is too dry
          activate_watering = true;
        }
        if (settings->getPlantMoisture(m_current_plant_id) < settings->getWetValue(m_current_plant_id) && //If plant is in expected moisture, but we are in watering cycle and should water until it is borderline wet
            settings->getIsInWateringCycle(m_current_plant_id)) {
          activate_watering = true;
        }
      }

      if (activate_watering) {
        activate(m_current_plant_id);
      }
    }
  }
}

void Drivhus::WaterPumps::onConfigChanged(Drivhus::OnConfigChangeListener::Type /*type*/, uint8_t /*plant_id*/) {
}

void Drivhus::WaterPumps::activate(uint8_t plant_id) {
  Drivhus::ValveStatus valve_status = Drivhus::getSettings()->getWaterValveStatus();
  digitalWrite(m_s0_pin, (plant_id&(1<<0))==0 ? LOW : HIGH);
  digitalWrite(m_s1_pin, (plant_id&(1<<1))==0 ? LOW : HIGH);
  digitalWrite(m_s2_pin, (plant_id&(1<<2))==0 ? LOW : HIGH);
  digitalWrite(m_s3_pin, (plant_id&(1<<3))==0 ? LOW : HIGH);
  digitalWrite(m_common_pin, plant_id==0 || valve_status==Drivhus::ValveStatus::NO_WATER ? LOW : HIGH);

  if (plant_id == 0) {
    m_previous_watering_time = 0L;
    m_previous_watering_duration = 0L;
  } else {
    m_previous_watering_time = millis();
    m_previous_watering_duration = Drivhus::getSettings()->getWateringDuration(plant_id);
    Drivhus::getSettings()->setWateringStarted(plant_id);
    Drivhus::getSettings()->setIsInWateringCycle(plant_id, true);
  }
}

void Drivhus::WaterPumps::deactivate() {
  if (Drivhus::isValidPlantId(m_current_plant_id)) {
    Drivhus::getSettings()->setWateringEnded(m_current_plant_id);
  }
  activate(0);
}
