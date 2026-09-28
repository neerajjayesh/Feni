# Changelog

## 3.5.4

- Wake for foreground gaming, coding, entertainment and custom PC activities;
  prevent sleep until activity ends or expires, then restart the sleep countdown.
- Keep neutral/background heartbeats passive and preserve the menu idle timeout.
- Lower the popcorn bucket with its lower half below the display edge.
- Add CS2, Antigravity, Codex and streaming-service defaults; remove YouTube.
- Retain Studio as the tray companion for foreground detection and app rules.

## 3.5.3

- Add the supplied 4.2-second PC-connected animation: lightning cut-outs in the
  eyes, entry flash, large bolt with typed text, and exit flash.
- Align cut-outs to the centered eyes and pause blinking during the introduction.
- Upgrade Studio's untouched built-in connection duration to 4200 ms while
  preserving custom code and durations.

## 3.5.2

- Lower gaming and popcorn eyes by 12 pixels from the top-edge position, including
  Studio previews. Keep ordinary buddy centering and overlay artwork unchanged.

## 3.5.1

- Add the supplied gaming controller and popcorn-eating overlays, with the
  requested raised eye position during these animations and centered eyes otherwise.
- Use Entertainment (slot 7) for popcorn; retain custom slots 2–8.
- Add optional foreground window-title matching and default YouTube/VLC rules.
  Browser use without a matching YouTube title keeps the normal rule behaviour.
- Keep window titles local to Studio; only the chosen behaviour is sent to Feni.

## 3.5.0

- Replace frame-file management with a C++ editor for each animation behaviour.
- Add code import, event durations, built-in fallback, source revisions and testing
  of the animation code currently installed on Feni.
- Build in isolated folders and flash through Arduino CLI over USB; compilation
  failures stop before upload. Keep app rules and tray operation.
- Remove PNG packing, FNA previews/uploads and filesystem playback. Restore the
  small four-colour framebuffer and retain existing coded animations by default.
- Preserve saved app rules and connection settings when upgrading Studio.

## 3.4.0

- Add Feni Studio for Windows with animation preview, PNG frame packing,
  persistent assignments and uploads over local Wi-Fi.
- Store startup, PC connected, sleep, wake, gaming, coding, idle and eight custom
  animation slots on Feni, with built-in fallbacks for unassigned slots.
- Add ordered app rules, foreground/background matching, executable selection,
  automatic reconnection and tray/sign-in operation.
- Validate FNA1 files before replacing stored clips; retain the white clock,
  existing menus, settings and unknown filesystem contents.
- Share framebuffer RAM with Calendar HTTPS to fit ESP8266 memory; explicitly
  select the 4 MB flash / 1 MB filesystem partition in the build helper.

## 3.3.0

- Add centered startup and wake animations, with sleep after five minutes idle.
- A single press wakes the sleeping buddy without opening the Auto clock.
- Double press opens the clock on Home; double press remains Back in menus.
- Add local Windows PC integration with flowing connection bolts, a gaming
  headset and coding brackets/cursor. Passive heartbeats do not prevent sleep.
- Include a Windows companion installer, app mappings and sign-in startup.
- Preserve the white clock, bold Calendar cards, themes and saved connections.

## 3.2.2

- Return to the buddy face after 60 seconds without button or web-control activity.
- Keep timers running and preserve saved mode/theme and network settings.
- Keep passive web polling from preventing the timeout; allow new alerts to appear.
- Defer Calendar HTTPS requests until the buddy face is visible to avoid delaying countdown redraws.
- Retain the v3.2.1 bold Calendar layout.

## 3.2.1

- Restore bold FreeSans fonts throughout the UI, retaining the current Calendar cards, colours and layout.

## 3.2.0

- Active Google Calendar meetings show the event name and time remaining.
- All-day events do not start countdowns; overlap and stale-cache behaviour are defined.
- Regular-weight FreeSans text replaces the enlarged bitmap/bold presentation.
- Rectangular menu blocks use Cyan, Orange, Green or Purple accents.
- Theme changes include the eyes; the clock stays white.
- Calendar cards separate date, title and time, using Google event colours.
- Wi-Fi options move under Settings > Wi-Fi; themes live under Settings > Accent Colours.
- Theme persistence preserves the existing Wi-Fi, Calendar and WLED configuration.
- Updated setup, Calendar, usage and testing documentation.

## 3.1.1

Initial public working baseline: centered face, onboard FLASH controls, Wi-Fi
setup, clock modes, 10/20/30-minute and custom timers, WLED favorites and secure
Google Calendar synchronization.
