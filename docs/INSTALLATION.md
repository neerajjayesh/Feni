# Install and upload

## Arduino IDE

1. Install Arduino IDE from the official Arduino website.
2. In Preferences, add the ESP8266 board-manager URL:
   `https://arduino.esp8266.com/stable/package_esp8266com_index.json`.
3. In Boards Manager, install **esp8266 by ESP8266 Community**, version **3.1.2**.
4. In Library Manager, install:

   | Library | Tested version |
   | --- | --- |
   | Adafruit GFX Library | 1.12.6 |
   | Adafruit BusIO | 1.17.4 |
   | Adafruit ST7735 and ST7789 Library | 1.11.0 |
   | ArduinoJson | 6.21.6 |

5. Open `FeniBuddy/FeniBuddy.ino` from this repository.
6. Select **NodeMCU 1.0 (ESP-12E Module)** and your USB serial port.
7. Set **MMU: 16KB cache + 48KB IRAM and 2nd Heap (shared)**. Keep other board
   settings at their defaults. The second heap is required for Calendar TLS buffers.
8. Upload. Automatic reset normally handles the bootloader. Release FLASH for
   the normal startup after uploading.

RoboEyes is included in the sketch; no separate RoboEyes installation is needed.
Use a data-capable USB cable. If the port is missing, install the USB-serial
driver appropriate to your board from its manufacturer's official site.

## Arduino CLI

Install Arduino CLI, then from the repository root:

```sh
arduino-cli core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266@3.1.2 --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli lib install "Adafruit GFX Library@1.12.6" "Adafruit BusIO@1.17.4" "Adafruit ST7735 and ST7789 Library@1.11.0" "ArduinoJson@6.21.6"
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2:mmu=4816H --build-path build-feni-buddy FeniBuddy
arduino-cli board list
arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2:mmu=4816H --port YOUR_PORT --input-dir build-feni-buddy FeniBuddy
```

Replace `YOUR_PORT` with the detected port, such as `COM8` or `/dev/ttyUSB0`.
On Windows, `FeniBuddy/Build.ps1` wraps compilation; add `-Upload -Port YOUR_PORT`
to upload. It finds Arduino CLI on PATH or in the standard Arduino IDE location.

## Connect to Wi-Fi

Join **Feni-Setup**, password **fenibuddy**. Open **http://192.168.4.1** and save
the exact network name and password. After connection, the setup hotspot closes
and the face appears. The local settings server stays available on your LAN.

Open **http://feni.local** or the numeric IP in Wi-Fi details. If multicast DNS
is unavailable on your computer/phone, use that numeric IP. Devices must be on
the same LAN without client isolation.

The ESP8266 uses 2.4 GHz, even when the router uses a shared 2.4/5 GHz SSID.
After 20 seconds offline, Feni reopens setup while retrying saved Wi-Fi.
Use Forget Wi-Fi to clear only the saved network and return to setup.
