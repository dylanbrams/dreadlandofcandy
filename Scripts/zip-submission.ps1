<#
.SYNOPSIS
  Zip the packaged Development build the way the jam wants it: TeamName_GameName.zip, under 1 GB.
  Output: Builds\<TeamName>_<GameName>.zip. Uploading is a separate, manual step.

.EXAMPLE
  .\Scripts\package.ps1
  .\Scripts\smoke-packaged.ps1
  .\Scripts\zip-submission.ps1 -TeamName DylanBrams -GameName MyGame
#>
param(
    [Parameter(Mandatory = $true)][string]$TeamName,
    [Parameter(Mandatory = $true)][string]$GameName,
    [ValidateSet("Development", "Shipping")][string]$Config = "Development"
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "_project.ps1")
foreach ($N in @($TeamName, $GameName)) { if ($N -notmatch '^[A-Za-z0-9-]+$') { throw "'$N': use letters, digits and dashes only (the underscore separates team and game)." } }

$BuildDir = Join-Path $ProjectDir "Builds\Win64-$Config"
# package.ps1 renames the root launcher to the game's title; prefer it over the inner target binary.
$Exe = Get-ChildItem -Path $BuildDir -Filter "TheDreadLandOfCandy.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $Exe) { $Exe = Get-ChildItem -Path $BuildDir -Filter "$ProjectName.exe" -Recurse -ErrorAction SilentlyContinue |
       Sort-Object { $_.FullName.Length } | Select-Object -First 1 }
if (-not $Exe) { throw "No packaged build in $BuildDir. Run .\Scripts\package.ps1 first." }
if ($Config -ne "Development") { Write-Host "Warning: the jam asks for a Development build." -ForegroundColor Yellow }

$Zip = Join-Path $ProjectDir "Builds\${TeamName}_${GameName}.zip"
if (Test-Path $Zip) { Remove-Item $Zip -Force }
Write-Host "== Zipping $($Exe.Directory.FullName) -> $Zip ==" -ForegroundColor Cyan
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
# Debug symbols (*.pdb, about 350 MB) are not needed to run the game and would push the zip toward the 1 GB limit.
$Root = $Exe.Directory.FullName.TrimEnd('\') + '\'
$Skipped = 0
$Archive = [System.IO.Compression.ZipFile]::Open($Zip, [System.IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($F in Get-ChildItem -Path $Root -Recurse -File) {
        if ($F.Extension -eq '.pdb') { $Skipped += $F.Length; continue }
        $Entry = $F.FullName.Substring($Root.Length).Replace('\', '/')
        [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($Archive, $F.FullName, $Entry, [System.IO.Compression.CompressionLevel]::Optimal)
    }
} finally { $Archive.Dispose() }
Write-Host ("Left out debug symbols: {0:N0} MB" -f ($Skipped / 1MB)) -ForegroundColor DarkGray

$SizeMB = [Math]::Round((Get-Item $Zip).Length / 1MB, 1)
if ($SizeMB -ge 1024) { Write-Host "FAIL: $SizeMB MB is over the jam's 1 GB limit." -ForegroundColor Red; exit 1 }
Write-Host "$Zip ($SizeMB MB, limit 1024 MB)" -ForegroundColor Green
exit 0
