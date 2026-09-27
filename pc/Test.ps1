$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'FeniPcCompanion.ps1') -LibraryOnly
$mappings = Get-Content (Join-Path $PSScriptRoot 'config.example.json') -Raw | ConvertFrom-Json
foreach ($case in @(@('steam',1), @('STEAM.EXE',1), @('steamwebhelper',1), @('Code',2), @('Code.exe',2), @('chrome',0), @('',0))) {
    if ((Get-FeniActivity $case[0] $mappings) -ne $case[1]) { throw "Wrong activity for $($case[0])" }
}
foreach ($url in @('http://feni.local','http://192.168.1.10','http://10.0.0.2','http://172.16.0.2')) {
    if (-not (Test-FeniUrl $url)) { throw "Rejected LAN URL: $url" }
}
foreach ($url in @('https://feni.local','http://example.com','http://8.8.8.8','http://192.168.1.2/path','http://user:password@192.168.1.2','http://192.168.1.2:81')) {
    if (Test-FeniUrl $url) { throw "Accepted invalid URL: $url" }
}
'PASS: PC app classification and LAN destination validation'
