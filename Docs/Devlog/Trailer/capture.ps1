<#
  Records one gameplay clip from the packaged build: the game window only (gdigrab title=), never the desktop.
  Video only; trailer music and Fenella lines go on in the edit.
  .\Docs\Devlog\Trailer\capture.ps1 -Name knight-fight -Seconds 40 -Skip 12 -Extra "-autoplay","-class=knight"
  Clips land in Docs\Devlog\Video\raw\broll\ (git-ignored).
#>
param(
  [Parameter(Mandatory)] [string]$Name,
  [int]$Seconds = 30,          # length recorded
  [int]$Skip = 10,             # seconds after the window appears before recording starts (loading, title)
  [string[]]$Extra = @(),
  [int]$Fps = 60
)
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path "$PSScriptRoot\..\..\..").Path
$Exe = Join-Path $Root "Builds\Win64-Development\TheDreadLandOfCandy.exe"
$OutDir = Join-Path $Root "Docs\Devlog\Video\raw\broll"
New-Item -ItemType Directory -Force $OutDir | Out-Null
$Out = Join-Path $OutDir "$Name.mp4"

$Args = @("-windowed", "-ResX=1920", "-ResY=1080", "-nomusic", "-nonotes") + $Extra
Write-Host "Launching: $($Args -join ' ')"
Start-Process -FilePath $Exe -ArgumentList $Args | Out-Null

# The root exe is a shim; find the real game window by process name.
$Win = $null
for ($i = 0; $i -lt 90 -and -not $Win; $i++) {
  Start-Sleep -Seconds 1
  $Win = Get-Process | Where-Object { $_.ProcessName -like "TheDreadLandOfCandy*" -and $_.MainWindowTitle } | Select-Object -First 1
}
if (-not $Win) { throw "No game window after 90 s" }
Write-Host "Window: '$($Win.MainWindowTitle)' (pid $($Win.Id)); recording in $Skip s"
Start-Sleep -Seconds $Skip

& ffmpeg -hide_banner -loglevel warning -y -f gdigrab -framerate $Fps -draw_mouse 0 -i "title=$($Win.MainWindowTitle)" `
  -t $Seconds -c:v h264_nvenc -preset p5 -cq 19 -pix_fmt yuv420p $Out

Get-Process | Where-Object { $_.ProcessName -like "TheDreadLandOfCandy*" } | Stop-Process -Force -Confirm:$false
Write-Host "Saved $Out"
