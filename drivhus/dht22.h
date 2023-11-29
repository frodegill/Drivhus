#ifndef _DHT22_H_
#define _DHT22_H_

# include <DHTesp.h>           // Library: DHT_sensor_library_for_ESPx


class DHT22
{
public:
  DHT22(uint8_t id, uint8_t pin);
  [[nodiscard]] bool init();
  void loop();

  [[nodiscard]] bool isPresent() const {return m_is_present;}
  [[nodiscard]] float getTemperature() const {return m_temperature;}
  [[nodiscard]] float getHumidity() const {return m_humidity;}

private:
  uint8_t m_id;
  uint8_t m_pin;
  DHTesp m_dht;
  unsigned long m_previous_sampling_time;
  bool m_is_present;
  float m_temperature;
  float m_humidity;
};

#endif // _DHT22_H_
