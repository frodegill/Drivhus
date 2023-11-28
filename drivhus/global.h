#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <memory>
#include <stdint.h>
#include <string>


static constexpr uint8_t DHT22_INDOOR_PIN = 23;
static constexpr uint8_t DHT22_OUTDOOR_PIN = 19;
static constexpr uint8_t KY018_PIN        = 35;
static constexpr uint8_t RS485_RX_PIN     = 13;
static constexpr uint8_t RS485_TX_PIN     = 27;
static constexpr uint8_t RS485_ENABLE_PIN = 14;
static constexpr uint8_t VOLTAGE_PIN      = 34;

static constexpr uint8_t SETUP_MODE_ENABLE_PIN = 33;


class Settings;
[[nodiscard]] std::shared_ptr<Settings> getSettings();

class DHT22; //Temperature/Humidity sensor
[[nodiscard]] std::shared_ptr<DHT22> getIndoorDHT22();
[[nodiscard]] std::shared_ptr<DHT22> getOutdoorDHT22();

class KY018; //Light sensor
[[nodiscard]] std::shared_ptr<KY018> getKY018();

class RS485; //Soil sensors bus
[[nodiscard]] std::shared_ptr<RS485> getRS485();

class Network;
[[nodiscard]] std::shared_ptr<Network> getNetwork();


std::string floatToString(const float& value, uint8_t precision);
std::string uint8ToHex(uint8_t value);
std::string uint16ToHex(uint16_t value);

#endif // _GLOBAL_H_
