$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/_project.ps1"
& (Join-Path $EnginePath 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe') $Project -run=pythonscript "-script=$PSScriptRoot/import_ult_mint.py" -unattended -nullrhi -nosound -nop4 "-abslog=$ProjectDir/Saved/ult-mint-import.log"
if ($LASTEXITCODE -ne 0) { throw 'Mint import failed' }
