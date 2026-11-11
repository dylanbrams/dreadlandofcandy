# Dot-sourced by the other scripts. Finds the project from the one .uproject in the root,
# so renaming the project needs no script changes.
$ProjectDir = Split-Path -Parent $PSScriptRoot
$UProject = Get-ChildItem -Path $ProjectDir -Filter "*.uproject" | Select-Object -First 1
if (-not $UProject) { throw "No .uproject in $ProjectDir" }
$Project     = $UProject.FullName
$ProjectName = $UProject.BaseName
$GameTarget   = $ProjectName
$EditorTarget = "${ProjectName}Editor"
if (-not $EnginePath) {
    $EnginePath = if ($env:UNREAL_ENGINE_ROOT) { $env:UNREAL_ENGINE_ROOT } else { "D:\Program Files\Epic\UE_5.8" }
}
