#pragma once

#include <Arduino.h>
#include <WiFi.h>

struct NetConfig {
  String ssid;
  String password;
  uint16_t tcpPort;
  uint32_t uartBaud;
  bool useStaticIp;
  IPAddress localIp;
  IPAddress gateway;
  IPAddress subnet;
  bool apFallback;   // SoftAP gdy STA fail
  String apSsid;
  String apPass;
};

void configLoad(NetConfig &cfg);
void configSave(const NetConfig &cfg);
void configResetDefaults(NetConfig &cfg);
void configPrint(const NetConfig &cfg);
