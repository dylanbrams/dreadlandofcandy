<#
.SYNOPSIS
  Build, cook, stage and package Win64 with BuildCookRun. Output: Builds\Win64-<Config>\ (git-ignored).
  The jam wants a Development build, so that is the default.

.EXAMPLE
  .\Scripts\package.ps1
  .\Scripts\package.ps1 -Config Shipping
#>
[CmdletBinding()]
param(
    [ValidateSet("Development", "Shipping", "DebugGame")]
    [string]$Config = "Development",
    [string]$EnginePath,
    [string]$ArchiveDir
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "_project.ps1")
$RunUAT = Join-Path $EnginePath "Engine\Build\BatchFiles\RunUAT.bat"
if (-not (Test-Path $RunUAT)) { throw "RunUAT.bat not found under $EnginePath (pass -EnginePath or set UNREAL_ENGINE_ROOT)" }

if (-not $ArchiveDir) { $ArchiveDir = Join-Path $ProjectDir "Builds\Win64-$Config" }
$ArchiveDir = [System.IO.Path]::GetFullPath($ArchiveDir)
New-Item -ItemType Directory -Force -Path $ArchiveDir | Out-Null

Write-Host "== Packaging $ProjectName Win64 $Config -> $ArchiveDir ==" -ForegroundColor Cyan
& $RunUAT BuildCookRun "-project=$Project" "-target=$GameTarget" -platform=Win64 "-clientconfig=$Config" `
    -build -cook -stage -package -pak -iostore -prereqs -archive "-archivedirectory=$ArchiveDir" -unattended -utf8output
if ($LASTEXITCODE -ne 0) { throw "RunUAT failed with exit code $LASTEXITCODE" }

# Give the player-facing launcher the game's title. The root exe is only a bootstrap stub that
# runs MegaJam\Binaries\Win64\..., so renaming it is safe; the inner binary keeps the target name.
$ExeName = "TheDreadLandOfCandy.exe"
$StubExe = Join-Path $ArchiveDir "$GameTarget.exe"
$NamedExe = Join-Path $ArchiveDir $ExeName
if (Test-Path $StubExe) {
    if (Test-Path $NamedExe) { Remove-Item $NamedExe -Force }
    Rename-Item $StubExe $ExeName
    Write-Host "Renamed launcher to $ExeName" -ForegroundColor Green
}
Write-Host "Packaged to $ArchiveDir" -ForegroundColor Green
