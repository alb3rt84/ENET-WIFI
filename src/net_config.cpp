#include "net_config.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;

void configResetDefaults(NetConfig &cfg) {
  cfg.ssid = WIFI_SSID_DEFAULT;
  cfg.password = WIFI_PASS_DEFAULT;
  cfg.tcpPort = TCP_PORT_DEFAULT;
  cfg.uartBaud = UART_BAUD_DEFAULT;
  cfg.useStaticIp = false;
  cfg.localIp = IPAddress(0, 0, 0, 0);
  cfg.gateway = IPAddress(0, 0, 0, 0);
  cfg.subnet = IPAddress(255, 255, 255, 0);
  cfg.apFallback = true;
  cfg.apSsid = AP_SSID_DEFAULT;
  cfg.apPass = AP_PASS_DEFAULT;
}

void configLoad(NetConfig &cfg) {
  configResetDefaults(cfg);
  if (!prefs.begin(NVS_NAMESPACE, true)) {
    return;
  }
  cfg.ssid = prefs.getString("ssid", cfg.ssid);
  cfg.password = prefs.getString("pass", cfg.password);
  cfg.tcpPort = prefs.getUShort("port", cfg.tcpPort);
  cfg.uartBaud = prefs.getULong("baud", cfg.uartBaud);
  cfg.useStaticIp = prefs.getBool("static", false);
  cfg.apFallback = prefs.getBool("apfb", true);
  cfg.apSsid = prefs.getString("apssid", cfg.apSsid);
  cfg.apPass = prefs.getString("appass", cfg.apPass);

  uint32_t ip = prefs.getULong("ip", 0);
  uint32_t gw = prefs.getULong("gw", 0);
  uint32_t sn = prefs.getULong("sn", 0x00FFFFFF);  // 255.255.255.0 little-endian packing via uint32
  if (ip) {
    cfg.localIp = IPAddress(ip);
  }
  if (gw) {
    cfg.gateway = IPAddress(gw);
  }
  if (sn) {
    cfg.subnet = IPAddress(sn);
  }
  prefs.end();
}

void configSave(const NetConfig &cfg) {
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    return;
  }
  prefs.putString("ssid", cfg.ssid);
  prefs.putString("pass", cfg.password);
  prefs.putUShort("port", cfg.tcpPort);
  prefs.putULong("baud", cfg.uartBaud);
  prefs.putBool("static", cfg.useStaticIp);
  prefs.putBool("apfb", cfg.apFallback);
  prefs.putString("apssid", cfg.apSsid);
  prefs.putString("appass", cfg.apPass);
  prefs.putULong("ip", (uint32_t)cfg.localIp);
  prefs.putULong("gw", (uint32_t)cfg.gateway);
  prefs.putULong("sn", (uint32_t)cfg.subnet);
  prefs.end();
}

void configPrint(const NetConfig &cfg) {
  Serial.println(F("--- ENET-WIFI config ---"));
  Serial.printf("SSID: %s\n", cfg.ssid.c_str());
  Serial.printf("TCP port: %u\n", cfg.tcpPort);
  Serial.printf("UART baud: %lu\n", (unsigned long)cfg.uartBaud);
  Serial.printf("AP fallback: %s (%s)\n", cfg.apFallback ? "yes" : "no",
                cfg.apSsid.c_str());
  if (cfg.useStaticIp) {
    Serial.printf("Static IP: %s gw %s\n", cfg.localIp.toString().c_str(),
                  cfg.gateway.toString().c_str());
  } else {
    Serial.println(F("IP: DHCP"));
  }
  Serial.println(F("------------------------"));
}
