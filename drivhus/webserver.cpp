#include "webserver.h"

#include "global.h"
#include "network.h"
#include "settings.h"

const char select_new_sensor_id[] PROGMEM = R"rawliteral(
&nbsp;<select name="new_sensor_id">
<option value="1">1</option>
<option value="2">2</option>
<option value="3">3</option>
<option value="4">4</option>
<option value="5">5</option>
<option value="6">6</option>
<option value="7">7</option>
<option value="8">8</option>
<option value="9">9</option>
<option value="10">10</option>
<option value="11">11</option>
<option value="12">12</option>
<option value="13">13</option>
<option value="14">14</option>
<option value="15">15</option>
</select><button type="submit">Set new sensor ID</button>
)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Drivhus</title>
<style>
 body {margin: 1em;}
 input,button {margin: 0.2em;}
 table,th,td {border: 1px solid black; border-collapse: collapse; padding: 0.5em;}
</style>
</head>
<body>
<form method="post" %SHOW_SETUP%>
SSID:<input type="text" name="ssid" required maxlength="32" autofocus value="%SSID%"><br>
SSID Password:<input type="password" name="ssid_password" maxlength="64" value="%SSID_PASSWORD%"><br>
MQTT server:<input type="text" name="mqtt_server" maxlength="64" value="%MQTT_SERVER%"><br>
MQTT port:<input type="number" name="mqtt_port" min="0" max="65535" value="%MQTT_PORT%"><br>
MQTT ID postfix:<input type="text" name="mqtt_id" maxlength="32" value="%MQTT_ID%"><br>
MQTT username:<input type="text" name="mqtt_username" maxlength="32" value="%MQTT_USERNAME%"><br>
MQTT password:<input type="password" name="mqtt_password" maxlength="32" value="%MQTT_PASSWORD%"><br>
<button type="submit">Submit</button>
<hr>
</form>
<form method="post">
<table>
<tr><th>Sensor ID</th><th>Sensor Value</th></tr>
<tr><td>1</td><td><span id="SV01">Not available</span>&nbsp;<span id="NS01"/></td></tr>
<tr><td>2</td><td><span id="SV02">Not available</span>&nbsp;<span id="NS02"/></td></tr>
<tr><td>3</td><td><span id="SV03">Not available</span>&nbsp;<span id="NS03"/></td></tr>
<tr><td>4</td><td><span id="SV04">Not available</span>&nbsp;<span id="NS04"/></td></tr>
<tr><td>5</td><td><span id="SV05">Not available</span>&nbsp;<span id="NS05"/></td></tr>
<tr><td>6</td><td><span id="SV06">Not available</span>&nbsp;<span id="NS06"/></td></tr>
<tr><td>7</td><td><span id="SV07">Not available</span>&nbsp;<span id="NS07"/></td></tr>
<tr><td>8</td><td><span id="SV08">Not available</span>&nbsp;<span id="NS08"/></td></tr>
<tr><td>9</td><td><span id="SV09">Not available</span>&nbsp;<span id="NS09"/></td></tr>
<tr><td>10</td><td><span id="SV10">Not available</span>&nbsp;<span id="NS10"/></td></tr>
<tr><td>11</td><td><span id="SV11">Not available</span>&nbsp;<span id="NS11"/></td></tr>
<tr><td>12</td><td><span id="SV12">Not available</span>&nbsp;<span id="NS12"/></td></tr>
<tr><td>13</td><td><span id="SV13">Not available</span>&nbsp;<span id="NS13"/></td></tr>
<tr><td>14</td><td><span id="SV14">Not available</span>&nbsp;<span id="NS14"/></td></tr>
<tr><td>15</td><td><span id="SV15">Not available</span>&nbsp;<span id="NS15"/></td></tr>
<tr><td id="SIX"></td><td>Unused (ID needs to be 1 to 15)&nbsp;<span id="NIX"/></td></tr>
</table>
</form>
</body>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    console.log('onMessage');
    console.log(event.data);
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
  }
</script>
</html>
)rawliteral";

bool WebServer::init() {
  Serial.println("WebServer init");
  m_server = std::unique_ptr<AsyncWebServer>(new AsyncWebServer(80));
  Serial.println("WebServer created completed");
  m_ws = std::unique_ptr<AsyncWebSocket>(new AsyncWebSocket("/ws"));
  Serial.println("WebServer websocket completed");

  m_ws->onEvent(WebServer::onEvent);
  Serial.println("WebServer onevent completed");
  m_server->addHandler(m_ws.get());
  Serial.println("WebServer addhandler completed");

  // Route for root / web page
  m_server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, WebServer::processor);
  });
  Serial.println("WebServer on completed");

  m_server->begin();
  Serial.println("WebServer begin completed");
  return true;
}

bool WebServer::loop(const unsigned long&) {
  m_ws->cleanupClients();
  return true;
}

void WebServer::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  Serial.println("WebServer onevent");
  switch (type) {
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_CONNECT:
      notifyClients("SHOW_SETUP", "hidden");
      break;
    default:
      break;
  }
}

void WebServer::handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
  Serial.println("WebServer handlewebsocketmessage");
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (info->final &&
      info->index==0 &&
      info->len==len &&
      info->opcode==WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
//      ledState = !ledState;
      notifyClients(String(""), String(""));
    }
  }
}

void WebServer::notifyClients(const String& key, const String& data) {
  Serial.println("WebServer notifyclients");
  ::getNetwork()->getWebServer()->textAll(key, data);
}

String WebServer::processor(const String& var){
  Serial.println("WebServer processor");
  Serial.println(var);
  if (var == "SHOW_SETUP") {
    return ::getSettings()->isInSetupMode() ? "" : "hidden";
  } else if (::getSettings()->isInSetupMode()) {
    if (var == "SSID") {
      return String(::getSettings()->getSSID().c_str());
    } else if (var == "SSID_PASSWORD") {
      return String(::getSettings()->getSSIDPassword().c_str());
    } else if (var == "MQTT_SERVER") {
      return String(::getSettings()->getMQTTServername().c_str());
    } else if (var == "MQTT_PORT") {
      return String(::getSettings()->getMQTTPort());
    } else if (var == "MQTT_ID") {
      return String(::getSettings()->getMQTTServerId().c_str());
    } else if (var == "MQTT_USERNAME") {
      return String(::getSettings()->getMQTTUsername().c_str());
    } else if (var == "MQTT_PASSWORD") {
      return String(::getSettings()->getMQTTPassword().c_str());
    }
  }
  return String();
}

void WebServer::textAll(const String& key, const String& data) {
  Serial.println("WebServer textall");
  m_ws->textAll(key+data);
}
/*
%SHOW_SETUP%
%SSID%
%SSID_PASSWORD%
%MQTT_SERVER%
%MQTT_PORT%
%MQTT_ID%
%MQTT_USERNAME%
%MQTT_PASSWORD%
%SV01%
%NS01%
%SV15%
%NS15%
%SIX%
%NIX%
*/