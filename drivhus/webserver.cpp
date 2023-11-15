#include "webserver.h"

#include "global.h"
#include "network.h"
#include "rs485.h"
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
<form id="SETUP" method="post" %SHOW_SETUP%>
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
<tr><th><div>Sensor ID</div><div>Scanning:<span id="CSI"/></div></th><th>Moisture %</th></tr>
<tr><td>1</td><td><span id="SV01"/>&nbsp;<span id="NS01"/></td></tr>
<tr><td>2</td><td><span id="SV02"/>&nbsp;<span id="NS02"/></td></tr>
<tr><td>3</td><td><span id="SV03"/>&nbsp;<span id="NS03"/></td></tr>
<tr><td>4</td><td><span id="SV04"/>&nbsp;<span id="NS04"/></td></tr>
<tr><td>5</td><td><span id="SV05"/>&nbsp;<span id="NS05"/></td></tr>
<tr><td>6</td><td><span id="SV06"/>&nbsp;<span id="NS06"/></td></tr>
<tr><td>7</td><td><span id="SV07"/>&nbsp;<span id="NS07"/></td></tr>
<tr><td>8</td><td><span id="SV08"/>&nbsp;<span id="NS08"/></td></tr>
<tr><td>9</td><td><span id="SV09"/>&nbsp;<span id="NS09"/></td></tr>
<tr><td>10</td><td><span id="SV0A"/>&nbsp;<span id="NS0A"/></td></tr>
<tr><td>11</td><td><span id="SV0B"/>&nbsp;<span id="NS0B"/></td></tr>
<tr><td>12</td><td><span id="SV0C"/>&nbsp;<span id="NS0C"/></td></tr>
<tr><td>13</td><td><span id="SV0D"/>&nbsp;<span id="NS0D"/></td></tr>
<tr><td>14</td><td><span id="SV0E"/>&nbsp;<span id="NS0E"/></td></tr>
<tr><td>15</td><td><span id="SV0F"/>&nbsp;<span id="NS0F"/></td></tr>
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
    console.log('onMessage '+event.data);
    if (event.data.startsWith('SV') || event.data.startsWith('NS')){
      var elm = document.getElementById(event.data.substring(0,4));
      if (elm) elm.innerHTML = event.data.substring(4); else console.log('Could not find '+event.data.substring(0,4));
    } else if (event.data.startsWith('CSI') || event.data.startsWith('SIX') || event.data.startsWith('NIX')){
      var elm = document.getElementById(event.data.substring(0,3));
      if (elm) elm.innerHTML = event.data.substring(3); else console.log('Could not find '+event.data.substring(0,3));
    } else if (event.data.startsWith('SHOW_SETUP')){
      var elm = document.getElementById('SETUP');
      if (elm) {
        if (event.data.length == 10) {
          elm.removeAttribute('hidden');
        } else {
          elm.setAttribute('hidden', 'hidden')
        }
      }
    } else {
      console.log('Unknown message: '+event.data)
    }
  }
  function onLoad(event) {
    initWebSocket();
  }
</script>
</html>
)rawliteral";

bool WebServer::init() {
  m_server = std::unique_ptr<AsyncWebServer>(new AsyncWebServer(80));
  m_ws = std::unique_ptr<AsyncWebSocket>(new AsyncWebSocket("/ws"));

  m_ws->onEvent(WebServer::onEvent);
  m_server->addHandler(m_ws.get());

  // Route for root / web page
  m_server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, WebServer::processor);
  });

  m_server->begin();
  return true;
}

bool WebServer::loop(const unsigned long&) {
  m_ws->cleanupClients();
  return true;
}

void WebServer::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  Serial.print("WebServer onEvent ");
  Serial.println(type);
  switch (type) {
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    default:
      break;
  }
}

void WebServer::setSetupMode(bool in_setup_mode) {
  notifyClients("SHOW_SETUP", in_setup_mode ? "" : "hidden");
}

void WebServer::updateSensor(uint8_t sensor_id) {
  if (sensor_id>=RS485::DRIVHUS_MIN_ID && sensor_id<=RS485::DRIVHUS_MAX_ID) {
    bool present = ::getRS485()->isSensorPresent(sensor_id);
    if (!present) {
      notifyClients(std::string("SV")+::uint8ToHex(sensor_id), std::string("Not available"));
      if (m_present_sensors.find(sensor_id) != m_present_sensors.end()) {
        m_present_sensors.erase(sensor_id);
      }
    } else {
      notifyClients(std::string("SV")+::uint8ToHex(sensor_id), ::floatToString(::getRS485()->getSensorHumidity(sensor_id), 2));
      if (m_present_sensors.find(sensor_id) == m_present_sensors.end()) {
        m_present_sensors.emplace(sensor_id);
      }
    }
  }
  notifyClients("CSI", std::to_string(sensor_id)); //Current Sensor ID
}

void WebServer::setSensorScanCompleted() {
  notifyClients("CSI", "-");
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
      notifyClients("", "");
    }
  }
}

void WebServer::notifyClients(const std::string& key, const std::string& data) {
  ::getNetwork()->getWebServer()->textAll(key, data);
}

String WebServer::processor(const String& var){
  Serial.println("WebServer processor");
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

void WebServer::textAll(const std::string& key, const std::string& data) {
  m_ws->textAll((key+data).c_str());
}
