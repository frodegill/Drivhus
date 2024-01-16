#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <memory>
#include <stdint.h>
#include <string>

#include "ntp.h"


namespace Drivhus {

#ifndef PI
# define PI (3.1415926535897932384626433832795)
#endif

#define degToRad(angleInDegrees) ((angleInDegrees) * PI / 180.0f)
#define radToDeg(angleInRadians) ((angleInRadians) * 180.0f / PI)

#define ON (true)
#define OFF (false)


static constexpr uint8_t O_CD74HC4067_COMMON_PIN = 32;
static constexpr uint8_t O_CD74HC4067_S0_PIN   = 26;
static constexpr uint8_t O_CD74HC4067_S1_PIN   = 25;
static constexpr uint8_t O_CD74HC4067_S2_PIN   = 18;
static constexpr uint8_t O_CD74HC4067_S3_PIN   = 5;
static constexpr uint8_t I_DHT22_INDOOR_PIN    = 23;
static constexpr uint8_t I_DHT22_OUTDOOR_PIN   = 19;
static constexpr uint8_t O_FAN_PIN             = 15;
static constexpr uint8_t O_GROWLIGHT_PIN       = 21;
static constexpr uint8_t I_KY018_PIN           = 35;
static constexpr uint8_t I_RS485_RX_PIN        = 13;
static constexpr uint8_t O_RS485_TX_PIN        = 27;
static constexpr uint8_t O_RS485_ENABLE_PIN    = 14;
static constexpr uint8_t I_SETUP_MODE_ENABLE_PIN = 33;
static constexpr uint8_t I_VOLT_PIN            = 34;
static constexpr uint8_t O_WATER_VALVE_PIN     = 22;
static constexpr uint8_t I_WATERLEVEL_LOW_PIN  = 36;
static constexpr uint8_t I_WATERLEVEL_HIGH_PIN = 39;

//A small selection of time zones
constexpr TimezoneInfo g_timezones[] = {{"CET", "Central European Time", {"", Last, Sun, Oct, 3, 1*60}, {"", Last, Sun, Mar, 2, 2*60}},
                                        {"BST", "United Kingdom", {"", Last, Sun, Oct, 2, 0*60}, {"", Last, Sun, Mar, 1, 1*60}},
                                        {"EST", "US Eastern Time", {"", First, Sun, Nov, 2, -5*60}, {"", Second, Sun, Mar, 2, -4*60}},
                                        {"PST", "US Pacific Time", {"", First, Sun, Nov, 2, -8*60}, {"", Second, Sun, Mar, 2, -7*60}}};

class Settings;
[[nodiscard]] std::shared_ptr<Settings> getSettings();

class CD74HC4067; //16-channel multiplexer
[[nodiscard]] std::shared_ptr<CD74HC4067> getCD74HC4067();

class DHT22; //Temperature/Humidity sensor
[[nodiscard]] std::shared_ptr<DHT22> getIndoorDHT22();
[[nodiscard]] std::shared_ptr<DHT22> getOutdoorDHT22();

class Fan;
[[nodiscard]] std::shared_ptr<Fan> getFan();

class Growlight;
[[nodiscard]] std::shared_ptr<Growlight> getGrowlight();

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

class Waterlevel; //Waterlevel sensors
[[nodiscard]] std::shared_ptr<Waterlevel> getWaterlevel();


std::string floatToString(const float& value, uint8_t precision);
std::string uint8ToHex(uint8_t value);
std::string uint16ToHex(uint16_t value);

} //namespace

#endif // _GLOBAL_H_
