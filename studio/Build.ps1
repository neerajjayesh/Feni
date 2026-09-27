param([string]$OutputDirectory=(Join-Path $PSScriptRoot 'bin'))
$ErrorActionPreference='Stop'
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$compiler=Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
if(-not (Test-Path -LiteralPath $compiler)){$compiler=Join-Path $env:WINDIR 'Microsoft.NET\Framework\v4.0.30319\csc.exe'}
$target=Join-Path $OutputDirectory 'Feni Studio.exe'
& $compiler /nologo /target:winexe /optimize+ "/out:$target" /r:System.dll /r:System.Core.dll /r:System.Drawing.dll /r:System.Windows.Forms.dll /r:System.Net.Http.dll /r:System.Web.Extensions.dll (Join-Path $PSScriptRoot 'Core.cs') (Join-Path $PSScriptRoot 'Studio.cs') (Join-Path $PSScriptRoot 'SelfTest.cs')
if($LASTEXITCODE -ne 0){throw 'Feni Studio build failed'}
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'Animation format.txt') -Destination $OutputDirectory -Force
Write-Output $target
