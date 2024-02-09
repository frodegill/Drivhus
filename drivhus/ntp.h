#ifndef _NTP_H_
#define _NTP_H_

#ifdef TESTING
# include "testing.h"
#else
# include <Arduino.h>
# include <WiFiUdp.h>
#endif

#include <memory>

#include "global.h"


namespace Drivhus {

class NTP
{
private:
  static constexpr const char* NTP_SERVERNAME = "0.pool.ntp.org";
  static constexpr uint8_t NTP_PORT = 123;
  static constexpr int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
  static constexpr unsigned int LOCAL_PORT = 8888;  // local port to listen for UDP packets
  static constexpr unsigned long NTP_SYNC_INTERVAL_SEC = 300;
  static constexpr unsigned long NTP_RESPONSE_TIMEOUT_MS = 1500;

public:
  NTP(Timezone&& timezone);
  [[nodiscard]] bool init();
  void loop();

public:
  [[nodiscard]] bool getLocalTime(time_t& local_time);
  [[nodiscard]] struct tm* getLocalTm();

private:
  [[nodiscard]] bool calculateSunriseSunset(float latitude, float longitude, float& sunrise, float& sunset);

public:
  static time_t requestNTPUTCTime();
private:
  void sendNTPpacket();
  void handleNTPResponse();

private:
  bool m_callbacks_are_set;
  uint8_t m_packet_buffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
  WiFiUDP m_udp;
  Timezone m_timezone;

  unsigned long m_previous_ntp_request_time;
  uint32_t m_previous_calculated_date;
};

} //namespace

#endif // _NTP_H_
