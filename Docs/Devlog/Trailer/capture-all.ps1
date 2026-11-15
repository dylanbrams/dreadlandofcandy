# Records the gameplay set for the demo and the devlog with capture.ps1, one run after another (~22 min).
# Keep hands off while it runs. Index of what each clip shows: Docs/Devlog/GameFootage/INDEX.md.
$C = "$PSScriptRoot\capture.ps1"
$Propose = '-autopropose="Bribe the rats with snacks so they go home"'
& $C -Name title-idle       -Seconds 14  -Skip 5
& $C -Name knight-run       -Seconds 80  -Skip 12 -Extra "-autoplay","-class=knight"
& $C -Name archer-run       -Seconds 80  -Skip 12 -Extra "-autoplay","-class=archer"
& $C -Name mage-run         -Seconds 80  -Skip 12 -Extra "-autoplay","-class=mage"
& $C -Name barbarian-run    -Seconds 80  -Skip 12 -Extra "-autoplay","-class=barbarian"
& $C -Name typed-note       -Seconds 45  -Skip 12 -Online -Extra "-autoplay","-class=knight","-autonote=typed:Add_more_rats_and_make_them_sad"
& $C -Name budget-out       -Seconds 40  -Skip 12 -Extra "-autoplay","-class=mage","-scene=3","-budgetout"
& $C -Name village-improvise -Seconds 150 -Skip 12 -Online -Extra "-autoplay","-autovillage","-autobranches","-class=archer","-scene=3"
& $C -Name village-third    -Seconds 150 -Skip 12 -Online -Extra "-autoplay","-autovillage","-autobranch=2","-class=knight","-scene=3"
& $C -Name village-propose  -Seconds 150 -Skip 12 -Online -Extra "-autoplay","-autovillage","-class=barbarian","-scene=3",$Propose
Write-Host "ALL DONE"
