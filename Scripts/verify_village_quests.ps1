<#
.SYNOPSIS
  Verify village arrival/UI, all ten quest routes, and optional backend branching on the terrain map.
.EXAMPLE
  .\Scripts\verify_village_quests.ps1 -Mode backend
  .\Scripts\verify_village_quests.ps1 -Mode fallback
  .\Scripts\verify_village_quests.ps1 -Mode choices
#>
param(
    [ValidateSet('arrival','journal','choices','offline','backend','fallback')]
    [string]$Mode = 'offline',
    [int]$Port = 8019,
    [int]$TimeoutSeconds = 240,
    [string]$EnginePath
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_project.ps1')
$LogDir = Join-Path $ProjectDir 'Saved\Automation'
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogPath = Join-Path $LogDir "Village-$Mode.log"
$VillageArgs = @("`"$Project`"", '-game', '-unattended', '-nosplash', '-nosound', '-ExecCmds="t.IdleWhenNotForeground 0"')
$Capture = $Mode -in @('arrival','journal','choices')
if ($Capture) {
    $VillageArgs += @('-notitle','-nonotes','-windowed','-ResX=1280','-ResY=800','-shotat=8',"-shotname=village-$Mode",'-quitat=11')
    if ($Mode -eq 'journal') { $VillageArgs += '-uiscreen=quests' }
    if ($Mode -eq 'choices') { $VillageArgs += '-uiscreen=questbranches' }
} else {
    $VillageArgs += @('-autoplay','-autovillage','-quitaftervillage','-timescale=4','-quitat=1800','-NullRHI')
    if ($Mode -eq 'offline') { $VillageArgs += '-nonotes' }
}
$Stub = $null; $Game = $null
try {
    if ($Mode -in @('backend','fallback')) {
        $Status = if ($Mode -eq 'fallback') { '503' } else { '200' }
        $StubArgs = @('-I', "`"$(Join-Path $PSScriptRoot 'note-stub.py')`"", '--port', $Port, '--delay', '0.05', '--status', $Status)
        $Stub = Start-Process -FilePath 'python' -ArgumentList $StubArgs -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $LogDir "Village-$Mode-stub.log") -RedirectStandardError (Join-Path $LogDir "Village-$Mode-stub-error.log")
        Start-Sleep -Seconds 1
        if ($Stub.HasExited) { throw 'The local branch stub failed to start; check its log or choose another port.' }
        $VillageArgs += @('-autobranches', "-noteurl=http://127.0.0.1:$Port")
    }
    $VillageArgs += "-abslog=`"$LogPath`""
    $Exe = Join-Path $EnginePath $(if ($Capture) { 'Engine\Binaries\Win64\UnrealEditor.exe' } else { 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe' })
    $Game = Start-Process -FilePath $Exe -ArgumentList $VillageArgs -WindowStyle Hidden -PassThru
    $Deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    while (-not $Game.WaitForExit(1000)) {
        if ([DateTime]::UtcNow -gt $Deadline) { throw "Game exceeded $TimeoutSeconds seconds; see $LogPath" }
    }
    $Game.Refresh()
    if ($Game.ExitCode -ne 0) { throw "Game exited with $($Game.ExitCode); see $LogPath" }
    $Log = Get-Content -LiteralPath $LogPath -Raw
    if ($Log -match 'Fatal error|Assertion failed|Critical error|JamAutoplay: FAIL|JamVillageAutoplay: FAIL') { throw "Game failure; see $LogPath" }
    if (-not $Capture -and $Log -notmatch 'JamAutoplay: PASS scenes=3') { throw 'Main quests did not finish.' }
    if (-not $Capture -and $Log -notmatch 'JamVillageAutoplay: PASS main=3 side=10') { throw 'All ten village quests did not finish.' }
    if ($Mode -eq 'backend' -and ([regex]::Matches($Log, 'backend branches ready')).Count -ne 10) { throw 'Expected ten successful backend branch offers.' }
    if ($Mode -eq 'fallback' -and ([regex]::Matches($Log, 'authored branches retained')).Count -ne 10) { throw 'Expected ten backend failures retaining authored routes.' }
    if ($Capture) {
        $Shot = Get-ChildItem -LiteralPath (Join-Path $ProjectDir 'Saved\Screenshots') -Filter "village-$Mode.png" -Recurse | Select-Object -First 1
        if (-not $Shot) { throw 'No screenshot was written.' }
        Write-Output $Shot.FullName
    }
    Write-Output "PASS $Mode : $LogPath"
} finally {
    if ($Game -and -not $Game.HasExited) { Stop-Process -Id $Game.Id }
    if ($Stub -and -not $Stub.HasExited) { Stop-Process -Id $Stub.Id }
}
