#ifndef _NTP_H_
#define _NTP_H_

#include <Arduino.h>

#include <memory>
#include <WiFiUdp.h>
#include <Timezone.h>


class NTP
{
private:
  static constexpr const char* NTP_SERVERNAME = "0.no.pool.ntp.org";
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
  bool getLocalTime(time_t& local_time);

public:
  static time_t requestNTPUTCTime();
private:
  void sendNTPpacket();
  void handleNTPResponse();

private:
  uint8_t m_packet_buffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
  WiFiUDP m_udp;
  Timezone m_timezone;

  unsigned long m_previous_ntp_request_time;
};

#endif // _NTP_H_
