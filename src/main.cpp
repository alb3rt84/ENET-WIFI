#include <Arduino.h>

#include "config.h"
#include "net_config.h"
#include "wifi_link.h"
#include "bridge.h"
#include "at_cmd.h"

/*
 * ENET-WIFI na WT32-ETH01
 *
 * Ten sam model co fabryczny most ENET (UART ↔ TCP), transport: WiFi.
 *
 *  CFG (GPIO32) = LOW  → tryb AT na Serial (UART0 / USB-UART)
 *  CFG (GPIO32) = HIGH → tryb transparent: Serial2 ↔ klienci TCP
 */

static NetConfig g_cfg;
static EnetWifiBridge g_bridge;
static AtCommand g_at;
static bool g_transparent = false;
static uint32_t g_lastStatusMs = 0;

static bool cfgPinTransparent() {
  // Wejście z pull-up: HIGH = transparent (jak typowy ENET), LOW = AT
  return digitalRead(PIN_CFG) == HIGH;
}

void setup() {
  pinMode(PIN_CFG, INPUT_PULLUP);
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("=== ENET-WIFI / WT32-ETH01 ==="));
  Serial.println(F("UART<->TCP bridge over WiFi (ENET-like)"));

  configLoad(g_cfg);
  configPrint(g_cfg);

  g_at.begin(g_cfg, g_bridge);

  if (!wifiStart(g_cfg)) {
    Serial.println(F("Network down — AT mode available on Serial"));
  } else {
    g_bridge.begin(g_cfg);
    Serial.printf("Connect TCP client to %s:%u\n",
                  wifiLocalIp().toString().c_str(), g_cfg.tcpPort);
  }

  g_transparent = cfgPinTransparent();
  Serial.printf("Mode: %s (CFG=%d)\n",
                g_transparent ? "TRANSPARENT" : "AT", digitalRead(PIN_CFG));
  Serial.println(F("AT help: AT / AT+GMR / AT+CWJAP? / AT+CIFSR / AT+STATUS"));
}

void loop() {
  const bool wantTransparent = cfgPinTransparent();
  if (wantTransparent != g_transparent) {
    g_transparent = wantTransparent;
    Serial.printf("\nMode switch -> %s\n",
                  g_transparent ? "TRANSPARENT" : "AT");
  }

  if (!wifiIsUp()) {
    // próba ponownego połączenia STA co ~10 s
    static uint32_t lastRetry = 0;
    if (millis() - lastRetry > 10000) {
      lastRetry = millis();
      Serial.println(F("WiFi retry..."));
      if (wifiStart(g_cfg)) {
        g_bridge.begin(g_cfg);
      }
    }
  }

  if (g_transparent && wifiIsUp()) {
    g_bridge.loop();
  } else {
    g_at.loop();
  }

  // krótkie mruganie LED gdy jest klient TCP
  digitalWrite(PIN_STATUS_LED, g_bridge.hasClients() ? HIGH : LOW);

  if (millis() - g_lastStatusMs > STATUS_PRINT_MS) {
    g_lastStatusMs = millis();
    if (!g_transparent) {
      Serial.printf("[%lu] %s  clients=%u\n", (unsigned long)(millis() / 1000),
                    wifiStatusLine().c_str(),
                    (unsigned)g_bridge.clientCount());
    }
  }
}
