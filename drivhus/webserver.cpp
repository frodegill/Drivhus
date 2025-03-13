#include <string>
#include "webserver.h"

#include <cstring>
#include <iomanip>
#include <sstream>

#include "global.h"
#include "log.h"
#include "network.h"
#include "settings.h"
#include "soilsensors.h"
#include "waterpumps.h"

// A = PreProcessor "SHOW SETUP"
// B = PreProcessor "SSID"
// C = PreProcessor "SSID_PASSWORD"
// D = PreProcessor "MQTT_SERVER"
// E = PreProcessor "MQTT_PORT"
// F = PreProcessor "MQTT_ID"
// G = PreProcessor "MQTT_USERNAME"
// H = PreProcessor "MQTT_PASSWORD"
// I = PreProcessor "TimeZone Options"
// J = PreProcessor "Emulate Latitude"
// K = PreProcessor "Emulate Longitude"
// l = PreProcessor "Sensor Value"
// m = PreProcessor "Sensor Enable/Disable"
// N = "Indoor temp"
// O = "Indoor humidity"
// P = "Indoor light"
// Q = "Outdoor temp"
// R = "Outdoor humidity"
// S = "Water low trigger"
// T = "Water high trigger"
// U = "Water valve"
// V = "Volt"
// W = PreProcessor "Volt multiplier"
// X = "Growlight time"
// Y = unused
// Z = "Volt multiplier"
// z = "Sensor Enable"

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
<div id="SETUP" %A%>
SSID:<input type="text" id="SSID" required maxlength="32" autofocus value="%B%"><br>
SSID Password:<input type="password" id="SSID_PASSWORD" maxlength="64" value="%C%"><br>
MQTT server:<input type="text" id="MQTT_SERVER" maxlength="64" value="%D%"><br>
MQTT port:<input type="number" id="MQTT_PORT" min="0" max="65535" value="%E%"><br>
MQTT ID postfix:<input type="text" id="MQTT_ID" maxlength="32" value="%F%"><br>
MQTT username:<input type="text" id="MQTT_USERNAME" maxlength="32" value="%G%"><br>
MQTT password:<input type="password" id="MQTT_PASSWORD" maxlength="32" value="%H%"><br>
Timezone:<select id="TZ">%I%</select><br>
Emulate Growlight location: <input type="number" id="EM_LATITUDE" min="-90" max="90" value="%J%">N/S <input type="number" id="EM_LONGITUDE" min="-180" max="180" value="%K%">E/W<br>
<button onClick="updateSetup()">Submit</button> <button onClick="testWaterpumps()">Test waterpumps</button>
<hr>
</div>
<div>
<table>
<tr><th><div>Sensor ID</div></th><th>Moisture %%</th></tr>
<tr><td>1</td><td><span id="lA">%lA%</span>&nbsp;<span id="mA">%mA%</span></td></tr>
<tr><td>2</td><td><span id="lB">%lB%</span>&nbsp;<span id="mB">%mB%</span></td></tr>
<tr><td>3</td><td><span id="lC">%lC%</span>&nbsp;<span id="mC">%mC%</span></td></tr>
<tr><td>4</td><td><span id="lD">%lD%</span>&nbsp;<span id="mD">%mD%</span></td></tr>
<tr><td>5</td><td><span id="lE">%lE%</span>&nbsp;<span id="mE">%mE%</span></td></tr>
<tr><td>6</td><td><span id="lF">%lF%</span>&nbsp;<span id="mF">%mF%</span></td></tr>
<tr><td>7</td><td><span id="lG">%lG%</span>&nbsp;<span id="mG">%mG%</span></td></tr>
<tr><td>8</td><td><span id="lH">%lH%</span>&nbsp;<span id="mH">%mH%</span></td></tr>
<tr><td>9</td><td><span id="lI">%lI%</span>&nbsp;<span id="mI">%mI%</span></td></tr>
<tr><td>10</td><td><span id="lJ">%lJ%</span>&nbsp;<span id="mJ">%mJ%</span></td></tr>
<tr><td>11</td><td><span id="lK">%lK%</span>&nbsp;<span id="mK">%mK%</span></td></tr>
<tr><td>12</td><td><span id="lL">%lL%</span>&nbsp;<span id="mL">%mL%</span></td></tr>
<tr><td>13</td><td><span id="lM">%lM%</span>&nbsp;<span id="mM">%mM%</span></td></tr>
<tr><td>14</td><td><span id="lN">%lN%</span>&nbsp;<span id="mN">%mN%</span></td></tr>
<tr><td>15</td><td><span id="lO">%lO%</span>&nbsp;<span id="mO">%mO%</span></td></tr>
</table>
<table>
<tr><th>Sensor</th><th>Value</th></tr>
<tr><td>Indoor temp</td><td><span id="N">%N%</span><span>&nbsp;C</span></td></tr>
<tr><td>Indoor humidity</td><td><span id="O">%O%</span><span>&nbsp;%%RH</span></td></tr>
<tr><td>Indoor light</td><td><span id="P">%P%</span><span>&nbsp;%%</span></td></tr>
<tr><td>Outdoor temp</td><td><span id="Q">%Q%</span><span>&nbsp;C</span></td></tr>
<tr><td>Outdoor humidity</td><td><span id="R">%R%</span></td></tr>
<tr><td>Water Low Trigger</td><td><span id="S">%S%</span></td></tr>
<tr><td>Water High Trigger</td><td><span id="T">%T%</span></td></tr>
<tr><td>Water Valve</td><td><span id="U">%U%</span></td></tr>
<tr><td>Voltage</td><td><span id="V">%V%</span><span>&nbsp;V</span><span id="W">%W%</span></td></tr>
<tr><td>Growlight time</td><td><span id="X">%X%</span></td></tr>
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
    if (-1!="NOPQRSTUVWX".indexOf(event.data.substring(0,1))){
      document.getElementById(event.data.substring(0,1)).innerHTML = event.data.substring(1);
    } else if (-1!="lm".indexOf(event.data.substring(0,1))){
      document.getElementById(event.data.substring(0,2)).innerHTML = event.data.substring(2);
    } else if (event.data.startsWith('A')){
      var elm = document.getElementById('SETUP');
      if (event.data.length == 1) { //'A' with no value
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
  function updateSetup(){websocket.send('SETUP'+elmValue('SSID')+'\n'+elmValue('SSID_PASSWORD')+'\n'+elmValue('MQTT_SERVER')+'\n'+elmValue('MQTT_PORT')+'\n'+elmValue('MQTT_ID')+'\n'+elmValue('MQTT_USERNAME')+'\n'+elmValue('MQTT_PASSWORD')+'\n'
    +elmValue('TZ')+'\n'+elmValue('EM_LATITUDE')+'\n'+elmValue('EM_LONGITUDE'));}
  function toggleSensor(id){websocket.send(id);}
  function testWaterpumps(){websocket.send('TP');}
  function updateVoltMultiplier(){websocket.send('W'+elmValue('Z'));}
  function elmValue(elmId){return document.getElementById(elmId).value;}
</script>
</html>
)rawliteral";


Drivhus::WebServer::WebServer()
: Drivhus::Component(),
  Drivhus::OnValueChangeListener(),
  Drivhus::OnConfigChangeListener(),
  m_is_showing_setup(false),
  m_warning_message_time(0L),
  m_is_testing_waterpumps(false),
  m_waterpumps_test_event_time(0L),
  m_waterpumps_test_index(0),
  m_waterpumps_test_on(false)
{
  Drivhus::getSettings()->addValueChangeListener(this);
  Drivhus::getSettings()->addConfigChangeListener(this);
  m_temp[0] = Drivhus::getSettings()->getIndoorTemp();
  m_humid[0] = Drivhus::getSettings()->getIndoorHumidity();
  m_temp[1] = Drivhus::getSettings()->getOutdoorTemp();
  m_humid[1] = Drivhus::getSettings()->getOutdoorHumidity();
  m_humid[2] = Drivhus::getSettings()->getOutdoorAsIndoorHumidity();
  m_light = Drivhus::getSettings()->getLight();
  m_volt = Drivhus::getSettings()->getVolt();
  m_sunrise = Drivhus::getSettings()->getSunrise();
  m_sunset = Drivhus::getSettings()->getSunset();
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
  updateGrowlightTime();
  return true;
}

void Drivhus::WebServer::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_warning_message_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_warning_message_time = current_time;
  }

  if (m_is_testing_waterpumps) {
    updateWaterpumpsTest();
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

void Drivhus::WebServer::onValueChanged(Drivhus::OnValueChangeListener::Type type, uint8_t plant_id) {
  switch(type) {
    case Drivhus::OnValueChangeListener::Type::INDOOR_TEMP: //[[fallthrough]]
    case Drivhus::OnValueChangeListener::Type::INDOOR_HUMIDITY: //[[fallthrough]]
    case Drivhus::OnValueChangeListener::Type::OUTDOOR_TEMP: //[[fallthrough]]
    case Drivhus::OnValueChangeListener::Type::OUTDOOR_HUMIDITY: {
      float value = Drivhus::getSettings()->getIndoorTemp();
      if (value<(m_temp[0]*0.99f) || value>(m_temp[0]*1.01f)) {
          m_temp[0] = value;
          notifyClients("N", String(value, 1).c_str());
      }
      value = Drivhus::getSettings()->getIndoorHumidity();
      if (value<(m_humid[0]*0.99f) || value>(m_humid[0]*1.01f)) {
        m_humid[0]=value;
        notifyClients("O", String(value, 1).c_str());
      }
      value = Drivhus::getSettings()->getOutdoorTemp();
      if (value<(m_temp[1]*0.99f) || value>(m_temp[1]*1.01f)) {
        m_temp[1]=value;
        notifyClients("Q", String(value, 1).c_str());
      }
      value = Drivhus::getSettings()->getOutdoorHumidity();
      float value2 = Drivhus::getSettings()->getOutdoorAsIndoorHumidity();
      if (value<(m_humid[1]*0.99f) || value>(m_humid[1]*1.01f) ||
          (value2==NAN && m_humid[2]!=NAN) || (value2!=NAN && m_humid[2]==NAN) ||
          (value2!=NAN && (value2<(m_humid[2]*0.99f) || value2>(m_humid[2]*1.01f)))) {
        m_humid[1]=value;
        m_humid[2]=value2;
        notifyClients("R", generateOutdoorHumidityAsString().c_str());
      }
      break;
    }
    case Drivhus::OnValueChangeListener::Type::PLANT_MOISTURE: {
      updateSensor(plant_id);
      break;
    }
    case Drivhus::OnValueChangeListener::Type::LIGHT: {
      float value = Drivhus::getSettings()->getLight();
      if (value<(m_light*0.99f) || value>(m_light*1.01f)) {
        m_light=value;
        notifyClients("P", String(value, 1).c_str());
      }
      break;
    }
    case Drivhus::OnValueChangeListener::Type::VOLT: {
      float value = Drivhus::getSettings()->getVolt();
      if (value<(m_volt*0.99f) || value>(m_volt*1.01f)) {
        m_volt=value;
        notifyClients("V", String(value, 2).c_str());
      }
      break;
    }
    case Drivhus::OnValueChangeListener::Type::WATER_LOW_TRIGGER: {
      notifyClients("S", Drivhus::getSettings()->getWaterLowTrigger()==LOW ? "Inactive" : "Active");
      break;
    }
    case Drivhus::OnValueChangeListener::Type::WATER_HIGH_TRIGGER: {
      notifyClients("T", Drivhus::getSettings()->getWaterHighTrigger()==LOW ? "Inactive" : "Active");
      break;
    }
    case Drivhus::OnValueChangeListener::Type::WATER_VALVE: {
      notifyClients("U", Drivhus::getSettings()->getWaterValveStatus()==Drivhus::ValveStatus::OPEN ? "Open" : "Closed");
      break;
    }
    case Drivhus::OnValueChangeListener::Type::SUNRISE: {
      m_sunrise = Drivhus::getSettings()->getSunrise();
      updateGrowlightTime();
      break;
    }
    case Drivhus::OnValueChangeListener::Type::SUNSET: {
      m_sunset = Drivhus::getSettings()->getSunset();
      updateGrowlightTime();
      break;
    }

    default: break;
  };
}

void Drivhus::WebServer::onConfigChanged(Drivhus::OnConfigChangeListener::Type type, uint8_t id) {
  switch(type) {
    case Drivhus::OnConfigChangeListener::Type::SETUP_MODE:
      {
        bool tmp = Drivhus::getSettings()->getIsInSetupMode();
        if (m_is_showing_setup!=tmp) {
          m_is_showing_setup = tmp;
          notifyClients("A", m_is_showing_setup ? "" : "hidden");
          for (uint8_t sensor_id=Drivhus::SoilSensors::MIN_ID; sensor_id<=Drivhus::SoilSensors::MAX_ID; sensor_id++) {
            notifyClients(std::string("m")+static_cast<char>('A'+sensor_id-1), generateSensorControl(sensor_id));
          }
          notifyClients("W", generateVoltMultiplierCalibration());
        }
      }
      break;
    case Drivhus::OnConfigChangeListener::Type::PLANT_ENABLED:
      notifyClients(std::string("m")+static_cast<char>('A'+id-1), generateSensorControl(id)); //[[fallthrough]]
    case Drivhus::OnConfigChangeListener::Type::PLANT_WET_VALUE: //[[fallthrough]]
    case Drivhus::OnConfigChangeListener::Type::PLANT_DRY_VALUE:
      updateSensor(id);
      break;

    default: break;
  };
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
          case 7: Drivhus::getSettings()->setTimezone(line); break;
          case 8: Drivhus::getSettings()->setEmulateLatitude(static_cast<int8_t>(std::min(90,std::max(-90,std::stoi(line))))); break;
          case 9: Drivhus::getSettings()->setEmulateLongitude(static_cast<int16_t>(std::min(180,std::max(-180,std::stoi(line))))); break;
        }
      }
      Drivhus::getSettings()->setShouldFlushSettings();
    } else if (len>=2 && *data_str=='z') {
      uint8_t sensor_id = data_str[1]-'A'+1;
      Drivhus::getSettings()->setEnabled(sensor_id, !Drivhus::getSettings()->getEnabled(sensor_id));
    } else if (len>=2 && std::strncmp("TP", data_str, 2)==0) {
      Drivhus::getWebServer()->activateWaterpumpsTests();
    } else if (*data_str=='W') {
      float value = std::stof(std::string(data_str+1));
      Drivhus::getSettings()->setVoltMultiplier(value/Volt::MAX_VOLT);
      Drivhus::getSettings()->setShouldFlushSettings();
    }
  }
}

void Drivhus::WebServer::notifyClients(const std::string& key, const std::string& data) {
  Drivhus::getWebServer()->textAll(key, data);
}

String Drivhus::WebServer::processor(const String& var){
  if (var == "A") {
    return Drivhus::getSettings()->getIsInSetupMode() ? "" : "hidden";
  } else if (var.startsWith("l") && var.length()>=2) {
    uint8_t sensor_id = var[1]-'A'+1;
    return String(Drivhus::getWebServer()->getSensorValueAsString(sensor_id).c_str());
  } else if (var.startsWith("m") && var.length()>=2) {
    uint8_t sensor_id = var[1]-'A'+1;
    return String(Drivhus::getWebServer()->generateSensorControl(sensor_id).c_str());
  } else if (var == "N") {
    return String(Drivhus::getWebServer()->getIndoorTemp(), 1);
  } else if (var == "O") {
    return String(Drivhus::getWebServer()->getIndoorHumid(), 1);
  } else if (var == "P") {
    return String(Drivhus::getWebServer()->getLight(), 1);
  } else if (var == "Q") {
    return String(Drivhus::getWebServer()->getOutdoorTemp(), 1);
  } else if (var == "R") {
    return String(Drivhus::getWebServer()->generateOutdoorHumidityAsString().c_str());
  } else if (var == "S") {
    return String(Drivhus::getSettings()->getWaterLowTrigger()==LOW ? "Inactive" : "Active");
  } else if (var == "T") {
    return String(Drivhus::getSettings()->getWaterHighTrigger()==LOW ? "Inactive" : "Active");
  } else if (var == "U") {
    return String(Drivhus::getSettings()->getWaterValveStatus()==Drivhus::ValveStatus::OPEN ? "Open" : "Closed");
  } else if (var == "V") {
    return String(Drivhus::getWebServer()->getVolt(), 2);
  } else if (var == "W") {
    return String(Drivhus::getWebServer()->generateVoltMultiplierCalibration().c_str());
  } else if (var == "X") {
    return String(Drivhus::getWebServer()->getGrowlightTime().c_str());
  } else if (Drivhus::getSettings()->getIsInSetupMode()) {
    if (var == "B") {
      return String(Drivhus::getSettings()->getSSID().c_str());
    } else if (var == "C") {
      return String(Drivhus::getSettings()->getSSIDPassword().c_str());
    } else if (var == "D") {
      return String(Drivhus::getSettings()->getMQTTServername().c_str());
    } else if (var == "E") {
      return String(Drivhus::getSettings()->getMQTTPort());
    } else if (var == "F") {
      return String(Drivhus::getSettings()->getMQTTServerId().c_str());
    } else if (var == "G") {
      return String(Drivhus::getSettings()->getMQTTUsername().c_str());
    } else if (var == "H") {
      return String(Drivhus::getSettings()->getMQTTPassword().c_str());
    } else if (var == "I") {
      return String(Drivhus::getWebServer()->generateTimezoneSelectOptions().c_str());
    } else if (var == "J") {
      return String(Drivhus::getSettings()->getEmulateLatitude());
    } else if (var == "K") {
      return String(Drivhus::getSettings()->getEmulateLongitude());
    }
  }
#if 0
  Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("Unknown PROCESSOR ")+var);
#endif
  return String();
}

void Drivhus::WebServer::textAll(const std::string& key, const std::string& data) {
  m_ws->textAll((key+data).c_str());
}

void Drivhus::WebServer::updateSensor(uint8_t sensor_id) {
  if (sensor_id>=Drivhus::SoilSensors::MIN_ID && sensor_id<=Drivhus::SoilSensors::MAX_ID) {
    notifyClients(std::string("l")+static_cast<char>('A'+sensor_id-1), getSensorValueAsString(sensor_id));
  }
}

void Drivhus::WebServer::updateGrowlightTime() {
  uint8_t from_hour = static_cast<uint8_t>(m_sunrise/60);
  uint8_t from_minutes = static_cast<uint8_t>(m_sunrise - from_hour*60);
  uint8_t to_hour = static_cast<uint8_t>(m_sunset/60);
  uint8_t to_minutes = static_cast<uint8_t>(m_sunset - to_hour*60);

  std::stringstream ss;
  ss << std::setw(2) << std::setfill('0') << std::to_string(from_hour);
  ss << ':';
  ss << std::setw(2) << std::setfill('0') << std::to_string(from_minutes);
  ss << " - ";
  ss << std::setw(2) << std::setfill('0') << std::to_string(to_hour);
  ss << ':';
  ss << std::setw(2) << std::setfill('0') << std::to_string(to_minutes);

  m_growlight_time = ss.str();
  notifyClients("X", m_growlight_time);
}

std::string Drivhus::WebServer::getSensorValueAsString(uint8_t sensor_id) const {
  if (!Drivhus::getSettings()->getEnabled(sensor_id)) {
    return "Disabled";
  }
  std::stringstream ss;
  ss << Drivhus::floatToString(Drivhus::getSettings()->getPlantMoisture(sensor_id), 2);
  ss << " (";
  ss << Drivhus::floatToString(Drivhus::getSettings()->getWetValue(sensor_id), 2);
  ss << " - ";
  ss << Drivhus::floatToString(Drivhus::getSettings()->getDryValue(sensor_id), 2);
  ss << ")";
  return ss.str();
}

std::string Drivhus::WebServer::generateSensorControl(uint8_t sensor_id) const {
  std::stringstream ss;
  if (Drivhus::getSettings()->getIsInSetupMode())
  {
    ss << "<input type=\"checkbox\" id=\"z" << static_cast<char>('A'+sensor_id-1) << "\" onChange=\"toggleSensor(this.id)\" "
       << (Drivhus::getSettings()->getEnabled(sensor_id)?"checked":"") << ">";
  }
  return ss.str();
}

std::string Drivhus::WebServer::generateOutdoorHumidityAsString() const {
  std::stringstream ss;
  ss << Drivhus::floatToString(m_humid[1], 1) << " %RH";
  if (m_humid[2] != NAN) {
    ss << " (" << Drivhus::floatToString(m_humid[2], 1) << " %RH)";
  }
  return ss.str();
}

std::string Drivhus::WebServer::generateVoltMultiplierCalibration() const {
  //<tr><td>Voltage</td><td><span id="V">%V%</span><span>&nbsp;V</span><span id="W">%W%</span></td></tr>
  return Drivhus::getSettings()->getIsInSetupMode() ?
    "&nbsp;<input type=\"number\" id=\"Z\" min=\"0.0\" max=\"14.5\" step=\"0.1\" value=\"%Z%\">V <button onCLick=\"updateVoltMultiplier()\">Calibrate</button>"
    : "";
}

std::string Drivhus::WebServer::generateTimezoneSelectOptions() const {
  std::stringstream ss;
  for (auto tz : g_timezones) {
    ss << "<option value=\"" << tz.code << "\">" << tz.description << "</option>";
  }
  return ss.str();
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

void Drivhus::WebServer::activateWaterpumpsTests() {
  if (!m_is_testing_waterpumps) {
    m_is_testing_waterpumps = true;
    m_waterpumps_test_event_time = millis();
    m_waterpumps_test_index = 0;
    m_waterpumps_test_on = false;
  }  
}

void Drivhus::WebServer::updateWaterpumpsTest() {
  const unsigned long current_time = millis();
  if (current_time < m_waterpumps_test_event_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_waterpumps_test_event_time = current_time;
  }

  if (!m_waterpumps_test_on && (m_waterpumps_test_event_time+WATERPUMPS_TEST_DELAY_MS)<current_time) {
    if (activateTestWaterpumps(true)) {
      m_waterpumps_test_event_time = current_time;
      m_waterpumps_test_on = true;
    }
  } else if (m_waterpumps_test_on && (m_waterpumps_test_event_time+WATERPUMPS_TEST_ON_MS)<current_time) {
    if (activateTestWaterpumps(false)) {
      m_waterpumps_test_event_time = current_time;
      m_waterpumps_test_on = false;
      if (++m_waterpumps_test_index >= (15+1+1+1)) {
        m_is_testing_waterpumps = false;
      }
    }
  }
}

bool Drivhus::WebServer::activateTestWaterpumps(bool turn_on) {
  if (m_waterpumps_test_index < Drivhus::SoilSensors::MAX_ID) {
    if (!Drivhus::getWaterPumps()->isBusy()) {
      if (turn_on) {
        uint8_t plant_id = m_waterpumps_test_index+1;
        addWarningMessage(std::string("Testing Waterpump ")+std::to_string(plant_id));
        Drivhus::getSettings()->setRequestWatering(plant_id);
      }
      return true; //Pumps turn themselves off.
    }
  } else if (m_waterpumps_test_index == (15+0)) {
    if (turn_on) addWarningMessage("Testing Fan");
    digitalWrite(O_FAN_PIN, turn_on ? HIGH : LOW);
    return true;
  } else if (m_waterpumps_test_index == (15+1)) {
    if (turn_on) addWarningMessage("Testing GrowLight");
    digitalWrite(O_GROWLIGHT_PIN, turn_on ? HIGH : LOW);
    return true;
  } else if (m_waterpumps_test_index == (15+2)) {
    if (turn_on) addWarningMessage("Testing Water Valve");
    digitalWrite(O_WATER_VALVE_PIN, turn_on ? HIGH : LOW);
    return true;
  }
  return false;
}
