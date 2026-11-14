# CODEX Fixes2 — HOLD

Read CLAUDE.md, STATUS, RELEASE_PLAN and Fixes2-plan. The Fixes2 split governs.
Only CODEX-owned files are edited. No tests, game/editor, screenshots, packaging,
pytest, deploys or subagents during HOLD. All behavior below is **unverified**.

## 8 — subtitles over menus

The existing HUD hid under Screen but not NoteEditor; ApplyView also ran before
overlay visibility was resolved. Voice is a UI sound and continued during pause.
Now resolve overlays first, clear the rendered caption/bubbles under a screen or
note editor, hide the HUD immediately on pause, and pause/resume current voice.
The game-state caption/queue is untouched: IsCaptionVisible uses world time,
which stops during pause, so the current story caption retains its time.
No speaking portrait exists in the current native HUD; the caption speaker name
is a child of the hidden HUD. The reported packaged layering issue still needs
reproduction after GO; source inspection alone cannot establish its runtime cause.

Compile: Scripts/build.ps1 editor target succeeded, including the final scoped
row 8 checkpoint (26.95 seconds). Behavior: unverified (HOLD).

## 7 — investigation before code (unresolved; no cooldown code changed)

Evidence collected during HOLD:
- JamHUD.cpp queries owning cooldown tags and reads pair Key / Value. UE 5.8
  GameplayEffect.cpp:5575-5593 confirms Key is remaining and Value is duration;
  its owning-tag query includes granted tags (6112-6127).
- Both ApplyCooldown implementations set positive progression durations with
  SetDuration(Seconds, true). Engine SetDuration respects the duration lock;
  these effects use scalable floats, not a missing SetByCaller magnitude.
- JamHudWidget retains its MID in a transient UPROPERTY array and changes only
  IconTexture when class icons load; no brush/MID replacement was found.
- Static inspection of M_SkillCooldown.uasset finds Cooldown, Unlocked and
  IconTexture. import_skill_icons.py uses those exact names and a radial mask.
  Historical log MegaJam-backup-2026.11.13-20.53.22.log records successful save
  and RADIAL COOLDOWN MATERIAL PASS. Earlier failed authoring attempts precede it.
- Historical RunTests.log records Jam.Gas.LectureCone success on Nov 14;
  that test checks the tag/blocking, not the HUD ratio or rendered material.

Root cause: **not established**. Hypotheses still requiring live evidence:
packaged material/shader state or a runtime difference in the queried effects.
Do not claim a duration or parameter bug based on guesses. No functional change
to cooldowns or ratio test until the actual failure is identified. Investigation
is blocked by HOLD's no-game/no-tests rule; remaining independent rows proceed.

## Requests

- CLAUDE: marker style will fold Fenella into active red/gold. Please replace
  "golden triangle" directions/hints with "candy cane" in your GameMode and
  VillageFlow files. No changes to those files by CODEX.
- CLAUDE: please own the requested note-stub.py proposal route; Scripts/ is
  outside CODEX ownership. HUD proposal hooks will be handled with row 4.
- CLAUDE: NoteEditor currently sanitizes/counts/truncates with NoteMaxChars=80
  and labels submission as a note. UI/JamNoteEditor.* is outside the CODEX list.
  Please provide a proposal mode accepting 160 characters (and suitable prompt,
  counter and submit text). CODEX can wire its current Open/OnSubmit API now;
  keyboard proposals are capped at 80 until this companion change lands.

## 2+3 — candy cane pointers (unverified)

Engine Cylinder and Cone components form a striped shaft, hooked top and down tip;
existing cooked M_FxUnlit supplies independent red and pale MIDs. Inactive is
red/white and stationary. Active is red/gold, 1.65x scale, bobbing and spinning.
FindActive selects one visible marker within 200 horizontal units of the compass
target, with a stable ID tie-break. It covers travel, camps, moving Fenella and
return-to-giver without editing CLAUDE's flow. Hidden markers cannot win. If the
flow hides all markers near the target, none is active (e.g. already at town).
Fenella deliberately uses this same active style. The compass gets the same
red/gold cane glyph; edge arrows still show the direction off the strip.
Existing cooking of engine basic shapes/FX must be checked in the GO package.

## Verify at GO

- 2+3: inspect recruitment, travel, active camp, multi-step destination change,
  Ready return-to-giver, abandonment, and village idle. When a visible target
  marker exists, exactly one is red/gold/larger/moving; other givers remain
  red/white/still. Check hook/down-tip silhouette and compass at distance and
  from both sides. Run full Jam suite and offline village flow; CLAUDE captures
  and inspects active/inactive marker field and checks basic shapes in package.
- 7: first reproduce while casting Lecture, Line and Judgment. Inspect the HUD's
  queried (remaining, duration) pairs, Slot.Cooldown, brush MID and actual material
  parameters in editor/game, then compare the P1 package/cooked material. A valid
  ratio must fall from 1 to 0 over the cooldown; the icon must visibly recover
  clockwise. Establish root cause before a functional patch; add the required
  headless ratio regression if the failure is in reading the effect.
- 8: run Scripts/run-tests.ps1; require log EXIT CODE: 0. Pause during a long
  arrival story caption, visit journal/controls and resume: no caption/name box
  over menus; voice and the same story caption resume with remaining time.
  Open quest choices and both note/proposal editors while a caption is active:
  no HUD speech or world bubbles over the screen; authored routes remain usable.
  CLAUDE captures and inspects the pause screenshot in the agreed GO sequence.
