#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#ifdef TESTING
# include "testing.h"
#else
# include <EEPROM.h>
#endif

#include <mutex>
#include <string>
#include <vector>

#include "global.h"
#include "component.h"
#include "plant.h"
#include "volt.h"


namespace Drivhus {

class OnValueChangeListener {
public:
  enum Type {
    PLANT_MOISTURE=0,
    //Reserve room for all plants here
    INDOOR_TEMP=Drivhus::MAX_PLANT_COUNT,
    INDOOR_HUMIDITY=Drivhus::MAX_PLANT_COUNT+1,
    OUTDOOR_TEMP=Drivhus::MAX_PLANT_COUNT+2,
    OUTDOOR_HUMIDITY=Drivhus::MAX_PLANT_COUNT+3,
    LIGHT=Drivhus::MAX_PLANT_COUNT+4,
    VOLT=Drivhus::MAX_PLANT_COUNT+5,
    SUNRISE=Drivhus::MAX_PLANT_COUNT+6,
    SUNSET=Drivhus::MAX_PLANT_COUNT+7
  };

  virtual void onValueChanged(OnValueChangeListener::Type /*type*/, uint8_t /*plant_id*/) {}
};

class OnConfigChangeListener {
public:
  enum Type {
    MS_BETWEEN_READING,
    FAN_ACTIVATE_TEMP,
    FAN_ACTIVATE_HUMIDITY,
    PLANT_REQUEST_WATERING,
    PLANT_IN_WATERING_CYCLE,
    PLANT_ENABLED,
    PLANT_WET_VALUE,
    PLANT_DRY_VALUE,
    PLANT_WATERING_DURATION,
    PLANT_WATERING_GRACE_VALUE,
    PLANT_WATERING_STARTED,
    PLANT_WATERING_ENDED
  };

  virtual void onConfigChanged(OnConfigChangeListener::Type /*type*/, uint8_t /*plant_id*/) {}
};

class Settings : public Component
{
public:
  static constexpr uint8_t  EEPROM_INITIALIZED_MARKER = 0xF1; //Just a magic number. CHange when EEPROM data format is incompatibly changed
  static constexpr const char* SETUP_SSID = "drivhus-setup";
  static constexpr const char*  DEFAULT_SERVERID = "/Drivhus/1/";
  static constexpr unsigned long SETUP_PIN_POLL_INTERVAL_MS = 2000; //If system has been disconnected this long, switch to AccessPoint mode

  static constexpr uint8_t MAX_SSID_LENGTH            = 32;
  static constexpr uint8_t MAX_SSID_PASSWORD_LENGTH   = 64;
  static constexpr uint8_t MAX_MQTT_SERVERNAME_LENGTH = 64;
  static constexpr uint8_t MAX_MQTT_SERVERPORT_LENGTH =  5; //5 digits, actually "0"-"65535"
  static constexpr uint8_t MAX_MQTT_SERVERID_LENGTH   = 32;
  static constexpr uint8_t MAX_MQTT_USERNAME_LENGTH   = 32;
  static constexpr uint8_t MAX_MQTT_PASSWORD_LENGTH   = 32;
  static constexpr uint8_t VOLT_MULTIPLIER_LENGTH     =  5; //5 digits, actually "0"-"65535". float value is found by dividing by 256 (so 8bit.8bit)
  static constexpr uint8_t MAX_TIMEZONE_LENGTH        =  3;
  static constexpr uint8_t MAX_EMULATE_LATLONG_LENGTH =  3;

public:
  Settings(uint8_t pin);
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "Settings";}

  [[nodiscard]] bool isInSetupMode(bool force_read = false);

  void addValueChangeListener(OnValueChangeListener* listener);
  void addConfigChangeListener(OnConfigChangeListener* listener);
private:
  void notifyValueChangeListeners(OnValueChangeListener::Type type, uint8_t plant_id=0);
  void notifyConfigChangeListeners(OnConfigChangeListener::Type type, uint8_t plant_id=0);

public:
  [[nodiscard]] const std::string& getSSID() const {return m_ssid_param;}
  [[nodiscard]] const std::string& getSSIDPassword() const {return m_ssid_password_param;}
  [[nodiscard]] const std::string& getMQTTServername() const {return m_mqtt_servername_param;}
  [[nodiscard]] uint16_t getMQTTPort() const {return m_mqtt_serverport_param;}
  [[nodiscard]] const std::string& getMQTTServerId() const {return m_mqtt_serverid_param;}
  [[nodiscard]] const std::string& getMQTTUsername() const {return m_mqtt_username_param;}
  [[nodiscard]] const std::string& getMQTTPassword() const {return m_mqtt_password_param;}
  [[nodiscard]] float getVoltMultiplier() const {return m_volt_multiplier_param;}
  [[nodiscard]] const std::string& getTimezone() const {return m_timezone_param;}
  [[nodiscard]] int8_t getEmulateLatitude() const {return m_emulate_latitude_param;}
  [[nodiscard]] int16_t getEmulateLongitude() const {return m_emulate_longitude_param;}

  [[nodiscard]] bool hasMQTTServer() const {return !m_mqtt_servername_param.empty();}

  void setSSID(const std::string& value) {if (isInSetupMode() && m_ssid_param!=value){m_ssid_param=value; m_settings_changed=true;}}
  void setSSIDPassword(const std::string& value) {if (isInSetupMode() && m_ssid_password_param!=value){m_ssid_password_param=value; m_settings_changed=true;}}
  void setMQTTServername(const std::string& value) {if (isInSetupMode() && m_mqtt_servername_param!=value){m_mqtt_servername_param=value; m_settings_changed=true;}}
  void setMQTTPort(uint16_t value) {if (isInSetupMode() && m_mqtt_serverport_param!=value){m_mqtt_serverport_param=value; m_settings_changed=true;}}
  void setMQTTServerId(const std::string& value) {if (isInSetupMode() && m_mqtt_serverid_param!=value){m_mqtt_serverid_param=value; m_settings_changed=true;}}
  void setMQTTUsername(const std::string& value) {if (isInSetupMode() && m_mqtt_username_param!=value){m_mqtt_username_param=value; m_settings_changed=true;}}
  void setMQTTPassword(const std::string& value) {if (isInSetupMode() && m_mqtt_password_param!=value){m_mqtt_password_param=value; m_settings_changed=true;}}
  void setVoltMultiplier(float value) {if (isInSetupMode() && value>=0.0f && value<=Volt::MAX_VOLT && m_volt_multiplier_param!=value){m_volt_multiplier_param=value; m_settings_changed=true;}}
  void setTimezone(const std::string& value) {if (isInSetupMode() && m_timezone_param!=value){m_timezone_param=value; m_settings_changed=true;}}
  void setEmulateLatitude(int8_t value) {if (isInSetupMode() && value>=-90 && value<=90 && m_emulate_latitude_param!=value){m_emulate_latitude_param=value; m_settings_changed=true;}}
  void setEmulateLongitude(int16_t value) {if (isInSetupMode() && value>=-180 && value<=180 && m_emulate_longitude_param!=value){m_emulate_longitude_param=value; m_settings_changed=true;}}
  void setShouldFlushSettings();

private:
  void readPersistentString(std::string& s, int max_length, int& adr);
  void readPersistentByte(uint8_t& b, int& adr);
  void readPersistentParams();
  void writePersistentString(const std::string& s, size_t max_length, int& adr);
  void writePersistentByte(uint8_t b, int& adr);
  [[nodiscard]] bool writePersistentParams();

  [[nodiscard]] size_t utf8ByteArrayLength(const std::string& s, size_t max_length);

  void checkIfSettingsShouldBeFlushed();
  void flushSettings();

public:
  void setPlantMoisture(uint8_t plant_id, float value) {if (Drivhus::isValidPlantId(plant_id)) {
                                                          m_plants[plant_id-1].current_value=value;
                                                          notifyValueChangeListeners(OnValueChangeListener::Type::PLANT_MOISTURE, plant_id);
                                                        }}
  void setIndoorTemp(float value) {m_indoor_temp=value; notifyValueChangeListeners(OnValueChangeListener::Type::INDOOR_TEMP);}
  void setIndoorHumidity(float value) {m_indoor_humidity=value; notifyValueChangeListeners(OnValueChangeListener::Type::INDOOR_HUMIDITY);}
  void setOutdoorTemp(float value) {m_outdoor_temp=value; notifyValueChangeListeners(OnValueChangeListener::Type::OUTDOOR_TEMP);}
  void setOutdoorHumidity(float value) {m_outdoor_humidity=value; notifyValueChangeListeners(OnValueChangeListener::Type::OUTDOOR_HUMIDITY);}
  void setLight(float value) {m_light=value; notifyValueChangeListeners(OnValueChangeListener::Type::LIGHT);}
  void setVolt(float value) {m_volt=value; notifyValueChangeListeners(OnValueChangeListener::Type::VOLT);}
  void setSunrise(float value) {m_sunrise=value; Serial.println("Setting sunrise"); notifyValueChangeListeners(OnValueChangeListener::Type::SUNRISE);}
  void setSunset(float value) {m_sunset=value; notifyValueChangeListeners(OnValueChangeListener::Type::SUNSET);}
  [[nodiscard]] float getPlantMoisture(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id) && getEnabled(plant_id) ? m_plants[plant_id-1].current_value : 0.0f;}
  [[nodiscard]] float getIndoorTemp() const {return m_indoor_temp;}
  [[nodiscard]] float getIndoorHumidity() const {return m_indoor_humidity;}
  [[nodiscard]] float getOutdoorTemp() const {return m_outdoor_temp;}
  [[nodiscard]] float getOutdoorHumidity() const {return m_outdoor_humidity;}
  [[nodiscard]] float getLight() const {return m_light;}
  [[nodiscard]] float getVolt() const {return m_volt;}
  [[nodiscard]] float getSunrise() const {return m_sunrise;}
  [[nodiscard]] float getSunset() const {return m_sunset;}

  void setMsBetweenReading(unsigned long value) {m_ms_between_reading=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::MS_BETWEEN_READING);}
  void setFanActivateTemp(float value) {m_fan_activate_temp_value=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::FAN_ACTIVATE_TEMP);}
  void setFanActivateHumidity(float value) {m_fan_activate_humid_value=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::FAN_ACTIVATE_HUMIDITY);}
  void setRequestWatering(uint8_t plant_id) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].watering_requested=true; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_REQUEST_WATERING, plant_id);}}
  void setIsInWateringCycle(uint8_t plant_id, bool value) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].in_watering_cycle=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_IN_WATERING_CYCLE, plant_id);}}
  void setEnabled(uint8_t plant_id, bool value) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].enabled=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_ENABLED, plant_id);}}
  void setWetValue(uint8_t plant_id, float value) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].dry_value=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_WET_VALUE, plant_id);}}
  void setDryValue(uint8_t plant_id, float value) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].dry_value=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_DRY_VALUE, plant_id);}}
  void setWateringDuration(uint8_t plant_id, unsigned long value_ms) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].watering_duration_ms=value_ms; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_WATERING_DURATION, plant_id);}}
  void setWateringGracePeriod(uint8_t plant_id, unsigned long value_ms) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].watering_grace_period_ms=value_ms; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_WATERING_GRACE_VALUE, plant_id);}}
  void setWateringStarted(uint8_t plant_id) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].previous_watering_time=millis(); notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_WATERING_STARTED, plant_id);}}
  void setWateringEnded(uint8_t plant_id) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].previous_watering_time=millis(); notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_WATERING_ENDED, plant_id);}}
  void setForceUpdateWateringTime(uint8_t plant_id, unsigned long value) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].previous_watering_time=value;}}
  [[nodiscard]] unsigned long getMsBetweenReading() const {return m_ms_between_reading;}
  [[nodiscard]] float getFanActivateTemp() const {return m_fan_activate_temp_value;}
  [[nodiscard]] float getFanActivateHumidity() const {return m_fan_activate_humid_value;}
  [[nodiscard]] bool getRequestWatering(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id) && getEnabled(plant_id) ? m_plants[plant_id-1].watering_requested : false;}
  [[nodiscard]] bool getIsInWateringCycle(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id) && getEnabled(plant_id) ? m_plants[plant_id-1].in_watering_cycle : false;}
  [[nodiscard]] bool getEnabled(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].enabled:false;}
  [[nodiscard]] float getWetValue(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].wet_value:0.0f;}
  [[nodiscard]] float getDryValue(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].dry_value:0.0f;}
  [[nodiscard]] unsigned long getWateringDuration(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].watering_duration_ms:0L;}
  [[nodiscard]] unsigned long getWateringGracePeriod(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].watering_grace_period_ms:0L;}
  [[nodiscard]] unsigned long getPreviousWateringTime(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].previous_watering_time:0L;}

private:
  uint8_t m_pin;

public:
  std::vector<OnValueChangeListener*> m_value_change_listeners;
  std::vector<OnConfigChangeListener*> m_config_change_listeners;

  std::string m_ssid_param;
  std::string m_ssid_password_param;
  std::string m_mqtt_servername_param;
  uint16_t m_mqtt_serverport_param;
  std::string m_mqtt_serverid_param;
  std::string m_mqtt_username_param;
  std::string m_mqtt_password_param;
  float m_volt_multiplier_param;
  std::string m_timezone_param;
  int8_t m_emulate_latitude_param; //-90 (South Pole) to 90 (North Pole)
  int16_t m_emulate_longitude_param; //-180 (West) to 180 (East)

  bool m_settings_changed;

  unsigned long m_previous_setup_pin_poll_time;
  bool m_in_setup_mode;

  bool m_should_flush_settings;
  std::recursive_mutex m_should_flush_settings_mutex;

  Plant m_plants[Drivhus::MAX_PLANT_COUNT];
  unsigned long m_ms_between_reading;
  float m_fan_activate_temp_value;
  float m_fan_activate_humid_value;

  float m_indoor_temp;
  float m_indoor_humidity;
  float m_outdoor_temp;
  float m_outdoor_humidity;
  float m_light;
  float m_volt;
  float m_sunrise;
  float m_sunset;
};

} //namespace

#endif // _SETTINGS_H_
