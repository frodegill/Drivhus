#ifndef _RS485_H_
#define _RS485_H_

#include <memory>
#include <set>
#include <HardwareSerial.h>
#include <ModbusRTUMaster.h>



class RS485
{
public:
  static constexpr uint32_t SERIAL_BAUD = 9600; 
  static constexpr uint32_t MODBUS_TIMEOUT = 200;
  static constexpr unsigned long SCAN_INTERVAL_MS = 15000L;

  static constexpr uint8_t UNDEFINED_ID = 0;
  static constexpr uint8_t MIN_ID = 1;
  static constexpr uint8_t MAX_ID = 255;

  static constexpr uint8_t DRIVHUS_MIN_ID = 1;
  static constexpr uint8_t DRIVHUS_MAX_ID = 15;

public:
  RS485();
  [[nodiscard]] bool init();
  [[nodiscard]] bool loop(const unsigned long& current_time);

  [[nodiscard]] bool isSensorPresent(uint8_t id) const;
  [[nodiscard]] std::set<uint8_t> getPresentSensors() const;

  [[nodiscard]] float getSensorTemp(uint8_t id);
  [[nodiscard]] float getSensorHumidity(uint8_t id);

private:
  void setSensorPresent(uint8_t id, bool is_present);
  void setSensorValues(uint8_t id, uint16_t temp, uint16_t humidity);

private:
  std::unique_ptr<ModbusRTUMaster> m_modbus;

  unsigned long m_previous_complete_scan_time;
  uint8_t m_previous_scanned_sensor_id;
  bool m_performed_full_scan;

  uint32_t m_sensor_present[8]; //8*32 = 256 bits. Enough to hold state for all possible Sensor IDs
  uint16_t m_sensor_temp[15];
  uint16_t m_sensor_humidity[15];

  uint16_t tmp_holding_registers[2];
};

#endif // _RS485_H_
