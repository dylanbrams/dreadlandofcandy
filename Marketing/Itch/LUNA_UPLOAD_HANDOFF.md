# Next itch.io upload — Luna handoff

## Current state

- Project root: `D:\Source\Unreal\MegaJam2026` (PowerShell).
- itch target: `dylanbrams/the-dread-land-of-candy`.
- Keep the page restricted. The secret link is in Dylan's conversation; do not copy its secret into files, logs or commands. Butler uses account authentication, not that link.
- Butler is installed at `$env:TEMP/MegaJamButler/bin/butler.exe` (v15.32.0). It successfully authenticated and uploaded from this Windows user session.
- Only a tiny test was uploaded. Channel `upload-test`, version `upload-test-only`, upload #20499369, processed build #2378717. It contains a README, not a playable game.
- The full game ZIP was NOT uploaded. Dylan stopped that upload before it ran because he needs to repackage.
- At handoff, the old ZIP `Builds/ImaginaryComponent_TheDreadLandOfCandy.zip` was 722,819,026 bytes (722.8 MB / 689.3 MiB). Do not assume this is the new candidate.

## Before the next upload

1. Read the current `Docs/STATUS.md` and `Docs/ENGINE_HANDOFF.md`. Coordinate with the session doing the repackaging; do not start another Unreal build/editor session in the shared checkout.
2. Wait for Dylan's new packaged candidate and explicit go-ahead for that upload. His latest instruction was to upload only the tiny test while he repackages; it does not authorize uploading the old game ZIP.
3. Confirm the candidate's absolute path, modification time and byte size. Use the exact ZIP Dylan approves. Check its package/smoke-test handoff; do not infer that a ZIP is current from its filename alone.
4. Preserve the existing restricted page visibility. Butler uploads builds; do not publish the page or change its access settings. Before uploading, verify the page still requires access using itch.io's owner settings if available. An unauthenticated request returned HTTP 404 during the test, but recheck if the page settings have changed.

## Upload the approved candidate

From the project root, run these PowerShell commands, replacing the ZIP path and version if needed:

```powershell
$butlerExe = Join-Path $env:TEMP 'MegaJamButler/bin/butler.exe'
$candidateZip = 'D:\Source\Unreal\MegaJam2026\Builds\ImaginaryComponent_TheDreadLandOfCandy.zip'
Get-Item -LiteralPath $candidateZip | Select-Object FullName, Length, LastWriteTime
& $butlerExe version
& $butlerExe push $candidateZip 'dylanbrams/the-dread-land-of-candy:windows' --userversion 'tester-YYYY-MM-DD-HHMM'
```

Replace `tester-YYYY-MM-DD-HHMM` with a meaningful unique version for the actual candidate. Use `windows` for the real Windows game, not `upload-test`. Subsequent uploads to `windows` update that channel. Butler accepts a ZIP and processes its contents directly.

Use the tool's normal network/sandbox escalation if required. Do not print credentials or read authentication files. If authentication fails, have Dylan complete `butler login` privately, then retry the approved candidate.

The existing `Scripts/upload-itch.ps1` uploads the unpacked `Builds/Win64-Development` directory, NOT the ZIP, and asks the operator to type PUSH. The direct command above deliberately uploads the exact approved ZIP instead. There is no need to modify that script for this handoff.

## Verify and report

```powershell
& $butlerExe status 'dylanbrams/the-dread-land-of-candy:windows'
```

Wait for successful processing (a check mark beside the build). Report channel, version, build ID, uploaded candidate path and its size. If processing fails or is still pending, say so explicitly. If possible, check the restricted game page as its owner and download/unzip/launch the uploaded candidate as a separate validation step.

Leave `upload-test` alone unless Dylan requests its removal. Uploading the game does not submit it to the jam; jam submission is a separate action.

## If Butler is missing after temporary files are cleaned

Download the official stable Windows CLI to a temporary tool folder and extract it:

```powershell
New-Item -ItemType Directory -Force -Path "$env:TEMP/MegaJamButler" | Out-Null
Invoke-WebRequest -Uri 'https://broth.itch.zone/butler/windows-amd64/LATEST/archive/default' -OutFile "$env:TEMP/MegaJamButler/butler.zip" -UseBasicParsing
Expand-Archive -LiteralPath "$env:TEMP/MegaJamButler/butler.zip" -DestinationPath "$env:TEMP/MegaJamButler/bin" -Force
```

Official documentation: https://itch.io/docs/butler/installing.html and https://itch.io/docs/butler/pushing.html.

## Page copy and art

`Marketing/Itch/` contains the cover, exact generation prompt, short description, full description, and four selected screenshots with captions/provenance in README.md. These were prepared locally, not automatically added to the live page. Two captures show an older HUD/knight; consult README.md before using them for the final submission.
