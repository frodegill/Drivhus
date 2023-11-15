#include "global.h"

#include <iomanip>
#include <sstream>

#include "network.h"
#include "rs485.h"
#include "settings.h"


std::shared_ptr<Network> g_network;
[[nodiscard]] std::shared_ptr<Network> getNetwork() {
  if (!g_network) {
    g_network = std::make_shared<Network>();
  }
  return g_network;
}

std::shared_ptr<RS485> g_rs485;
[[nodiscard]] std::shared_ptr<RS485> getRS485() {
  if (!g_rs485) {
    g_rs485 = std::make_shared<RS485>();
  }
  return g_rs485;
}

std::shared_ptr<Settings> g_settings;
std::shared_ptr<Settings> getSettings() {
  if (!g_settings) {
    g_settings = std::make_shared<Settings>();
  }
  return g_settings;
}

std::string floatToString(const float& value, uint8_t precision) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(precision) << value;
  return ss.str();
}

std::string uint8ToHex(uint8_t value) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << static_cast<unsigned int>(value);
  return ss.str();
}

std::string uint16ToHex(uint16_t value) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << static_cast<unsigned int>(value);
  return ss.str();
}
