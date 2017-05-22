# esp32-wireless-display
ESP32-WROVER-KIT programmed to be a wireless display / debugger / logger for numeric and text data logs

This is an example project for the 
[ESP-WROVER-KIT](https://www.adafruit.com/product/3384), 
a development / breakout board for the 
[Espressif ESP32](https://espressif.com/en/products/hardware/esp32/overview).

This project uses the WROVER's LCD to act as a wired / wireless display for data
received by the serial port or over wifi / bluetooth.

There are two *tasks* in this example:

1. **SerialTask** : which handles writing data to and receiving data from 
   the UART port connected to the USB. The serial interrupt buffers the 
   incoming serial data, and the HFSM code periodically (in the state) 
   reads from the queue. The SerialTask also periodically prints to the 
   UART which state it is in. The serial interrupt is configured to detect
   a pattern of `+++`, after which it will trigger the `changeState` variable 
   within the **InputOutputTask**, the **Serialtask**, and the **DisplayTask**.
   This task also spawns a serial receive task which is defined in its `definitions`.
2. **DisplayTask** : which handles drawing to the display periodically. 
   It receives data from the `Serial Task` and determines if it is numeric
   data or text data.
   
There are two *components* in this example:

1. **Display** : which handles the SPI communication to the **ILI9341** display
   and provides functions for drawing primitives and text. The driver contains 
   a 16bit palette which is used to write out data from the 8bit video ram (vram).
   The component makes use of the **Fonts** component to get its font raster data.
2. **Fonts** : which contains the raster data for two different font sizes.
