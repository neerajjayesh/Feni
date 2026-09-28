# Feni Studio

Studio manages **C++ animation code**, USB builds and application reactions for
Feni v3.6.3. It runs on Windows 10/11 with .NET Framework 4.7.2 or newer. There is
no video/frame library, PNG packer or animation-file upload workflow.

## Install

From PowerShell in the repository:

```powershell
.\studio\Install.ps1
```

The installer builds Studio, bundles the firmware source and creates desktop,
Start menu and sign-in shortcuts. Use `-NoStartup` to omit sign-in startup.
Closing the window keeps app reactions running in the notification area; use
its **Quit** command to stop. Studio replaces the legacy PowerShell companion.
Existing app rules and the saved device address are preserved on upgrade.
Old animation files are not deleted; the new firmware does not use them.

## Edit a behaviour

1. Select **Animation code**, then Startup, PC connected, Sleeping, Wake from
   sleep, Gaming, AI, Idle buddy, Entertainment or Custom 2–8.
2. Paste your C++ drawing code or **Import code** from a `.cpp`, `.h` or `.txt` file.
3. **Save code** (Ctrl+S). Switching behaviours also saves your current draft.
4. Set an event duration for startup, PC connected and wake: 100–10000 ms.
5. **Build & flash** applies the saved code to the device.

The editor accepts a drawing function body, not a complete sketch. Studio wraps
it in the correct behaviour callback. Available variables are `canvas`,
`elapsed` (milliseconds since entering the behaviour) and `now` (device uptime).
The surface is cleared before each call and displayed afterwards.

```cpp
int height = (elapsed % 4000 < 150) ? 4 : 38;
canvas.fillRoundRect(36, 64-height/2, 34, height, 6, 1);
canvas.fillRoundRect(90, 64-height/2, 34, height, 6, 1);
```

Draw one update and return; use elapsed time instead of delays or blocking loops.
Canvas colours are indices: 0 black, 1 eye/theme colour, 2 accent, 3 dim accent.
Adafruit_GFX drawing methods and Feni text helpers are supported. See the
[code guide](../studio/Code%20guide.txt) for the complete contract.

Blank code keeps the existing coded animation. **Use built-in** clears the local
replacement; flash afterwards to apply it. A `return false;` in your code also
requests the built-in fallback. Gaming, AI, idle, sleep and custom code run
while their behaviour is active; use `elapsed % period` to repeat motion.

**Test installed code** previews the behaviour already compiled into Feni for up
to 12 seconds. It does not run unsaved edits or execute ESP8266 code on Windows.
Menus, alerts and the white clock keep priority. Startup, sleep and wake also
work when the PC is off.

## Build and flash

Install Arduino CLI, ESP8266 core 3.1.2 and the libraries listed in
[Installation](INSTALLATION.md). In **Build & flash**, select the bundled
`FeniBuddy` source folder, Arduino CLI executable and Feni's USB COM port.

- **Check / build** compiles without changing Feni.
- **Build & flash** compiles first, then uploads only when compilation succeeds.
- Compiler output identifies `behaviour-0.cpp` through `behaviour-14.cpp` in the
  behaviour list order. The Device tab shows installed and saved code revisions.

Each build uses a separate copy of the firmware with a generated
`CustomAnimations.h`. The source folder is not edited. Ordinary USB uploads retain
Wi-Fi, Calendar and other saved settings. Code must be reflashed after changes;
app-rule changes work immediately after saving and use local Wi-Fi.

Native C++ can compile yet contain runtime bugs. If a replacement causes resets,
clear that behaviour to built-in and flash again using USB.

## Gaming and entertainment

The supplied controller and popcorn animations are built into Gaming and
Entertainment. Blank replacement code keeps these overlays. Gaming repeats its
8-second sequence; popcorn repeats its 9-second sequence while the behaviour is
active. Both use an upper eye position with a 12-pixel top margin, returning to normal when the
behaviour changes. The PC connection animation retains priority.

The adapted source is [FeniAnimations-pop-game.h](../FeniBuddy/FeniAnimations-pop-game.h).
The popcorn bucket is lowered so roughly its upper half remains visible. Canvas
parameters are templates for firmware and host tests. RoboEyes supplies the eyes.

## Application rules

Select **App rules** to add executable names, choose **Foreground** or **Running**,
and assign Gaming, AI, Idle, Entertainment or Custom 2–8. The first enabled matching rule
wins; Move up/down changes priority. Names may be comma-separated. Choose app
`.exe` and Running apps help fill them. Save rules when done.

Entertainment defaults include VLC and browser-title matches for Prime Video,
Hotstar, NetMirror / net77.cc and Cineby / cineby.rocks. YouTube is excluded.
Browser rules require both the browser executable and the service name/domain
in the foreground window title; they do not inspect URLs or background tabs.
A player that omits the service from its title needs an additional title rule.
AI includes Antigravity, Antigravity IDE, Claude, ChatGPT and Codex desktop
processes. Browser-title rules also recognise Claude and ChatGPT in supported
browsers. Ordinary code editors do not select AI by default. Gaming includes `cs2.exe` (Steam game 730).
Existing installations preserve saved rules; edit App rules to adopt these defaults.
**Title contains (optional)** is case-insensitive and applies only to Foreground
rules. Leave it blank for ordinary executable matching.

Foreground follows the app being used. Running also matches background processes,
so a launcher left open can keep a reaction active. Add individual game executable
names if needed. No match selects Idle buddy. Only the resulting behaviour number, foreground flag and AI state are sent to Feni; process names and window titles stay local.

A matched foreground activity wakes a sleeping Feni with its wake animation and
prevents sleep while active. Neutral or background-only matches do not keep it
awake. When activity ends, or its heartbeat expires after 45 seconds, a fresh
five-minute sleep countdown begins. The one-minute menu return still applies.
Studio must keep running in the tray even when animations are hardcoded: it
provides Windows foreground detection and local device messages.

## AI status scenes

AI replaces Coding in slot 5. The supplied pixel-logo animation has four scenes:
normal prompting (creature only), thinking (small logo and three bouncing
dots), needs input (typing scene with waving arms and an exclamation mark),
and limit exhausted (slumped creature with X eyes and pulsing R.I.P. text).
The normal scene is rendered at 75% size in a centered 120x96 area, leaving
20-pixel side margins and 16-pixel top/bottom margins. Thinking and attention
retain their original size. Within the normal scene, the creature is now 72x45
pixels, with the typing lines and cursor removed from the normal scene. The
limit scene is half-size in a centered 80x64 area. The logo is always RGB #DE7B58, independent of the selected accent. Dots, alert,
code lines and bubble use the accent and its darker shade. The white clock is
unchanged. Blank AI replacement code selects these built-in scenes.

Studio checks the foreground window every two seconds. For AI it inspects
visible accessibility button labels locally. Question cards are recognised even
when their Send/Submit button is disabled until an answer is selected. Only
enabled approval and Stop controls are used for those states. Stop controls indicate
thinking; recognised approval or question controls indicate needs input and
take priority. Otherwise the normal prompting scene is used. This reads no edit
values and sends no button labels, warning text, prompts or conversation text
to Feni. Limit detection additionally reads short notices inside accessible
alert/status/dialog regions, matching explicit usage/message/session/quota
exhaustion wording. A matching notice takes priority and clears when the warning
disappears. It does not scan the conversation history or infer a limit from
Upgrade buttons, network errors or a full context window. If an app does not
expose its warning in those regions, select Limit exhausted in the tray menu.
No account usage API is polled, and no credits or resets are consumed.

Detection depends on the app exposing supported English accessibility labels.
Other languages, inaccessible controls or changed app versions may fall back to
prompting. Right-click the Studio tray icon, then **AI state > Automatic /
Prompting / Thinking / Needs input / Limit exhausted** to override it while an AI app is foreground.
The override lasts until changed or Studio exits; it does not force AI over other
foreground activities. The Device tab shows the chosen state and detection mode.

Live checks confirmed Codex working and Antigravity idle. Question and approval
classification is unit-tested; live approval/working requests in Antigravity
were not submitted during verification.

For other integrations, authenticated POST `/pc` accepts `activity=2` for AI,
`foreground=1` and `aiState=0` (prompting), `1` (thinking), `2` (needs input), or `3` (limit exhausted).
Omitting aiState resets it to prompting; unsupported values return HTTP 400.
Foreground AI retains the same wake/sleep behaviour as the other PC activities.

## Files and verification

- App: `%LOCALAPPDATA%\FeniStudio\app`
- Code/durations: `%LOCALAPPDATA%\FeniStudio\animations-code.json`
- Preferences: `%LOCALAPPDATA%\FeniStudio\config.json`
- Isolated build outputs: `%LOCALAPPDATA%\FeniStudio\builds`

```powershell
.\studio\Build.ps1
$test = Start-Process '.\studio\bin\Feni Studio.exe' -ArgumentList '--self-test' -Wait -PassThru
if ($test.ExitCode -ne 0) { throw 'Studio tests failed' }
Get-Content "$env:TEMP\feni-studio-tests.txt"
```

The self-test also writes a generated C++ fixture covering all 15 callbacks for
compilation with a host C++ compiler. No user animation code is bundled in the
repository; the defaults keep Feni's original coded face and reactions.
