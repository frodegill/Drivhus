#include "webserver.h"

#include "global.h"


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 2</h2>
      <p class="state">state: <span id="state">%STATE%</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
</script>
</body>
</html>
)rawliteral";


bool WebServer::init() {
  m_server = std::unique_ptr<AsyncWebServer>(new AsyncWebServer(80));
  m_ws = std::unique_ptr<AsyncWebSocket>(new AsyncWebSocket("/ws"));

  m_ws->onEvent(WebServer::onEvent);
  m_server->addHandler(m_ws.get());

  // Route for root / web page
  m_server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, WebServer::processor);
  });

  m_server->begin();
}

bool WebServer::loop(const unsigned long&) {
  m_ws->cleanupClients();
  return true;
}

void WebServer::textAll() {
  int ledState = 1;
  m_ws->textAll(String(ledState));
}

void WebServer::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    default:
      break;
  }
}

void WebServer::handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (info->final &&
      info->index==0 &&
      info->len==len &&
      info->opcode==WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
//      ledState = !ledState;
      notifyClients();
    }
  }
}

void WebServer::notifyClients() {
  g_web_server.textAll();
}

String WebServer::processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    int ledState = 1;
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}
