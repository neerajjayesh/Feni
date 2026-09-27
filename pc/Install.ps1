param([string]$Url = 'http://feni.local')
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'FeniPcCompanion.ps1') -LibraryOnly
if (-not (Test-FeniUrl $Url)) { throw 'Use http://feni.local or a private LAN IPv4 address.' }
$destination = Join-Path $env:LOCALAPPDATA 'FeniPcCompanion'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
$installedScript = Join-Path $destination 'FeniPcCompanion.ps1'
Get-CimInstance Win32_Process | Where-Object {
    $_.Name -in @('powershell.exe','pwsh.exe') -and $_.CommandLine -and $_.CommandLine.Contains($installedScript)
} | ForEach-Object { Stop-Process -Id $_.ProcessId -ErrorAction SilentlyContinue }
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'FeniPcCompanion.ps1') -Destination $installedScript -Force
$configPath = Join-Path $destination 'config.json'
$config = if (Test-Path -LiteralPath $configPath) { Get-Content -LiteralPath $configPath -Raw | ConvertFrom-Json } else { Get-Content -LiteralPath (Join-Path $PSScriptRoot 'config.example.json') -Raw | ConvertFrom-Json }
$config.url = $Url
$config | ConvertTo-Json | Set-Content -LiteralPath $configPath -Encoding UTF8
$powershell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
$arguments = '-NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File "' + $installedScript + '"'
$shell = New-Object -ComObject WScript.Shell
$shortcutPath = Join-Path ([Environment]::GetFolderPath('Startup')) 'Feni PC Companion.lnk'
$shortcut = $shell.CreateShortcut($shortcutPath)
$shortcut.TargetPath = $powershell; $shortcut.Arguments = $arguments
$shortcut.WorkingDirectory = $destination; $shortcut.WindowStyle = 7; $shortcut.Save()
Start-Process -FilePath $powershell -ArgumentList $arguments -WindowStyle Hidden
Write-Output "Installed and started. App mappings: $configPath"
Write-Output 'The companion also starts at Windows sign-in.'
