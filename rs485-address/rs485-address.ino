#include <memory>
#include <HardwareSerial.h>
#include <ModbusRTUMaster.h>

static constexpr uint8_t MIN_ID = 1;
static constexpr uint8_t MAX_ID = 15;
static constexpr uint32_t SERIAL_BAUD = 9600; 
static constexpr uint32_t MODBUS_TIMEOUT = 200;

static constexpr uint8_t I_RS485_RX_PIN        = 13;
static constexpr uint8_t O_RS485_TX_PIN        = 27;
static constexpr uint8_t O_RS485_ENABLE_PIN    = 14;


std::unique_ptr<ModbusRTUMaster> m_modbus;
uint16_t tmp_holding_registers[2];

bool setNewSensorId(uint8_t old_id, uint8_t new_id, uint8_t call_count) {
  delay(500);
  m_modbus->clearTimeoutFlag();
  Serial.print("Setting address for sensor ");
  Serial.print(old_id);
  bool ret = m_modbus->writeSingleHoldingRegister(old_id, 0x0001, new_id);
  Serial.println(ret ? " SUCCEEDED!" :  " FAILED.");
  call_count--;
  if (!ret && call_count>0) {
    delay(100);
    return setNewSensorId(old_id, new_id, call_count);
  }
  return ret;
}

bool checkIfSensorIsPresent(uint8_t id, uint8_t call_count) {
  delay(500);
  bool is_present = false;
  m_modbus->clearTimeoutFlag();
  Serial.print("Sensor ");
  Serial.print(id);
  bool ret = m_modbus->readHoldingRegisters(id, 0x0001, tmp_holding_registers, 2);
  Serial.println(ret ? " IS present!" :  " is NOT present.");
  call_count--;
  if (!ret && call_count>0) {
    delay(100);
    return checkIfSensorIsPresent(id, call_count);
  }
  return ret;
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting...");
  Serial.println("");
  Serial.flush();

  Serial2.begin(SERIAL_BAUD, SERIAL_8E1, I_RS485_RX_PIN, O_RS485_TX_PIN);
  m_modbus = std::unique_ptr<ModbusRTUMaster>(new ModbusRTUMaster(Serial2, O_RS485_ENABLE_PIN)); // serial port, driver enable pin for rs-485 (optional)
  m_modbus->begin(Serial2.baudRate());
  m_modbus->setTimeout(MODBUS_TIMEOUT);
}

void loop() {
  int new_id = 14;

  checkIfSensorIsPresent(new_id, 3);

  for (int i=MIN_ID; i<=MAX_ID; i++) {
    if (i != new_id) {
      if (checkIfSensorIsPresent(i, 5)) {
        setNewSensorId(i, new_id, 5);
      }
    }
    delay(200);
  }
  
  for (int i=MIN_ID; i<=MAX_ID; i++) {
    checkIfSensorIsPresent(i, 5);
    delay(200);
  }

  delay(60L*60L*1000L);
}
