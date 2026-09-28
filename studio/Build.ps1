param([string]$OutputDirectory=(Join-Path $PSScriptRoot 'bin'))
$ErrorActionPreference='Stop'
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$framework=Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319'
if(-not (Test-Path -LiteralPath (Join-Path $framework 'csc.exe'))){$framework=Join-Path $env:WINDIR 'Microsoft.NET\Framework\v4.0.30319'}
$compiler=Join-Path $framework 'csc.exe'
$target=Join-Path $OutputDirectory 'Feni Studio.exe'
$sources=@('Core.cs','Studio.cs','SelfTest.cs','CodeProject.cs','AiStatus.cs') | ForEach-Object {Join-Path $PSScriptRoot $_}
$references=@('UIAutomationClient.dll','UIAutomationTypes.dll','WindowsBase.dll') | ForEach-Object {'/r:'+(Join-Path $framework ('WPF\'+$_))}
& $compiler /nologo /target:winexe /optimize+ "/out:$target" @references /r:System.dll /r:System.Core.dll /r:System.Drawing.dll /r:System.Windows.Forms.dll /r:System.Net.Http.dll /r:System.Web.Extensions.dll @sources
if($LASTEXITCODE -ne 0){throw 'Feni Studio build failed'}
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'Code guide.txt') -Destination $OutputDirectory -Force
Write-Output $target
