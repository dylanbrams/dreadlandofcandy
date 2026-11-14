<#
.SYNOPSIS
  Smoke-test the PACKAGED build (the thing that gets submitted, not the editor).

.DESCRIPTION
  Launches the packaged exe, waits, checks it is still running and that its log shows
  what only breaks once a build is cooked. Add a check to $Checks for each such failure
  as it is found (missing data, missing widget classes...). -Extra passes game flags
  (e.g. -autoplay once the game has it).

  The exe at the build root is a launcher shim that starts the real binary from
  <Project>\Binaries\Win64 and exits, so the real process is tracked by path.

.EXAMPLE
  .\Scripts\smoke-packaged.ps1
  .\Scripts\smoke-packaged.ps1 -Extra "-autoplay" -RunSeconds 60
  .\Scripts\smoke-packaged.ps1 -Map /Game/World/L_Sugarborough -Extra "-open","-autoplay","-timescale=4"
#>
[CmdletBinding()]
param(
    [ValidateSet("Development", "Shipping", "DebugGame")]
    [string]$Config = "Development",
    [int]$RunSeconds = 60,   # a full -autoplay -timescale=4 film needs about 30 s
    [string[]]$Extra = @(),
    [string]$Map,            # e.g. /Game/World/L_Sugarborough; must be the first argument, so it gets its own parameter
    [switch]$Play
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "_project.ps1")
$BuildDir = Join-Path $ProjectDir "Builds\Win64-$Config"

# package.ps1 renames the root launcher to the game's title; prefer it over the inner target binary.
$Exe = Get-ChildItem -Path $BuildDir -Filter "TheDreadLandOfCandy.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $Exe) { $Exe = Get-ChildItem -Path $BuildDir -Filter "$ProjectName.exe" -Recurse -ErrorAction SilentlyContinue |
       Sort-Object { $_.FullName.Length } | Select-Object -First 1 }
if (-not $Exe) { throw "No packaged $ProjectName.exe under $BuildDir. Run .\Scripts\package.ps1 -Config $Config first." }
$BinDir = Join-Path $Exe.Directory.FullName "$ProjectName\Binaries\Win64"

$LogDir = Join-Path $ProjectDir "Saved\Automation"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogPath = Join-Path $LogDir "Packaged-Smoke.log"
if (Test-Path $LogPath) { Remove-Item $LogPath -Force }

function Get-GameProcess {
    Get-Process -ErrorAction SilentlyContinue | Where-Object {
        $p = $null; try { $p = $_.Path } catch { }
        $p -and $p.StartsWith($BinDir, [StringComparison]::OrdinalIgnoreCase)
    }
}

Write-Host "== Smoke-testing $($Exe.FullName) ==" -ForegroundColor Cyan
$Launcher = Start-Process -FilePath $Exe.FullName -ArgumentList (@($Map | Where-Object { $_ }) + @("-log", "-abslog=`"$LogPath`"", "-windowed", "-ResX=1280", "-ResY=800") + $Extra) -PassThru

$Game = $null; $Waited = 0
while (-not $Game -and $Waited -lt 30) { Start-Sleep -Seconds 1; $Waited++; $Game = Get-GameProcess | Select-Object -First 1 }
if (-not $Game) { Write-Host "FAIL: no game process appeared under $BinDir within $Waited s." -ForegroundColor Red; exit 1 }
Write-Host ("   game up after {0}s (pid {1})" -f $Waited, $Game.Id) -ForegroundColor DarkGray

Start-Sleep -Seconds ([Math]::Max(0, $RunSeconds - $Waited))
$Game.Refresh()
$Survived = -not $Game.HasExited
$ExitCode = if ($Survived) { $null } else { $Game.ExitCode }
if (-not $Play) {
    foreach ($P in (Get-GameProcess)) { try { $P.Kill() } catch { } }
    try { if (-not $Launcher.HasExited) { $Launcher.Kill() } } catch { }
}
if (-not $Survived) { Write-Host "FAIL: the build exited on its own within $RunSeconds s (code $ExitCode)." -ForegroundColor Red; exit 1 }

if ($Config -eq "Shipping") {
    Write-Host "PASS (liveness only): Shipping has no log; the build was still running after $RunSeconds s." -ForegroundColor Yellow
    exit 0
}
Start-Sleep -Seconds 1
if (-not (Test-Path $LogPath)) { Write-Host "FAIL: the build produced no log at $LogPath" -ForegroundColor Red; exit 1 }
$Log = Get-Content $LogPath -Raw

$Checks = @(
    @{ Name = "game module loaded"; Pattern = "LogJam: MegaJam module started"; Want = $true },
    @{ Name = "world began play";   Pattern = "LogJam: World began play"; Want = $true },
    @{ Name = "no fatal errors";    Pattern = "Fatal error|LogWindows: Error: === Critical error"; Want = $false },
    @{ Name = "no missing widget classes"; Pattern = "widget class '[^']+' could not be loaded"; Want = $false },
    # ConstructorHelpers paths (e.g. /Engine/BasicShapes) that were not cooked.
    @{ Name = "no missing constructor assets"; Pattern = "CDO Constructor.*Failed to find"; Want = $false }
)
if ($Extra -contains "-autoplay") {
    $Checks += @{ Name = "autoplay passed"; Pattern = "JamAutoplay: PASS"; Want = $true }
    $Checks += @{ Name = "autoplay did not fail"; Pattern = "JamAutoplay: FAIL"; Want = $false }
}
if ($Extra -contains "-autovillage") {
    $Checks += @{ Name = "village autoplay completed all quests"; Pattern = "JamVillageAutoplay: PASS main=3 side=10"; Want = $true }
    $Checks += @{ Name = "village autoplay did not fail"; Pattern = "JamVillageAutoplay: FAIL"; Want = $false }
}
$Failed = 0
foreach ($C in $Checks) {
    $Found = [regex]::IsMatch($Log, $C.Pattern)
    if ($Found -eq $C.Want) { Write-Host "  [PASS] $($C.Name)" -ForegroundColor Green }
    else { Write-Host "  [FAIL] $($C.Name)" -ForegroundColor Red; $Failed++ }
}
Write-Host "Log: $LogPath"
if ($Failed -gt 0) { Write-Host "$Failed check(s) failed." -ForegroundColor Red; exit 1 }
Write-Host "Packaged build looks good." -ForegroundColor Green
exit 0
