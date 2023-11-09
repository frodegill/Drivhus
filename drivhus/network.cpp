#include "network.h"

#include "global.h"
#include "settings.h"


Network::Network()
: m_wifi_disconnected_since(0L),
  m_is_in_accesspoint_mode(false) {
  m_ap_ip = std::make_shared<IPAddress>(192, 168, 4, 1);
  m_webserver = std::make_shared<WebServer>();
}

bool Network::init() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(::getSettings()->getSSID().c_str(), ::getSettings()->getSSIDPassword().c_str());
  m_webserver->init();
  return true;
}

bool Network::loop(const unsigned long& current_time) {
  if (current_time < m_wifi_disconnected_since) { //Time will wrap around every ~50 days. DOn't consider this an error
    m_wifi_disconnected_since = current_time;
  }

  if (!isWiFiConnected()) {
    if (m_wifi_disconnected_since == 0L) {
      Serial.println("WiFi got disconnected");
      m_wifi_disconnected_since = current_time;
    }

    //If WiFi fails too long, switch to AccessPoint mode
    if (!m_is_in_accesspoint_mode &&
        (::getSettings()->getSSID().empty() ||
         (m_wifi_disconnected_since+MAX_WIFI_RECOVERY_DURATION_MS)<current_time)) {
      Serial.println("WiFi couldn't connect. Activating AP mode");
      WiFi.disconnect();
      m_is_in_accesspoint_mode = true;
      m_wifi_disconnected_since = 0L;
      WiFi.softAP(SETUP_SSID);
      m_dns_server.start(DNS_PORT, "*", *m_ap_ip.get());
    }
  } else {
    if (m_wifi_disconnected_since != 0L) {
      Serial.println("WiFi got reconnected");
      m_wifi_disconnected_since = 0L;
    }

    if (m_is_in_accesspoint_mode) {
      m_dns_server.processNextRequest(); //Route everything to 192.168.4.1
    }

    m_webserver->loop(current_time);
  }
  return true;
}

bool Network::isWiFiConnected() {
  return WiFi.isConnected();
}
