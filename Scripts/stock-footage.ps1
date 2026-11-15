<#
.SYNOPSIS
  Launch the stock-footage tour: no HUD, no hero, a camera slowly flying a loop through the whole open world.

.DESCRIPTION
  Starts the project in the AJamTourGameMode. Esc quits. Capture it with OBS or the editor's own recorder.
  By default runs the editor build (-game); -Packaged runs the packaged exe in Builds\Win64-Development instead.

.EXAMPLE
  .\Scripts\stock-footage.ps1
  .\Scripts\stock-footage.ps1 -Packaged -Speed 450 -Height 500 -Minutes 10
  .\Scripts\stock-footage.ps1 -Start 0.5 -ResX 3840 -ResY 2160
#>
param(
    [float]$Speed = 650,       # uu/s along the loop (slower near places)
    [float]$Height = 380,      # uu above the highest ground just ahead
    [float]$Start = 0,         # 0..1 of the loop to start from
    [float]$Minutes = 0,       # quit after this long (0 = until Esc)
    [int]$ResX = 1920,
    [int]$ResY = 1080,
    [switch]$Fullscreen,
    [switch]$Packaged,
    [string[]]$Extra = @(),
    [string]$EnginePath
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "_project.ps1")

$Map = "/Game/World/L_Sugarborough?game=/Script/MegaJam.JamTourGameMode"
$Flags = @("-tourspeed=$Speed", "-tourheight=$Height", "-tourstart=$Start", "-ResX=$ResX", "-ResY=$ResY", "-nosplash")
if ($Minutes -gt 0) { $Flags += "-tourtime=$([int]($Minutes * 60))" }
$Flags += $(if ($Fullscreen) { "-fullscreen" } else { "-windowed" })

if ($Packaged) {
    $Exe = Get-ChildItem -Path (Join-Path $ProjectDir "Builds\Win64-Development") -Filter "*.exe" -Recurse |
        Where-Object { $_.FullName -notmatch "Binaries\\Win64" -and $_.Name -notmatch "Crash|Prereq" } | Select-Object -First 1
    if (-not $Exe) { throw "No packaged exe under Builds\Win64-Development; run Scripts\package.ps1 first" }
    Write-Host "Tour (packaged): $($Exe.FullName)" -ForegroundColor Cyan
    Start-Process -FilePath $Exe.FullName -ArgumentList (@($Map) + $Flags + $Extra) | Out-Null
} else {
    Write-Host "Tour (editor -game). Esc quits." -ForegroundColor Cyan
    $ArgList = @("`"$Project`"", $Map, "-game") + $Flags + $Extra
    Start-Process -FilePath "$EnginePath\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList $ArgList | Out-Null
}
