#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <memory>
#include <stdint.h>
#include <string>


static constexpr uint8_t CD74HC4067_COMMON_PIN = 32;
static constexpr uint8_t CD74HC4067_S0_PIN = 26;
static constexpr uint8_t CD74HC4067_S1_PIN = 25;
static constexpr uint8_t CD74HC4067_S2_PIN = 18;
static constexpr uint8_t CD74HC4067_S3_PIN = 5;
static constexpr uint8_t DHT22_INDOOR_PIN = 23;
static constexpr uint8_t DHT22_OUTDOOR_PIN = 19;
static constexpr uint8_t KY018_PIN        = 35;
static constexpr uint8_t RS485_RX_PIN     = 13;
static constexpr uint8_t RS485_TX_PIN     = 27;
static constexpr uint8_t RS485_ENABLE_PIN = 14;
static constexpr uint8_t SETUP_MODE_ENABLE_PIN = 33;
static constexpr uint8_t VOLT_PIN         = 34;


class Settings;
[[nodiscard]] std::shared_ptr<Settings> getSettings();

class CD74HC4067; //16-channel multiplexer
[[nodiscard]] std::shared_ptr<CD74HC4067> getCD74HC4067();

class DHT22; //Temperature/Humidity sensor
[[nodiscard]] std::shared_ptr<DHT22> getIndoorDHT22();
[[nodiscard]] std::shared_ptr<DHT22> getOutdoorDHT22();

class KY018; //Light sensor
[[nodiscard]] std::shared_ptr<KY018> getKY018();

class RS485; //Soil sensors bus
[[nodiscard]] std::shared_ptr<RS485> getRS485();

class Network;
[[nodiscard]] std::shared_ptr<Network> getNetwork();

class NTP;
[[nodiscard]] std::shared_ptr<NTP> getNTP();

class Volt; //Volt sensor
[[nodiscard]] std::shared_ptr<Volt> getVolt();


std::string floatToString(const float& value, uint8_t precision);
std::string uint8ToHex(uint8_t value);
std::string uint16ToHex(uint16_t value);

#endif // _GLOBAL_H_
