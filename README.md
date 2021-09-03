# esp32-wireless-display
This is an example project for the
[ESP-WROVER-KIT](https://www.adafruit.com/product/3384). In the future
it should also support the `ESP32-S2-HMI-DEVKIT-V1`.

This project uses the WROVER's LCD to act as a wired / wireless display for data
received by the serial port or over wifi / bluetooth.

## Setup

Menuconfig setup:
```
idf.py menuconfig
```
- `WiFi Connection Configuration -> WiFi SSID`
- `WiFi Connection Configuration -> WiFi Password`
- `Component Config -> Common ESP-related -> Main task stack size = 8192`
- `Component Config -> Common ESP-related -> Event Loop task stack size = 4096`
- `Partition Table -> Partition Table -> Custom partition table CSV`
- `Serial flasher config -> Flash Size -> 4MB`
