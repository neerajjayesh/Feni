# Build and verification

The v3.1.1 baseline was compiled for ESP8266 core 3.1.2 and tested on a NodeMCU
with the documented ST7735 wiring. Two live HTTPS Calendar syncs succeeded.
This is not a guarantee for every display variant, router or Google Workspace policy.

Host tests cover input gestures, navigation, timers and rollover, EEPROM failure
handling, centered eyes, bounded HTTP transfers, WLED replies, Calendar filtering
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
