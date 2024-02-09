#undef GPIO_DEBUG

#ifdef GPIO_DEBUG
# include <gpio_viewer.h>
#endif

#include "global.h"

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

#ifdef GPIO_DEBUG
  GPIOViewer gpio_viewer;
#endif

std::vector<std::shared_ptr<Drivhus::Component>> g_components;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Drivhus");

  g_components.push_back(Drivhus::getLog());
  g_components.push_back(Drivhus::getSettings());
  g_components.push_back(Drivhus::getCD74HC4067());
  g_components.push_back(Drivhus::getIndoorDHT22());
  g_components.push_back(Drivhus::getOutdoorDHT22());
  g_components.push_back(Drivhus::getFan());
  g_components.push_back(Drivhus::getGrowlight());
  g_components.push_back(Drivhus::getKY018());
  g_components.push_back(Drivhus::getVolt());
  g_components.push_back(Drivhus::getNetwork());
  g_components.push_back(Drivhus::getNTP());
  g_components.push_back(Drivhus::getRS485());
  g_components.push_back(Drivhus::getWaterlevel());
  g_components.push_back(Drivhus::getMQTT());

  for (auto component : g_components) {
    if (!component->init()) {
      Serial.println("Setup failed");
    }
  }

#ifdef GPIO_DEBUG
  gpio_viewer.connectToWifi("<ssid>", "<password>");
  gpio_viewer.begin();
#endif
}

void loop() {
  for (auto component : g_components) {
    component->loop();
  }

  delay(200);
}
