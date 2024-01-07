#include "ntp.h"

#ifdef TESTING
#else
# include <TimeLib.h>
#endif

#include <cstring>

#include "global.h"


/* This class is based on NTP example code, but made non-blocking */

Drivhus::NTP::NTP(Timezone&& timezone)
: m_timezone(timezone),
  m_previous_ntp_request_time(0L) {
}

bool Drivhus::NTP::init() {
  m_udp.begin(NTP::LOCAL_PORT);
  setSyncProvider(NTP::requestNTPUTCTime);
  setSyncInterval(NTP::NTP_SYNC_INTERVAL_SEC);
  return true;
}

void Drivhus::NTP::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_ntp_request_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_ntp_request_time = current_time;
    return;
  }

  if (m_previous_ntp_request_time != 0) {
    if ((m_previous_ntp_request_time+NTP_RESPONSE_TIMEOUT_MS)<current_time) {
      //Timeout
      m_previous_ntp_request_time = 0L;
    } else {
      handleNTPResponse();
    }
  }
}

bool Drivhus::NTP::getLocalTime(time_t& local_time) {
  if (timeStatus() == timeNotSet)
    return false;

  local_time = m_timezone.toLocal(now());
  return true;
}

time_t Drivhus::NTP::requestNTPUTCTime() {
  Drivhus::getNTP()->sendNTPpacket();
  return 0; // Handle response in loop() to avoid blocking
}

void Drivhus::NTP::sendNTPpacket() {
  while (m_udp.parsePacket() > 0) ; // discard any previously received packets

  std::memset(m_packet_buffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  m_packet_buffer[0] = 0xE3;  // LI, Version, Mode
  m_packet_buffer[1] = 0;     // Stratum, or type of clock
  m_packet_buffer[2] = 6;     // Polling Interval
  m_packet_buffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  m_packet_buffer[12]  = 49;
  m_packet_buffer[13]  = 0x4E;
  m_packet_buffer[14]  = 49;
  m_packet_buffer[15]  = 52;

  if (0!=m_udp.beginPacket(NTP_SERVERNAME, 123)) // NTP requests are to port 123. Returns 0 on error
  {
    m_udp.write(m_packet_buffer, NTP_PACKET_SIZE);
    m_udp.endPacket();
    m_previous_ntp_request_time = millis();
  }
}

void Drivhus::NTP::handleNTPResponse() {
  if (m_udp.parsePacket() >= NTP_PACKET_SIZE) {
    m_udp.read(m_packet_buffer, NTP_PACKET_SIZE);

    unsigned long secsSince1900;
    // convert four bytes starting at location 40 to a long integer
    secsSince1900 =  (unsigned long)m_packet_buffer[40] << 24;
    secsSince1900 |= (unsigned long)m_packet_buffer[41] << 16;
    secsSince1900 |= (unsigned long)m_packet_buffer[42] << 8;
    secsSince1900 |= (unsigned long)m_packet_buffer[43];

    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    
    setTime(secsSince1900 - seventyYears);
    m_previous_ntp_request_time = 0L;
  }
}
