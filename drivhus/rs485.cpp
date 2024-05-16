#include "rs485.h"

#include <sstream>

#include "global.h"

#include "network.h"


Drivhus::RS485::RS485(uint8_t rx_pin, uint8_t tx_pin, uint8_t enable_pin)
: Drivhus::Component(),
  m_rx_pin(rx_pin),
  m_tx_pin(tx_pin),
  m_enable_pin(enable_pin),
  m_previous_complete_scan_time(0L),
  m_previous_scanned_sensor_id(UNDEFINED_ID),
  m_performed_full_scan(false) {
  for (uint32_t i=0; i<SENSOR_PRESENT_UINT32_NEEDED; i++) {
    m_sensor_present[i] = 0;
  }
}

bool Drivhus::RS485::init() {
  Serial2.begin(SERIAL_BAUD, SERIAL_8E1, m_rx_pin, m_tx_pin);
  m_modbus = std::unique_ptr<ModbusRTUMaster>(new ModbusRTUMaster(Serial2, m_enable_pin)); // serial port, driver enable pin for rs-485 (optional)
  m_modbus->begin(Serial2.baudRate());
  m_modbus->setTimeout(MODBUS_TIMEOUT);
  return true;
}

void Drivhus::RS485::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_complete_scan_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_complete_scan_time = current_time;
    return;
  }

  checkIfSensorsShouldBeReassigned();

  if (m_previous_scanned_sensor_id!=UNDEFINED_ID ||
      (m_previous_complete_scan_time+SCAN_INTERVAL_MS)<current_time) {
      //Increase Sensor ID and check if we are done
      if (m_previous_scanned_sensor_id >= (m_performed_full_scan ? DRIVHUS_MAX_ID : MAX_ID)) {
        m_previous_scanned_sensor_id = UNDEFINED_ID;
        m_previous_complete_scan_time = current_time;
        if (!m_performed_full_scan) {
          m_performed_full_scan = true;
        }
        Drivhus::getSettings()->setSensorScanEnded();
        return;
      } else {
        if (m_previous_scanned_sensor_id == UNDEFINED_ID) {
          Drivhus::getSettings()->setSensorScanStarted();
          m_previous_scanned_sensor_id = m_performed_full_scan ? DRIVHUS_MIN_ID : MIN_ID;
        } else {
          m_previous_scanned_sensor_id++;
        }
      }

      if (checkIfSensorIsPresent(m_previous_scanned_sensor_id)) {
        Drivhus::getSettings()->setPlantMoisture(m_previous_scanned_sensor_id, tmp_holding_registers[1]/100.0f);
      }
      Drivhus::getSettings()->setSensorUpdated(m_previous_scanned_sensor_id);
  }
}

bool Drivhus::RS485::isSensorPresent(uint8_t id) const {
  uint8_t index = id>>5;
  uint8_t bit = id&0x1F;
  return (m_sensor_present[index]&(1<<bit)) != 0;
}

std::set<uint8_t> Drivhus::RS485::getPresentSensors() const {
  std::set<uint8_t> present_sensors;
  for (uint8_t i=MIN_ID; i<=MAX_ID; i++) {
    if (isSensorPresent(i)) {
      present_sensors.emplace(i);
    }
  }
  return present_sensors;
}

void Drivhus::RS485::setSensorShouldBeReassigned(uint8_t old_id, uint8_t new_id) {
  const std::lock_guard<std::recursive_mutex> lock(m_reassign_sensor_ids_mutex);
  m_reassign_sensor_ids.emplace_back(old_id, new_id);
}

bool Drivhus::RS485::checkIfSensorIsPresent(uint8_t id) {
  bool is_present = false;
  if (id>=MIN_ID && id<=MAX_ID) {
    m_modbus->clearTimeoutFlag();
    is_present = m_modbus->readHoldingRegisters(id, 0x0001, tmp_holding_registers, 2);
    if (!is_present && m_modbus->getTimeoutFlag()) { //For whatever reason, the Honde Technology moisture sensors seems to fail first call, but then work if called a second time
      is_present = m_modbus->readHoldingRegisters(id, 0x0001, tmp_holding_registers, 2);
    }
  }
  setSensorPresent(id, is_present);
  return is_present;
}

void Drivhus::RS485::setSensorPresent(uint8_t id, bool is_present) {
  uint8_t index = id>>5;
  uint8_t bit = id&0x1F;
  if (is_present) {
    m_sensor_present[index] |= 1<<bit;
  } else {
    m_sensor_present[index] &= ~(1<<bit);
  }
  Drivhus::getSettings()->setEnabled(id, is_present);
}

void Drivhus::RS485::checkIfSensorsShouldBeReassigned() {
  const std::lock_guard<std::recursive_mutex> lock(m_reassign_sensor_ids_mutex);
  if (!m_reassign_sensor_ids.empty()) {
    std::pair<uint8_t, uint8_t> work = m_reassign_sensor_ids.front();
    m_reassign_sensor_ids.pop_front();
    bool result = setNewSensorId(work.first, work.second);

    std::stringstream ss;
    ss << "Reassigning sensor " << work.first << " to " << work.second << (result ? "SUCCEEDED" : "FAILED");
    Drivhus::getNetwork()->getWebServer()->addWarningMessage(ss.str());
  }
}

bool Drivhus::RS485::setNewSensorId(uint8_t old_id, uint8_t new_id, bool force) {
  if (!force &&
      (checkIfSensorIsPresent(new_id) ||
       !checkIfSensorIsPresent(old_id) ||
       new_id<RS485::DRIVHUS_MIN_ID ||
       new_id>RS485::DRIVHUS_MAX_ID)) {
    return false;
  }
  m_modbus->clearTimeoutFlag();
  bool ret = m_modbus->writeSingleHoldingRegister(old_id, 0x0001, new_id);
  if (ret) {
    setSensorPresent(old_id, false);
    setSensorPresent(new_id, true);
    Drivhus::getSettings()->setSensorUpdated(old_id);
    Drivhus::getSettings()->setSensorUpdated(new_id);
  }
  return ret;
}
