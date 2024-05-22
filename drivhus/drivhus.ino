//#define GPIO_DEBUG

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
#include "webserver.h"
#include "waterlevel.h"

#ifdef GPIO_DEBUG
  GPIOViewer gpio_viewer;
#endif

std::vector<std::shared_ptr<Drivhus::Component>> g_components;

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting Drivhus");

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
  g_components.push_back(Drivhus::getWebServer());
  g_components.push_back(Drivhus::getRS485());
  g_components.push_back(Drivhus::getWaterlevel());
  g_components.push_back(Drivhus::getMQTT());

  Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, "Initialising components");
  for (auto component : g_components) {
    if (!component->init()) {
      Serial.print("Init ");
      Serial.print(component->getName());
      Serial.println(" FAILED");
    }
  }
  for (auto component : g_components) {
    if (!component->postInit()) {
      Serial.print("PostInit ");
      Serial.print(component->getName());
      Serial.println(" FAILED");
    }
  }

#ifdef GPIO_DEBUG
  gpio_viewer.connectToWifi("<ssid>", "<password>");
  gpio_viewer.begin();
#endif

  Serial.println("Drivhus initialised");
}

void loop() {
  for (auto component : g_components) {
    component->loop();
  }

  delay(200);
}
