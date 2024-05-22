#include "ntp.h"

#ifdef TESTING
#else
# include <TimeLib.h>
#endif

#include <cmath>
#include <cstring>

#include "global.h"
#include "network.h"
#include "settings.h"


/* This class is based on NTP example code, but made non-blocking */

Drivhus::NTP::NTP(Timezone&& timezone)
: Drivhus::Component(),
  m_callbacks_are_set(false),
  m_timezone(timezone),
  m_previous_ntp_request_time(0L),
  m_previous_calculated_date(0) {
}

bool Drivhus::NTP::init() {
  m_udp.begin(NTP::LOCAL_PORT);
  //Don't set up callbacks until network is available
  return true;
}

void Drivhus::NTP::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_previous_ntp_request_time) { //Time will wrap around every ~50 days. Don't consider this an error
    m_previous_ntp_request_time = current_time;
    return;
  }

  if (!m_callbacks_are_set) {
    if (Drivhus::getNetwork()->isConnected()) { //Set up NTP callbacks
      setSyncProvider(NTP::requestNTPUTCTime);
      setSyncInterval(NTP::NTP_SYNC_INTERVAL_SEC);
      m_callbacks_are_set = true;
    }
  } else {
    if (m_previous_ntp_request_time != 0) {
      if ((m_previous_ntp_request_time+NTP_RESPONSE_TIMEOUT_MS)<current_time) {
        //Timeout
        Serial.println("NTP request timed out");
        m_previous_ntp_request_time = 0L;
      } else {
        handleNTPResponse();
      }
    }

    struct tm* local_tm = getLocalTm();
    if (local_tm != nullptr) {
      uint32_t local_date=(1900+local_tm->tm_year)*10000 + (1+local_tm->tm_mon)*100 + local_tm->tm_mday;
      if (local_date != m_previous_calculated_date) {
        float sunrise, sunset;
        if (calculateSunriseSunset(Drivhus::getSettings()->getEmulateLatitude(),
                                  Drivhus::getSettings()->getEmulateLongitude(),
                                  sunrise,
                                  sunset)) {
          m_previous_calculated_date = local_date;
          Drivhus::getSettings()->setSunrise(sunrise);
          Drivhus::getSettings()->setSunset(sunset);
        }
      }
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

bool Drivhus::NTP::calculateSunriseSunset(float latitude, float longitude, float& sunrise, float& sunset) {
  // https://gml.noaa.gov/grad/solcalc/NOAA_Solar_Calculations_day.ods

  time_t now_utc = now();
  time_t now_local = m_timezone.toLocal(now_utc);
  double tz_minute_offset = static_cast<double>(now_local-now_utc)/60.0;
  struct tm* local_tm = getLocalTm();
  if (local_tm == nullptr) {
    return false;
  }
  int year = local_tm->tm_year+1900;
  int month = local_tm->tm_mon+1;
  int date = local_tm->tm_mday;
  double julian_day_number = (1461*(year + 4800 + (month - 14)/12.0))/4.0
                            + (367*(month - 2 - 12 * ((month - 14)/12.0)))/12.0
                            - (3*((year + 4900 + (month - 14)/12.0)/100.0))/4.0
                            + date
                            - 32074.5;
  double julian_century = (julian_day_number-2451545)/36525.0;
  double mean_obliq_elliptic_deg = 23+(26+((21.448-julian_century*(46.815+julian_century*(0.00059-julian_century*0.001813))))/60.0)/60.0;
  double obliq_corr_deg = mean_obliq_elliptic_deg+0.00256*::cos(degToRad(125.04-1934.136*julian_century));
  double y = ::tan(degToRad(obliq_corr_deg/2.0)) * ::tan(degToRad(obliq_corr_deg/2.0));
  double geom_mean_long_sun_deg = std::fmod(280.46646+julian_century*(36000.76983+julian_century*0.0003032), 360);
  double earth_eccent_orbit = 0.016708634 - julian_century*(0.000042037+0.0000001267*julian_century);
  double geom_mean_anom_sun_deg = 357.52911+julian_century*(35999.05029-0.0001537*julian_century);
  double eq_of_time_minutes = 4*radToDeg(y*::sin(2*degToRad(geom_mean_long_sun_deg))
                             -2*earth_eccent_orbit*::sin(degToRad(geom_mean_anom_sun_deg))
                             +4*earth_eccent_orbit*y*::sin(degToRad(geom_mean_anom_sun_deg))*::cos(2*degToRad(geom_mean_long_sun_deg))
                             -0.5*y*y*::sin(4*degToRad(geom_mean_long_sun_deg))
                             -1.25*earth_eccent_orbit*earth_eccent_orbit*::sin(2*degToRad(geom_mean_anom_sun_deg)));
  double solar_noon = (720 - 4*longitude - eq_of_time_minutes);
  double sun_eq_of_ctr = ::sin(degToRad(geom_mean_anom_sun_deg))*(1.914602-julian_century*(0.004817+0.000014*julian_century))
                        +::sin(degToRad(2*geom_mean_anom_sun_deg))*(0.019993-0.000101*julian_century)
                        +::sin(degToRad(3*geom_mean_anom_sun_deg))*0.000289;
  double sun_true_long_deg = geom_mean_long_sun_deg+sun_eq_of_ctr;
  double sun_app_long_deg = sun_true_long_deg-0.00569-0.00478*::sin(degToRad(125.04-1934.136*julian_century));
  double sun_decl_deg = radToDeg(::asin(::sin(degToRad(obliq_corr_deg))*::sin(degToRad(sun_app_long_deg))));
  double ha_sunrise_deg = radToDeg(::acos(::cos(degToRad(90.833)) / (::cos(degToRad(latitude))*::cos(degToRad(sun_decl_deg)))
                          - ::tan(degToRad(latitude))*::tan(degToRad(sun_decl_deg))));

  if (std::isnan(ha_sunrise_deg)) {
    if ((latitude>=0 && local_tm->tm_mon>=3 && local_tm->tm_mon<=8) ||
        (latitude<0 && (local_tm->tm_mon<3 || local_tm->tm_mon>8))) { //Midnight sun
      sunrise = 0;
      sunset = 23*60+59;
    } else {
      sunrise = sunset = 12*60;
    }
  } else {
    sunrise = static_cast<float>(solar_noon - ha_sunrise_deg*4 + tz_minute_offset);
    sunset = static_cast<float>(solar_noon + ha_sunrise_deg*4 + tz_minute_offset);
    if (sunrise<0.0) {sunrise+=1440.0f;}
    if (sunrise>=1440.0) {sunrise-=1440.0f;}
    if (sunset<0.0) {sunset+=1440.0f;}
    if (sunset>=1440.0) {sunset-=1440.0f;}
  }
  return true;
}

time_t Drivhus::NTP::requestNTPUTCTime() {
  Drivhus::getNTP()->sendNTPpacket();
  return 0; // Handle response in loop() to avoid blocking
}

void Drivhus::NTP::sendNTPpacket() {
  while (m_udp.parsePacket() > 0) ; // discard any previously received packets

  std::memset(m_packet_buffer, 0, NTP_PACKET_SIZE);
  // Initialise values needed to form NTP request
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
