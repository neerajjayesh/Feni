param(
    [string]$ZigPath = '',
    [string]$ArduinoJsonPath = (Join-Path $env:USERPROFILE 'Documents\Arduino\libraries\ArduinoJson\src')
)
$ErrorActionPreference = 'Stop'
$buddyRoot = Split-Path $PSScriptRoot -Parent
if (-not $ZigPath) { $ZigPath = Join-Path $buddyRoot '.tools\buddy-test-compiler\ziglang\zig.exe' }
if (-not (Test-Path -LiteralPath $ZigPath)) { throw 'Set -ZigPath to zig.exe, or install the test compiler as described in README.md.' }
if (-not (Test-Path -LiteralPath $ArduinoJsonPath)) { throw 'Set -ArduinoJsonPath to the ArduinoJson src directory.' }
# Keep test executables outside the Arduino build directory, which the CLI cleans.
$buddyTests = Join-Path $buddyRoot 'runtime\buddy-tests'
New-Item -ItemType Directory -Force -Path $buddyTests | Out-Null
foreach ($buddyCase in @('core', 'wled', 'settings', 'face', 'body')) {
    $buddySource = Join-Path $PSScriptRoot ('tests\' + $buddyCase + '_test.cpp')
    $buddyExecutable = Join-Path $buddyTests ($buddyCase + '-tests.exe')
    & $ZigPath c++ -std=c++11 -O1 -I $ArduinoJsonPath $buddySource -o $buddyExecutable
    if ($LASTEXITCODE -ne 0) { throw "Could not compile $buddyCase tests." }
    & $buddyExecutable
    if ($LASTEXITCODE -ne 0) { throw "$buddyCase tests failed." }
}
& node (Join-Path $PSScriptRoot 'tests\calendar_test.cjs')
if ($LASTEXITCODE -ne 0) { throw 'Calendar/browser tests failed.' }
