#include "network.h"

#include "global.h"
#include "settings.h"


Drivhus::Network::Network()
: m_wifi_disconnected_since(0L),
  m_wifi_accesspoint_mode_since(0L),
  m_is_in_accesspoint_mode(false) {
  m_ap_ip = std::make_shared<IPAddress>(192, 168, 4, 1);
  m_webserver = std::make_shared<WebServer>();
}

bool Drivhus::Network::init() {
  if (Drivhus::getSettings()->isInSetupMode()) {
    activateWiFiAccessPoint();
  } else {
    activateWiFiStation();
  }
  return m_webserver->init();
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
    if (Drivhus::getSettings()->isInSetupMode()) {
      Serial.println("Activating AP mode for Setup");
      activateWiFiAccessPoint();
    } else if (!isWiFiConnected()) {
      if (m_wifi_disconnected_since == 0L) {
        Serial.println("WiFi is in Disconnected state");
        m_wifi_disconnected_since = current_time;
      }

      //If WiFi fails too long, switch to AccessPoint mode
      if (Drivhus::getSettings()->getSSID().empty() || (m_wifi_disconnected_since+MAX_WIFI_RECOVERY_DURATION_MS)<current_time) {
        Serial.println("WiFi couldn't connect. Activating AP mode");
        activateWiFiAccessPoint();
      }
    } else {
      if (m_wifi_disconnected_since != 0L) {
        Serial.println("WiFi is in Connected state");
        m_wifi_disconnected_since = 0L;
      }
    }
  } else {
    if (!Drivhus::getSettings()->isInSetupMode() &&
        !Drivhus::getSettings()->getSSID().empty() &&
        m_webserver->wsClientCount()==0 &&
        (m_wifi_accesspoint_mode_since+MAX_AP_WITHOUT_CLIENTS_DURATION_MS)<current_time) {
      Serial.println("Switching back to normal WiFi mode");
      activateWiFiStation();
    }

    m_dns_server.processNextRequest(); //Route everything to 192.168.4.1
  }

  m_webserver->loop();
}

bool Drivhus::Network::isWiFiConnected() {
  return WiFi.isConnected();
}

void Drivhus::Network::activateWiFiStation() {
  if (Drivhus::getSettings()->getSSID().empty()) {
    Serial.println("No SSID configured for STA");
    activateWiFiAccessPoint();
  } else {
    Serial.println("activateWiFiStation");
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
  Serial.println("activateWiFiAccessPoint");
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
