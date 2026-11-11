<#
.SYNOPSIS
  Run the game (editor build, -game) and save one in-engine screenshot to Saved\Screenshots.

.DESCRIPTION
  Uses the agent hooks: -shotat/-shotname take the screenshot, -quitat closes the game.
  -Extra passes game flags through (add -autoplay, -uiscreen=... once the game has them).

.EXAMPLE
  .\Scripts\screenshot.ps1
  .\Scripts\screenshot.ps1 -ResX 1024 -ResY 768 -At 10 -Name menu
  .\Scripts\screenshot.ps1 -Extra "-autoplay"
#>
param(
    [int]$ResX = 1280,
    [int]$ResY = 800,
    [float]$At = 6,
    [string]$Name = "",
    [string[]]$Extra = @(),
    [string]$EnginePath
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "_project.ps1")
if (-not $Name) { $Name = "shot-${ResX}x${ResY}" }
$Log = Join-Path $ProjectDir "Saved\Logs\Screenshot.log"

$ArgList = @("`"$Project`"", "-game", "-windowed", "-ResX=$ResX", "-ResY=$ResY", "-shotat=$At", "-shotname=$Name", "-quitat=$($At + 3)", "-nosplash", "-log", "-abslog=`"$Log`"") + $Extra

Write-Host "== $Name ==" -ForegroundColor Cyan
$P = Start-Process -FilePath "$EnginePath\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList $ArgList -PassThru
if (-not $P.WaitForExit(([int]$At + 120) * 1000)) { $P.Kill(); throw "The game did not quit; see $Log" }
$Shot = Get-ChildItem -Path (Join-Path $ProjectDir "Saved\Screenshots") -Filter "$Name.png" -Recurse -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $Shot) { throw "No screenshot $Name.png; see $Log" }
Write-Host $Shot.FullName -ForegroundColor Green
