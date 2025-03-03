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
    INDOOR_TEMP,
    INDOOR_HUMIDITY,
    OUTDOOR_TEMP,
    OUTDOOR_HUMIDITY,
    LIGHT,
    VOLT,
    WATER_LOW_TRIGGER,
    WATER_HIGH_TRIGGER,
    WATER_VALVE,
    FAN,
    SUNRISE,
    SUNSET,
    //Types dependent on plant id:
    PLANT_MOISTURE,
    PLANT_IN_WATERING_CYCLE,
    PLANT_WATERING_STARTED,
    PLANT_WATERING_ENDED
  };

  virtual void onValueChanged(OnValueChangeListener::Type /*type*/, uint8_t /*plant_id*/) {}
};

class OnConfigChangeListener {
public:
  enum Type {
    SETUP_MODE,
    MS_BETWEEN_READING,
    FAN_ACTIVATE_TEMP,
    FAN_ACTIVATE_HUMIDITY,
    PLANT_REQUEST_WATERING,
    PLANT_ENABLED,
    PLANT_WET_VALUE,
    PLANT_DRY_VALUE,
    PLANT_WATERING_DURATION,
    PLANT_WATERING_GRACE_VALUE
  };

  virtual void onConfigChanged(OnConfigChangeListener::Type /*type*/, uint8_t /*plant_id/sensor_id*/) {}
};

enum ValveStatus {
  OPEN,
  CLOSED,
  NO_WATER
};


class Settings : public Component
{
private:
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
  [[nodiscard]] virtual bool postInit() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "Settings";}

  void addValueChangeListener(OnValueChangeListener* listener);
  void addConfigChangeListener(OnConfigChangeListener* listener);
private:
  void notifyValueChangeListeners(OnValueChangeListener::Type type, uint8_t plant_id=0);
  void notifyConfigChangeListeners(OnConfigChangeListener::Type type, uint8_t plant_id=0);

public:
  [[nodiscard]] bool getIsSystemReady() const {return m_system_ready;}

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

  void setSSID(const std::string& value) {if (getIsInSetupMode() && m_ssid_param!=value){m_ssid_param=value; m_settings_changed=true;}}
  void setSSIDPassword(const std::string& value) {if (getIsInSetupMode() && m_ssid_password_param!=value){m_ssid_password_param=value; m_settings_changed=true;}}
  void setMQTTServername(const std::string& value) {if (getIsInSetupMode() && m_mqtt_servername_param!=value){m_mqtt_servername_param=value; m_settings_changed=true;}}
  void setMQTTPort(uint16_t value) {if (getIsInSetupMode() && m_mqtt_serverport_param!=value){m_mqtt_serverport_param=value; m_settings_changed=true;}}
  void setMQTTServerId(const std::string& value) {if (getIsInSetupMode() && m_mqtt_serverid_param!=value){m_mqtt_serverid_param=value; m_settings_changed=true;}}
  void setMQTTUsername(const std::string& value) {if (getIsInSetupMode() && m_mqtt_username_param!=value){m_mqtt_username_param=value; m_settings_changed=true;}}
  void setMQTTPassword(const std::string& value) {if (getIsInSetupMode() && m_mqtt_password_param!=value){m_mqtt_password_param=value; m_settings_changed=true;}}
  void setVoltMultiplier(float value) {if (getIsInSetupMode() && value>=0.0f && value<=Volt::MAX_VOLT && m_volt_multiplier_param!=value){m_volt_multiplier_param=value; m_settings_changed=true;}}
  void setTimezone(const std::string& value) {if (getIsInSetupMode() && m_timezone_param!=value){m_timezone_param=value; m_settings_changed=true;}}
  void setEmulateLatitude(int8_t value) {if (getIsInSetupMode() && value>=-90 && value<=90 && m_emulate_latitude_param!=value){m_emulate_latitude_param=value; m_settings_changed=true;}}
  void setEmulateLongitude(int16_t value) {if (getIsInSetupMode() && value>=-180 && value<=180 && m_emulate_longitude_param!=value){m_emulate_longitude_param=value; m_settings_changed=true;}}
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
  void setPlantMoisture(uint8_t plant_id, float value);
  void setIsInWateringCycle(uint8_t plant_id, bool value) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].in_watering_cycle=value; notifyValueChangeListeners(OnValueChangeListener::Type::PLANT_IN_WATERING_CYCLE, plant_id);}}
  void setForceUpdateWateringTimeMs(uint8_t plant_id, unsigned long value) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].previous_watering_time=value;}}
  void setIndoorTemp(float value) {if (m_indoor_temp!=value) {m_indoor_temp=value; calculateOutdoorHumidityIndoor(); notifyValueChangeListeners(OnValueChangeListener::Type::INDOOR_TEMP);}}
  void setIndoorHumidity(float value) {if (m_indoor_humidity!=value) {m_indoor_humidity=value; calculateOutdoorHumidityIndoor(); notifyValueChangeListeners(OnValueChangeListener::Type::INDOOR_HUMIDITY);}}
  void setOutdoorTemp(float value) {if (m_outdoor_temp!=value) {m_outdoor_temp=value; calculateOutdoorHumidityIndoor(); notifyValueChangeListeners(OnValueChangeListener::Type::OUTDOOR_TEMP);}}
  void setOutdoorHumidity(float value) {if (m_outdoor_humidity!=value) {m_outdoor_humidity=value; calculateOutdoorHumidityIndoor(); notifyValueChangeListeners(OnValueChangeListener::Type::OUTDOOR_HUMIDITY);}}
  void setLight(float value) {if (m_light!=value) {m_light=value; notifyValueChangeListeners(OnValueChangeListener::Type::LIGHT);}}
  void setVolt(float value) {if (m_volt!=value) {m_volt=value; notifyValueChangeListeners(OnValueChangeListener::Type::VOLT);}}
  void setWaterLowTrigger(int value) {if (m_water_low_trigger!=value) {m_water_low_trigger=value; notifyValueChangeListeners(OnValueChangeListener::Type::WATER_LOW_TRIGGER);}}
  void setWaterHighTrigger(int value) {if (m_water_high_trigger!=value) {m_water_high_trigger=value; notifyValueChangeListeners(OnValueChangeListener::Type::WATER_HIGH_TRIGGER);}}
  void setWaterValveStatus(ValveStatus value) {if (m_water_valve_status!=value) {m_water_valve_status=value; notifyValueChangeListeners(OnValueChangeListener::Type::WATER_VALVE);}}
  void setFanActive(bool value) {if (m_fan_active!=value) {m_fan_active=value; notifyValueChangeListeners(OnValueChangeListener::Type::FAN);}}
  void setSunrise(float value) {if (m_sunrise!=value) {m_sunrise=value; notifyValueChangeListeners(OnValueChangeListener::Type::SUNRISE);}}
  void setSunset(float value) {if (m_sunset!=value) {m_sunset=value; notifyValueChangeListeners(OnValueChangeListener::Type::SUNSET);}}
  void setWateringStarted(uint8_t plant_id);
  void setWateringEnded(uint8_t plant_id) {if (Drivhus::isValidPlantId(plant_id)) {m_plants[plant_id-1].previous_watering_time=millis(); notifyValueChangeListeners(OnValueChangeListener::Type::PLANT_WATERING_ENDED, plant_id);}}
  [[nodiscard]] float getPlantMoisture(uint8_t plant_id) const {return getEnabled(plant_id) ? m_plants[plant_id-1].current_value : 0.0f;}
  [[nodiscard]] bool getIsInWateringCycle(uint8_t plant_id) const {return getEnabled(plant_id) ? m_plants[plant_id-1].in_watering_cycle : false;}
  [[nodiscard]] bool getIsWateringPlant(uint8_t plant_id) const;
  [[nodiscard]] unsigned long getPreviousWateringTimeMs(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].previous_watering_time:0L;}
  [[nodiscard]] float getIndoorTemp() const {return m_indoor_temp;}
  [[nodiscard]] float getIndoorHumidity() const {return m_indoor_humidity;}
  [[nodiscard]] float getOutdoorTemp() const {return m_outdoor_temp;}
  [[nodiscard]] float getOutdoorHumidity() const {return m_outdoor_humidity;}
  [[nodiscard]] float getOutdoorAsIndoorHumidity() const {return m_outdoor_as_indoor_humidity;}
  [[nodiscard]] float getLight() const {return m_light;}
  [[nodiscard]] float getVolt() const {return m_volt;}
  [[nodiscard]] int   getWaterLowTrigger() const {return m_water_low_trigger;}
  [[nodiscard]] int   getWaterHighTrigger() const {return m_water_high_trigger;}
  [[nodiscard]] ValveStatus getWaterValveStatus() const {return m_water_valve_status;}
  [[nodiscard]] bool getFanActive() const {return m_fan_active;}
  [[nodiscard]] float getSunrise() const {return m_sunrise;}
  [[nodiscard]] float getSunset() const {return m_sunset;}
private:
  void calculateOutdoorHumidityIndoor();

private:
  void setIsInSetupMode(bool value) {if (m_in_setup_mode!=value) {m_in_setup_mode=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::SETUP_MODE);}}
public:
  void setMsBetweenReading(unsigned long value) {if (m_ms_between_reading!=value) {m_ms_between_reading=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::MS_BETWEEN_READING);}}
  void setFanActivateTemp(float value) {if (m_fan_activate_temp_value!=value) {m_fan_activate_temp_value=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::FAN_ACTIVATE_TEMP);}}
  void setFanActivateHumidity(float value) {if (m_fan_activate_humid_value!=value) {m_fan_activate_humid_value=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::FAN_ACTIVATE_HUMIDITY);}}
  void setRequestWatering(uint8_t plant_id) {if (Drivhus::isValidPlantId(plant_id) && !m_plants[plant_id-1].watering_requested) {m_plants[plant_id-1].watering_requested=true; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_REQUEST_WATERING, plant_id);}}
  void setEnabled(uint8_t plant_id, bool value);
  void setWetValue(uint8_t plant_id, float value) {if (Drivhus::isValidPlantId(plant_id) && m_plants[plant_id-1].wet_value!=value) {m_plants[plant_id-1].wet_value=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_WET_VALUE, plant_id);}}
  void setDryValue(uint8_t plant_id, float value) {if (Drivhus::isValidPlantId(plant_id) && m_plants[plant_id-1].dry_value!=value) {m_plants[plant_id-1].dry_value=value; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_DRY_VALUE, plant_id);}}
  void setWateringDuration(uint8_t plant_id, unsigned long value_ms) {if (Drivhus::isValidPlantId(plant_id) && m_plants[plant_id-1].watering_duration_ms!=value_ms) {m_plants[plant_id-1].watering_duration_ms=value_ms; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_WATERING_DURATION, plant_id);}}
  void setWateringGracePeriodMs(uint8_t plant_id, unsigned long value_ms) {if (Drivhus::isValidPlantId(plant_id) && m_plants[plant_id-1].watering_grace_period_ms!=value_ms) {m_plants[plant_id-1].watering_grace_period_ms=value_ms; notifyConfigChangeListeners(OnConfigChangeListener::Type::PLANT_WATERING_GRACE_VALUE, plant_id);}}
  [[nodiscard]] bool getIsInSetupMode();
  [[nodiscard]] unsigned long getMsBetweenReading() const {return m_ms_between_reading;}
  [[nodiscard]] float getFanActivateTemp() const {return m_fan_activate_temp_value;}
  [[nodiscard]] float getFanActivateHumidity() const {return m_fan_activate_humid_value;}
  [[nodiscard]] bool getRequestWatering(uint8_t plant_id) const {return m_plants[plant_id-1].watering_requested;} //Should be possible to force watering of disabled plants
  [[nodiscard]] bool getEnabled(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].enabled:false;}
  [[nodiscard]] float getWetValue(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].wet_value:0.0f;}
  [[nodiscard]] float getDryValue(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].dry_value:0.0f;}
  [[nodiscard]] unsigned long getWateringDuration(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].watering_duration_ms:0L;}
  [[nodiscard]] unsigned long getWateringGracePeriodMs(uint8_t plant_id) const {return Drivhus::isValidPlantId(plant_id)?m_plants[plant_id-1].watering_grace_period_ms:0L;}

private:
  uint8_t m_pin;

  bool m_system_ready;

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
  float m_outdoor_as_indoor_humidity;
  float m_light;
  float m_volt;
  int m_water_low_trigger;
  int m_water_high_trigger;
  ValveStatus m_water_valve_status;
  bool m_fan_active;
  float m_sunrise;
  float m_sunset;
};

} //namespace

#endif // _SETTINGS_H_
