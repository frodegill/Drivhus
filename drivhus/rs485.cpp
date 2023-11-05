#include "rs485.h"

#include "global.h"


RS485::RS485()
: m_previous_complete_scan_time(0L),
  m_previous_scanned_sensor_id(RS485_UNDEFINED_ID) {
  int i;
  for (i=0; i<8; i++) {
    m_sensor_present[i] = 0;
  }
  for (i=0; i<15; i++) {
    m_sensor_temp[15] = 0x00;
    m_sensor_humidity[i] = 0x00;
  }
}

bool RS485::init() {
  Serial2.begin(RS485_SERIAL_BAUD, SERIAL_8E1, RS485_RX_PIN, RS485_TX_PIN);
  m_modbus = std::unique_ptr<ModbusRTUMaster>(new ModbusRTUMaster(Serial2, RS485_ENABLE_PIN)); // serial port, driver enable pin for rs-485 (optional)
  m_modbus->begin(Serial2.baudRate());
  m_modbus->setTimeout(200);
  return true;
}

bool RS485::loop(const unsigned long& current_time) {
  if (current_time < m_previous_complete_scan_time) { //Time will wrap around every ~50 days. DOn't consider this an error
    m_previous_complete_scan_time = current_time;
    return true;
  }

  if (m_previous_scanned_sensor_id!=RS485_UNDEFINED_ID ||
      (m_previous_complete_scan_time+RS485_SCAN_INTERVAL_MS)<current_time) {
      
      //Increase Sensor ID and check if we are done
      if (m_previous_scanned_sensor_id == RS485_MAX_ID) {
        m_previous_scanned_sensor_id = RS485_UNDEFINED_ID;
        m_previous_complete_scan_time = current_time;
        return true;
      } else {
        m_previous_scanned_sensor_id = (m_previous_scanned_sensor_id==RS485_UNDEFINED_ID) ? RS485_MIN_ID : m_previous_scanned_sensor_id+1;
      }

      bool is_present = m_modbus->readHoldingRegisters(m_previous_scanned_sensor_id, 0x0001, tmp_holding_registers, 2);
      setSensorPresent(m_previous_scanned_sensor_id, is_present);
      if (is_present) {
        setSensorValues(m_previous_scanned_sensor_id, tmp_holding_registers[0], tmp_holding_registers[1]);
      }
  }
  return true;
}

bool RS485::isSensorPresent(uint8_t id) const {
  uint8_t index = id>>5;
  uint8_t bit = id&0x1F;
  return (m_sensor_present[index]&(1<<bit)) != 0;
}

std::vector<uint8_t> RS485::presentSensors() const {
  std::vector<uint8_t> present_sensors;
  for (uint8_t i=RS485_MIN_ID; i<=RS485_MAX_ID; i++) {
    if (isSensorPresent(i)) {
      present_sensors.emplace_back(i);
    }
  }
  return present_sensors;
}

float RS485::getSensorTemp(uint8_t id) {
  if (id<DRIVHUS_RS485_MIN_ID || id>DRIVHUS_RS485_MAX_ID || !isSensorPresent(id))
    return 0.0f;

  return m_sensor_temp[id-1]/10.0f;
}

float RS485::getSensorHumidity(uint8_t id) {
  if (id<DRIVHUS_RS485_MIN_ID || id>DRIVHUS_RS485_MAX_ID || !isSensorPresent(id))
    return 0.0f;

  return m_sensor_humidity[id-1]/100.0f;
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
  if (id>=DRIVHUS_RS485_MIN_ID && id<=DRIVHUS_RS485_MAX_ID && isSensorPresent(id)) {
    m_sensor_temp[id-1] = temp;
    m_sensor_humidity[id-1] = humidity;
  }
}
