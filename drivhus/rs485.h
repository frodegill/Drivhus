#ifndef _RS485_H_
#define _RS485_H_

#include <memory>
#include <vector>
#include <HardwareSerial.h>
#include <ModbusRTUMaster.h>


const uint32_t RS485_SERIAL_BAUD = 9600; 
const unsigned long RS485_SCAN_INTERVAL_MS = 15000L;

const uint8_t RS485_UNDEFINED_ID = 0;
const uint8_t RS485_MIN_ID = 1;
const uint8_t RS485_MAX_ID = 255;

const uint8_t DRIVHUS_RS485_MIN_ID = 1;
const uint8_t DRIVHUS_RS485_MAX_ID = 15;


class RS485
{
public:
  RS485();
  [[nodiscard]] bool init();
  [[nodiscard]] bool loop(const unsigned long& current_time);

  [[nodiscard]] bool isSensorPresent(uint8_t id) const;
  [[nodiscard]] std::vector<uint8_t> presentSensors() const;

  [[nodiscard]] float getSensorTemp(uint8_t id);
  [[nodiscard]] float getSensorHumidity(uint8_t id);

private:
  void setSensorPresent(uint8_t id, bool is_present);
  void setSensorValues(uint8_t id, uint16_t temp, uint16_t humidity);

private:
  std::unique_ptr<ModbusRTUMaster> m_modbus;

  unsigned long m_previous_complete_scan_time;
  uint8_t m_previous_scanned_sensor_id;

  uint32_t m_sensor_present[8]; //8*32 = 256 bits. Enough to hold state for all possible Sensor IDs
  uint16_t m_sensor_temp[15];
  uint16_t m_sensor_humidity[15];

  uint16_t tmp_holding_registers[2];
};

#endif // _RS485_H_
