# Feni Studio

Feni Studio is a native Windows application for your own animation files and app
reactions. It runs on Windows 10/11 with .NET Framework 4.7.2 or newer. No Python
or video player is needed. Use firmware v3.4.0 or later on Feni.

## Install

From a PowerShell terminal in the repository:

```powershell
.\studio\Install.ps1
```

The script builds the app with the Windows .NET Framework compiler and installs
it in `%LOCALAPPDATA%\FeniStudio\app`. It creates desktop and Start menu shortcuts
and starts the app in the notification area at sign-in. Use `-NoStartup` to omit
automatic startup. Closing the window keeps reactions running; use **Quit** in
the notification-area menu to stop them.

The installer stops the old Feni PC Companion and backs up its startup shortcut.
Studio imports its device address and gaming/coding mappings on first launch.
Do not run both companions together. Preferences are stored in
`%LOCALAPPDATA%\FeniStudio\config.json`; your animation files stay in your folder.

## Prepare your animations

1. Make your animation in your preferred editor, including MP4 if convenient.
2. Export numbered **160 x 128 PNG frames** with zero-padded filenames such as
   `0001.png`, `0002.png`. Use a separate folder for each clip.
3. In **Library > Pack PNG frames**, select those frames, choose the frame rate
   and save the resulting `.fna` file in your chosen animation library folder.
4. Select **Choose folder** to browse your library. **Rescan** picks up new files.
   Select a clip to preview it.

Studio accepts `.fna` clips and packs PNG sequences; it does not decode MP4
directly. Export from your video editor or use the FFmpeg example in the
[format guide](../studio/Animation%20format.txt).

Use flat colours for compact clips. The packer chooses a shared 16-colour RGB565
palette, so gradients and photographic footage lose detail. Transparency becomes
black. Start at 10–15 fps. Limits are 1–20 fps, 300 frames, 30 seconds, and 512 KiB
per clip. Startup, PC connected and wake clips must be 10 seconds or shorter.
Actual frame rate depends on display and flash access; network operations can
briefly pause playback.

## Connect and assign

1. Connect the PC and Feni to the same local network.
2. Under **Device**, enter `http://feni.local` or Feni's private LAN IP, then
   **Connect / refresh**. The storage readout shows the shared device capacity.
3. Under **Animations**, select a behaviour, choose **Assign file**, then
   **Send selected**. **Send all assigned** uploads every local assignment.
4. **Test on Feni** previews an installed clip for up to 12 seconds.

Saving an assignment only updates this PC. Uploads are copies. Empty local
assignments do not erase clips already on the device. **Restore built-in** removes
only the selected device clip and retains the local file.

Startup, PC connected and wake play once. Sleep, gaming, coding, idle and custom
clips loop while that state is active. Menus, timers, meeting alerts and the
white clock keep priority. The first press while sleeping wakes Feni; double tap
opens the clock from Home. Uploaded artwork keeps its own colours; theme changes
apply to the built-in face and menus.

Clips persist in LittleFS through reboot and ordinary firmware uploads. Device
startup/sleep/wake animations work without the PC. App reactions need Studio
running. On a new device, firmware initializes only an entirely erased storage
partition. If storage is unavailable, existing unknown contents are preserved;
back them up before intentionally preparing a LittleFS partition.

## Application rules

In **App rules**, add a rule, choose executable names, select **Foreground** or
**Running**, and choose gaming, coding, idle or Custom 1–8. **Choose app .exe** and
**Running apps** help fill executable names. Multiple names may be comma-separated.

The first enabled matching rule wins; **Move up/down** changes priority. Foreground
uses the app you are currently using. Running also matches background processes,
so a launcher left open can keep its animation active. Add each game's actual
executable if you want it to match after the launcher loses focus. Save your rules.
With no match, the idle buddy is used. Studio sends only the resulting activity
and slot number to Feni, not the PC's process list or window contents.

## Build and verify

```powershell
.\studio\Build.ps1
$test = Start-Process '.\studio\bin\Feni Studio.exe' -ArgumentList '--self-test' -Wait -PassThru
if ($test.ExitCode -ne 0) { throw 'Studio tests failed' }
Get-Content "$env:TEMP\feni-studio-tests.txt"
```

The [binary format guide](../studio/Animation%20format.txt) documents FNA1 for
independent converters. There are no bundled replacement animations; supply
your own artwork or keep the firmware's built-in animations.
