<# Capture the motion pass in the real game, one hidden renderer at a time. #>
param(
    [ValidateSet('Lineup','All','Close','Final','Village','Villager','Rats','Goblins')][string]$View = 'Village',
    [float]$At = 8,
    [string]$Name = 'npc-motion',
    [string]$EnginePath
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_project.ps1')
if ($View -eq 'All') {
    & $PSCommandPath -View Village -At 9 -Name "$Name-village-b" -EnginePath $EnginePath
    & $PSCommandPath -View Villager -At 8 -Name "$Name-villager-a" -EnginePath $EnginePath
    & $PSCommandPath -View Villager -At 9 -Name "$Name-villager-b" -EnginePath $EnginePath
    & $PSCommandPath -View Rats -At 36 -Name "$Name-rats" -EnginePath $EnginePath
    & $PSCommandPath -View Goblins -At 45 -Name "$Name-goblins" -EnginePath $EnginePath
    return
}
if ($View -eq 'Close') {
    & $PSCommandPath -View Villager -At 8 -Name "$Name-baker-a" -EnginePath $EnginePath
    & $PSCommandPath -View Villager -At 9 -Name "$Name-baker-b" -EnginePath $EnginePath
    & $PSCommandPath -View Rats -At 20 -Name "$Name-rats-close" -EnginePath $EnginePath
    & $PSCommandPath -View Goblins -At 25 -Name "$Name-goblins-close" -EnginePath $EnginePath
    return
}
if ($View -eq 'Final') {
    & $PSCommandPath -View Villager -At 9 -Name "$Name-baker-final" -EnginePath $EnginePath
    & $PSCommandPath -View Rats -At 18 -Name "$Name-rats-final" -EnginePath $EnginePath
    & $PSCommandPath -View Goblins -At 16 -Name "$Name-goblins-final" -EnginePath $EnginePath
    return
}
$motionDir = Join-Path $ProjectDir 'Saved/NpcMotion'
New-Item -ItemType Directory -Force -Path $motionDir | Out-Null
$motionLog = Join-Path $motionDir ($Name + '.log')
$motionArgs = @("`"$Project`"")
if ($View -eq 'Lineup') { $motionArgs += '/Engine/Maps/Templates/Template_Default' }
$motionArgs += @('-game', '-windowed', '-RenderOffscreen', '-ForceRes', '-ResX=1600', '-ResY=1000',
    '-notitle', '-nosplash', '-nosound', '-unattended', '-nop4', "-shotat=$At", "-shotname=$Name",
    "-quitat=$($At + 2)", "-abslog=`"$motionLog`"")
switch ($View) {
    'Lineup' { $motionArgs += @('-backlot', '-artreview=motion', '-ExecCmds="r.MotionBlurQuality 0"') }
    'Village' { $motionArgs += @('-camdist=2100', '-campitch=-35', '-camyaw=160') }
    'Villager' { $motionArgs += @('-camdist=1000', '-campitch=-30', '-camyaw=-50', '-ExecCmds="BugItGo 850 -700 200 0 -50 0,r.MotionBlurQuality 0"') }
    'Rats' { $motionArgs += @('-autoplay', '-scene=1', '-timescale=2', '-camdist=1600', '-campitch=-45', '-camyaw=0', '-ExecCmds="r.MotionBlurQuality 0"') }
    'Goblins' { $motionArgs += @('-autoplay', '-scene=2', '-timescale=2', '-camdist=1600', '-campitch=-45', '-camyaw=0', '-ExecCmds="r.MotionBlurQuality 0"') }
}
$motionProcess = Start-Process -FilePath "$EnginePath/Engine/Binaries/Win64/UnrealEditor.exe" -ArgumentList $motionArgs -WindowStyle Hidden -PassThru
if (-not $motionProcess.WaitForExit(180000)) { $motionProcess.Kill(); throw "Capture timed out: $motionLog" }
if ($motionProcess.ExitCode -ne 0) { throw "Game exited with $($motionProcess.ExitCode): $motionLog" }
$motionShot = Join-Path $ProjectDir "Saved/Screenshots/WindowsEditor/$Name.png"
if (-not (Test-Path -LiteralPath $motionShot)) { throw "Missing capture: $motionShot" }
Write-Output $motionShot
