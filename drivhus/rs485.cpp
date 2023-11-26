#include "rs485.h"

#include <sstream>

#include "global.h"

#include "network.h"


RS485::RS485()
: m_previous_complete_scan_time(0L),
  m_previous_scanned_sensor_id(UNDEFINED_ID),
  m_performed_full_scan(false) {
  int i;
  for (i=0; i<8; i++) {
    m_sensor_present[i] = 0;
  }
  for (i=0; i<15; i++) {
    m_sensor_temp[i] = 0x00;
    m_sensor_humidity[i] = 0x00;
  }
}

bool RS485::init() {
  Serial2.begin(SERIAL_BAUD, SERIAL_8E1, RS485_RX_PIN, RS485_TX_PIN);
  m_modbus = std::unique_ptr<ModbusRTUMaster>(new ModbusRTUMaster(Serial2, RS485_ENABLE_PIN)); // serial port, driver enable pin for rs-485 (optional)
  m_modbus->begin(Serial2.baudRate());
  m_modbus->setTimeout(MODBUS_TIMEOUT);
  return true;
}

bool RS485::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_complete_scan_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_complete_scan_time = current_time;
    return true;
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
        ::getNetwork()->getWebServer()->setSensorScanCompleted();
        return true;
      } else {
        m_previous_scanned_sensor_id = (m_previous_scanned_sensor_id==UNDEFINED_ID) ? (m_performed_full_scan ? DRIVHUS_MIN_ID : MIN_ID) : m_previous_scanned_sensor_id+1;
      }

      bool is_present = checkIfSensorIsPresent(m_previous_scanned_sensor_id);
      if (is_present) {
        setSensorValues(m_previous_scanned_sensor_id, tmp_holding_registers[0], tmp_holding_registers[1]);
      }
      ::getNetwork()->getWebServer()->updateSensor(m_previous_scanned_sensor_id);
  }
  return true;
}

bool RS485::isSensorPresent(uint8_t id) const {
  uint8_t index = id>>5;
  uint8_t bit = id&0x1F;
  return (m_sensor_present[index]&(1<<bit)) != 0;
}

std::set<uint8_t> RS485::getPresentSensors() const {
  std::set<uint8_t> present_sensors;
  for (uint8_t i=MIN_ID; i<=MAX_ID; i++) {
    if (isSensorPresent(i)) {
      present_sensors.emplace(i);
    }
  }
  return present_sensors;
}

float RS485::getSensorTemp(uint8_t id) {
  if (id<DRIVHUS_MIN_ID || id>DRIVHUS_MAX_ID || !isSensorPresent(id))
    return 0.0f;

  return m_sensor_temp[id-1]/10.0f;
}

float RS485::getSensorHumidity(uint8_t id) {
  if (id<DRIVHUS_MIN_ID || id>DRIVHUS_MAX_ID || !isSensorPresent(id))
    return 0.0f;

  return m_sensor_humidity[id-1]/100.0f;
}

void RS485::setSensorShouldBeReassigned(uint8_t old_id, uint8_t new_id) {
  const std::lock_guard<std::recursive_mutex> lock(m_reassign_sensor_ids_mutex);
  m_reassign_sensor_ids.emplace_back(old_id, new_id);
}

bool RS485::checkIfSensorIsPresent(uint8_t id) {
  bool is_present = false;
  if (id>=MIN_ID && id<=MAX_ID) {
    m_modbus->clearTimeoutFlag();
    is_present = m_modbus->readHoldingRegisters(id, 0x0001, tmp_holding_registers, 2);
    if (!is_present && m_modbus->getTimeoutFlag()) { //For whatever reason, the Honde Technology moisture sensors seems to fail first call, but then work if called a second time
      is_present = m_modbus->readHoldingRegisters(id, 0x0001, tmp_holding_registers, 2);
#if 0
      if (id>=1 && id<=4) {
        Serial.print("Sensor ");
        Serial.print(id);
        Serial.print(" was retried, and ");
        Serial.println(is_present ? "FOUND" : "NOT FOUND");
      }
#endif
    }
  }
  setSensorPresent(id, is_present);
  return is_present;
}

void RS485::setSensorPresent(uint8_t id, bool is_present) {
  uint8_t index = id>>5;
  uint8_t bit = id&0x1F;
  if (is_present) {
    m_sensor_present[index] |= 1<<bit;
  } else {
    m_sensor_present[index] &= ~(1<<bit);
  }
}

void RS485::setSensorValues(uint8_t id, uint16_t temp, uint16_t humidity) {
  if (id>=DRIVHUS_MIN_ID && id<=DRIVHUS_MAX_ID && isSensorPresent(id)) {
    m_sensor_temp[id-1] = temp;
    m_sensor_humidity[id-1] = humidity;
  }
}

void RS485::checkIfSensorsShouldBeReassigned() {
  const std::lock_guard<std::recursive_mutex> lock(m_reassign_sensor_ids_mutex);
  if (!m_reassign_sensor_ids.empty()) {
    std::pair<uint8_t, uint8_t> work = m_reassign_sensor_ids.front();
    m_reassign_sensor_ids.pop_front();
    bool result = setNewSensorId(work.first, work.second);

    std::stringstream ss;
    ss << "Reassigning sensor " << work.first << " to " << work.second << (result ? "SUCCEEDED" : "FAILED");
    ::getNetwork()->getWebServer()->addWarningMessage(ss.str());
  }
}

bool RS485::setNewSensorId(uint8_t old_id, uint8_t new_id, bool force) {
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
    ::getNetwork()->getWebServer()->updateSensor(old_id);
    ::getNetwork()->getWebServer()->updateSensor(new_id);
  }
  return ret;
}
