#include "network.h"

#include "global.h"
#include "settings.h"


Network::Network()
: m_wifi_disconnected_since(0L),
  m_wifi_accesspoint_mode_since(0L),
  m_is_in_accesspoint_mode(false) {
  m_ap_ip = std::make_shared<IPAddress>(192, 168, 4, 1);
  m_webserver = std::make_shared<WebServer>();
}

bool Network::init() {
  if (::getSettings()->isInSetupMode()) {
    activateWiFiAccessPoint();
  } else {
    activateWiFiStation();
  }
  m_webserver->init();
  return true;
}

void Network::loop() {
  const unsigned long current_time = millis();
  if (current_time < m_wifi_disconnected_since) { //Time will wrap around every ~50 days. Don't consider this an error
    m_wifi_disconnected_since = current_time;
  }
  if (current_time < m_wifi_accesspoint_mode_since) { //Time will wrap around every ~50 days. Don't consider this an error
    m_wifi_accesspoint_mode_since = current_time;
  }

  if (!m_is_in_accesspoint_mode) {
    if (!isWiFiConnected()) {
      if (m_wifi_disconnected_since == 0L) {
        Serial.println("WiFi is in Disconnected state");
        m_wifi_disconnected_since = current_time;
      }

      //If WiFi fails too long, switch to AccessPoint mode
      if (::getSettings()->getSSID().empty() || (m_wifi_disconnected_since+MAX_WIFI_RECOVERY_DURATION_MS)<current_time) {
        Serial.println("WiFi couldn't connect. Activating AP mode");
        activateWiFiAccessPoint();
      } else if (m_is_in_accesspoint_mode && //If we have been in AP mode too long, and no clients use it, try to switch back to normal mode
                !::getSettings()->getSSID().empty() &&
                m_webserver->wsClientCount()==0 &&
                (m_wifi_accesspoint_mode_since+MAX_AP_WITHOUT_CLIENTS_DURATION_MS)<current_time) {
        Serial.println("Switching back to normal WiFi mode");
        activateWiFiStation();
      }
    } else {
      if (m_wifi_disconnected_since != 0L) {
        Serial.println("WiFi is in Connected state");
        m_wifi_disconnected_since = 0L;
      }
    }
  } else {
    if (!::getSettings()->getSSID().empty() &&
        m_webserver->wsClientCount()==0 &&
        (m_wifi_accesspoint_mode_since+MAX_AP_WITHOUT_CLIENTS_DURATION_MS)<current_time) {
      Serial.println("Switching back to normal WiFi mode");
      activateWiFiStation();
    }

    m_dns_server.processNextRequest(); //Route everything to 192.168.4.1
  }

  m_webserver->loop();
}

bool Network::isWiFiConnected() {
  return WiFi.isConnected();
}

void Network::activateWiFiStation() {
  if (::getSettings()->getSSID().empty()) {
    Serial.println("No SSID configured for STA");
    activateWiFiAccessPoint();
  } else {
    Serial.println("activateWiFiStation");
    deactivateWiFi();
    m_dns_server.stop();
    WiFi.enableAP(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(::getSettings()->getSSID().c_str(), ::getSettings()->getSSIDPassword().c_str());
    m_wifi_accesspoint_mode_since = millis();
    m_is_in_accesspoint_mode = false;
  }
}

void Network::activateWiFiAccessPoint() {
  Serial.println("activateWiFiAccessPoint");
  deactivateWiFi();
  WiFi.enableAP(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SETUP_SSID);
  m_dns_server.start(DNS_PORT, "*", *m_ap_ip.get());
  m_wifi_accesspoint_mode_since = millis();
  m_is_in_accesspoint_mode = true;
}

void Network::deactivateWiFi() {
  WiFi.disconnect();
  m_wifi_disconnected_since = millis();
}
