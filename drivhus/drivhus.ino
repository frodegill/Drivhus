#undef GPIO_DEBUG

#ifdef GPIO_DEBUG
# include <gpio_viewer.h>
#endif

#include "global.h"

#include "dht22.h"
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
      !Drivhus::getIndoorDHT22()->init() ||
      !Drivhus::getOutdoorDHT22()->init() ||
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
  Drivhus::getIndoorDHT22()->loop();
  Drivhus::getOutdoorDHT22()->loop();
  Drivhus::getKY018()->loop();
  Drivhus::getVolt()->loop();
  Drivhus::getNetwork()->loop();
  Drivhus::getNTP()->loop();
  Drivhus::getRS485()->loop();
  Drivhus::getWaterlevel()->loop();

  delay(200);
}
