#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include <memory>
#include <ESPAsyncWebSrv.h>


class WebServer
{
public:
  [[nodiscard]] bool init();
  [[nodiscard]] bool loop(const unsigned long& current_time);

  static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

private:
  static void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);
  static void notifyClients(const String& key, const String& data);
  static String processor(const String& var);

  void textAll(const String& key, const String& data);

private:
  std::unique_ptr<AsyncWebServer> m_server;
  std::unique_ptr<AsyncWebSocket> m_ws;
};

#endif // _WEBSERVER_H_
