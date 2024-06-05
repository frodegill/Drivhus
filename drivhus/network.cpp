#include "network.h"

#include "global.h"
#include "log.h"
#include "mqtt.h"
#include "settings.h"
#include "webserver.h"


Drivhus::Network::Network()
: Drivhus::Component(),
  m_wifi_disconnected_since(0L),
  m_wifi_accesspoint_mode_since(0L),
  m_is_in_accesspoint_mode(false) {
  m_ap_ip = std::make_shared<IPAddress>(192, 168, 4, 1);
}

bool Drivhus::Network::init() {
#if 0
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.persistent(false);
  WiFi.setSleep(true);
  WiFi.setAutoReconnect(true);
#endif
  if (Drivhus::getSettings()->getIsInSetupMode()) {
    activateWiFiAccessPoint();
  } else {
    activateWiFiStation();
  }
  return true;
}

void Drivhus::Network::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_wifi_disconnected_since) { //Time will wrap around every ~50 days. Don't consider this an error
    m_wifi_disconnected_since = current_time;
  }
  if (current_time < m_wifi_accesspoint_mode_since) { //Time will wrap around every ~50 days. Don't consider this an error
    m_wifi_accesspoint_mode_since = current_time;
  }

  if (!m_is_in_accesspoint_mode) {
    if (Drivhus::getSettings()->getIsInSetupMode()) {
      Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("Activating AP mode for Setup"));
      activateWiFiAccessPoint();
    } else if (!WiFi.isConnected()) {
      if (m_wifi_disconnected_since == 0L) {
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("WiFi is in Disconnected state"));
        m_wifi_disconnected_since = current_time;
      }

      //If WiFi fails too long, switch to AccessPoint mode
      if (Drivhus::getSettings()->getSSID().empty() || (m_wifi_disconnected_since+MAX_WIFI_RECOVERY_DURATION_MS)<current_time) {
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("WiFi couldn't connect. Activating AP mode"));
        activateWiFiAccessPoint();
      }
    } else {
      if (m_wifi_disconnected_since != 0L) {
        Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_INFO, std::string("WiFi is in Connected state. Got IP ")+WiFi.localIP().toString().c_str());
        m_wifi_disconnected_since = 0L;
        Drivhus::getMQTT()->requestMQTTConnection();
      }
    }
  } else {
    if (!Drivhus::getSettings()->getIsInSetupMode() &&
        !Drivhus::getSettings()->getSSID().empty() &&
        Drivhus::getWebServer()->wsClientCount()==0 &&
        (m_wifi_accesspoint_mode_since+MAX_AP_WITHOUT_CLIENTS_DURATION_MS)<current_time) {
      Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("Switching back to normal WiFi mode"));
      activateWiFiStation();
    }

    m_dns_server.processNextRequest(); //Route everything to 192.168.4.1
  }
}

void Drivhus::Network::activateWiFiStation() {
  if (Drivhus::getSettings()->getSSID().empty()) {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("No SSID configured for STA"));
    activateWiFiAccessPoint();
  } else {
    Drivhus::getLog()->print(Drivhus::Log::LogLevel::LEVEL_DEBUG, std::string("activateWiFiStation to ")+Drivhus::getSettings()->getSSID());
    deactivateWiFi();
    m_dns_server.stop();
    WiFi.enableAP(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(Drivhus::getSettings()->getSSID().c_str(), Drivhus::getSettings()->getSSIDPassword().c_str());
    m_wifi_accesspoint_mode_since = millis();
    m_is_in_accesspoint_mode = false;
  }
}

void Drivhus::Network::activateWiFiAccessPoint() {
  deactivateWiFi();
  WiFi.enableAP(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SETUP_SSID);
  m_dns_server.start(DNS_PORT, "*", *m_ap_ip.get());
  m_wifi_accesspoint_mode_since = millis();
  m_is_in_accesspoint_mode = true;
}

void Drivhus::Network::deactivateWiFi() {
  WiFi.disconnect();
  m_wifi_disconnected_since = millis();
}
