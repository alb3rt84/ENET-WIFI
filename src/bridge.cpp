#include "bridge.h"

void EnetWifiBridge::begin(const NetConfig &cfg) {
  stop();
  _port = cfg.tcpPort;
  _uart = &Serial2;
  _uart->begin(cfg.uartBaud, SERIAL_8N1, DATA_UART_RX, DATA_UART_TX);
  pinMode(PIN_485_EN, OUTPUT);
  rs485Tx(false);

  _server = WiFiServer(_port);
  _server.begin();
  _server.setNoDelay(true);
  Serial.printf("TCP server (ENET-WIFI) listening on %u\n", _port);
}

void EnetWifiBridge::stop() {
  for (auto &c : _clients) {
    if (c) {
      c.stop();
    }
  }
  _server.stop();
  if (_uart) {
    _uart->end();
    _uart = nullptr;
  }
}

void EnetWifiBridge::setBaud(uint32_t baud) {
  if (!_uart) {
    return;
  }
  _uart->updateBaudRate(baud);
}

size_t EnetWifiBridge::clientCount() {
  size_t n = 0;
  for (auto &c : _clients) {
    if (c && c.connected()) {
      ++n;
    }
  }
  return n;
}

void EnetWifiBridge::rs485Tx(bool enable) {
  digitalWrite(PIN_485_EN, enable ? HIGH : LOW);
}

void EnetWifiBridge::acceptClients() {
  WiFiClient incoming = _server.available();
  if (!incoming) {
    return;
  }
  for (auto &c : _clients) {
    if (!c || !c.connected()) {
      if (c) {
        c.stop();
      }
      c = incoming;
      c.setNoDelay(true);
      Serial.printf("TCP client connected from %s  count=%u\n",
                    c.remoteIP().toString().c_str(),
                    (unsigned)clientCount());
      return;
    }
  }
  Serial.println(F("TCP reject: max clients"));
  incoming.println(F("BUSY"));
  incoming.stop();
}

void EnetWifiBridge::purgeClients() {
  for (auto &c : _clients) {
    if (c && !c.connected()) {
      c.stop();
    }
  }
}

void EnetWifiBridge::uartToTcp() {
  if (!_uart) {
    return;
  }
  int avail = _uart->available();
  if (avail <= 0) {
    return;
  }
  if (avail > (int)sizeof(_rxBuf)) {
    avail = sizeof(_rxBuf);
  }
  const int n = _uart->readBytes(_rxBuf, avail);
  if (n <= 0) {
    return;
  }
  for (auto &c : _clients) {
    if (c && c.connected()) {
      c.write(_rxBuf, n);
    }
  }
}

void EnetWifiBridge::tcpToUart() {
  if (!_uart) {
    return;
  }
  for (auto &c : _clients) {
    if (!c || !c.connected()) {
      continue;
    }
    int avail = c.available();
    if (avail <= 0) {
      continue;
    }
    if (avail > (int)sizeof(_rxBuf)) {
      avail = sizeof(_rxBuf);
    }
    const int n = c.read(_rxBuf, avail);
    if (n <= 0) {
      continue;
    }
    rs485Tx(true);
    _uart->write(_rxBuf, n);
    _uart->flush();
    rs485Tx(false);
  }
}

void EnetWifiBridge::loop() {
  acceptClients();
  purgeClients();
  uartToTcp();
  tcpToUart();
}
