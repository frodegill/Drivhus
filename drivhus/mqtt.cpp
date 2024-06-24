#include <string>
#include "mqtt.h"

#include <iomanip>
#include <sstream>

#include "global.h"
#include "log.h"
#include "network.h"
#include "settings.h"


Drivhus::MQTT::MQTT()
: Drivhus::Component(),
  Drivhus::OnValueChangeListener(),
  m_reconnect_time(0L),
  m_cached_packet_time(0L),
  m_max_cache_time(0L),
  m_fields_changed(0),
  m_plants_changed(0) {
  Drivhus::getSettings()->addValueChangeListener(this);
}

bool Drivhus::MQTT::init() {
  m_mqtt_client.setClient(m_esp_client);
  m_mqtt_client.setServer(Drivhus::getSettings()->getMQTTServername().c_str(), 1883);
  m_mqtt_client.setCallback(globalMQTTCallback);
  Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("MQTT initialised"));
  return true;
}

void Drivhus::MQTT::loop() {
  if (!Drivhus::getSettings()->hasMQTTServer()) {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("No MQTT server"));
    return;
  }

  const unsigned long current_time = millis();
  if (current_time < m_cached_packet_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_cached_packet_time = current_time;
  }

  if (current_time < m_reconnect_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_reconnect_time = current_time;
  }

  if (m_reconnect_time!=0L && //Have connection succeeded?
      (m_reconnect_time+MQTT_CONNECTION_TIMEOUT_MS)>=current_time) {
    m_reconnect_time = 0L;
    if (Drivhus::getNetwork()->isConnected()) {
      subscribe();
    }
  }

  publishPendingFields();

  if (m_esp_client.connected()) {
    m_mqtt_client.loop();
  }
}

unsigned long Drivhus::MQTT::changeTypeToCacheTime(OnValueChangeListener::Type type) {
  switch(type) {
    case LIGHT:
    case VOLT: return 30*1000;

    case WATER_LOW_TRIGGER:
    case WATER_HIGH_TRIGGER:
    case WATER_VALVE:
    case PLANT_IN_WATERING_CYCLE:
    case PLANT_WATERING_STARTED:
    case PLANT_WATERING_ENDED: return 1*1000;

    default: return 15*1000;
  };
}

std::string Drivhus::MQTT::errorStateMessage() {
  switch(m_mqtt_client.state()) {
    case (-4): return "the server didn't respond within the keepalive time";
    case (-3): return "the network connection was broken";
    case (-2): return "the network connection failed";
    case (-1): return "the client is disconnected cleanly";
    case 0: return "the client is connected";
    case 1: return "the server doesn't support the requested version of MQTT";
    case 2: return "the server rejected the client identifier";
    case 3: return "the server was unable to accept the connection";
    case 4: return "the username/password were rejected";
    case 5: return "the client was not authorized to connect";
    default: return "unknown error";
  };
}

void Drivhus::MQTT::onValueChanged(OnValueChangeListener::Type type, uint8_t plant_id) {
  unsigned long max_cache_time = changeTypeToCacheTime(type);
  switch(type) {
    case PLANT_MOISTURE:
    case PLANT_IN_WATERING_CYCLE:
    case PLANT_WATERING_STARTED:
    case PLANT_WATERING_ENDED:
      if (Drivhus::isValidPlantId(plant_id)) {
        m_plants_changed |= 1<<(plant_id-1);
      }
      break;

    default:
      m_fields_changed |= 1<<type;
  };
  
  if (m_cached_packet_time == 0) {
    m_cached_packet_time = millis();
  }

  if (m_max_cache_time==0 || max_cache_time<m_max_cache_time) {
    m_max_cache_time = max_cache_time;
  }
}

void Drivhus::MQTT::globalMQTTCallback(char* topic, uint8_t* payload, unsigned int length) {
  Drivhus::getMQTT()->callback(topic, payload, length);  
}

void Drivhus::MQTT::callback(char* topic, uint8_t* payload, unsigned int length) {
  Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("MQTT callback for ")+topic);

  auto config_topic = Drivhus::getSettings()->getMQTTServerId()+CONFIG_TOPIC;
  if (0 != strncmp(config_topic.c_str(), topic, config_topic.length())) {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("Ignoring unrelated mqtt::callback ")+topic);
    return;
  }
 
  std::istringstream input(std::string(reinterpret_cast<const char*>(payload), length));
  for (std::string line; std::getline(input, line);) {
    if (0 == line.rfind("sec_between_reading=", 0)) {
      unsigned long value = max(1L, atol(line.substr(20).c_str()));
      Drivhus::getSettings()->setMsBetweenReading(value*1000);
    } else if (0 == line.rfind("fan_activate_temp=", 0)) {
      float value = max(0.0, min(100.0, atof(line.substr(18).c_str())));
      Drivhus::getSettings()->setFanActivateTemp(value);
    } else if (0 == line.rfind("fan_activate_humid=", 0)) {
      float value = max(0.0, min(100.0, atof(line.substr(19).c_str())));
      Drivhus::getSettings()->setFanActivateHumidity(value);
    } else if (0==line.rfind("plant", 0)) {
      uint8_t plant_id = 0;
      size_t index = 5;
      char c;
      bool in_number = true;
      while ((c=line[index++]) != 0) {
        if (in_number && c>='0' && c<='9') {
          plant_id = plant_id*10 + (c-'0');
        } else if (c == '.') {
          break;
        } else {
          in_number = false;
        }
      }

      if (c==0 || !Drivhus::isValidPlantId(plant_id)) { //If c==0, we reached end of line without finding '.'
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("Skipping invalid conf line: ")+line);
      } else {
        line = line.substr(index); //Skip "plantXX."

        if (0 == line.rfind("water_now=", 0)) {
          if (0 != atoi(line.substr(10).c_str())) { //1 = activate
            Drivhus::getSettings()->setRequestWatering(plant_id);
          }
        } else if (0 == line.rfind("enabled=", 0)) {
          Drivhus::getSettings()->setEnabled(plant_id, atoi(line.substr(8).c_str())!=0);
        } else if (0 == line.rfind("dry_value=", 0)) {
          Drivhus::getSettings()->setDryValue(plant_id, max(0.0, min(100.0, atof(line.substr(10).c_str()))));
        } else if (0 == line.rfind("wet_value=", 0)) {
          Drivhus::getSettings()->setWetValue(plant_id, max(0.0, min(100.0, atof(line.substr(10).c_str()))));
        } else if (0 == line.rfind("watering_duration_ms=", 0)) {
          Drivhus::getSettings()->setWateringDuration(plant_id, max(1L, atol(line.substr(21).c_str())));
        } else if (0 == line.rfind("watering_grace_period_sec=", 0)) {
          Drivhus::getSettings()->setWateringGracePeriodMs(plant_id, max(1L, atol(line.substr(26).c_str())) * 1000);
        }
      }
    }
  }
}

void Drivhus::MQTT::requestMQTTConnection() {
  auto settings = Drivhus::getSettings();
  if (m_mqtt_client.connected() || //Already connected
      !settings->hasMQTTServer() || //Not supposed to connect
      m_reconnect_time!=0L) { //Has a pending connection attempt
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("MQTT request not needed"));
    return;
  }

  m_mqtt_client.connect(settings->getMQTTServerId().c_str(), settings->getMQTTUsername().c_str(), settings->getMQTTPassword().c_str());
  m_reconnect_time = millis();
}

void Drivhus::MQTT::log(const std::string& msg) {
  if (!m_mqtt_client.publish((Drivhus::getSettings()->getMQTTServerId()+LOG_TOPIC).c_str(), msg.c_str(), true)) {
    Serial.println((std::string("Publishing MQTT log packet failed with error: ")+errorStateMessage()).c_str()); //Use only Serial to prevent infinite MQTT publish loop
  }
}

void Drivhus::MQTT::subscribe() {
  if (!Drivhus::getNetwork()->isConnected()) {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("Network not connected. Delaying MQTT subscription"));
    return;
  }

  Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("MQTT: Subscribing to ")
                                                                         + Drivhus::getSettings()->getMQTTServername() + ":" + std::to_string(Drivhus::getSettings()->getMQTTPort())
                                                                         + " topic " + Drivhus::getSettings()->getMQTTServerId()+CONFIG_TOPIC);

  if (!m_mqtt_client.subscribe((Drivhus::getSettings()->getMQTTServerId()+CONFIG_TOPIC).c_str())) {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_ERROR, std::string("MQTT topics subscribed with error: ")+errorStateMessage());
    m_reconnect_time = millis(); //To request a new subscribe
  }
}

void Drivhus::MQTT::publishPendingFields() {
  const unsigned long current_time = millis();
  if (!Drivhus::getSettings()->getIsSystemReady() ||
      m_cached_packet_time==0 ||
      (m_fields_changed==0 && m_plants_changed==0) ||
      (m_cached_packet_time+m_max_cache_time)>=current_time) {
    return;
  }

  if (!m_mqtt_client.connected()) {
    requestMQTTConnection();
    return;
  }

  uint32_t fields_pushed = 0;
  uint16_t plants_pushed = 0;
  std::stringstream ss;
  bool first = true;
  ss << std::fixed << "{\n";

  for (auto i=1; i<=Drivhus::MAX_PLANT_COUNT; i++) {
    if ((m_plants_changed & 1<<(i-1)) != 0) {
      if (appendField(ss, first, "plant"+std::to_string(i)+".moisture", Drivhus::getSettings()->getPlantMoisture(i), 2) &&
          appendField(ss, first, "plant"+std::to_string(i)+".watering", Drivhus::getSettings()->getIsWateringPlant(i) ? 1 : 0)) {
        plants_pushed |= 1<<(i-1);
      }
    }
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::INDOOR_TEMP)!=0) {
    if (appendField(ss, first, "itemp", Drivhus::getSettings()->getIndoorTemp(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::INDOOR_TEMP;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::INDOOR_HUMIDITY)!=0) {
    if (appendField(ss, first, "ihumid", Drivhus::getSettings()->getIndoorHumidity(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::INDOOR_HUMIDITY;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::OUTDOOR_TEMP)!=0) {
    if (appendField(ss, first, "otemp", Drivhus::getSettings()->getOutdoorTemp(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::OUTDOOR_TEMP;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::OUTDOOR_HUMIDITY)!=0) {
    if (appendField(ss, first, "ohumid", Drivhus::getSettings()->getOutdoorHumidity(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::OUTDOOR_HUMIDITY;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::LIGHT)!=0) {
    if (appendField(ss, first, "light", Drivhus::getSettings()->getLight(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::LIGHT;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::VOLT)!=0) {
    if (appendField(ss, first, "volt", Drivhus::getSettings()->getVolt(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::VOLT;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::WATER_LOW_TRIGGER)!=0) {
    if (appendField(ss, first, "water_low", Drivhus::getSettings()->getWaterLowTrigger())) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::WATER_LOW_TRIGGER;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::WATER_HIGH_TRIGGER)!=0) {
    if (appendField(ss, first, "water_high", Drivhus::getSettings()->getWaterHighTrigger(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::WATER_HIGH_TRIGGER;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::WATER_VALVE)!=0) {
    if (appendField(ss, first, "water_valve", static_cast<int>(Drivhus::getSettings()->getWaterValveStatus()))) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::WATER_VALVE;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::SUNRISE)!=0) {
    if (appendField(ss, first, "sunrise", Drivhus::getSettings()->getSunrise(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::SUNRISE;}
  }
  if ((m_fields_changed & 1<<Drivhus::OnValueChangeListener::Type::SUNSET)!=0) {
    if (appendField(ss, first, "sunset", Drivhus::getSettings()->getSunset(), 2)) {fields_pushed |= 1<<Drivhus::OnValueChangeListener::Type::SUNSET;}
  }

  //Ignore any other fields
  fields_pushed |= ~(1<<Drivhus::OnValueChangeListener::Type::INDOOR_TEMP|
                     1<<Drivhus::OnValueChangeListener::Type::INDOOR_HUMIDITY|
                     1<<Drivhus::OnValueChangeListener::Type::OUTDOOR_TEMP|
                     1<<Drivhus::OnValueChangeListener::Type::OUTDOOR_HUMIDITY|
                     1<<Drivhus::OnValueChangeListener::Type::LIGHT|
                     1<<Drivhus::OnValueChangeListener::Type::VOLT|
                     1<<Drivhus::OnValueChangeListener::Type::WATER_LOW_TRIGGER|
                     1<<Drivhus::OnValueChangeListener::Type::WATER_HIGH_TRIGGER|
                     1<<Drivhus::OnValueChangeListener::Type::WATER_VALVE|
                     1<<Drivhus::OnValueChangeListener::Type::SUNRISE|
                     1<<Drivhus::OnValueChangeListener::Type::SUNSET);

  ss << "}\n";

  std::string mqtt_packet = ss.str();
  if (mqtt_packet.length() > m_mqtt_client.getBufferSize()) {
    if (!m_mqtt_client.setBufferSize(mqtt_packet.length())) {
      Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("Growing MQTT buffer to ")+std::to_string(mqtt_packet.length())+" bytes failed.");
    }
  }

  Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("Publishing ")+mqtt_packet+" to "+Drivhus::getSettings()->getMQTTServerId()+VALUES_TOPIC);
  if (m_mqtt_client.publish((Drivhus::getSettings()->getMQTTServerId()+VALUES_TOPIC).c_str(), mqtt_packet.c_str(), true)) {
    m_fields_changed &= ~fields_pushed;
    m_plants_changed &= ~plants_pushed;
  } else {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_ERROR, std::string("Publishing MQTT packet failed with error: ")+errorStateMessage());
  }

  if (m_fields_changed==0 && m_plants_changed==0) {
    m_cached_packet_time = 0L;
    m_max_cache_time = 0L;
  } else {
    m_cached_packet_time = millis();
    m_max_cache_time = 500L;
  }
}

bool Drivhus::MQTT::appendField(std::stringstream& stream, bool& first, std::string&& field, float value, uint8_t precision) {
  if (stream.str().length()+field.length()+10 > BUFFER_SIZE) {
    return false;
  }

  if (!first) {
    stream << ",";
  }
  first = false;
  stream << "\"" << field << "\":" << std::fixed << std::setprecision(precision) << value << "\n";
  return true;
}

bool Drivhus::MQTT::appendField(std::stringstream& stream, bool& first, std::string&& field, int value) {
  if (stream.str().length()+field.length()+10 > BUFFER_SIZE) {
    return false;
  }

  if (!first) {
    stream << ",";
  }
  first = false;
  stream << "\"" << field << "\":" << value << "\n";
  return true;
}
