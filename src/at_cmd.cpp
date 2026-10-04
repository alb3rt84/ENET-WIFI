#include "at_cmd.h"
#include "config.h"
#include "wifi_link.h"

#include <WiFi.h>

void AtCommand::begin(NetConfig &cfg, EnetWifiBridge &bridge) {
  _cfg = &cfg;
  _bridge = &bridge;
  _line.reserve(160);
}

void AtCommand::replyOk() { Serial.println(F("OK")); }
void AtCommand::replyError() { Serial.println(F("ERROR")); }

String AtCommand::unquote(String s) {
  s.trim();
  if (s.length() >= 2 && s[0] == '"' && s[s.length() - 1] == '"') {
    return s.substring(1, s.length() - 1);
  }
  return s;
}

void AtCommand::loop() {
  while (Serial.available()) {
    const char ch = (char)Serial.read();
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      if (_line.length()) {
        handleLine(_line);
        _line = "";
      }
      continue;
    }
    if (_line.length() < 200) {
      _line += ch;
    }
  }
}

void AtCommand::handleLine(String line) {
  line.trim();
  if (!line.length()) {
    return;
  }

  String cmd = line;
  cmd.toUpperCase();

  if (cmd == "AT") {
    replyOk();
    return;
  }

  if (cmd == "AT+RST") {
    replyOk();
    delay(50);
    ESP.restart();
    return;
  }

  if (cmd == "AT+GMR") {
    Serial.println(F("ENET-WIFI for WT32-ETH01"));
    Serial.println(F("UART<->TCP over WiFi (ENET-compatible bridge)"));
    Serial.printf("SDK %s\n", ESP.getSdkVersion());
    replyOk();
    return;
  }

  if (cmd == "AT+CWMODE?") {
    Serial.println(F("+CWMODE:1"));
    replyOk();
    return;
  }

  if (cmd == "AT+CWJAP?") {
    Serial.printf("+CWJAP:\"%s\"\n", _cfg->ssid.c_str());
    replyOk();
    return;
  }

  if (cmd == "AT+CWSAP?") {
    Serial.printf("+CWSAP:\"%s\",\"%s\"\n", _cfg->apSsid.c_str(),
                  _cfg->apPass.c_str());
    replyOk();
    return;
  }

  if (cmd.startsWith("AT+CWSAP=")) {
    // AT+CWSAP="ssid","pass"
    String args = line.substring(String("AT+CWSAP=").length());
    int comma = -1;
    bool inQ = false;
    for (int i = 0; i < (int)args.length(); ++i) {
      if (args[i] == '"') {
        inQ = !inQ;
      } else if (args[i] == ',' && !inQ) {
        comma = i;
        break;
      }
    }
    if (comma < 0) {
      replyError();
      return;
    }
    String apSsid = unquote(args.substring(0, comma));
    String apPass = unquote(args.substring(comma + 1));
    if (!apSsid.length() || apPass.length() < 8) {
      Serial.println(F("+CWSAP: haslo SoftAP musi miec min. 8 znakow"));
      replyError();
      return;
    }
    _cfg->apSsid = apSsid;
    _cfg->apPass = apPass;
    _cfg->apFallback = true;
    // Restart sieci — SoftAP z nowym haslem (gdy STA fail)
    if (!wifiStart(*_cfg)) {
      replyError();
      return;
    }
    _bridge->begin(*_cfg);
    replyOk();
    return;
  }

  if (cmd.startsWith("AT+CWJAP=")) {
    // AT+CWJAP="ssid","pass"
    String args = line.substring(String("AT+CWJAP=").length());
    int comma = -1;
    bool inQ = false;
    for (int i = 0; i < (int)args.length(); ++i) {
      if (args[i] == '"') {
        inQ = !inQ;
      } else if (args[i] == ',' && !inQ) {
        comma = i;
        break;
      }
    }
    if (comma < 0) {
      replyError();
      return;
    }
    _cfg->ssid = unquote(args.substring(0, comma));
    _cfg->password = unquote(args.substring(comma + 1));
    if (!wifiStart(*_cfg)) {
      replyError();
      return;
    }
    _bridge->begin(*_cfg);
    replyOk();
    return;
  }

  if (cmd == "AT+CIFSR") {
    Serial.printf("+CIFSR:STAIP,\"%s\"\n", wifiLocalIp().toString().c_str());
    Serial.printf("+CIFSR:STAMAC,\"%s\"\n", WiFi.macAddress().c_str());
    replyOk();
    return;
  }

  if (cmd.startsWith("AT+CIPSERVER=")) {
    String args = line.substring(String("AT+CIPSERVER=").length());
    int comma = args.indexOf(',');
    int mode = args.substring(0, comma >= 0 ? comma : args.length()).toInt();
    if (mode == 0) {
      _bridge->stop();
      replyOk();
      return;
    }
    if (mode == 1) {
      if (comma >= 0) {
        _cfg->tcpPort = (uint16_t)args.substring(comma + 1).toInt();
      }
      if (!wifiIsUp()) {
        if (!wifiStart(*_cfg)) {
          replyError();
          return;
        }
      }
      _bridge->begin(*_cfg);
      replyOk();
      return;
    }
    replyError();
    return;
  }

  if (cmd.startsWith("AT+UART_DEF=") || cmd.startsWith("AT+UART=")) {
    const int eq = line.indexOf('=');
    String args = line.substring(eq + 1);
    const int baud = args.substring(0, args.indexOf(',')).toInt();
    if (baud < 1200) {
      replyError();
      return;
    }
    _cfg->uartBaud = (uint32_t)baud;
    _bridge->setBaud(_cfg->uartBaud);
    replyOk();
    return;
  }

  if (cmd == "AT+SAVE") {
    configSave(*_cfg);
    replyOk();
    return;
  }

  if (cmd == "AT+STATUS") {
    Serial.println(wifiStatusLine());
    Serial.printf("TCP port=%u clients=%u CFG=%d\n", _bridge->port(),
                  (unsigned)_bridge->clientCount(), digitalRead(PIN_CFG));
    replyOk();
    return;
  }

  if (cmd == "AT+PASSCHANNEL=1" || cmd == "AT+TRANSPARENT=1") {
    Serial.println(F("+PASSCHANNEL: enter transparent when CFG=HIGH"));
    replyOk();
    return;
  }

  if (cmd.startsWith("AT+CIPSTA=")) {
    // AT+CIPSTA="ip","gw","mask"
    String args = line.substring(String("AT+CIPSTA=").length());
    String parts[3];
    int idx = 0;
    bool inQ = false;
    String cur;
    for (size_t i = 0; i < args.length() && idx < 3; ++i) {
      char c = args[i];
      if (c == '"') {
        inQ = !inQ;
        continue;
      }
      if (c == ',' && !inQ) {
        parts[idx++] = cur;
        cur = "";
        continue;
      }
      cur += c;
    }
    if (idx < 3) {
      parts[idx++] = cur;
    }
    if (idx < 3) {
      replyError();
      return;
    }
    IPAddress ip, gw, sn;
    if (!ip.fromString(parts[0]) || !gw.fromString(parts[1]) ||
        !sn.fromString(parts[2])) {
      replyError();
      return;
    }
    _cfg->localIp = ip;
    _cfg->gateway = gw;
    _cfg->subnet = sn;
    _cfg->useStaticIp = true;
    replyOk();
    return;
  }

  if (cmd == "AT+CIPSTA=\"DHCP\"" || cmd == "AT+CIPSTA=DHCP") {
    _cfg->useStaticIp = false;
    replyOk();
    return;
  }

  replyError();
}
