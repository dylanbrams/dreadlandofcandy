param(
    [ValidateSet('Overview', 'Village', 'Meadow', 'Fort', 'DenseMeadow', 'DenseSnow', 'DenseGrove', 'Explore')]
    [string]$View = 'Village',
    [string]$PackagedExe,
    [switch]$MainMap,
    [switch]$StartPlaying,
    [int]$At = 15
)
$ErrorActionPreference = 'Stop'
$TerrainRoot = $PSScriptRoot
$TerrainProject = [IO.Path]::GetFullPath((Join-Path $TerrainRoot '../..'))
$TerrainLog = Join-Path $TerrainProject "Saved/Logs/Terrain-$View.log"
$TerrainShot = "terrain-$($View.ToLower())"
$TerrainMap = "/Game/World/Review/L_Terrain$View"
if ($View.StartsWith('Dense')) { $TerrainMap = "/Game/World/Review/L_$View" }
$TerrainReviewIndex = Join-Path $TerrainRoot 'Dense/review_maps.json'
if (Test-Path -LiteralPath $TerrainReviewIndex) {
    $TerrainReviewMaps = Get-Content -LiteralPath $TerrainReviewIndex -Raw | ConvertFrom-Json
    if ($TerrainReviewMaps.PSObject.Properties.Name -contains $View) { $TerrainMap = $TerrainReviewMaps.$View }
}
if ($MainMap) {
    $TerrainMap = '/Game/World/L_Sugarborough'
    $TerrainShot = 'terrain-main'
    $TerrainLog = Join-Path $TerrainProject 'Saved/Logs/Terrain-Main.log'
}
$TerrainArgs = @($TerrainMap, '-RenderOffscreen', '-windowed', '-ForceRes', '-ResX=1600', '-ResY=1000', '-ExecCmds="r.SetRes 1600x1000w"', '-nosplash', '-unattended', "-shotat=$At", "-shotname=$TerrainShot", "-quitat=$($At + 4)", "-abslog=`"$TerrainLog`"")
if ($MainMap) { $TerrainArgs += '-open' }
if ($StartPlaying) { $TerrainArgs += '-notitle' }
if ($PackagedExe) {
    $TerrainExecutable = [IO.Path]::GetFullPath($PackagedExe)
    $TerrainDirectExe = Join-Path (Split-Path $TerrainExecutable -Parent) 'MegaJam/Binaries/Win64/MegaJam.exe'
    if (Test-Path -LiteralPath $TerrainDirectExe) { $TerrainExecutable = $TerrainDirectExe }
} else {
    $TerrainExecutable = 'D:/Program Files/Epic/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe'
    $TerrainArgs = @("`"$(Join-Path $TerrainProject 'MegaJam.uproject')`"", '-game') + $TerrainArgs
}
$TerrainStarted = Get-Date
$TerrainProcess = Start-Process -FilePath $TerrainExecutable -ArgumentList $TerrainArgs -WindowStyle Hidden -PassThru -WorkingDirectory $TerrainProject
if (-not $TerrainProcess.WaitForExit(180000)) {
    $TerrainProcess.Kill()
    throw "Terrain $View review timed out: $TerrainLog"
}
if ($TerrainProcess.ExitCode -ne 0) { throw "Terrain review exited $($TerrainProcess.ExitCode): $TerrainLog" }
# AgentHooks logs the actual screenshot location, which differs in a cooked build.
$TerrainCaptureLine = Get-Content -LiteralPath $TerrainLog | Where-Object { $_ -match 'Screenshot requested: (.+\.png)' } | Select-Object -Last 1
$TerrainImage = $null
if ($TerrainCaptureLine -match 'Screenshot requested: (.+\.png)') {
    $TerrainCapturePath = $Matches[1].Trim()
    if (Test-Path -LiteralPath $TerrainCapturePath) { $TerrainImage = Get-Item -LiteralPath $TerrainCapturePath }
}
if (-not $TerrainImage) { throw "No $TerrainShot screenshot: $TerrainLog" }
if ($TerrainImage.LastWriteTime -lt $TerrainStarted) { throw "Stale $TerrainShot screenshot: $TerrainLog" }
$TerrainExpectedMap = ($TerrainMap -split '/')[-1]
if (-not (Select-String -LiteralPath $TerrainLog -SimpleMatch "World began play: $TerrainExpectedMap" -Quiet)) { throw "Wrong map loaded: $TerrainLog" }
if (Select-String -LiteralPath $TerrainLog -Pattern 'Fatal error|LogLoad: Error:|CDO Constructor.*Failed to find' -Quiet) { throw "Terrain runtime errors: $TerrainLog" }
Write-Output $TerrainImage.FullName
