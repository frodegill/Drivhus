
#include "global.h"

#include "dht22.h"
#include "ky018.h"
#include "network.h"
#include "ntp.h"
#include "rs485.h"
#include "settings.h"
#include "volt.h"
#include "waterlevel.h"


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Drivhus");

  if (!::getSettings()->init() ||
      !::getIndoorDHT22()->init() ||
      !::getOutdoorDHT22()->init() ||
      !::getKY018()->init() ||
      !::getVolt()->init() ||
      !::getNetwork()->init() ||
      !::getNTP()->init() ||
      !::getRS485()->init() ||
      !::getWaterlevel()->init()) {
    Serial.println("Setup failed");
  }
}

void loop() {
  ::getSettings()->loop();
  ::getIndoorDHT22()->loop();
  ::getOutdoorDHT22()->loop();
  ::getKY018()->loop();
  ::getVolt()->loop();
  ::getNetwork()->loop();
  ::getNTP()->loop();
  ::getRS485()->loop();
  ::getWaterlevel()->loop();

  delay(200);
}
