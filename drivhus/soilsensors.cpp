#include "soilsensors.h"

#include "global.h"

#include "log.h"
#include "network.h"
#include "webserver.h"


Drivhus::SoilSensors::SoilSensors(uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin, uint8_t common_pin)
: Drivhus::Component(),
  m_s0_pin(s0_pin),
  m_s1_pin(s1_pin),
  m_s2_pin(s2_pin),
  m_s3_pin(s3_pin),
  m_common_pin(common_pin),
  m_previous_sensor_activate_time(0L),
  m_previous_complete_scan_time(0L),
  m_active_sensor_id(UNDEFINED_ID) {
}

bool Drivhus::SoilSensors::init() {
  pinMode(m_s0_pin, OUTPUT);
  pinMode(m_s1_pin, OUTPUT);
  pinMode(m_s2_pin, OUTPUT);
  pinMode(m_s3_pin, OUTPUT);
  pinMode(m_common_pin, INPUT);
  return true;
}

void Drivhus::SoilSensors::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_sensor_activate_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_sensor_activate_time = current_time;
    return;
  }
  if (current_time < m_previous_complete_scan_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_complete_scan_time = current_time;
    return;
  }

  if (m_previous_sensor_activate_time!=0 &&
     (m_previous_sensor_activate_time+SENSOR_ACTIVATE_DELAY_MS) < current_time) {
    if (Drivhus::getSettings()->getEnabled(m_active_sensor_id)) {
      Drivhus::getSettings()->setPlantMoisture(m_active_sensor_id, getCurrentSensorValue());
    }
    activateNextSensor();
  } else if (m_active_sensor_id==UNDEFINED_ID &&
      (m_previous_complete_scan_time+SCAN_INTERVAL_MS)<current_time) {
      activateNextSensor();
  }
}

void Drivhus::SoilSensors::activateNextSensor() {
  m_active_sensor_id = (m_active_sensor_id == UNDEFINED_ID) ? MIN_ID : ++m_active_sensor_id;
  if (m_active_sensor_id > MAX_ID) {
    m_active_sensor_id = UNDEFINED_ID;
    m_previous_complete_scan_time = millis();
  }

  activate(m_active_sensor_id);
}

void Drivhus::SoilSensors::activate(uint8_t plant_id) {
  digitalWrite(m_s0_pin, (plant_id&(1<<0))==0 ? LOW : HIGH);
  digitalWrite(m_s1_pin, (plant_id&(1<<1))==0 ? LOW : HIGH);
  digitalWrite(m_s2_pin, (plant_id&(1<<2))==0 ? LOW : HIGH);
  digitalWrite(m_s3_pin, (plant_id&(1<<3))==0 ? LOW : HIGH);
  m_previous_sensor_activate_time = (plant_id==UNDEFINED_ID) ? 0L : millis();
}

float Drivhus::SoilSensors::getCurrentSensorValue() const {
  return analogRead(m_common_pin)/40.95f;
}
