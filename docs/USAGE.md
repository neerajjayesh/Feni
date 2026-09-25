# Controls and menus

Tap advances, hold selects, and double tap goes Back. One tap waits about 350 ms
to distinguish a double tap. A hold triggers at about 900 ms.

| Menu | Behavior |
| --- | --- |
| CALENDAR | Tap through cached events; double tap returns to Menu. |
| MODE | Buddy, Clock, or Auto. Hold saves the highlighted choice. |
| TIMER | 10, 20, 30 minutes, or Custom. Hold starts; hold a running timer to cancel. |
| WLED MODE | Tap through favorite presets; hold applies one. |
| SETTINGS | Wi-Fi details, temporary password view, and Forget Wi-Fi. |

The current release is v3.1.1. Menu changes in later releases are documented here.

## Timers

Custom timers accept 001–180 minutes. Tap increases the highlighted digit.
Hold advances through the digits and onto Start timer; hold again to start.
Timers continue when you back out of their screen. Expiry shows a visual alert
for up to 15 seconds. An active timer does not survive power loss.

## Clock

Auto normally shows the face; a tap shows the clock for ten seconds. Clock mode
keeps the clock visible, and Buddy mode keeps the face. The default timezone is
India (`IST-5:30`); change the POSIX timezone in the local page if needed.

NTP sets the time. The browser also has a manual time-sync control. Without a
battery-backed RTC, the time must be set again after a complete power loss.

## WLED

Connect the WLED device to the same LAN. Save presets in WLED and note their
numeric IDs (1–250). Enter its IPv4 address and up to six favorites in Feni's
web page. Use a router DHCP reservation so the address stays consistent.

Applying a favorite selects an existing WLED preset; it does not overwrite that
preset. Feni verifies the reported preset after sending the request. Device
unavailability, invalid IDs and oversized/unreadable replies produce an error.

## Wi-Fi settings

Wi-Fi details displays the saved SSID and connected IP. The password view hides
after 15 seconds and cannot be downloaded through the framebuffer endpoint.
Forget Wi-Fi defaults to Cancel and requires selecting Forget network and holding.
It preserves display mode, timezone, Calendar and WLED preferences.
