# Feni

An ESP8266 desk companion with a 1.8-inch TFT: a centered animated face, clock,
Google Calendar reminders, countdown timers, and WLED preset control.

**Current firmware: v3.2.1.** The original working baseline is tagged `v3.1.1`. Designed for a NodeMCU ESP8266 and an ST7735
128 x 160 SPI display, used in landscape at 160 x 128.

## Components

- NodeMCU 1.0 / ESP-12E development board, ESP8266, 4 MB flash.
- 1.8-inch ST7735 SPI TFT, 128 x 160 pixels.
- Jumper wires and a USB data cable / suitable USB power source.

No additional input component is required. The board's built-in FLASH control
provides tap, hold and double-tap actions.

## Wiring

Disconnect power before wiring. Use 3.3 V for this setup.

| TFT pin | NodeMCU pin | GPIO |
| --- | --- | --- |
| VCC | 3V3 | — |
| LED / backlight | 3V3 | — |
| GND | GND | — |
| SCK / CLK | D5 | 14 |
| SDA / MOSI | D7 | 13 |
| CS | D0 | 16 |
| A0 / DC | D1 | 5 |
| RESET / RST | D2 | 4 |

The TFT's A0 means data/command; do not connect it to the board's analog A0.
The display uses the ST7735 black-tab initialization and rotation 1. Other TFT
variants may need changes in [Hardware.h](FeniBuddy/Hardware.h).

## Get started

1. Follow [Installation](docs/INSTALLATION.md) to install dependencies and upload.
2. Join **Feni-Setup**, password **fenibuddy**, and open **http://192.168.4.1**.
3. Enter your 2.4 GHz Wi-Fi details. Feni saves them and closes the setup hotspot
   after connecting. A router may use one SSID for both bands; ESP8266 uses 2.4 GHz.
4. Rejoin your normal network and open **http://feni.local**, or Feni's IP shown
   in Settings / Wi-Fi / Network details or USB `STATUS` output.
5. Optionally follow [Google Calendar setup](docs/CALENDAR.md) and
   [WLED setup](docs/USAGE.md#wled).

## Features

- Centered RoboEyes animations and Buddy / Clock / Auto display modes.
- In Auto, a tap shows the clock for ten seconds. The clock remains white.
- Cyan, Orange, Green and Purple accent themes, including the eyes.
- Larger, bold FreeSans text and filled rectangular menu selections.
- NTP clock, default India timezone, with manual time sync in the local page.
- Google Calendar feed with current/upcoming events, and visual reminders ten
  minutes before timed events and at their start. No PC bridge is needed.
- Active meeting name and remaining-time display, with Google event colour cards.
- 10-, 20-, 30-minute timers and custom durations from 1 to 180 minutes.
- WLED IP and preset favorites configured through the local web page.
- Settings / Wi-Fi contains saved Wi-Fi details, temporary password display, and a Forget network option.
- Setup hotspot recovery when a saved network is unavailable.

## Controls

| Action | Result |
| --- | --- |
| Tap FLASH | Next item; temporary clock in Auto |
| Hold FLASH for about 0.9 seconds | Open menu / select item |
| Double tap FLASH | Back, or dismiss an alert |

Release FLASH during power-up/reset so the ESP8266 starts normally. Holding it
at reset selects the bootloader. See [Usage](docs/USAGE.md) for menu details.

## Repository layout

- `FeniBuddy/`: firmware sketch and headers, build helpers, and host tests.
- `FeniBuddy/calendar/`: read-only Google Apps Script bridge and manifest.
- `FeniBuddy/src/roboeyes/`: bundled RoboEyes source with its original license.
- `docs/`: installation, Calendar setup, controls and testing instructions.

Saved device credentials, personal Calendar deployments, flash backups and build
outputs are intentionally not part of this repository. Wi-Fi and Calendar keys
are configured at runtime. The default setup-hotspot password above is not a
personal Wi-Fi credential.

## Limitations

This ESP8266 does not support 5 GHz Wi-Fi. The clock needs network/manual sync
after power loss; there is no battery-backed RTC. Notifications and timer alarms
are visual. Secure Calendar requests can briefly pause display redraws. Cached
events and active timers are kept in RAM and do not survive a reboot.

## License and credits

[GPL-3.0](LICENSE), consistent with the bundled
[FluxGarage RoboEyes](https://github.com/FluxGarage/RoboEyes) library.
Display support uses Adafruit GFX and Adafruit ST7735/ST7789. JSON parsing uses
ArduinoJson. Dependency licenses remain with their respective authors.
