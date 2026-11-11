<#
.SYNOPSIS
  Build a target with UnrealBuildTool (no cook, no package).

.EXAMPLE
  .\Scripts\build.ps1                         # editor target, Development
  .\Scripts\build.ps1 -Game -Config Shipping  # game target
#>
param(
    [switch]$Game,
    [ValidateSet("Development", "DebugGame", "Shipping")]
    [string]$Config = "Development",
    [string]$EnginePath
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "_project.ps1")
$Target = if ($Game) { $GameTarget } else { $EditorTarget }

Write-Host "== Building $Target $Config ==" -ForegroundColor Cyan
& "$EnginePath\Engine\Build\BatchFiles\Build.bat" $Target Win64 $Config -Project="$Project" -WaitMutex -NoHotReload
if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)" }
Write-Host "Build succeeded." -ForegroundColor Green
