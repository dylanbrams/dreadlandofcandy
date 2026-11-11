<#
.SYNOPSIS
  Push the packaged build to itch.io with butler. Publishing is public: run it only when
  Dylan says so. It shows what it will push and asks before pushing.

.DESCRIPTION
  Needs butler (https://itch.io/docs/butler/) on PATH and logged in (`butler login`, off stream).
  The jam submission itself is made on the jam page, from the game's itch.io page.
  Set the AI disclosure on the itch.io page (Edit game > AI generation disclosure).

.EXAMPLE
  .\Scripts\upload-itch.ps1 -Target someone/my-jam-game -Version 1.0.0 -DryRun
#>
param(
    [Parameter(Mandatory = $true)][string]$Target,
    [Parameter(Mandatory = $true)][string]$Version,
    [string]$Channel = "windows",
    [ValidateSet("Development", "Shipping")][string]$Config = "Development",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "_project.ps1")
$BuildDir = Join-Path $ProjectDir "Builds\Win64-$Config"
$Exe = Get-ChildItem -Path $BuildDir -Filter "$ProjectName.exe" -Recurse -ErrorAction SilentlyContinue |
       Sort-Object { $_.FullName.Length } | Select-Object -First 1
if (-not $Exe) { throw "No packaged build in $BuildDir. Run .\Scripts\package.ps1 -Config $Config first." }
if (-not (Get-Command butler -ErrorAction SilentlyContinue)) { throw "butler is not on PATH (https://itch.io/docs/butler/)." }
$Root = $Exe.Directory.FullName

Write-Host "Will push: $Root" -ForegroundColor Cyan
Write-Host "To:        ${Target}:$Channel   version $Version" -ForegroundColor Cyan
if ($DryRun) { & butler push $Root "${Target}:$Channel" --userversion $Version --dry-run; exit $LASTEXITCODE }
$Answer = Read-Host "Type PUSH to upload"
if ($Answer -ne "PUSH") { Write-Host "Not pushed."; exit 1 }
& butler push $Root "${Target}:$Channel" --userversion $Version
exit $LASTEXITCODE
