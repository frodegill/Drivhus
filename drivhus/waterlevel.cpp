#include "waterlevel.h"

#include "log.h"


Drivhus::Waterlevel::Waterlevel(uint8_t low_pin, uint8_t high_pin, uint8_t valve_pin)
: Drivhus::Component(),
  Drivhus::OnValueChangeListener(),
  m_low_pin(low_pin),
  m_high_pin(high_pin),
  m_valve_pin(valve_pin),
  m_previous_valve_event_time(0L) {
  Drivhus::getSettings()->addValueChangeListener(this);
}

bool Drivhus::Waterlevel::init() {
  pinMode(m_low_pin, INPUT);
  pinMode(m_high_pin, INPUT);
  pinMode(m_valve_pin, OUTPUT);
  return true;
}

bool Drivhus::Waterlevel::postInit() {
  Drivhus::getSettings()->setWaterValveStatus(Drivhus::ValveStatus::CLOSED);
  return true;
}

void Drivhus::Waterlevel::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_valve_event_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_valve_event_time = current_time;
  }

  int low_trigger_value = digitalRead(m_low_pin);
  if (low_trigger_value != Drivhus::getSettings()->getWaterLowTrigger()) {
    Drivhus::getSettings()->setWaterLowTrigger(low_trigger_value);
  }
  int high_trigger_value = digitalRead(m_high_pin);
  if (high_trigger_value != Drivhus::getSettings()->getWaterHighTrigger()) {
    Drivhus::getSettings()->setWaterHighTrigger(high_trigger_value);
  }

  switch(Drivhus::getSettings()->getWaterValveStatus()) {
    case ValveStatus::NO_WATER:
      if ((m_previous_valve_event_time+VALVE_FORCED_CLOSE_TIMEOUT_MS)<current_time) { //If grace period is over, open valve and see if there is water now
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("Water valve grace period passed"));
        m_previous_valve_event_time = millis();
        Drivhus::getSettings()->setWaterValveStatus(Drivhus::ValveStatus::OPEN);
      }
      break;
    case ValveStatus::OPEN:
      if ((m_previous_valve_event_time+MAXIMUM_VALVE_OPEN_MS)<current_time) { //Valve have been open too long. No more reservoir water?
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("Water valve open too long"));
        m_previous_valve_event_time = millis();
        Drivhus::getSettings()->setWaterValveStatus(Drivhus::ValveStatus::NO_WATER);
      }
      break;
    default:
      break;
  }
}

void Drivhus::Waterlevel::onValueChanged(Drivhus::OnValueChangeListener::Type type, uint8_t /*plant_id*/) {
  switch(type) {
    case WATER_LOW_TRIGGER: {
      if (Drivhus::getSettings()->getWaterValveStatus()==Drivhus::ValveStatus::CLOSED &&
          Drivhus::getSettings()->getWaterLowTrigger()==HIGH) {
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("Low trigger is HIGH. Opening water valve"));
        m_previous_valve_event_time = millis();
        Drivhus::getSettings()->setWaterValveStatus(Drivhus::ValveStatus::OPEN);
      }
      break;
    }
    case WATER_HIGH_TRIGGER: {
      if (Drivhus::getSettings()->getWaterValveStatus()!=Drivhus::ValveStatus::CLOSED &&
          Drivhus::getSettings()->getWaterHighTrigger()==LOW) {
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("High trigger is LOW. Closing water valve"));
        m_previous_valve_event_time = millis();
        Drivhus::getSettings()->setWaterValveStatus(Drivhus::ValveStatus::CLOSED);
      }
      break;
    }
    case WATER_VALVE: {
      ValveStatus status = Drivhus::getSettings()->getWaterValveStatus();
      Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string(status==Drivhus::ValveStatus::OPEN ? "Opening water valve" : "Closing water valve"));
      digitalWrite(m_valve_pin, status==Drivhus::ValveStatus::OPEN ? HIGH : LOW);
      break;
    }
    default: break;
  };
}
