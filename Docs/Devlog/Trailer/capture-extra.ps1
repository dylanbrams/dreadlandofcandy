# Second capture batch for the devlog's missing shots (BROLL_INDEX.md section 3), ~8 min. Hands off while it runs.
$C = "$PSScriptRoot\capture.ps1"
$Tour = "/Game/World/L_Sugarborough?game=/Script/MegaJam.JamTourGameMode"
& $C -Name controls-screen -Seconds 10  -Skip 6  -Extra "-uiscreen=controls"
& $C -Name village-aerial  -Seconds 15  -Skip 10 -Extra "-notitle","-campitch=-55","-camdist=3800"
& $C -Name hero-close      -Seconds 60  -Skip 12 -Extra "-autoplay","-class=knight","-closecam=450"
& $C -Name tour-low        -Seconds 150 -Skip 10 -Map $Tour
& $C -Name tour-high       -Seconds 120 -Skip 10 -Map $Tour -Extra "-tourheight=1600"
Write-Host "ALL DONE"
