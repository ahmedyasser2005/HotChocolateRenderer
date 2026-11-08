param([switch]$DryRun)

$solutionDir = Split-Path -Parent $PSScriptRoot

$targets = @()

# Solution-level build folder
$targets += Join-Path $solutionDir 'build'

# Every HCR.* project: its build and build-int folders
Get-ChildItem -Path $solutionDir -Directory -Filter 'HCR.*' | ForEach-Object {
    $targets += Join-Path $_.FullName 'build'
    $targets += Join-Path $_.FullName 'build-int'
}

foreach ($path in $targets) {
    if (Test-Path -LiteralPath $path -PathType Container) {
        Write-Host "Removing $path"
        if (-not $DryRun) { Remove-Item -LiteralPath $path -Recurse -Force }
    }
}

Write-Host "Done.$(if ($DryRun) { ' (dry run, nothing was deleted)' })"