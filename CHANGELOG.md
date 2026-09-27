# Changelog

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
