#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <memory>
#include <stdint.h>


static constexpr uint8_t RS485_RX_PIN     = 13;
static constexpr uint8_t RS485_TX_PIN     = 27;
static constexpr uint8_t RS485_ENABLE_PIN = 14;
static constexpr uint8_t SETUP_MODE_ENABLE_PIN = 33;


class Settings;
[[nodiscard]] std::shared_ptr<Settings> getSettings();

class RS485;
[[nodiscard]] std::shared_ptr<RS485> getRS485();

class Network;
[[nodiscard]] std::shared_ptr<Network> getNetwork();

#endif // _GLOBAL_H_
