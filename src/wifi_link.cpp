#include "wifi_link.h"
#include "config.h"

static WifiMode g_mode = WifiMode::Disconnected;

bool wifiStart(const NetConfig &cfg) {
  wifiStop();
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);

  if (cfg.useStaticIp && cfg.localIp != IPAddress(0, 0, 0, 0)) {
    if (!WiFi.config(cfg.localIp, cfg.gateway, cfg.subnet)) {
      Serial.println(F("WiFi static IP config failed"));
    }
  }

  Serial.printf("WiFi STA connecting to \"%s\"...\n", cfg.ssid.c_str());
  WiFi.begin(cfg.ssid.c_str(), cfg.password.c_str());

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    g_mode = WifiMode::Station;
    Serial.printf("WiFi STA OK  IP=%s  RSSI=%d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    return true;
  }

  Serial.println(F("WiFi STA failed"));
  if (!cfg.apFallback) {
    g_mode = WifiMode::Disconnected;
    return false;
  }

  Serial.printf("Starting SoftAP \"%s\"...\n", cfg.apSsid.c_str());
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(cfg.apSsid.c_str(), cfg.apPass.c_str());
  if (!ok) {
    g_mode = WifiMode::Disconnected;
    Serial.println(F("SoftAP failed"));
    return false;
  }
  g_mode = WifiMode::SoftAP;
  Serial.printf("SoftAP OK  IP=%s\n", WiFi.softAPIP().toString().c_str());
  return true;
}

void wifiStop() {
  WiFi.disconnect(true, true);
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  g_mode = WifiMode::Disconnected;
}

WifiMode wifiCurrentMode() { return g_mode; }

bool wifiIsUp() {
  if (g_mode == WifiMode::Station) {
    return WiFi.status() == WL_CONNECTED;
  }
  if (g_mode == WifiMode::SoftAP) {
    return true;
  }
  return false;
}

IPAddress wifiLocalIp() {
  if (g_mode == WifiMode::SoftAP) {
    return WiFi.softAPIP();
  }
  return WiFi.localIP();
}

String wifiStatusLine() {
  String s;
  switch (g_mode) {
    case WifiMode::Station:
      s = "STA ";
      s += WiFi.SSID();
      s += " IP=";
      s += WiFi.localIP().toString();
      s += " RSSI=";
      s += String(WiFi.RSSI());
      break;
    case WifiMode::SoftAP:
      s = "AP ";
      s += WiFi.softAPSSID();
      s += " IP=";
      s += WiFi.softAPIP().toString();
      s += " STA=";
      s += String(WiFi.softAPgetStationNum());
      break;
    default:
      s = "DOWN";
      break;
  }
  return s;
}
