# Windows PC companion

For custom animations and configurable app rules, use [Feni Studio](STUDIO.md).
It replaces this legacy PowerShell companion. Do not run both at the same time.

Feni v3.3.0 reacts to the foreground app on a Windows PC. Gaming adds a small
headset and microphone around the centered square eyes; coding adds brackets
and a blinking cursor. A new connection plays flowing lightning symbols with
**PC connected**. The companion uses Windows PowerShell; no Python is needed.

## Install

Connect the PC and Feni to the same trusted local network. From this repository:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\pc\Install.ps1 -Url http://feni.local
```

If name discovery fails, use the address from **Settings > Wi-Fi > Network
details**, for example `http://192.168.1.50`. Reserve that IP in your router.

The installer copies the companion into `%LOCALAPPDATA%\FeniPcCompanion`, starts
it hidden, and creates **Feni PC Companion.lnk** in your Windows Startup folder.
No administrator rights are needed. Existing app mappings are preserved.
Run the installer again to update, restart or change Feni's address.

## App mappings

Edit `%LOCALAPPDATA%\FeniPcCompanion\config.json`. Lists contain process names,
case-insensitive, with or without `.exe`. Defaults include Steam, Steam's web
helper, Epic, Battle.net and Playnite for gaming; VS Code, VS Code Insiders,
Visual Studio, IntelliJ and PyCharm for coding. Add game executable names to
`gaming` to keep the expression when switching from a launcher into a game.
Re-run the installer after editing to restart with the new mappings.

Only the **foreground app** chooses the expression. Background Steam does not
override VS Code. Unlisted apps return to normal. The script checks once per
second, sends changes immediately, and sends a heartbeat every five seconds.
Feni drops PC activity after 45 seconds without a message. Calendar HTTPS sync
can briefly delay delivery. Connection animations wait for an awake buddy
screen and expire if hidden for over 30 seconds.

On firmware 3.5.4 and later, non-neutral foreground activities wake Feni and
prevent sleep. Neutral messages do not. After activity ends or expires, Feni
starts a fresh five-minute sleep countdown. Menus, clocks and alerts retain
priority. Use Studio for entertainment and custom rules; this legacy companion
sends foreground-only gaming/coding categories.

## Connection and privacy

The script reads the foreground process name locally and sends only
`activity=0` (neutral), `1` (gaming), or `2` (coding) to Feni's `/pc` endpoint.
It does not read keystrokes, window titles, files or screen contents. Requests
use Feni's session token, refreshed after a reboot or connection failure.
This is local HTTP, not an internet-facing authentication service. Destinations
are restricted to `feni.local` or private IPv4 addresses; redirects are disabled.

`%LOCALAPPDATA%\FeniPcCompanion\status.json` shows connection success, activity
category and last update time. The companion retries when Feni is offline.
A single-instance guard prevents duplicate loops.

## Stop or remove

Delete **Feni PC Companion.lnk** from the folder opened by `shell:startup` to
disable automatic startup. To stop the running installed companion:

```powershell
$feniScript = Join-Path $env:LOCALAPPDATA 'FeniPcCompanion\FeniPcCompanion.ps1'
Get-CimInstance Win32_Process | Where-Object {
    $_.Name -in @('powershell.exe','pwsh.exe') -and $_.CommandLine -and
    $_.CommandLine.Contains($feniScript)
} | ForEach-Object { Stop-Process -Id $_.ProcessId }
```

The files in `%LOCALAPPDATA%\FeniPcCompanion` can then be removed if desired.
