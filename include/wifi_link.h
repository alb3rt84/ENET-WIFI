#pragma once

#include "net_config.h"

enum class WifiMode : uint8_t {
  Disconnected = 0,
  Station,
  SoftAP,
};

bool wifiStart(const NetConfig &cfg);
void wifiStop();
WifiMode wifiCurrentMode();
bool wifiIsUp();
IPAddress wifiLocalIp();
String wifiStatusLine();
