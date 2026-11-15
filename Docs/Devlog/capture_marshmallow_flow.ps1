$ErrorActionPreference='Stop'
foreach($time in @(8,18)) {
 $shot="marshmallow-flow-$time"
 $args=@('"D:/Source/Unreal/MegaJam2026/MegaJam.uproject"','/Game/World/CandyPBR/L_ChocolateReview','-game','-unattended','-nosplash','-RenderOffscreen','-windowed','-ForceRes','-ResX=1600','-ResY=1000',"-shotat=$time","-shotname=$shot",('-quitat='+($time+3)),"-abslog=D:/Source/Unreal/MegaJam2026/Docs/Devlog/$shot.log")
 $run=Start-Process -FilePath 'D:/Program Files/Epic/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe' -ArgumentList $args -WindowStyle Hidden -PassThru
 if(-not $run.WaitForExit(180000)){throw "$shot timed out"}
 if($run.ExitCode -ne 0){throw "$shot failed"}
 Write-Output "$shot captured"
}
