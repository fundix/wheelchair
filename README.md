# ESP32-C3 Arduino Project

This project is a simple Arduino sketch configuration for the ESP32-C3-DevKitM-1 board developed with PlatformIO.
Prerequisites

- [PlatformIO](https://platformio.org/) installed in [VS Code](https://code.visualstudio.com/).
- [ESP32-C3](https://docs.platformio.org/en/latest/boards/espressif32/esp32-c3-devkitm-1.html) board.

## PlatformIO configuration

The `platformio.ini` file is configured as follows:

```ini
[env:esp32c3]
platform = espressif32
board = esp32-c3-devkitm-1
board_build.mcu = esp32c3
framework = arduino

build_flags = 
  -DARDUINO_USB_CDC_ON_BOOT=1
  -DARDUINO_USB_MODE=1
  -DARDUINO_TINYUSB=1
  -DCORE_DEBUG_LEVEL=5  ; Set the debug level (0-5, with 5 being the most verbose)

lib_deps = 

monitor_filters = 
    esp32_exception_decoder
    time      ; Add timestamp with milliseconds for each new line
```


## Code example

This is the code in the `main.cpp` file:
```c++
#include <Arduino.h>

void setup()
{
  Serial.begin();
  // No matter what is set. Measured 256k = ~2^18 ASCII characters per second
  Serial.setDebugOutput(true);

  while (!Serial)
    ; // If you want to wait for a USB connection to be established

  // put your setup code here, to run once:

  log_d("setup done");           // for debugging purposes
  Serial.printf("setup done\n"); // writes "setup done" to the usb interface
}

void loop()
{
  // put your main code here, to run repeatedly:
}

```
