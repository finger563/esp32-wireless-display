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

## Use

This code receives string data both from a UDP server as well as from
a UART serial port. It will parse that string data and determine which
of the following three types of data it is:

* *Commands*: contain the prefix (`+++`) in the string.
* *Plot data*: contain the delimeter (`::`) in the string followed by a
  single value which can be converted successfully to a number. If the
  conversion fails, the message will be printed as a log.
* *Log / text data*: all data that is not a command and cannot be
  plotted.

They are parsed in that priority order.

### Commands

There are a limited set of commands in the system, which are
determined by a prefix and the command itself. If the prefix is found
_ANYWHERE_ in the string message (where messages are separated by
newlines), then the message is determined to be a command.

**PREFIX:** `+++` - three plus characters in a row

* **Remove Plot:** this command (`RP:` followed by the string plot name) will remove the named plot from the graph.
* **Clear Plots:** this command (`CP`) will remove _all_ plots from the graph.
* **Clear Logs:** this command (`CL`) will remove _all_ logs / text.

### Plotting

Messages which contain the string `::` and which have a value that
successfully and completely converts into a number are determined to
be a plot. Plots are grouped by their name, which is any string
preceding the `::`.

### Logging

All other text is treated as a log and written out to the log
window. Note, we do not wrap lines, so any text that would go off the
edge of the screen is simply not rendered.
