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
#include "network.h"
#include "ntp.h"
#include "rs485.h"
#include "settings.h"
#include "volt.h"
#include "waterlevel.h"

#ifdef GPIO_DEBUG
  GPIOViewer gpio_viewer;
#endif

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Drivhus");

  if (!Drivhus::getSettings()->init() ||
      !Drivhus::getCD74HC4067()->init() ||
      !Drivhus::getIndoorDHT22()->init() ||
      !Drivhus::getOutdoorDHT22()->init() ||
      !Drivhus::getFan()->init() ||
      !Drivhus::getGrowlight()->init() ||
      !Drivhus::getKY018()->init() ||
      !Drivhus::getVolt()->init() ||
      !Drivhus::getNetwork()->init() ||
      !Drivhus::getNTP()->init() ||
      !Drivhus::getRS485()->init() ||
      !Drivhus::getWaterlevel()->init()) {
    Serial.println("Setup failed");
  }

#ifdef GPIO_DEBUG
  gpio_viewer.connectToWifi("<ssid>", "<password>");
  gpio_viewer.begin();
#endif
}

void loop() {
  Drivhus::getSettings()->loop();
  Drivhus::getCD74HC4067()->loop();
  Drivhus::getIndoorDHT22()->loop();
  Drivhus::getOutdoorDHT22()->loop();
  Drivhus::getFan()->loop();
  Drivhus::getGrowlight()->loop();
  Drivhus::getKY018()->loop();
  Drivhus::getVolt()->loop();
  Drivhus::getNetwork()->loop();
  Drivhus::getNTP()->loop();
  Drivhus::getRS485()->loop();
  Drivhus::getWaterlevel()->loop();

  delay(200);
}
