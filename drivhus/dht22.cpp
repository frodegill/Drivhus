#include "dht22.h"

#include "global.h"
#include "network.h"
#include "webserver.h"


Drivhus::DHT22::DHT22(uint8_t id, uint8_t pin)
: m_id(id),
  m_pin(pin),
  m_previous_sampling_time(0L),
  m_is_present(false),
  m_temperature(0.0f),
  m_humidity(0.0f) {
}

bool Drivhus::DHT22::init() {
  pinMode(m_pin, OUTPUT); //DHT handles this pin itself, but it should be OUTPUT before setup
  m_dht.setup(m_pin, DHTesp::AM2302);
  return true;
}

void Drivhus::DHT22::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_sampling_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_sampling_time = current_time;
  }

  if ((m_previous_sampling_time+m_dht.getMinimumSamplingPeriod())<current_time) {
    m_previous_sampling_time = current_time;
    TempAndHumidity temp_and_humidity = m_dht.getTempAndHumidity();
    if (m_dht.getStatus() == DHTesp::ERROR_NONE) {
      m_is_present = true;
      m_temperature = temp_and_humidity.temperature;
      m_humidity = temp_and_humidity.humidity;
      Drivhus::getNetwork()->getWebServer()->updateTempHumid(m_id, m_temperature, m_humidity);
    } else {
      m_is_present = false;
    }
  }
}
