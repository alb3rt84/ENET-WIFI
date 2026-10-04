#pragma once

#include <WiFi.h>
#include "config.h"
#include "net_config.h"

/*
 * Most TCP jak ENET: serwer TCP nasłuchuje, dane płyną transparentnie
 * między DATA UART a połączonymi klientami TCP.
 */
class EnetWifiBridge {
 public:
  void begin(const NetConfig &cfg);
  void stop();
  void loop();                 // wywoływać w loop() w trybie transparent
  void setBaud(uint32_t baud);
  uint16_t port() const { return _port; }
  size_t clientCount();
  bool hasClients() { return clientCount() > 0; }

 private:
  WiFiServer _server{0};
  WiFiClient _clients[MAX_CLIENTS];
  uint16_t _port = 0;
  HardwareSerial *_uart = nullptr;
  uint8_t _rxBuf[BRIDGE_BUF_SIZE];
  bool _rs485 = false;

  void acceptClients();
  void purgeClients();
  void uartToTcp();
  void tcpToUart();
  void rs485Tx(bool enable);
};
