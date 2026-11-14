# Fixes2 clean-session handoff — Nov 14

**User requested this checkpoint and a fresh session. Stop this session here.**
Fixes2 is **not finished**. Round three is in `Docs/Feedback/Fixes3.txt`; do not
start it until Fixes2 is finished. Engine resource is **FREE**, no jobs running.

## Governing instructions and permissions

- Read CLAUDE.md, STATUS, RELEASE_PLAN and `Docs/Feedback/Fixes2-plan.md`.
  Its cc64ce0 update assigns all eight fixes to CODEX; Claude is out of quota.
- Dylan explicitly said **GO**, later closed his game and said **Go ahead**.
  Earlier HOLD was respected. Builds, tests, imports and captures are now allowed.
- Shield approval is granted: "The shield is fine from this angle, we'll just
  go with it." Do not regenerate it or request approval again.
- Note/proposal editor stays at **80 characters**, per Dylan. Server's existing
  160-character validation ceiling is compatible; do not enlarge the editor.
- Dylan wants small chunks focused on the current code area to save usage.
  Delegate bounded routine checks to `gpt-6-luna`, fork none. One Unreal job at
  a time, with explicit handoff. Primary owns implementation and integration.
- Compile with Scripts/build.ps1. Keep scoped `[untested]` commits as requested,
  `git commit -m "[untested] ..." -- <paths>`. No push, deploy or itch upload.
- Do not commit unrelated DefaultEngine.ini/token changes, Scripts/package.ps1,
  Assets/DylanGeneration, PersonalNotes, music, source vocals, or Fixes3.txt.
  Never read credentials.

## Completed work and exact checkpoints

| Row | Commits | State / files |
| --- | --- | --- |
| 1 | ac33908, 31916cc | Reality-show intro and 20 pickups hooked in JamLines/GameMode/VillageFlow; all 25 WAVs imported to `/Game/Audio/VO/VO_<LineId>`. Captions last clip duration +0.4s. |
| 2+3 | 835b8a4, 2599c7e (also route hints in d4b29c0) | JamQuestMarker uses cylinder bands/hook/cone; inactive red/white, active red/gold/larger/spinning/bobbing. One visible marker nearest compass target wins, stable ID tie-break, 200cm horizontal range. Active/inactive scales now 0.85/0.60 after first shot was too large. Fenella shares active style. Compass glyph matches. |
| 4 | aa99964 contract, 7bc1f8b client, 03bd006 HUD, d4b29c0 guards/helpers | Propose button opens existing editor, submits to RequestVillageProposal, third route selectable. Encore proposal at slot 1 also maps correctly. Async generators cannot overlap; starting proposal prevents later improvise overwrite. Authored routes remain enabled. |
| 4 backend | nested backend repo a0af558, 650d97e | `/v1/director/quest/propose`, exact closed vocabulary, screening, session/IP/model limits, shared budget/cache, one call/no retries. qp2 merges pacify/feed before limits and preserves max cohort. **Not deployed.** |
| 5 | 7bc1f8b | Relationship levels and larger/multi-step jobs, five repeatable encores. Pure rules/proposal tests pass; main count remains ten. |
| 6 | 2fc58bc generation, 767b039 import/attachment | Approved 4,769-triangle Meshy shield, three 2K textures, 30 credits used. `/Game/Art/Props/Shield/SM_KnightShield`; JamHero component on hand_l, fits to 76cm, front outward. Screenshot inspected. |
| 7 | 400f787 | Current cooldown read works; Dylan confirmed the sweep while playing. No production GAS patch. Added GAS/MID regression diagnostics. User then requested big icons/small text: JamHudWidget now 112x112 icon-backed faces, key chip, compact name/rank footer. Screenshot inspected. |
| 8 | caa6046 | Main HUD speech/bubbles hide under screens/editors; voice pauses with game; world-time caption expiry retains paused story time. **Still incomplete: the screen's OWN caption overlaps bottom buttons (below).** |

All source/import changes above are committed. No production code changed after
the latest successful build/tests. Remaining dirty files should be only the
unrelated items named above plus handoff docs until their checkpoint commit.
The generated GLB stays local under the project's existing ignore rule;
manifest/job/preview/review metadata and imported uassets are committed.

## Verified — do not repeat without a new change or failure

- Editor build: `& .\Scripts\build.ps1`, succeeded, 214.16s.
- Full suite after imports: `& .\Scripts\run-tests.ps1 -NoBuild`, **82 passed,
  0 failed**, shell 0 and `Saved/Automation/RunTests.log` ends **EXIT CODE: 0**.
  Luna held exclusive Unreal resource. GameplayFlow's old empty-queue assertion
  was corrected: recruitment clears intro IDs, then intentionally queues
  RealityPickup_01. Production caption queue was not changed for this test.
- GAS diagnostic on real PlayerState: Lecture 1.5/1.5, Line 4/4, Judgment 30/30,
  ratios start 1 and decrease. HUD test logs MID retained through K02 texture
  replacement, scalar 0.25. Original P1 cooldown failure was not reproduced;
  **do not invent a root cause**. P1 cooked registry includes material and K02.
- Backend: Luna ran `.venv\Scripts\python.exe -m pytest -q` in `backend/` after
  qp2 change: **149 passed in 2.06s**, exit 0. FakeModel/MemoryStore only.
- All 25 source captions exactly match voice manifest; mono 48kHz WAVs,
  durations 3.437–10.217s. Import log `Saved/Automation/Fixes2-Import.log` has
  VO PASS count=25, SHIELD PASS, exit 0. Imported durations positive/<30s.
  Details: `ImportSource/Meshy/fixes2_import_report.json`.

Inspected screenshots in `Saved/Screenshots/WindowsEditor/`:

- `fixes2-large-skill-icons-settled.png`: all three large icon backgrounds,
  readable compact labels and partial Line sweep; knight shield on left arm.
  Use this settled 12-second capture. Earlier 3-second shot had pending shaders.
- `fixes2-knight-shield.png`: close raised-sword pose, shield front outward,
  fitting left forearm without obvious torso clipping. Material/crest visible.
- `fixes2-pause-caption-hidden.png`: main HUD hidden, but **screen caption
  "Why did everyone stop? Who called lunch?" covers the final menu button**.
- `fixes2-quest-proposal.png`: three routes and Friend level visible, but
  **Fenella's status/reaction caption covers Maybe later**.

## Next small chunk: finish row 8's screen caption layout

This is the immediate task. The reported overlap is not fully solved by hiding
UJamHudWidget: UJamScreenWidget owns another CaptionPanel, placed near the bottom
in `UI/JamScreenWidget.cpp` (~219), and ApplyView shows View.Caption (~324).
Pause's own authored quip and quest proposal status occupy that panel, colliding
with tall option lists. Preserve the quest reaction/status and all buttons;
give them separate space or adapt layout. The screenshots above are evidence.
Do not change the story queue. User assigned all Fixes2 work to CODEX; make the
necessary bounded UI fix, compile, relevant UI tests and inspect both captures.
The pause capture is a `-uiscreen=pause` sample with an active story caption;
it proves overlay layout, not a manual real-pause/resume audio check.

Capture commands (sequential, one engine at a time):

```powershell
& .\Scripts\screenshot.ps1 -At 8 -Name fixes2-pause-caption-hidden -ResX 1920 -ResY 1080 -Extra @('-uiscreen=pause','-class=knight','-RenderOffscreen','-nonotes','-nosound')
& .\Scripts\screenshot.ps1 -At 8 -Name fixes2-quest-proposal -ResX 1920 -ResY 1080 -Extra @('-uiscreen=questproposal','-class=knight','-RenderOffscreen','-nonotes','-nosound')
```

## Remaining verify-at-GO checks after the UI chunk

1. Run existing offline village verification, then new proposal/failure modes:
   `Scripts/verify_village_quests.ps1 -Mode offline`, `-Mode proposal`,
   `-Mode proposalfallback`, sequentially. **These have not been run yet.**
   The last two use local note-stub.py only; no live model/server spending.
   Helpers are committed in d4b29c0 but their new modes remain unverified.
   PASS: main scenes=3 and village main=3 side=10; proposal mode additionally
   ten `proposal ready` and ten accepted `route=proposal steps=2`; fallback
   ten `proposal unusable` and authored routes still complete all ten jobs.
2. Check real keyboard/gamepad proposal open/cancel/submit/third-route selection,
   authored choice while pending, and late reply after closing. Pure UI/state
   tests pass, but this interactive path has not been exercised.
3. Inspect active versus inactive markers together (only active/recruit marker
   has been rendered so far), multi-step camp changes, Ready return-to-giver,
   relationship level-up/journal and encore. Pure rules tests pass.
4. Check actual pause/resume during a story line and voice skip, plus listening
   to the imported intro. Source text/duration/import checks are complete;
   subjective listening and real pause/resume have not been verified.
5. Package/cooked smoke remains outstanding. Do not claim P1 zip contains these
   fixes. Dylan has round three ready; coordinate release packaging after code
   fixes, avoid unnecessary intermediate packages. No upload/deploy authorized.

Legacy phrasing: recorded RealityIntro_05 says "golden triangle"; its caption
remains exactly synced to the recording. Runtime directions now say candy cane.
Do not regenerate voice without a task needing it.
