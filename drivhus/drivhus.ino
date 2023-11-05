
#include "global.h"

#include "rs485.h"
#include "webserver.h"


WebServer g_web_server;
RS485 g_rs485;

void setup() {
  Serial.begin(115200);

  g_rs485.init();
  g_web_server.init();
}

void loop() {
  unsigned long current_time = millis();

  g_rs485.loop(current_time);
  g_web_server.loop(current_time);
  delay(2000);
}
