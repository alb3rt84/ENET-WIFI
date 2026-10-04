#pragma once

/*
 * Konfiguracja WT32-ETH01 — most UART↔TCP jak ENET, transport: WiFi.
 *
 * Zachowanie jak fabryczny most ENET:
 *  - CFG (GPIO32) LOW  → tryb AT (konfiguracja)
 *  - CFG (GPIO32) HIGH → tryb transparent (passthrough UART ↔ TCP)
 */

#include <Arduino.h>

// --- Piny WT32-ETH01 ---
#ifndef PIN_CFG
#define PIN_CFG           32   // wejście w tryb AT gdy LOW
#endif
#ifndef PIN_485_EN
#define PIN_485_EN        33   // opcjonalny driver RS485 (HIGH = TX)
#endif
#ifndef PIN_STATUS_LED
#define PIN_STATUS_LED    2    // nie wszystkie płytki mają LED
#endif

// UART danych (UART2) — jak typowy kanał użytkownika na WT32-ETH01
#ifndef DATA_UART_RX
#define DATA_UART_RX      5
#endif
#ifndef DATA_UART_TX
#define DATA_UART_TX      17
#endif

// Ethernet PHY (pozostawione dla dokumentacji / przyszłego dual-mode)
#define ETH_PHY_ADDR      1
#define ETH_PHY_MDC       23
#define ETH_PHY_MDIO       18
#define ETH_PHY_POWER      16
#define ETH_CLK_MODE      ETH_CLOCK_GPIO0_IN

#ifndef WIFI_SSID_DEFAULT
#define WIFI_SSID_DEFAULT "ENET WIFI"
#endif
#ifndef WIFI_PASS_DEFAULT
#define WIFI_PASS_DEFAULT "123456"
#endif
#ifndef TCP_PORT_DEFAULT
#define TCP_PORT_DEFAULT  8080
#endif
#ifndef UART_BAUD_DEFAULT
#define UART_BAUD_DEFAULT 115200
#endif

// SoftAP awaryjny gdy STA się nie łączy
// Uwaga: WPA2 SoftAP na ESP32 wymaga hasła min. 8 znaków
#ifndef AP_SSID_DEFAULT
#define AP_SSID_DEFAULT   "ENET WIFI"
#endif
#ifndef AP_PASS_DEFAULT
#define AP_PASS_DEFAULT   "12345678"
#endif

#define NVS_NAMESPACE     "enetwifi2"
#define BRIDGE_BUF_SIZE   1024
#define MAX_CLIENTS       4
#define WIFI_CONNECT_TIMEOUT_MS 20000
#define STATUS_PRINT_MS   5000
