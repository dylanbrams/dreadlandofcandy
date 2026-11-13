param([string]$View = 'DenseGrove', [switch]$Baseline, [int]$At = 12, [int]$CaptureFrames = 600)
$ErrorActionPreference = 'Stop'
$terrainPolishProject = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../..'))
$terrainPolishIndex = if ($Baseline) { 'baseline_review_maps.json' } else { 'review_maps.json' }
$terrainPolishMaps = Get-Content -LiteralPath (Join-Path $PSScriptRoot $terrainPolishIndex) -Raw | ConvertFrom-Json
$terrainPolishMap = $terrainPolishMaps.$View
if (-not $terrainPolishMap) { throw "Missing review map: $View" }
$terrainPolishTag = if ($Baseline) { "polish-before-$View" } else { "polish-$View" }
$terrainPolishLog = Join-Path $terrainPolishProject "Saved/Logs/$terrainPolishTag.log"
$terrainPolishArgs = @("`"$terrainPolishProject/MegaJam.uproject`"", '-game', $terrainPolishMap,
    '-RenderOffscreen', '-windowed', '-ForceRes', '-ResX=1600', '-ResY=1000', '-nosplash', '-unattended',
    "-csvCaptureFrames=$CaptureFrames", '-csvGpuStats', '-novsync', '-ExecCmds="t.MaxFPS 0"',
    "-shotat=$At", "-shotname=$terrainPolishTag", "-quitat=$($At + 5)", "-abslog=`"$terrainPolishLog`"")
$terrainPolishStarted = Get-Date
$terrainPolishProcess = Start-Process -FilePath 'D:/Program Files/Epic/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe' -ArgumentList $terrainPolishArgs -WindowStyle Hidden -PassThru -WorkingDirectory $terrainPolishProject
if (-not $terrainPolishProcess.WaitForExit(240000)) { $terrainPolishProcess.Kill(); throw 'Terrain review timeout' }
if ($terrainPolishProcess.ExitCode -ne 0) { throw "Review exit $($terrainPolishProcess.ExitCode)" }
$terrainPolishText = Get-Content -LiteralPath $terrainPolishLog
if (-not ($terrainPolishText | Select-String -SimpleMatch "World began play: $(($terrainPolishMap -split '/')[-1])")) { throw 'Wrong map' }
if ($terrainPolishText | Select-String -Pattern 'Failed to compile Material|Shader error|Fatal error') { throw 'Render errors; inspect log' }
$terrainPolishLine = $terrainPolishText | Select-String -Pattern 'Screenshot requested: (.+\.png)' | Select-Object -Last 1
if (-not $terrainPolishLine) { throw 'Missing screenshot' }
$terrainPolishImage = $terrainPolishLine.Matches[0].Groups[1].Value.Trim()
if ((Get-Item -LiteralPath $terrainPolishImage).LastWriteTime -lt $terrainPolishStarted) { throw 'Stale screenshot' }
New-Item -ItemType Directory -Path (Join-Path $PSScriptRoot 'Review') -Force | Out-Null
Copy-Item -LiteralPath $terrainPolishImage -Destination (Join-Path $PSScriptRoot "Review/$terrainPolishTag.png")
Write-Output $terrainPolishImage
