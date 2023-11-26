
#include "global.h"

#include "network.h"
#include "rs485.h"
#include "settings.h"


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Drivhus");

  if (!::getSettings()->init() ||
      !::getNetwork()->init() ||
      !::getRS485()->init()) {
    Serial.println("Setup failed");
  }
}

void loop() {
  unsigned long current_time = millis();

  ::getSettings()->loop();
  ::getNetwork()->loop();
  ::getRS485()->loop();

  delay(200);
}
