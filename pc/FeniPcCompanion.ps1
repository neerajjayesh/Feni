param([string]$ConfigPath = (Join-Path $PSScriptRoot 'config.json'), [switch]$Once, [switch]$LibraryOnly)
$ErrorActionPreference = 'Stop'

function Get-FeniActivity([string]$ProcessName, $Mappings) {
    $name = $ProcessName.ToLowerInvariant() -replace '\.exe$', ''
    if (@($Mappings.coding | ForEach-Object { $_.ToLowerInvariant() -replace '\.exe$', '' }) -contains $name) { return 2 }
    if (@($Mappings.gaming | ForEach-Object { $_.ToLowerInvariant() -replace '\.exe$', '' }) -contains $name) { return 1 }
    return 0
}
function Test-FeniUrl([string]$Value) {
    try { $uri = [uri]$Value } catch { return $false }
    if (-not $uri.IsAbsoluteUri -or $uri.Scheme -ne 'http' -or $uri.UserInfo -or $uri.Port -ne 80 -or $uri.Query -or $uri.Fragment -or $uri.AbsolutePath -ne '/') { return $false }
    if ($uri.Host -eq 'feni.local') { return $true }
    $address = $null
    if (-not [Net.IPAddress]::TryParse($uri.Host, [ref]$address) -or $address.AddressFamily -ne [Net.Sockets.AddressFamily]::InterNetwork) { return $false }
    $bytes = $address.GetAddressBytes()
    return $bytes[0] -eq 10 -or ($bytes[0] -eq 192 -and $bytes[1] -eq 168) -or ($bytes[0] -eq 172 -and $bytes[1] -ge 16 -and $bytes[1] -le 31)
}
function Invoke-FeniRequest([string]$Url, [string]$Token = '', [string]$Body = '') {
    $request = [Net.HttpWebRequest]::Create($Url)
    $request.Proxy = $null; $request.Timeout = 6000; $request.ReadWriteTimeout = 6000
    $request.AllowAutoRedirect = $false; $request.KeepAlive = $false
    if ($Body) {
        $request.Method = 'POST'; $request.ContentType = 'application/x-www-form-urlencoded'
        $request.Headers.Add('X-Feni-Token', $Token)
        $bytes = [Text.Encoding]::ASCII.GetBytes($Body); $request.ContentLength = $bytes.Length
        $stream = $request.GetRequestStream()
        try { $stream.Write($bytes, 0, $bytes.Length) } finally { $stream.Dispose() }
    }
    $response = $request.GetResponse()
    try {
        if ([int]$response.StatusCode -ne 200) { throw 'Unexpected HTTP response' }
        $reader = New-Object IO.StreamReader($response.GetResponseStream())
        try { return $reader.ReadToEnd() } finally { $reader.Dispose() }
    } finally { $response.Dispose() }
}
if ($LibraryOnly) { return }
$config = Get-Content -LiteralPath $ConfigPath -Raw | ConvertFrom-Json
if (-not (Test-FeniUrl $config.url)) { throw 'Use http://feni.local or a private LAN IPv4 address.' }
$baseUrl = $config.url.TrimEnd('/')
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class FeniForeground {
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr window, out uint processId);
}
'@
$mutex = New-Object Threading.Mutex($false, 'Local\FeniPcCompanion')
if (-not $mutex.WaitOne(0)) { $mutex.Dispose(); return }
$token = ''; $previous = -1; $sentAt = [datetime]::MinValue
$statusPath = Join-Path (Split-Path -Parent $ConfigPath) 'status.json'
try {
    do {
        $activity = 0
        try {
            [uint32]$foregroundId = 0
            [void][FeniForeground]::GetWindowThreadProcessId([FeniForeground]::GetForegroundWindow(), [ref]$foregroundId)
            if ($foregroundId) { $activity = Get-FeniActivity (Get-Process -Id $foregroundId -ErrorAction Stop).ProcessName $config }
        } catch { $activity = 0 }
        if ($Once -or $activity -ne $previous -or ([datetime]::UtcNow - $sentAt).TotalSeconds -ge 5) {
            $connected = $false
            try {
                if (-not $token) {
                    $token = (Invoke-FeniRequest "$baseUrl/session").Trim()
                    if ($token -notmatch '^[0-9a-f]{32}$') { throw 'Not a Feni session' }
                }
                $reply = Invoke-FeniRequest "$baseUrl/pc" $token "activity=$activity"
                if ($reply.Trim() -ne 'OK') { throw 'Not a Feni PC response' }
                $connected = $true; $previous = $activity
            } catch { $token = ''; $previous = -1 }
            $sentAt = [datetime]::UtcNow
            @{ connected = $connected; activity = $activity; updatedUtc = $sentAt.ToString('o') } | ConvertTo-Json | Set-Content -LiteralPath $statusPath -Encoding UTF8
            if (-not $connected -and -not $Once) { Start-Sleep -Seconds 5 }
        }
        if (-not $Once) { Start-Sleep -Seconds 1 }
    } while (-not $Once)
} finally { $mutex.ReleaseMutex(); $mutex.Dispose() }
