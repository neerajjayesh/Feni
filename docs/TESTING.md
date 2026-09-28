# Build and verification

## Gaming/popcorn / v3.5.1

Host checks cover overlay drawing across complete cycles, fade envelopes,
Entertainment routing, unrelated custom-slot isolation and PC-intro priority.
RoboEyes tests verify the upper target during overlays and restoration to center.
Studio tests cover VLC, case-insensitive YouTube title matching, ordinary browser
use, unrelated apps with YouTube in their titles, and background browser isolation.
Live device checks confirmed both behaviour routes, raised eye targets, return
to the centered target, the white clock and preservation of Auto/Purple settings.
Framebuffer captures verified the supplied controller and popcorn artwork.
Calendar synced after flashing. Actual browser/VLC launches were not automated;
rule matching and device behaviour delivery were tested separately.

## Code animations / v3.5.0

Studio self-tests cover migration from the frame-based configuration while
retaining app rules, source generation for all 15 behaviours, built-in fallback,
event durations, revision changes and project serialization. The generated C++
fixture is compiled and run with the host compiler. All 15 generated replacement
callbacks also compiled against the real ESP8266 firmware through Studio's build
pipeline. A deliberately failing compiler verified that upload is never invoked
after a failed build.

The final firmware was built and uploaded with Studio's production build helper.
Existing gesture, sleep/wake, timer, settings, Calendar and WLED host suites pass.
Live checks verified the code-only API, all 15 behaviour entries, event duration
metadata, preview authorization and bounds, installed-code preview, menu/white
clock priority and preservation of the Purple theme and Auto mode. Calendar
synced successfully; free heap was about 26 KiB after the checks.
The Windows code editor and USB build controls were inspected at 125% scaling.
Custom user code remains the author's responsibility; compilation alone does
not verify arbitrary animation timing or runtime behaviour.

## Earlier frame implementation (removed in v3.5.0)

## Feni Studio / v3.4.0

Firmware builds for ESP8266 core 3.1.2 with the second IRAM heap and the explicit
4 MB flash / 1 MB filesystem layout. Host tests include FNA1 header/frame bounds,
palette indices, truncation and byte-for-byte compatibility with a clip produced
by the Windows packer. Studio's self-test covers rule priority, disabled rules,
foreground/background matching, custom slots, local destination validation,
PNG packing, preview colour rendering and malformed file rejection.

Live NodeMCU tests exercised the production .NET HTTP client, session-token
exchange, multipart upload, playback and removal. Device tests covered rejected
unauthenticated, malformed and empty uploads; preservation of the prior clip;
custom PC slots; and menu/white-clock priority during custom playback.
Framebuffer captures confirmed uploaded palette colours and a grayscale clock.
Calendar HTTPS synced with animation storage mounted, while the saved Purple
theme and Auto mode remained unchanged. The display framebuffer is released
during the existing HTTPS drawing pause and recreated afterwards to preserve
certificate-validation memory.

The Windows interface was inspected at 125% scaling. The onboard FLASH electrical
input and real game/editor launches were not automated; tests inject the same
gesture/activity paths, with rule matching tested separately. Maximum-rate,
maximum-size clips are hardware-dependent; use simple short artwork and preview
it on your own display.

## Earlier firmware verification

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

## v3.2.2 inactivity return

All 264 control checks passed, including every page's idle cutoff, held input,
timer continuation, new meeting wake-up and millisecond counter rollover. The
WLED, persistence, face, HTTP body, meeting and Calendar/browser suites also passed.

The production firmware was flashed and exercised through local HTTP controls.
With status polling every second, a tap at 30 seconds restarted the deadline.
The board entered idle at 60.012 seconds after that input; the next status read
observed it at 60.064 seconds. A running ten-minute timer continued behind the
face, and a tap reopened the white clock. The test-created timer was cancelled,
the original mode restored, and theme and Wi-Fi settings were preserved.
Calendar HTTPS sync succeeded afterward. Physical presses were not automated.

## v3.3.0 animations and PC companion

The firmware host suites include startup timing, sleep and wake in every display
mode, the double-press clock shortcut, timer continuation, rollover, centered
opening geometry, PC reconnect/expiry and sleep with continuous PC heartbeats.
Run `FeniBuddy/Test.ps1` as before. For the Windows companion, run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\pc\Test.ps1
```

The companion checks cover app-name normalization, coding/gaming classification
and local destination validation. A live Windows PowerShell run also verified
the foreground-process API path and session-token exchange with the device.

On-board HTTP tests passed the single/double/hold controls, white clock capture,
PC authorization and input validation, connection animation, gaming/coding
decorations, and stale connection expiry. With a running ten-minute timer and
continuous PC heartbeats, the one-minute idle return remained active and sleep
was observed at 304.113 seconds (five-second polling). One press played the
wake sequence without a clock peek; double press then opened the clock. The
temporary timer was cancelled and original theme, mode and Wi-Fi preserved.
Framebuffer captures were inspected for connection, gaming, coding, sleep and
wake. The test suite also covers a wake queued during blocked network drawing.
Physical FLASH presses and launching Steam/VS Code were not automated; device
tests used the same gesture handler and injected PC categories, while companion
tests exercised foreground detection, category mapping and live delivery.
