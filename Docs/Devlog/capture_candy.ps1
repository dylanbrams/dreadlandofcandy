param([string[]]$Views=@('Chocolate','Village'))
$ErrorActionPreference='Stop'
foreach($view in $Views) {
 $map='/Game/World/CandyPBR/L_'+$view+'Review'
 $shot='candy-'+$view.ToLower()
 $args=@('"D:/Source/Unreal/MegaJam2026/MegaJam.uproject"',$map,'-game','-unattended','-nosplash','-RenderOffscreen','-windowed','-ForceRes','-ResX=1600','-ResY=1000','-shotat=15',"-shotname=$shot",'-quitat=20',"-abslog=D:/Source/Unreal/MegaJam2026/Docs/Devlog/$shot.log")
 $run=Start-Process -FilePath 'D:/Program Files/Epic/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe' -ArgumentList $args -WindowStyle Hidden -PassThru
 if(-not $run.WaitForExit(180000)){throw "$view capture timed out"}
 if($run.ExitCode -ne 0){throw "$view capture failed"}
 Write-Output "$view capture complete"
}
