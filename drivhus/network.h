#ifndef _NETWORK_H_
#define _NETWORK_H_

#ifdef TESTING
#else
# include <DNSServer.h>
# include <WiFi.h>
#endif

#include <memory>

#include "component.h"


namespace Drivhus {

class Network : public Component
{
public:
  static constexpr const char* SETUP_SSID = "drivhus-setup";
  static constexpr unsigned long MAX_WIFI_RECOVERY_DURATION_MS = 30000; //If system has been disconnected this long, switch to AccessPoint mode
  static constexpr unsigned long MAX_AP_WITHOUT_CLIENTS_DURATION_MS = 60000; //If system has been disconnected this long, switch to AccessPoint mode
  static constexpr uint8_t DNS_PORT = 53;

public:
  Network();
  [[nodiscard]] virtual bool init() override;
  virtual void loop() override;
  virtual const char* getName() const override {return "Network";}

  [[nodiscard]] bool isConnected() {return m_wifi_disconnected_since==0L;}

private:
  void activateWiFiStation();
  void activateWiFiAccessPoint();
  void deactivateWiFi();

public:
  DNSServer m_dns_server;
  std::shared_ptr<IPAddress> m_ap_ip;

  unsigned long m_wifi_disconnected_since;
  unsigned long m_wifi_accesspoint_mode_since;
  bool m_is_in_accesspoint_mode;
};

} //namespace

#endif // _NETWORK_H_
