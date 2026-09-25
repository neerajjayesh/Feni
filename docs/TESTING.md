# Build and verification

The v3.2.0 firmware was compiled for ESP8266 core 3.1.2 and tested on a NodeMCU
with the documented ST7735 wiring. Live HTTPS Calendar syncs include Google event colours. The v3.1.1 baseline
remains available as a tag.
This is not a guarantee for every display variant, router or Google Workspace policy.

Host tests cover input gestures, navigation, timers and rollover, EEPROM failure
handling, centered eyes, bounded HTTP transfers, WLED replies, Calendar filtering
event colour bounds, active meeting boundaries/overlaps, theme storage isolation,
and embedded browser JavaScript syntax.

On Windows, install Node.js, the documented Arduino libraries, and Zig 0.13.0.
Then run:

```powershell
.\FeniBuddy\Test.ps1 -ZigPath C:\path\to\zig.exe
```

For the optional USB smoke test, install Python and pyserial:

```sh
python -m pip install pyserial
python FeniBuddy/CheckDevice.py --port YOUR_PORT
```

The smoke test uses serial controls and runs a short custom timer. It does not
prove the electrical response of physical FLASH presses. It preserves stored
preferences and does not forget your network. With Wi-Fi available, it also
checks HTTP controls and captures non-password display frames into `runtime/`.

At 115200 baud, `STATUS` reports health without credentials. `TAP`, `HOLD` and
`BACK` exercise navigation. `WIFI` scans for the saved network; `RECONNECT`
retries it. Avoid committing runtime logs or framebuffer captures containing
personal calendar data.

## Temporary on-board meeting fixture

For development, `Build.ps1 -TestScenes -Upload -Port YOUR_PORT` enables two
USB commands: `TEST_MEETING` and `TEST_END`. The first temporarily replaces the
RAM event cache with one 45-second timed meeting and an all-day entry. It
restores the real cache automatically two seconds after the meeting ends;
`TEST_END` restores it immediately. Google Calendar and saved credentials are
never edited. This fixture lets you inspect countdowns, clock peeking, Back,
Calendar cards and exclusive end-time behaviour.

Build and upload again **without `-TestScenes`** for normal use. The fixture
commands and backup allocation are absent from that production build.

Theme preferences use a separate EEPROM record after the existing v3.1
configuration. Tests check that all Wi-Fi, Calendar and WLED bytes survive
saving each theme and a failed write. A default/invalid theme record selects Cyan.

The display uses a 5 KB indexed framebuffer, with a 24-bit BMP export for visual
inspection. Keep the documented MMU setting: Calendar TLS uses the separate
IRAM heap. Inspect `heap`, `maxBlock` and `iramMaxBlock` in `STATUS` when changing
memory use. Runtime screenshots and reports stay outside version control.

## v3.2.0 hardware checks

On the documented NodeMCU/ST7735 setup, the USB/HTTP checks passed timer presets,
one-minute expiry, nested Wi-Fi navigation, password-frame blocking, and Cancel
on Forget Wi-Fi. A temporary on-board meeting verified automatic display,
countdown progress, white clock peeking, dismissal, Calendar reopening and
end-time expiry without treating an all-day event as a timer. All four themes
were exercised on the display and each clock capture contained only black and
white. Physical button wiring still requires a manual check; automated controls
exercise the same UI gesture handler through USB.

The final regular-weight UI build was flashed and checked again. A saved theme
survived the upload/reboot, Wi-Fi and Calendar settings remained intact, and a
fresh HTTPS sync returned three coloured events. Final free heap was 27,312
bytes, largest DRAM block 24,560 bytes and largest IRAM block 20,016 bytes.
The normal build rejected the development-only meeting command. Cyan was restored.

## v3.2.1 font update

Restored bold FreeSans fonts and verified the menu, clock and Calendar card on
the TFT framebuffer after flashing. The clock remained white, saved theme/mode
and connections were preserved, and live Calendar sync returned three events.
