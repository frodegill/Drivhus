#include "ntp.h"

#ifdef TESTING
#else
# include <TimeLib.h>
#endif

#include <cstring>

#include "global.h"
#include "settings.h"


/* This class is based on NTP example code, but made non-blocking */

Drivhus::NTP::NTP(Timezone&& timezone)
: m_timezone(timezone),
  m_previous_ntp_request_time(0L),
  m_previous_calculated_date(0),
  m_sunrise(12*60.0f),
  m_sunset(12*60.0f) {
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

  struct tm* local_tm = getLocalTm();
  if (local_tm != nullptr) {
    uint32_t local_date=(1900+local_tm->tm_year)*10000 + (1+local_tm->tm_mon)*100 + local_tm->tm_mday;
    if (local_date != m_previous_calculated_date) {
      float latitude = Drivhus::getSettings()->getEmulateLatitude();
      float longitude = Drivhus::getSettings()->getEmulateLongitude();
      float eqtime, ha;
      calculateEqtimeMinutesAndHourAngleDegrees(latitude, longitude, eqtime, ha);
      m_sunrise = 720 - 4*(longitude + ha) - eqtime;
      m_sunset = 720 - 4*(longitude - ha) - eqtime;
      m_previous_calculated_date = local_date;
    }
  }
}

bool Drivhus::NTP::getLocalTime(time_t& local_time) {
  if (timeStatus() == timeNotSet)
    return false;

  local_time = m_timezone.toLocal(now());
  return true;
}

struct tm* Drivhus::NTP::getLocalTm() {
  time_t local_time;
  if (!getLocalTime(local_time)) {
    return nullptr;
  }
  return ::localtime(&local_time);
}

bool Drivhus::NTP::isLeapYear() {
  struct tm* local_tm = getLocalTm();
  return (local_tm == nullptr) ? false : isLeapYear(1900+local_tm->tm_year);
}

const Drivhus::TimezoneInfo* Drivhus::NTP::getTimezoneInfo(const std::string& timezone) {
  for (auto tz : g_timezones) {
    if (timezone.compare(tz.code)==0) {
      return &tz;
    }
  }
  return &g_timezones[0];
}

void Drivhus::NTP::calculateEqtimeMinutesAndHourAngleDegrees(float latitude, float longitude, float& eqtime, float& ha) {
  // https://gml.noaa.gov/grad/solcalc/solareqns.PDF

  struct tm* local_tm = getLocalTm();
  if (local_tm == nullptr) {
    eqtime = 0.0f;
    ha = -longitude;
    return;
  }

  int days_in_year = isLeapYear(1900+local_tm->tm_year) ? 366 : 365;
  float fractional_year_rad = (2*PI/days_in_year) * (local_tm->tm_yday - 1 + (local_tm->tm_hour-12.0f)/24.0f);
  
  float solar_declination_rad = 0.006918f -
                                0.399912f * ::cos(fractional_year_rad) +
                                0.070257f * ::sin(fractional_year_rad) -
                                0.006758f * ::cos(2*fractional_year_rad) +
                                0.000907f * ::sin(2*fractional_year_rad) -
                                0.002697f * ::cos(3*fractional_year_rad) +
                                0.001480f * ::sin (3*fractional_year_rad);

  eqtime = 229.18f * (0.000075f +
                      0.001868f * ::cos(fractional_year_rad) -
                      0.032077f * ::sin(fractional_year_rad) -
                      0.014615f * ::cos(2*fractional_year_rad) -
                      0.040849f * ::sin(2*fractional_year_rad) );

  ha = radToDeg(::acos(
                       (::cos(degToRad(90.833f)) / (::cos(degToRad(latitude))*::cos(solar_declination_rad)))
                       - ::tan(degToRad(latitude))*::tan(solar_declination_rad)
                      ));
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
