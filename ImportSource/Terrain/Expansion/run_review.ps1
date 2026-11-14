param([ValidateSet('validate','village','river')][string]$Mode='validate')
$ErrorActionPreference='Stop'
$ExpansionProject=(Resolve-Path "$PSScriptRoot/../../..").Path
$ExpansionExe='D:/Program Files/Epic/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe'
$ExpansionArgs=@("`"$ExpansionProject/MegaJam.uproject`"",'-unattended','-nosplash',"-abslog=`"$ExpansionProject/Saved/Logs/Riverlands-$Mode.log`"")
if($Mode -eq 'validate') {
    $ExpansionArgs+=@("-ExecutePythonScript=`"$PSScriptRoot/validate_expansion.py`"",'-nullrhi')
} else {
    $ExpansionMap=if($Mode -eq 'village'){'L_VillageReview'}else{'L_RiverReview'}
    $ExpansionArgs+=@("/Game/World/Expansion/$ExpansionMap",'-game','-RenderOffscreen','-windowed','-ForceRes','-ResX=1600','-ResY=1000','-shotat=35',"-shotname=riverlands-$Mode",'-quitat=40')
}
$ExpansionProcess=Start-Process -FilePath $ExpansionExe -ArgumentList $ExpansionArgs -WindowStyle Hidden -PassThru
if(-not $ExpansionProcess.WaitForExit(240000)){throw "Unreal review timed out: $Mode"}
if($ExpansionProcess.ExitCode -ne 0){throw "Unreal review failed: $Mode"}
