#pragma once

#include "bridge.h"
#include "net_config.h"
#include "wifi_link.h"

/*
 * Minimalny dialekt AT inspirowany fabrycznym firmware ENET WT32-ETH01,
 * dostosowany do WiFi.
 *
 * Komendy (kończone CR/LF), odpowiedzi: OK / ERROR / wartości.
 *
 *  AT                 — ping
 *  AT+RST             — soft reset
 *  AT+GMR             — wersja
 *  AT+CWMODE?         — 1=STA
 *  AT+CWJAP="ssid","pass"
 *  AT+CWJAP?          — aktualne SSID
 *  AT+CIFSR           — IP
 *  AT+CIPSERVER=1,port  — start serwera TCP (jak ENET)
 *  AT+CIPSERVER=0     — stop serwera
 *  AT+UART_DEF=baud,8,1,0,0
 *  AT+SAVE            — zapisz NVS
 *  AT+PASSCHANNEL=1   — wyjście do trybu transparent (wymaga CFG=HIGH)
 *  AT+STATUS          — status WiFi/TCP
 */
class AtCommand {
 public:
  void begin(NetConfig &cfg, EnetWifiBridge &bridge);
  void loop();  // czyta Serial (USB/UART0)

 private:
  NetConfig *_cfg = nullptr;
  EnetWifiBridge *_bridge = nullptr;
  String _line;

  void handleLine(String line);
  void replyOk();
  void replyError();
  static String unquote(String s);
};
