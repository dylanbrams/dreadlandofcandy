<# Build the isolated attack FX library, including the required saved-bytecode repair.
   -Sources regenerates the original procedural sources. -Review captures all attacks.
   Run only while this thread owns Unreal; commands execute sequentially.
#>
param([switch]$Sources, [switch]$Review, [string]$EnginePath)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_project.ps1')
$fxEditor = Join-Path $EnginePath 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$fxLogDir = Join-Path $ProjectDir 'Saved/FX'
New-Item -ItemType Directory -Force -Path $fxLogDir | Out-Null
if (Test-Path -LiteralPath (Join-Path $fxLogDir 'build_filter.txt')) {
    throw 'A partial-build filter exists in Saved/FX/build_filter.txt. Remove it before a full pipeline build.'
}
function Invoke-FxPython([string]$ScriptName, [string]$LogName, [switch]$ForceCompile) {
    $fxScript = Join-Path $PSScriptRoot $ScriptName
    $fxLog = Join-Path $fxLogDir $LogName
    $fxLaunchLog = Join-Path $fxLogDir ("launch-" + $LogName)
    $fxArgs = @($Project, '-run=pythonscript', "-script=$fxScript", '-unattended', '-nullrhi', '-nosound', '-nop4', "-abslog=$fxLog")
    if ($ForceCompile) { $fxArgs += '-dpcvars=fx.ForceCompileOnLoad=1' }
    Write-Host "FX: $ScriptName"
    & $fxEditor @fxArgs *> $fxLaunchLog
    if ($LASTEXITCODE -ne 0) { throw "FX stage failed: $ScriptName. See $fxLog" }
}
if ($Sources) {
    & python (Join-Path $PSScriptRoot 'build_fx_sources.py')
    if ($LASTEXITCODE -ne 0) { throw 'FX source generation failed.' }
}
Invoke-FxPython 'import_fx_assets.py' 'assets.log'
Invoke-FxPython 'build_niagara_attacks.py' 'niagara.log'
# This must be a fresh process: forcing an in-memory audit does not repair saved assets.
Invoke-FxPython 'recompile_fx_systems.py' 'recompile.log' -ForceCompile
Invoke-FxPython 'build_fx_review.py' 'review.log'
$fxAuditLog = Join-Path $fxLogDir 'compile.log'
$fxAuditOutput = Join-Path $fxLogDir 'Bytecode'
& $fxEditor $Project -run=NiagaraDumpBytecode -PackagePaths=/Game/FX/Attacks/Systems "-AuditOutputFolder=$fxAuditOutput" -BAKED -unattended -nullrhi -nosound -nop4 "-abslog=$fxAuditLog" *> (Join-Path $fxLogDir 'launch-compile.log')
if ($LASTEXITCODE -ne 0) { throw "Niagara audit failed. See $fxAuditLog" }
if ($Review) { & (Join-Path $PSScriptRoot 'review_attack_fx.ps1') -EnginePath $EnginePath }
& python (Join-Path $PSScriptRoot 'validate_attack_fx.py')
if ($LASTEXITCODE -ne 0) { throw 'FX validation failed.' }
