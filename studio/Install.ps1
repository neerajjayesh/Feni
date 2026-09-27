param([switch]$NoStartup, [switch]$NoLaunch)
$ErrorActionPreference = 'Stop'
$studioRoot = Join-Path $env:LOCALAPPDATA 'FeniStudio'
$studioApp = Join-Path $studioRoot 'app'
$studioExe = Join-Path $studioApp 'Feni Studio.exe'
$studioBuild = Join-Path $PSScriptRoot 'bin\install'
# Build before stopping the previous companion, so build failures leave it running.
& (Join-Path $PSScriptRoot 'Build.ps1') -OutputDirectory $studioBuild
if (-not (Test-Path -LiteralPath (Join-Path $studioBuild 'Feni Studio.exe'))) { throw 'Missing application build.' }
foreach ($studioProcess in Get-Process -Name 'Feni Studio' -ErrorAction SilentlyContinue) {
    if ($studioProcess.Path -eq $studioExe -or $studioProcess.Path -eq (Join-Path $PSScriptRoot 'bin\Feni Studio.exe')) { Stop-Process -Id $studioProcess.Id }
}
New-Item -ItemType Directory -Force -Path $studioApp | Out-Null
Copy-Item -LiteralPath (Join-Path $studioBuild 'Feni Studio.exe'), (Join-Path $studioBuild 'Code guide.txt') -Destination $studioApp -Force
$studioFirmware = Join-Path $studioApp 'firmware'
New-Item -ItemType Directory -Force -Path $studioFirmware | Out-Null
Copy-Item -LiteralPath (Join-Path (Split-Path $PSScriptRoot -Parent) 'FeniBuddy') -Destination $studioFirmware -Recurse -Force
$studioOldGuide = Join-Path $studioApp 'Animation format.txt'
if (Test-Path -LiteralPath $studioOldGuide) { Remove-Item -LiteralPath $studioOldGuide }
$studioStartup = [Environment]::GetFolderPath('Startup')
$studioLegacy = Join-Path $env:LOCALAPPDATA 'FeniPcCompanion\FeniPcCompanion.ps1'
foreach ($studioProcess in Get-CimInstance Win32_Process -Filter "Name = 'powershell.exe' OR Name = 'pwsh.exe'") {
    if ($studioProcess.CommandLine -and $studioProcess.CommandLine.Contains($studioLegacy)) { Stop-Process -Id $studioProcess.ProcessId -ErrorAction SilentlyContinue }
}
$studioLegacyLink = Join-Path $studioStartup 'Feni PC Companion.lnk'
if (Test-Path -LiteralPath $studioLegacyLink) {
    Copy-Item -LiteralPath $studioLegacyLink -Destination (Join-Path $studioRoot 'Previous PC Companion.lnk') -Force
    Remove-Item -LiteralPath $studioLegacyLink
}
$studioShell = New-Object -ComObject WScript.Shell
foreach ($studioLocation in @([Environment]::GetFolderPath('Desktop'), [Environment]::GetFolderPath('Programs'))) {
    $studioLink = $studioShell.CreateShortcut((Join-Path $studioLocation 'Feni Studio.lnk'))
    $studioLink.TargetPath = $studioExe
    $studioLink.WorkingDirectory = $studioApp
    $studioLink.Description = 'Configure Feni animations and application reactions'
    $studioLink.Save()
}
$studioStartupLink = Join-Path $studioStartup 'Feni Studio.lnk'
if (-not $NoStartup) {
    $studioLink = $studioShell.CreateShortcut($studioStartupLink)
    $studioLink.TargetPath = $studioExe
    $studioLink.Arguments = '--tray'
    $studioLink.WorkingDirectory = $studioApp
    $studioLink.Save()
} elseif (Test-Path -LiteralPath $studioStartupLink) { Remove-Item -LiteralPath $studioStartupLink }
if (-not $NoLaunch) { Start-Process -FilePath $studioExe }
Write-Output "Installed: $studioExe"
