#ifndef _GLOBAL_H_
#define _GLOBAL_H_

const uint8_t RS485_RX_PIN     = 13;
const uint8_t RS485_TX_PIN     = 27;
const uint8_t RS485_ENABLE_PIN = 14;


class RS485;
extern RS485 g_rs485;

class WebServer;
extern WebServer g_web_server;

#endif // _GLOBAL_H_
