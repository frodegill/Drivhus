#ifndef _RS485_H_
#define _RS485_H_

#ifdef TESTING
# include "testing.h"
#else
# include <HardwareSerial.h>
# include <ModbusRTUMaster.h>
#endif

#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <utility>


class RS485
{
public:

  static constexpr uint8_t UNDEFINED_ID = 0;
  static constexpr uint8_t MIN_ID = 1;
  static constexpr uint8_t MAX_ID = 247;

  static constexpr uint8_t DRIVHUS_MIN_ID = 1;
  static constexpr uint8_t DRIVHUS_MAX_ID = 15;

  static constexpr uint8_t SENSOR_COUNT = DRIVHUS_MAX_ID - DRIVHUS_MIN_ID + 1;

private:
  static constexpr uint32_t SERIAL_BAUD = 9600; 
  static constexpr uint32_t MODBUS_TIMEOUT = 200;
  static constexpr unsigned long SCAN_INTERVAL_MS = 15000L;

  static constexpr uint32_t SENSOR_PRESENT_UINT32_NEEDED = (MAX_ID/32) + 1;

public:
  RS485(uint8_t rx_pin, uint8_t tx_pin, uint8_t enable_pin);
  [[nodiscard]] bool init();
  void loop();

  [[nodiscard]] bool isSensorPresent(uint8_t id) const;
  [[nodiscard]] std::set<uint8_t> getPresentSensors() const;

  [[nodiscard]] float getSensorTemp(uint8_t id);
  [[nodiscard]] float getSensorHumidity(uint8_t id);

  void setSensorShouldBeReassigned(uint8_t old_id, uint8_t new_id);

private:
  [[nodiscard]] bool checkIfSensorIsPresent(uint8_t id);
  void setSensorPresent(uint8_t id, bool is_present);
  void setSensorValues(uint8_t id, uint16_t temp, uint16_t humidity);

  void checkIfSensorsShouldBeReassigned();
  [[nodiscard]] bool setNewSensorId(uint8_t old_id, uint8_t new_id, bool force=false);

private:
  uint8_t m_rx_pin;
  uint8_t m_tx_pin;
  uint8_t m_enable_pin;
  std::unique_ptr<ModbusRTUMaster> m_modbus;

  std::list<std::pair<uint8_t, uint8_t>> m_reassign_sensor_ids;
  std::recursive_mutex m_reassign_sensor_ids_mutex;

  unsigned long m_previous_complete_scan_time;
  uint8_t m_previous_scanned_sensor_id;
  bool m_performed_full_scan;

  uint32_t m_sensor_present[SENSOR_PRESENT_UINT32_NEEDED];
  uint16_t m_sensor_temp[SENSOR_COUNT];
  uint16_t m_sensor_humidity[SENSOR_COUNT];

  uint16_t tmp_holding_registers[2];
};

#endif // _RS485_H_
