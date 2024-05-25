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
  m_changed_fields(0) {
  Drivhus::getSettings()->addValueChangeListener(this);
}

bool Drivhus::MQTT::init() {
  m_mqtt_client.setClient(m_esp_client);
  m_mqtt_client.setServer(Drivhus::getSettings()->getMQTTServername().c_str(), 1883);
  m_mqtt_client.setCallback(globalMQTTCallback);
  Serial.println("MQTT initialised");
  return true;
}

void Drivhus::MQTT::loop() {
  if (!Drivhus::getSettings()->hasMQTTServer()) {
    Serial.println("No MQTT server");
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
    case SUNRISE:
    case SUNSET: return 1*1000;

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
  if (type==Drivhus::OnValueChangeListener::Type::PLANT_MOISTURE) {
    if (Drivhus::isValidPlantId(plant_id)) {
      m_changed_fields |= 1<<(Drivhus::OnValueChangeListener::Type::PLANT_MOISTURE + plant_id - 1);
    }
  } else {
    m_changed_fields |= 1<<type;
  }
  
  if (m_cached_packet_time == 0) {
    m_cached_packet_time = millis();
  }

  if (m_max_cache_time==0 || max_cache_time<m_max_cache_time) {
    m_max_cache_time = max_cache_time;
  }
}

void Drivhus::MQTT::globalMQTTCallback(char* topic, uint8_t* payload, unsigned int length)
{
  Drivhus::getMQTT()->callback(topic, payload, length);  
}

void Drivhus::MQTT::callback(char* topic, uint8_t* payload, unsigned int length)
{
  Serial.print("MQTT callback for ");
  Serial.println(topic);

  auto server_id = Drivhus::getSettings()->getMQTTServerId();
  if (0 != strncmp(server_id.c_str(), topic, server_id.length())) {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("Ignoring unrelated mqtt::callback ")+topic);
    return;
  }

  const std::string value(reinterpret_cast<const char*>(payload), length);
  Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("mqttCallback: ") + topic + " value " + value);

  const char* key = topic + server_id.length();
  if (0 == strcmp("config/sec_between_reading", key)) {
    unsigned long key_value = max(1, atoi(value.c_str()));
    Drivhus::getSettings()->setMsBetweenReading(key_value*1000);
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback conf_sec_between_reading=") + std::to_string(key_value));
  } else if (0 == strcmp("config/fan_activate_temp", key)) {
    float key_value = max(0.0, min(100.0, atof(value.c_str())));
    Drivhus::getSettings()->setFanActivateTemp(key_value);
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback conf_fan_activate_temp_value=") + std::to_string(key_value));
  } else if (0 == strcmp("config/fan_activate_humid", key)) {
    float key_value = max(0.0, min(100.0, atof(value.c_str())));
    Drivhus::getSettings()->setFanActivateHumidity(key_value);
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback conf_fan_activate_humid_value=") + std::to_string(key_value));
  } else if (0==strncmp("plant", key, 5) ||
             0==strncmp("config/plant", key, 12)) {
    key += (*key=='p') ? 5 : 12;
    uint8_t plant_id = 0;
    while (*key>='0' && *key<='9')
    {
      plant_id = plant_id*10 + (*key++-'0');
    }

    if (Drivhus::isValidPlantId(plant_id) && *key++=='/')
    {
      if (0 == strcmp("water_now", key)) {
        Drivhus::getSettings()->setRequestWatering(plant_id);
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback request water plant=") + std::to_string(plant_id));
      } else if (0 == strcmp("enabled", key)) {
        bool key_value = atoi(value.c_str())!=0;
        Drivhus::getSettings()->setEnabled(plant_id, key_value);
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback plant=")+std::to_string(plant_id)+" enabled="+(key_value?"true":"false"));
      } else if (0 == strcmp("dry_value", key)) {
        float key_value = max(0.0, min(100.0, atof(value.c_str())));
        Drivhus::getSettings()->setDryValue(plant_id, key_value);
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback plant=")+std::to_string(plant_id)+" conf_dry_value="+std::to_string(key_value));
      } else if (0 == strcmp("wet_value", key)) {
        float key_value = max(0.0, min(100.0, atof(value.c_str())));
        Drivhus::getSettings()->setWetValue(plant_id, key_value);
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback plant=")+std::to_string(plant_id)+" conf_wet_value="+std::to_string(key_value));
      } else if (0 == strcmp("watering_duration_ms", key)) {
        unsigned long key_value = max(1, atoi(value.c_str()));
        Drivhus::getSettings()->setWateringDuration(plant_id, key_value);
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback plant=")+std::to_string(plant_id)+" conf_watering_duration_ms="+std::to_string(key_value));
      } else if (0 == strcmp("watering_grace_period_sec", key)) {
        unsigned long key_value = max(1, atoi(value.c_str()));
        Drivhus::getSettings()->setWateringGracePeriod(plant_id, key_value*1000);
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("mqttCallback plant=")+std::to_string(plant_id)+" conf_watering_grace_period_sec="+std::to_string(key_value));
      }
    }
  }
}

void Drivhus::MQTT::requestMQTTConnection() {
  auto settings = Drivhus::getSettings();
  if (m_mqtt_client.connected() || //Already connected
      !settings->hasMQTTServer() || //Not supposed to connect
      m_reconnect_time!=0L) { //Has a pending connection attempt
    Serial.print("MQTT request not needed");
    return;
  }

  m_mqtt_client.connect(settings->getMQTTServerId().c_str(), settings->getMQTTUsername().c_str(), settings->getMQTTPassword().c_str());
  m_reconnect_time = millis();
}

void Drivhus::MQTT::log(const std::string& msg) {
  Serial.print("MQTT log");
  //TODO
}

void Drivhus::MQTT::subscribe() {
  if (!Drivhus::getNetwork()->isConnected()) {
    Serial.println("Network not connected. Delaying MQTT subscription");
    return;
  }

  Serial.print("MQTT: Subscribing to ");
  Serial.print(Drivhus::getSettings()->getMQTTServername().c_str());
  Serial.print(":");
  Serial.print(Drivhus::getSettings()->getMQTTPort());
  Serial.print(" topic ");
  Serial.println((Drivhus::getSettings()->getMQTTServerId()+"#").c_str());

  if (!m_mqtt_client.subscribe((Drivhus::getSettings()->getMQTTServerId()+"#").c_str())) {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_ERROR, (std::string("MQTT topics subscribed with error: ")+errorStateMessage()).c_str());
    m_reconnect_time = millis(); //To request a new subscribe
  }
}

void Drivhus::MQTT::publishPendingFields() {
  const unsigned long current_time = millis();
  if (m_cached_packet_time==0 ||
      m_changed_fields==0 ||
      (m_cached_packet_time+m_max_cache_time)>=current_time) {
    return;
  }

  if (!m_mqtt_client.connected()) {
    requestMQTTConnection();
    return;
  }

  std::stringstream ss;
  bool first = true;
  ss << std::fixed << "{\n";

  for (auto i=1; i<=Drivhus::MAX_PLANT_COUNT; i++) {
    if ((m_changed_fields & 1<<(i-1)) != 0) {
      appendField(ss, first, "plant"+std::to_string(i), Drivhus::getSettings()->getPlantMoisture(i), 2);
    }
  }
  if ((m_changed_fields & 1<<Drivhus::OnValueChangeListener::Type::INDOOR_TEMP)!=0) {appendField(ss, first, "itemp", Drivhus::getSettings()->getIndoorTemp(), 2);}
  if ((m_changed_fields & 1<<Drivhus::OnValueChangeListener::Type::INDOOR_HUMIDITY)!=0) {appendField(ss, first, "ihumid", Drivhus::getSettings()->getIndoorHumidity(), 2);}
  if ((m_changed_fields & 1<<Drivhus::OnValueChangeListener::Type::OUTDOOR_TEMP)!=0) {appendField(ss, first, "otemp", Drivhus::getSettings()->getOutdoorTemp(), 2);}
  if ((m_changed_fields & 1<<Drivhus::OnValueChangeListener::Type::OUTDOOR_HUMIDITY)!=0) {appendField(ss, first, "ohumid", Drivhus::getSettings()->getOutdoorHumidity(), 2);}
  if ((m_changed_fields & 1<<Drivhus::OnValueChangeListener::Type::LIGHT)!=0) {appendField(ss, first, "light", Drivhus::getSettings()->getLight(), 2);}
  if ((m_changed_fields & 1<<Drivhus::OnValueChangeListener::Type::VOLT)!=0) {appendField(ss, first, "volt", Drivhus::getSettings()->getVolt(), 2);}

  ss << "}\n";

  std::string mqtt_packet = ss.str();
  if (mqtt_packet.length() > m_mqtt_client.getBufferSize()) {
    if (!m_mqtt_client.setBufferSize(mqtt_packet.length())) {
      Serial.print("Growing MQTT buffer to ");
      Serial.print(mqtt_packet.length());
      Serial.println(" bytes failed.");
    }
  }

  Serial.print("Publishing ");
  Serial.println(mqtt_packet.c_str());
  if (!m_mqtt_client.publish("topic", mqtt_packet.c_str(), true)) {
    Serial.println("Publishing MQTT packet failed.");
  }

  m_cached_packet_time = 0L;
  m_max_cache_time = 0L;
  m_changed_fields = 0;
}

void Drivhus::MQTT::appendField(std::stringstream& stream, bool& first, std::string&& field, float value, uint8_t precision) {
  if (!first) {
    stream << ",";
  }
  first = false;
  stream << "\"" << field << "\":" << std::fixed << std::setprecision(precision) << value << "\n";
}
