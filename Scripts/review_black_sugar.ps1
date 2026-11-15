<# Run only after the engine slot is handed over. Screenshots are deliberately sequential. #>
param([switch]$Full, [switch]$Small)
$ErrorActionPreference = "Stop"
$Screens = if ($Full) { @("rating", "title", "pause", "options", "controls", "quit", "character", "quests", "questbranches", "questproposal", "hud", "slate", "action", "cut", "retake", "wrap", "meter", "dignity", "bubbles", "caption", "noteeditor", "premiere", "theatrical", "generated", "credits") } else { @("title", "pause", "hud", "cut", "wrap", "questbranches") }
$ReviewWidth = if ($Small) { 1280 } else { 1920 }
$ReviewHeight = if ($Small) { 720 } else { 1080 }
foreach ($ScreenName in $Screens) {
    & "$PSScriptRoot\screenshot.ps1" -ResX $ReviewWidth -ResY $ReviewHeight -At 3 -Name "black-sugar-$ScreenName-${ReviewWidth}" -Extra "-notitle -nomusic -uiscreen=$ScreenName"
}
