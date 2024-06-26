#include "global.h"

#ifdef TESTING
#else
# include <Timezone.h>
#endif

#include <iomanip>
#include <sstream>

#include "cd74hc4067.h"
#include "dht22.h"
#include "fan.h"
#include "growlight.h"
#include "ky018.h"
#include "log.h"
#include "mqtt.h"
#include "network.h"
#include "ntp.h"
#include "rs485.h"
#include "settings.h"
#include "volt.h"
#include "waterlevel.h"
#include "webserver.h"


std::shared_ptr<Drivhus::CD74HC4067> g_cd74hc4067;
[[nodiscard]] std::shared_ptr<Drivhus::CD74HC4067> Drivhus::getCD74HC4067() {
  if (!g_cd74hc4067) {
    g_cd74hc4067 = std::make_shared<Drivhus::CD74HC4067>(O_CD74HC4067_S0_PIN, O_CD74HC4067_S1_PIN, O_CD74HC4067_S2_PIN, O_CD74HC4067_S3_PIN, O_CD74HC4067_COMMON_PIN);
  }
  return g_cd74hc4067;
}

std::shared_ptr<Drivhus::DHT22> g_indoor_dht22;
[[nodiscard]] std::shared_ptr<Drivhus::DHT22> Drivhus::getIndoorDHT22() {
  if (!g_indoor_dht22) {
    g_indoor_dht22 = std::make_shared<Drivhus::DHT22>(0, I_DHT22_INDOOR_PIN);
  }
  return g_indoor_dht22;
}

std::shared_ptr<Drivhus::DHT22> g_outdoor_dht22;
[[nodiscard]] std::shared_ptr<Drivhus::DHT22> Drivhus::getOutdoorDHT22() {
  if (!g_outdoor_dht22) {
    g_outdoor_dht22 = std::make_shared<Drivhus::DHT22>(1, I_DHT22_OUTDOOR_PIN);
  }
  return g_outdoor_dht22;
}

std::shared_ptr<Drivhus::Fan> g_fan;
[[nodiscard]] std::shared_ptr<Drivhus::Fan> Drivhus::getFan() {
  if (!g_fan) {
    g_fan = std::make_shared<Drivhus::Fan>(O_FAN_PIN);
  }
  return g_fan;
}

std::shared_ptr<Drivhus::Growlight> g_growlight;
[[nodiscard]] std::shared_ptr<Drivhus::Growlight> Drivhus::getGrowlight() {
  if (!g_growlight) {
    g_growlight = std::make_shared<Drivhus::Growlight>(O_GROWLIGHT_PIN);
  }
  return g_growlight;
}

std::shared_ptr<Drivhus::KY018> g_ky018;
[[nodiscard]] std::shared_ptr<Drivhus::KY018> Drivhus::getKY018() {
  if (!g_ky018) {
    g_ky018 = std::make_shared<Drivhus::KY018>(I_KY018_PIN);
  }
  return g_ky018;
}

std::shared_ptr<Drivhus::Log> g_log;
[[nodiscard]] std::shared_ptr<Drivhus::Log> Drivhus::getLog() {
  if (!g_log) {
    g_log = std::make_shared<Drivhus::Log>(Drivhus::Log::LogLevel::LEVEL_INFO, Drivhus::Log::LogMode::MODE_SERIAL_AND_MQTT);
  }
  return g_log;
}

std::shared_ptr<Drivhus::MQTT> g_mqtt;
[[nodiscard]] std::shared_ptr<Drivhus::MQTT> Drivhus::getMQTT() {
  if (!g_mqtt) {
    g_mqtt = std::make_shared<Drivhus::MQTT>();
  }
  return g_mqtt;
}

std::shared_ptr<Drivhus::Network> g_network;
[[nodiscard]] std::shared_ptr<Drivhus::Network> Drivhus::getNetwork() {
  if (!g_network) {
    g_network = std::make_shared<Drivhus::Network>();
  }
  return g_network;
}

std::shared_ptr<Drivhus::NTP> g_ntp;
[[nodiscard]] std::shared_ptr<Drivhus::NTP> Drivhus::getNTP() {
  if (!g_ntp) {
    const Drivhus::TimezoneInfo* tz = Drivhus::getTimezoneInfo(Drivhus::getSettings()->getTimezone());
    g_ntp = std::make_shared<Drivhus::NTP>(Timezone(tz->dst, tz->regular));
  }
  return g_ntp;
}

std::shared_ptr<Drivhus::WebServer> g_webserver;
[[nodiscard]] std::shared_ptr<Drivhus::WebServer> Drivhus::getWebServer() {
  if (!g_webserver) {
    g_webserver = std::make_shared<Drivhus::WebServer>();
  }
  return g_webserver;
}

std::shared_ptr<Drivhus::RS485> g_rs485;
[[nodiscard]] std::shared_ptr<Drivhus::RS485> Drivhus::getRS485() {
  if (!g_rs485) {
    g_rs485 = std::make_shared<Drivhus::RS485>(I_RS485_RX_PIN, O_RS485_TX_PIN, O_RS485_ENABLE_PIN);
  }
  return g_rs485;
}

std::shared_ptr<Drivhus::Settings> g_settings;
[[nodiscard]] std::shared_ptr<Drivhus::Settings> Drivhus::getSettings() {
  if (!g_settings) {
    g_settings = std::make_shared<Drivhus::Settings>(I_SETUP_MODE_ENABLE_PIN);
  }
  return g_settings;
}

std::shared_ptr<Drivhus::Volt> g_volt;
[[nodiscard]] std::shared_ptr<Drivhus::Volt> Drivhus::getVolt() {
  if (!g_volt) {
    g_volt = std::make_shared<Drivhus::Volt>(I_VOLT_PIN);
  }
  return g_volt;
}

std::shared_ptr<Drivhus::Waterlevel> g_waterlevel;
[[nodiscard]] std::shared_ptr<Drivhus::Waterlevel> Drivhus::getWaterlevel() {
  if (!g_waterlevel) {
    g_waterlevel = std::make_shared<Drivhus::Waterlevel>(I_WATERLEVEL_LOW_PIN, I_WATERLEVEL_HIGH_PIN, O_WATER_VALVE_PIN);
  }
  return g_waterlevel;
}

const Drivhus::TimezoneInfo* Drivhus::getTimezoneInfo(const std::string& timezone) {
  for (size_t i=0; i<sizeof(g_timezones)/sizeof(g_timezones[0]); i++) {
    if (timezone.compare(g_timezones[i].code)==0) {
      return &g_timezones[i];
    }
  }
  return &g_timezones[0];
}

std::string Drivhus::floatToString(const float& value, uint8_t precision) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(precision) << value;
  return ss.str();
}

std::string Drivhus::uint8ToHex(uint8_t value) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << static_cast<unsigned int>(value);
  return ss.str();
}

std::string Drivhus::uint16ToHex(uint16_t value) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << static_cast<unsigned int>(value);
  return ss.str();
}
