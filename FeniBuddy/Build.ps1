param([switch]$Upload, [string]$Port = 'COM8')
$ErrorActionPreference = 'Stop'
$buddyCliCommand = Get-Command arduino-cli -ErrorAction SilentlyContinue
$buddyCli = if ($buddyCliCommand) { $buddyCliCommand.Source } else {
    Join-Path $env:LOCALAPPDATA 'Programs\arduino-ide\resources\app\lib\backend\resources\arduino-cli.exe'
}
if (-not (Test-Path -LiteralPath $buddyCli)) { throw 'Install Arduino IDE or arduino-cli first.' }
$buddyBuild = Join-Path (Split-Path $PSScriptRoot -Parent) 'build-feni-buddy'
$buddyFqbn = 'esp8266:esp8266:nodemcuv2:mmu=4816H'
& $buddyCli compile --fqbn $buddyFqbn --build-path $buddyBuild $PSScriptRoot
if ($LASTEXITCODE -ne 0) { throw 'Firmware build failed.' }
if ($Upload) {
    & $buddyCli upload --fqbn $buddyFqbn --port $Port --input-dir $buddyBuild $PSScriptRoot
    if ($LASTEXITCODE -ne 0) { throw 'Firmware upload failed.' }
}
