<# Capture isolated Niagara presets through the existing in-game screenshot hook.
   -Attack B02 captures one; omit to capture all twelve. Uses offscreen rendering.
#>
param([string]$Attack = '', [string]$EnginePath)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_project.ps1')
$reviewEntries = Get-Content -LiteralPath (Join-Path $ProjectDir 'ImportSource/FX/review_maps.json') -Raw | ConvertFrom-Json
$reviewRequest = Join-Path $ProjectDir 'Saved/FX/render_request.json'
@{ attack = $Attack } | ConvertTo-Json | Set-Content -LiteralPath $reviewRequest -Encoding UTF8
$reviewScript = Join-Path $ProjectDir 'Scripts/render_fx_review.py'
$reviewLog = Join-Path $ProjectDir 'Saved/FX/render.log'
$reviewLaunchLog = Join-Path $ProjectDir 'Saved/FX/render-launch.log'
$reviewError = Join-Path $ProjectDir 'Saved/FX/render_error.txt'
if (Test-Path -LiteralPath $reviewError) { Remove-Item -LiteralPath $reviewError }
& "$EnginePath/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" $Project "-ExecutePythonScript=$reviewScript" -RenderOffscreen -unattended -nosound -nop4 -NoSplash "-abslog=$reviewLog" *> $reviewLaunchLog
if ($LASTEXITCODE -ne 0 -or (Test-Path -LiteralPath $reviewError)) { throw "FX render failed. See Saved/FX/render_error.txt and $reviewLog" }
foreach ($reviewEntry in $reviewEntries) {
    $reviewName = ($reviewEntry.system -split '\.')[-1]
    $reviewId = ($reviewName -split '_')[1]
    if ($Attack -and $reviewId -ne $Attack) { continue }
    $reviewShot = "fx-$($reviewId.ToLower())"
    $reviewImage = Join-Path $ProjectDir "Saved/Screenshots/WindowsEditor/$reviewShot.png"
    if (-not (Test-Path -LiteralPath $reviewImage)) { throw "Missing review screenshot: $reviewImage" }
    Write-Host "$reviewId -> $reviewImage"
}
