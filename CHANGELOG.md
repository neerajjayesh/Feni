# Changelog

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
