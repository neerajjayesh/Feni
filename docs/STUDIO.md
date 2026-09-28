# Feni Studio

Studio manages **C++ animation code**, USB builds and application reactions for
Feni v3.5.1. It runs on Windows 10/11 with .NET Framework 4.7.2 or newer. There is
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
   sleep, Gaming, Coding, Idle buddy, Entertainment or Custom 2–8.
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
requests the built-in fallback. Gaming, coding, idle, sleep and custom code run
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
active. Both use the supplied upper eye position, returning to normal when the
behaviour changes. The PC connection animation retains priority.

The adapted source is [FeniAnimations-pop-game.h](../FeniBuddy/FeniAnimations-pop-game.h).
Its drawing geometry is unchanged; canvas parameters were made templates for
firmware and host-test compatibility. RoboEyes supplies the blinking eyes.

## Application rules

Select **App rules** to add executable names, choose **Foreground** or **Running**,
and assign Gaming, Coding, Idle, Entertainment or Custom 2–8. The first enabled matching rule
wins; Move up/down changes priority. Names may be comma-separated. Choose app
`.exe` and Running apps help fill them. Save rules when done.

The default VLC rule selects Entertainment for `vlc.exe`. The YouTube rule
requires both a supported browser process and `YouTube` in its foreground window
title. It follows the active browser window/tab, not background playback, and is
a title match rather than URL inspection. Other browsers/apps can be added.
**Title contains (optional)** is case-insensitive and applies only to Foreground
rules. Leave it blank for ordinary executable matching.

Foreground follows the app being used. Running also matches background processes,
so a launcher left open can keep a reaction active. Add individual game executable
names if needed. No match selects Idle buddy. Only the resulting behaviour number
is sent to Feni; the process list and window contents stay on the PC.

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
