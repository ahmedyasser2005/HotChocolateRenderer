# Builds the solution from the command line (no need to open Visual Studio).
#   .\scripts\build.ps1                          Debug x64
#   .\scripts\build.ps1 -Configuration Release
#   .\scripts\build.ps1 -Rebuild
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [string]$Platform = 'x64',
    [switch]$Rebuild
)

$solutionDir = Split-Path -Parent $PSScriptRoot

$slnx = Get-ChildItem -Path $solutionDir -Filter '*.slnx' | Select-Object -First 1
if (-not $slnx)
{
    Write-Error "No .slnx file found in $solutionDir"
    exit 2
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere))
{
    Write-Error 'vswhere.exe not found. Is Visual Studio installed?'
    exit 2
}

$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (-not $msbuild)
{
    Write-Error 'MSBuild.exe not found.'
    exit 2
}

$target = if ($Rebuild) { 'Rebuild' } else { 'Build' }

Write-Host "Building $($slnx.Name) [$Configuration|$Platform] ($target)"
& $msbuild $slnx.FullName "/t:$target" "/p:Configuration=$Configuration" "/p:Platform=$Platform" /m /nologo /v:minimal
exit $LASTEXITCODE