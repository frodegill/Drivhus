
#include "global.h"

#include "dht22.h"
#include "ky018.h"
#include "network.h"
#include "rs485.h"
#include "settings.h"
#include "volt.h"


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Drivhus");

  if (!::getSettings()->init() ||
      !::getIndoorDHT22()->init() ||
      !::getOutdoorDHT22()->init() ||
      !::getKY018()->init() ||
      !::getVolt()->init() ||
      !::getNetwork()->init() ||
      !::getRS485()->init()) {
    Serial.println("Setup failed");
  }
}

void loop() {
  unsigned long current_time = millis();

  ::getSettings()->loop();
  ::getIndoorDHT22()->loop();
  ::getOutdoorDHT22()->loop();
  ::getKY018()->loop();
  ::getVolt()->loop();
  ::getNetwork()->loop();
  ::getRS485()->loop();

  delay(200);
}
