#include <string>
#include "webserver.h"

#include <cstring>
#include <sstream>

#include "global.h"
#include "cd74hc4067.h"
#include "network.h"
#include "rs485.h"
#include "settings.h"


const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Drivhus</title>
<style>
 body {margin: 1em;}
 input,button {margin: 0.2em;}
 table,th,td {border: 1px solid black; border-collapse: collapse; padding: 0.5em;}
 div#MSG {font-weight: bold; color: red;}
</style>
</head>
<body>
<div id="MSG" hidden></div>
<div id="SETUP" %SHOW_SETUP%>
SSID:<input type="text" id="SSID" required maxlength="32" autofocus value="%SSID%"><br>
SSID Password:<input type="password" id="SSID_PASSWORD" maxlength="64" value="%SSID_PASSWORD%"><br>
MQTT server:<input type="text" id="MQTT_SERVER" maxlength="64" value="%MQTT_SERVER%"><br>
MQTT port:<input type="number" id="MQTT_PORT" min="0" max="65535" value="%MQTT_PORT%"><br>
MQTT ID postfix:<input type="text" id="MQTT_ID" maxlength="32" value="%MQTT_ID%"><br>
MQTT username:<input type="text" id="MQTT_USERNAME" maxlength="32" value="%MQTT_USERNAME%"><br>
MQTT password:<input type="password" id="MQTT_PASSWORD" maxlength="32" value="%MQTT_PASSWORD%"><br>
<button onClick="updateSetup()">Submit</button> <button onClick="testRelays()">Test relays</button>
<hr>
</div>
<div>
<table>
<tr><th><div>Sensor ID</div><div>Scanning:<span id="CSI"></span></div></th><th>Moisture %%</th></tr>
<tr><td>1</td><td><span id="SV01">%SV01%</span>&nbsp;<span id="NS01">%NS01%</span></td></tr>
<tr><td>2</td><td><span id="SV02">%SV02%</span>&nbsp;<span id="NS02">%NS02%</span></td></tr>
<tr><td>3</td><td><span id="SV03">%SV03%</span>&nbsp;<span id="NS03">%NS03%</span></td></tr>
<tr><td>4</td><td><span id="SV04">%SV04%</span>&nbsp;<span id="NS04">%NS04%</span></td></tr>
<tr><td>5</td><td><span id="SV05">%SV05%</span>&nbsp;<span id="NS05">%NS05%</span></td></tr>
<tr><td>6</td><td><span id="SV06">%SV06%</span>&nbsp;<span id="NS06">%NS06%</span></td></tr>
<tr><td>7</td><td><span id="SV07">%SV07%</span>&nbsp;<span id="NS07">%NS07%</span></td></tr>
<tr><td>8</td><td><span id="SV08">%SV08%</span>&nbsp;<span id="NS08">%NS08%</span></td></tr>
<tr><td>9</td><td><span id="SV09">%SV09%</span>&nbsp;<span id="NS09">%NS09%</span></td></tr>
<tr><td>10</td><td><span id="SV0A">%SV0A%</span>&nbsp;<span id="NS0A">%NS0A%</span></td></tr>
<tr><td>11</td><td><span id="SV0B">%SV0B%</span>&nbsp;<span id="NS0B">%NS0B%</span></td></tr>
<tr><td>12</td><td><span id="SV0C">%SV0C%</span>&nbsp;<span id="NS0C">%NS0C%</span></td></tr>
<tr><td>13</td><td><span id="SV0D">%SV0D%</span>&nbsp;<span id="NS0D">%NS0D%</span></td></tr>
<tr><td>14</td><td><span id="SV0E">%SV0E%</span>&nbsp;<span id="NS0E">%NS0E%</span></td></tr>
<tr><td>15</td><td><span id="SV0F">%SV0F%</span>&nbsp;<span id="NS0F">%NS0F%</span></td></tr>
<tr><td id="SIX">%SIX%</td><td>Unused (ID needs to be 1 to 15)&nbsp;<span id="NIX">%NIX%</span></td></tr>
</table>
<table>
<tr><th>Sensor</th><th>Value</th></tr>
<tr><td>Indoor temp</td><td><span id="ITEMP">%ITEMP%</span><span>&nbsp;C</span></td></tr>
<tr><td>Indoor humidity</td><td><span id="IHUMID">%IHUMID%</span><span>&nbsp;%%RH</span></td></tr>
<tr><td>Indoor light</td><td><span id="ILIGHT">%ILIGHT%</span><span>&nbsp;%%</span></td></tr>
<tr><td>Outdoor temp</td><td><span id="OTEMP">%OTEMP%</span><span>&nbsp;C</span></td></tr>
<tr><td>Outdoor humidity</td><td><span id="OHUMID">%OHUMID%</span><span>&nbsp;%%RH</span></td></tr>
<tr><td>Voltage</td><td><span id="VOLT">%VOLT%</span><span>&nbsp;V</span><span id="VM">%VM%</span></td></tr>
</table>
</div>
</body>
<script>
  var gw = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    websocket = new WebSocket(gw);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
  }
  function onOpen(event){}
  function onClose(event){setTimeout(initWebSocket, 2000);}
  function onMessage(event) {
    if (event.data.startsWith('VM')){
      document.getElementById(event.data.substring(0,2)).innerHTML = event.data.substring(2);
    } else if (event.data.startsWith('SV') || event.data.startsWith('NS')){
      document.getElementById(event.data.substring(0,4)).innerHTML = event.data.substring(4);
    } else if (event.data.startsWith('CSI') || event.data.startsWith('SIX') || event.data.startsWith('NIX')){
      document.getElementById(event.data.substring(0,3)).innerHTML = event.data.substring(3);
    } else if (event.data.startsWith('VOLT')){
      document.getElementById(event.data.substring(0,4)).innerHTML = event.data.substring(4);
    } else if (event.data.startsWith('ITEMP') || event.data.startsWith('OTEMP')){
      document.getElementById(event.data.substring(0,5)).innerHTML = event.data.substring(5);
    } else if (event.data.startsWith('IHUMID') || event.data.startsWith('ILIGHT') || event.data.startsWith('OHUMID')){
      document.getElementById(event.data.substring(0,6)).innerHTML = event.data.substring(6);
    } else if (event.data.startsWith('SHOW_SETUP')){
      var elm = document.getElementById('SETUP');
      if (event.data.length == 10) { //'SHOW_SETUP' with no value
        elm.removeAttribute('hidden');
      } else {
        elm.setAttribute('hidden', 'hidden')
      }
    } else if (event.data.startsWith('MSG')) {
      var elm = document.getElementById('MSG');
      elm.innerHTML = event.data.substring(3);
      if (event.data.length == 3) { //'MSG' with no value
        elm.setAttribute('hidden', 'hidden')
      } else {
        elm.removeAttribute('hidden');
      }
    } else {
      console.log('Unknown message '+event.data)
    }
  }
  function onLoad(event){initWebSocket();}
  function updateSetup(){websocket.send('SETUP'+elmValue('SSID')+'\n'+elmValue('SSID_PASSWORD')+'\n'+elmValue('MQTT_SERVER')+'\n'+elmValue('MQTT_PORT')+'\n'+elmValue('MQTT_ID')+'\n'+elmValue('MQTT_USERNAME')+'\n'+elmValue('MQTT_PASSWORD'));}
  function testRelays(){websocket.send('TR');}
  function updateSensorId(sensorIdHex){websocket.send('NSI'+sensorIdHex+elmValue('SO'+sensorIdHex));}
  function updateVoltMultiplier(){websocket.send('VM'+elmValue('VOLT_MULTIPLIER'));}
  function elmValue(elmId){return document.getElementById(elmId).value;}
</script>
</html>
)rawliteral";


Drivhus::WebServer::WebServer()
: m_is_showing_setup(false),
  m_light(0.0f),
  m_volt(0.0f),
  m_warning_message_time(0L),
  m_is_testing_relays(false),
  m_relay_test_event_time(0L),
  m_relay_test_index(0),
  m_relay_test_on(false)
{
  m_temp[0] = m_temp[1] = 0.0f,
  m_humid[0] = m_humid[1] = 0.0f;
}

bool Drivhus::WebServer::init() {
  m_server = std::unique_ptr<AsyncWebServer>(new AsyncWebServer(80));
  m_ws = std::unique_ptr<AsyncWebSocket>(new AsyncWebSocket("/ws"));
  m_ws->onEvent(WebServer::onEvent);
  m_server->addHandler(m_ws.get());

  // Route for root / web page
  m_server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, WebServer::processor);
  });

  m_server->begin();
  return true;
}

void Drivhus::WebServer::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_warning_message_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_warning_message_time = current_time;
  }

  if (m_is_testing_relays) {
    updateRelayTest();
  }

  if (m_warning_message_time!=0L) {
    if ((m_warning_message_time+WARNING_MESSAG_DELAY_MS)<current_time) {
      showWarningMessage("");
    }
  } else {
    checkIfWarningMessageShouldBeShown();
  }

  m_ws->cleanupClients();
}

void Drivhus::WebServer::onEvent([[maybe_unused]] AsyncWebSocket* server, [[maybe_unused]] AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len); break;
    default:
      break;
  }
}

void Drivhus::WebServer::updateSetupMode() {
  bool tmp = Drivhus::getSettings()->isInSetupMode();
  if (m_is_showing_setup!=tmp) {
    notifyClients("SHOW_SETUP", tmp ? "" : "hidden");
    m_is_showing_setup = tmp;
    for (uint8_t sensor_id=RS485::DRIVHUS_MIN_ID; sensor_id<=RS485::DRIVHUS_MAX_ID; sensor_id++) {
      updateNewSensorIdButtons(sensor_id);
    }
    uint8_t unused_sensor_id = getUnusedSensorId();
    if (unused_sensor_id != RS485::UNDEFINED_ID) {
      updateNewSensorIdButtons(unused_sensor_id);
    }
    notifyClients("VM", generateVoltMultiplierCalibration());
  } 
}

void Drivhus::WebServer::updateSensor(uint8_t sensor_id) {
  updateSetupMode();

  bool present = Drivhus::getRS485()->isSensorPresent(sensor_id);
  if (present && m_present_sensors.find(sensor_id) == m_present_sensors.end()) {
    m_present_sensors.emplace(sensor_id);
  } else if (!present && m_present_sensors.find(sensor_id) != m_present_sensors.end()) {
    m_present_sensors.erase(sensor_id);
  }

  if (sensor_id>=RS485::DRIVHUS_MIN_ID && sensor_id<=RS485::DRIVHUS_MAX_ID) {
    notifyClients(std::string("SV")+Drivhus::uint8ToHex(sensor_id), getSensorValueAsString(sensor_id));
    updateNewSensorIdButtons(sensor_id);
  } else if (sensor_id>=RS485::MIN_ID && sensor_id<=RS485::MAX_ID && present) {
    notifyClients("SIX", getUnusedSensorIdsAsString().c_str());
    updateNewSensorIdButtons(sensor_id);
  }

  notifyClients("CSI", std::to_string(sensor_id)); //Current Sensor ID
}

void Drivhus::WebServer::setSensorScanCompleted() {
  notifyClients("CSI", "-");
}

void Drivhus::WebServer::updateTempHumid(uint8_t id, float temp, float humid) {
  if (id < 2) {
    if (temp<(m_temp[id]*0.99f) || temp>(m_temp[id]*1.01f)) {
      m_temp[id] = temp;
      notifyClients(id==0 ? "ITEMP" : "OTEMP", String(m_temp[id], 1).c_str());
    }
    if (humid<(m_humid[id]*0.99f) || humid>(m_humid[id]*1.01f)) {
      m_humid[id] = humid;
      notifyClients(id==0 ? "IHUMID" : "OHUMID", String(m_humid[id], 1).c_str());
    }
  }
}

void Drivhus::WebServer::updateLight(float light) {
  if (light<(m_light*0.99f) || light>(m_light*1.01f)) {
    m_light = light;
    notifyClients("ILIGHT", String(m_light, 1).c_str());
  }
}

void Drivhus::WebServer::updateVolt(float volt) {
  if (volt<(m_volt*0.99f) || volt>(m_volt*1.01f)) {
    m_volt = volt;
    notifyClients("VOLT", String(m_volt, 2).c_str());
  }
}

void Drivhus::WebServer::addWarningMessage(const std::string& msg) {
  const std::lock_guard<std::recursive_mutex> lock(m_warning_messages_mutex);
  m_warning_messages.push_back(msg);
}

void Drivhus::WebServer::handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (info->final && info->index==0 && info->len==len && info->opcode==WS_TEXT) {
    const char* data_str = reinterpret_cast<const char*>(data);
    if (std::strncmp("SETUP", data_str, 5)==0) {
      std::istringstream is(std::string(data_str+5, len-5));
      std::string line;
      uint8_t i = 0;
      while (std::getline(is, line)) {
        switch(i++) {
          case 0: Drivhus::getSettings()->setSSID(line); break;
          case 1: Drivhus::getSettings()->setSSIDPassword(line); break;
          case 2: Drivhus::getSettings()->setMQTTServername(line); break;
          case 3: Drivhus::getSettings()->setMQTTPort(static_cast<uint16_t>(std::stoi(line)&0xFFFF)); break;
          case 4: Drivhus::getSettings()->setMQTTServerId(line); break;
          case 5: Drivhus::getSettings()->setMQTTUsername(line); break;
          case 6: Drivhus::getSettings()->setMQTTPassword(line); break;
        }
      }
      Drivhus::getSettings()->setShouldFlushSettings();
    } else if (len>=2 && std::strncmp("TR", data_str, 2)==0) {
      Drivhus::getNetwork()->getWebServer()->activateRelayTests();
    } else if (len>=(3+2+2) && std::strncmp("NSI", data_str, 3)==0) {
      std::string s(data_str+3, 4);
      unsigned long value = std::stoul(s, nullptr, 16);
      uint8_t old_id = (value&0xFF00)>>8;
      uint8_t new_id = (value&0x00FF);
      Drivhus::getRS485()->setSensorShouldBeReassigned(old_id, new_id);
    } else if (std::strncmp("VM", data_str, 2)==0) {
      float value = std::stof(std::string(data_str+2));
      Drivhus::getSettings()->setVoltMultiplier(value/Volt::MAX_VOLT);
      Drivhus::getSettings()->setShouldFlushSettings();
    }
  }
}

void Drivhus::WebServer::notifyClients(const std::string& key, const std::string& data) {
  Drivhus::getNetwork()->getWebServer()->textAll(key, data);
}

String Drivhus::WebServer::processor(const String& var){
  if (var == "SHOW_SETUP") {
    return Drivhus::getSettings()->isInSetupMode() ? "" : "hidden";
  } else if (var == "SIX") {
    return String(Drivhus::getNetwork()->getWebServer()->getUnusedSensorIdsAsString().c_str());
  } else if (var == "NIX") {
    uint8_t sensor_id = Drivhus::getNetwork()->getWebServer()->getUnusedSensorId();
    return String(Drivhus::getNetwork()->getWebServer()->generateSelectOptions(sensor_id).c_str());
  } else if (var.startsWith("SV")) {
    uint8_t sensor_id = static_cast<uint8_t>(std::stoul(var.substring(2).c_str(), nullptr, 16)&0xFF);
    return String(Drivhus::getNetwork()->getWebServer()->getSensorValueAsString(sensor_id).c_str());
  } else if (var.startsWith("NS")) {
    uint8_t sensor_id = static_cast<uint8_t>(std::stoul(var.substring(2).c_str(), nullptr, 16)&0xFF);
    return String(Drivhus::getNetwork()->getWebServer()->generateSelectOptions(sensor_id).c_str());
  } else if (var == "ITEMP") {
    return String(Drivhus::getNetwork()->getWebServer()->getIndoorTemp(), 1);
  } else if (var == "IHUMID") {
    return String(Drivhus::getNetwork()->getWebServer()->getIndoorHumid(), 1);
  } else if (var == "ILIGHT") {
    return String(Drivhus::getNetwork()->getWebServer()->getLight(), 1);
  } else if (var == "OTEMP") {
    return String(Drivhus::getNetwork()->getWebServer()->getOutdoorTemp(), 1);
  } else if (var == "OHUMID") {
    return String(Drivhus::getNetwork()->getWebServer()->getOutdoorHumid(), 1);
  } else if (var == "VOLT") {
    return String(Drivhus::getNetwork()->getWebServer()->getVolt(), 2);
  } else if (var == "VM") {
    return String(Drivhus::getNetwork()->getWebServer()->generateVoltMultiplierCalibration().c_str());
  } else if (Drivhus::getSettings()->isInSetupMode()) {
    if (var == "SSID") {
      return String(Drivhus::getSettings()->getSSID().c_str());
    } else if (var == "SSID_PASSWORD") {
      return String(Drivhus::getSettings()->getSSIDPassword().c_str());
    } else if (var == "MQTT_SERVER") {
      return String(Drivhus::getSettings()->getMQTTServername().c_str());
    } else if (var == "MQTT_PORT") {
      return String(Drivhus::getSettings()->getMQTTPort());
    } else if (var == "MQTT_ID") {
      return String(Drivhus::getSettings()->getMQTTServerId().c_str());
    } else if (var == "MQTT_USERNAME") {
      return String(Drivhus::getSettings()->getMQTTUsername().c_str());
    } else if (var == "MQTT_PASSWORD") {
      return String(Drivhus::getSettings()->getMQTTPassword().c_str());
    }
  }
#if 0
  Serial.print("Unknown PROCESSOR ");
  Serial.println(var);
#endif
  return String();
}

void Drivhus::WebServer::textAll(const std::string& key, const std::string& data) {
  m_ws->textAll((key+data).c_str());
}

void Drivhus::WebServer::updateNewSensorIdButtons(uint8_t sensor_id) {
  if (sensor_id>=RS485::DRIVHUS_MIN_ID && sensor_id<=RS485::DRIVHUS_MAX_ID) {
    if (!Drivhus::getSettings()->isInSetupMode() || !Drivhus::getRS485()->isSensorPresent(sensor_id)) {
      notifyClients(std::string("NS")+Drivhus::uint8ToHex(sensor_id), "");
    } else {
      notifyClients(std::string("NS")+Drivhus::uint8ToHex(sensor_id), generateSelectOptions(sensor_id));
    }
  } else if (sensor_id>=RS485::MIN_ID && sensor_id<=RS485::MAX_ID) {
    uint8_t sensor_id = getUnusedSensorId();
    if (!Drivhus::getSettings()->isInSetupMode() || sensor_id == RS485::UNDEFINED_ID) {
      notifyClients("NIX", "");
    } else {
      notifyClients("NIX", generateSelectOptions(sensor_id));
    }
  }
}

std::string Drivhus::WebServer::getSensorValueAsString(uint8_t sensor_id) const {
  return Drivhus::getRS485()->isSensorPresent(sensor_id) ? Drivhus::floatToString(Drivhus::getRS485()->getSensorHumidity(sensor_id), 2) : "Not available";
}

std::string Drivhus::WebServer::getUnusedSensorIdsAsString() const {
  std::stringstream ss;
  for (uint8_t item: m_present_sensors) {
    if (item>=RS485::MIN_ID && item<=RS485::MAX_ID &&
        !(item>=RS485::DRIVHUS_MIN_ID && item<=RS485::DRIVHUS_MAX_ID)) {
      if (ss.tellp() != std::streampos(0)) {
        ss << ',';
      }
      ss << std::to_string(item);
    }
  }
  return ss.str();
}

uint8_t Drivhus::WebServer::getUnusedSensorId() const {
  uint8_t sensor_id = RS485::UNDEFINED_ID;
  for (uint8_t item: m_present_sensors) {
    if (item>=RS485::MIN_ID && item<=RS485::MAX_ID &&
        !(item>=RS485::DRIVHUS_MIN_ID && item<=RS485::DRIVHUS_MAX_ID)) {
      if (sensor_id != RS485::UNDEFINED_ID) {
        return RS485::UNDEFINED_ID;
      } else {
        sensor_id = item;
      }
    }
  }
  return sensor_id;
}

std::string Drivhus::WebServer::generateSelectOptions(uint8_t sensor_id) const {
  if (!Drivhus::getSettings()->isInSetupMode() ||
      sensor_id<RS485::MIN_ID || sensor_id>RS485::MAX_ID ||
      !Drivhus::getRS485()->isSensorPresent(sensor_id)) {
    return "";
  }

 std::stringstream ss;
  ss << "&nbsp;<select id=\"SO" << Drivhus::uint8ToHex(sensor_id) << "\">"; //Sensor ID Option
  for (uint8_t i=RS485::DRIVHUS_MIN_ID; i<=RS485::DRIVHUS_MAX_ID; i++) {
    if (m_present_sensors.find(i) == m_present_sensors.end()) {
      ss << "<option value=\"" << Drivhus::uint8ToHex(i) << "\">" << std::to_string(i) << "</option>";
    }
  }
  ss << "</select><button onCLick=\"updateSensorId('" << Drivhus::uint8ToHex(sensor_id) << "')\">Set new sensor ID</button>";
  return ss.str();
}

std::string Drivhus::WebServer::generateVoltMultiplierCalibration() const {
  if (!Drivhus::getSettings()->isInSetupMode()) {
    return "";
  }

  return "&nbsp;<input type=\"number\" id=\"VOLT_MULTIPLIER\" min=\"0.0\" max=\"14.5\" step=\"0.1\" value=\"%VOLT_MULTIPLIER%\">V <button onCLick=\"updateVoltMultiplier()\">Calibrate</button>";
}

void Drivhus::WebServer::checkIfWarningMessageShouldBeShown() {
  const std::lock_guard<std::recursive_mutex> lock(m_warning_messages_mutex);
  if (!m_warning_messages.empty()) {
    std::string msg = m_warning_messages.front();
    m_warning_messages.pop_front();
    showWarningMessage(msg);
  }
}

void Drivhus::WebServer::showWarningMessage(const std::string& msg) {
  notifyClients("MSG", msg);
  m_warning_message_time = msg.empty() ? 0L : millis();
}

void Drivhus::WebServer::activateRelayTests() {
  if (!m_is_testing_relays) {
    m_is_testing_relays = true;
    m_relay_test_event_time = millis();
    m_relay_test_index = 0;
    m_relay_test_on = false;
  }  
}

void Drivhus::WebServer::updateRelayTest() {
  const unsigned long current_time = millis();
  if (current_time < m_relay_test_event_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_relay_test_event_time = current_time;
  }

  if (!m_relay_test_on && (m_relay_test_event_time+RELAY_TEST_DELAY_MS)<current_time) {
    if (activateTestRelay(true)) {
      m_relay_test_event_time = current_time;
      m_relay_test_on = true;
    }
  } else if (m_relay_test_on && (m_relay_test_event_time+RELAY_TEST_ON_MS)<current_time) {
    if (activateTestRelay(false)) {
      m_relay_test_event_time = current_time;
      m_relay_test_on = false;
      if (++m_relay_test_index >= (15+1+1+1)) {
        m_is_testing_relays = false;
      }
    }
  }
}

bool Drivhus::WebServer::activateTestRelay(bool turn_on) {
  if (m_relay_test_index < 15) {
    if (!Drivhus::getCD74HC4067()->isActive()) {
      if (turn_on) {
        addWarningMessage(std::string("Testing WaterPump ")+std::to_string(m_relay_test_index+1));
        Drivhus::getCD74HC4067()->activate(m_relay_test_index+1, RELAY_TEST_ON_MS);
        return true;
      } else {
        return true; //Relay turns itself off.
      }
    }
  } else if (m_relay_test_index == 15) {
    if (turn_on) addWarningMessage("Testing Fan");
    digitalWrite(O_FAN_PIN, turn_on ? HIGH : LOW);
    return true;
  } else if (m_relay_test_index == 16) {
    if (turn_on) addWarningMessage("Testing GrowLight");
    digitalWrite(O_GROWLIGHT_PIN, turn_on ? HIGH : LOW);
    return true;
  } else if (m_relay_test_index == 17) {
    if (turn_on) addWarningMessage("Testing Water Valve");
    digitalWrite(O_WATER_VALVE_PIN, turn_on ? HIGH : LOW);
    return true;
  }
  return false;
}
