<#
.SYNOPSIS
  Build the editor target and run the Jam.* automation tests headless.

.EXAMPLE
  .\Scripts\run-tests.ps1                     # every test under "Jam"
  .\Scripts\run-tests.ps1 -Filter Jam.Skeleton
  .\Scripts\run-tests.ps1 -Game               # client (-game) context
  .\Scripts\run-tests.ps1 -NoBuild
#>
param(
    [string]$Filter = "Jam",
    [switch]$NoBuild,
    [switch]$Game,
    [string]$EnginePath
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "_project.ps1")
$ReportDir = Join-Path $ProjectDir "Saved\Automation\Reports"
$LogFile   = Join-Path $ProjectDir ("Saved\Automation\" + $(if ($Game) { "RunTests-Game.log" } else { "RunTests.log" }))

if (-not $NoBuild) { & (Join-Path $PSScriptRoot "build.ps1") -EnginePath $EnginePath }

New-Item -ItemType Directory -Force $ReportDir | Out-Null
if (Test-Path $LogFile) { Remove-Item $LogFile -Force }

Write-Host "== Running tests: $Filter $(if ($Game) { '(-game)' }) ==" -ForegroundColor Cyan
$ArgList = @("`"$Project`"")
if ($Game) { $ArgList += "-game" }
$ArgList += @(
    "-ExecCmds=`"Automation RunTests $Filter; Quit`"",
    "-ReportExportPath=`"$ReportDir`"",
    "-unattended", "-nopause", "-nosplash", "-NullRHI", "-NoSound",
    "-abslog=`"$LogFile`""
)
$Proc = Start-Process -FilePath "$EnginePath\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" -ArgumentList $ArgList -Wait -PassThru -NoNewWindow -RedirectStandardOutput "$env:TEMP\jam-tests-stdout.txt"

$Passed = 0; $Failed = 0; $Summary = @()
if (Test-Path $LogFile) {
    foreach ($Line in Get-Content $LogFile) {
        if ($Line -match "Test Completed\. Result=\{(\w+)\} Name=\{([^}]+)\}") {
            if ($Matches[1] -eq "Passed" -or $Matches[1] -eq "Success") { $Passed++ } else { $Failed++ }
            $Summary += "  [$($Matches[1])] $($Matches[2])"
        }
        elseif ($Line -match "LogAutomationController: Error: ") {
            $Summary += "  " + ($Line -replace "^\[[^\]]*\]\[\s*\d+\]", "").Trim()
        }
    }
}
else { Write-Host "No log written at $LogFile (engine exit code $($Proc.ExitCode))" -ForegroundColor Red }

Write-Host ""
Write-Host "== Results ==" -ForegroundColor Cyan
foreach ($S in $Summary) { if ($S -match "\[(Passed|Success)\]") { Write-Host $S -ForegroundColor Green } else { Write-Host $S -ForegroundColor Red } }
Write-Host ""
Write-Host "Passed: $Passed  Failed: $Failed  (log: $LogFile)"
# Zero tests found counts as a failure: a filter that matches nothing is a broken run.
if ($Failed -gt 0 -or $Passed -eq 0) { exit 1 }
exit 0
