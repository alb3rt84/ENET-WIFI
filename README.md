# ENET-WIFI — WT32-ETH01

Most **UART ↔ TCP** jak fabryczny ENET na WT32-ETH01 (ESP32-WT01), ale transportem jest **WiFi** zamiast Ethernet.

## Co robi

| ENET (fabrycznie) | ENET-WIFI (ten projekt) |
|---|---|
| łącze RJ45 / LAN8720 | WiFi STA (z SoftAP awaryjnym) |
| serwer TCP | serwer TCP (domyślnie port `8080`) |
| UART danych ↔ TCP | `Serial2` (GPIO17 TX / GPIO5 RX) ↔ TCP |
| CFG → AT / transparent | GPIO32: LOW = AT, HIGH = transparent |

## Szybki start

1. Zainstaluj [PlatformIO](https://platformio.org/).
2. Ustaw sieć w `platformio.ini` (`WIFI_SSID_DEFAULT` / `WIFI_PASS_DEFAULT`) albo po wgraniu przez AT.
3. Wgraj firmware:

```bash
pio run -t upload
pio device monitor
```

4. Połącz klienta TCP z IP płytki (port `8080`):

```bash
python3 tools/tcp_client.py 192.168.x.x 8080
```

Dane z TCP lecą na UART2 i odwrotnie — tak jak przy klasycznym ENET.

## Piny (WT32-ETH01)

| Funkcja | GPIO |
|---|---|
| CFG (AT / transparent) | 32 |
| RS485 DE/RE (opcjonalnie) | 33 |
| UART danych TX | 17 |
| UART danych RX | 5 |
| ETH MDC / MDIO / POWER (nieużywane w tym firmware) | 23 / 18 / 16 |

Debug / AT: `Serial` (UART0, 115200).

## Komendy AT (Serial, CFG=LOW)

```
AT
AT+GMR
AT+CWJAP="ssid","haslo"
AT+CWJAP?
AT+CWSAP="WT01-WIFI","wt01wifi"
AT+CWSAP?
AT+CIFSR
AT+CIPSERVER=1,8080
AT+UART_DEF=115200,8,1,0,0
AT+CIPSTA="192.168.1.50","192.168.1.1","255.255.255.0"
AT+SAVE
AT+STATUS
AT+RST
```

Po `AT+CWJAP=...` i `AT+SAVE` ustaw CFG w stan HIGH — most działa transparentnie bez AT na linii danych.

## SoftAP awaryjny (`WT01-WIFI`)

Gdy płytka **nie połączy się z Twoim routerem**, odpala własną sieć:

| | |
|---|---|
| **SSID** | `WT01-WIFI` |
| **Hasło** | `wt01wifi` |
| **IP płytki** | `192.168.4.1` |
| **TCP** | port `8080` |

Połączenie: telefon/PC → sieć `WT01-WIFI` → hasło `wt01wifi` → TCP na `192.168.4.1:8080`.

Zmiana hasła SoftAP (Serial, CFG=LOW), potem zapisz:

```
AT+CWSAP="WT01-WIFI","nowehaslo"
AT+SAVE
```

Żeby płytka łączyła się z **Twoją** siecią domową (zamiast SoftAP):

```
AT+CWJAP="nazwa-routera","haslo-routera"
AT+SAVE
AT+RST
```

## Uwagi

- To nie jest EtherCAT / przemysłowy stack ENET — chodzi o **ten sam model komunikacji** co most szeregowy ENET na WT01: TCP server + passthrough UART.
- Ethernet PHY na płytce pozostaje niewykorzystany; WiFi zastępuje łącze przewodowe.
- Hasła produkcyjne ustawiaj przez AT + `AT+SAVE` albo lokalny `secrets.h` (wzór: `include/secrets.h.example`).
