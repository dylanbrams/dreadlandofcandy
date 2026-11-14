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
  outside CODEX ownership. HUD proposal hooks are implemented with row 4.
- Dylan confirmed in this thread: keep the note/proposal editor at **80
  characters**. No NoteEditor change requested. Backend's 160-character ceiling
  remains a validation upper bound, not a request to enlarge the editor.

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
Compile: actor/header and compass glyph built successfully with Scripts/build.ps1.
No behavior verification during HOLD. Row 8 commit: caa6046.
Marker commit: 835b8a4; final editor compile succeeded in 19.17 seconds.

## 4 — proposal endpoint and requested HUD hooks (unverified)

Contract prerequisite met: QuestBranches.md contains Proposals, committed by
CLAUDE in aa99964. Added POST /v1/director/quest/propose in the nested backend
repository. proposals.py defines the exact client vocabulary (also compared
against JamWorldTable and JamNoteClient), prompt, request validation, canonical
cache key and output whitelist/clamp. Unknown verbs/targets/camps fail; counts,
text, level step limits and total are bounded; feed/brush includes pacifying.
At most two camps. Rewards are never returned. Branch history now accepts
`:proposal` so a completed plan does not break later branch requests.

The endpoint uses existing request/model/IP limits, screen_note, cached
answers, shared daily budget, usage accounting, 4.5-second configured timeout
and one model call without retries. New session proposal cap defaults to 6/hour
via DIRECTOR_SESSION_QUEST_PROPOSALS_PER_HOUR. Context/capitalization/whitespace
cache behavior, invalid output/input, refusal, budget, timeout and branch history
tests are written using FakeModel; **not run during HOLD**, no deployment.
No backend Python code was executed for validation during HOLD.

As requested in CLAUDE's progress file, HUD now opens the existing note editor
for ProposePlan, routes submit to RequestVillageProposal, supports QuestBranch2,
and returns Esc/B to the offer. It hides the higher-Z offer while editing and
restores its focus on return. Ordinary typed notes keep their submission path.
The editor deliberately retains the 80-character cap at Dylan's request.
HUD editor compile succeeded via Scripts/build.ps1 (80.61 seconds).
Backend commit: a0af558 in backend's own repository. All behavior unverified.
HUD commit: 03bd006. No Unreal job is running after this compile.

## 6 — Meshy knight shield (generated; STOP for Dylan's approval)

Read GenerationRunbook, styles.json, ASSETS budgets, existing generation status
and the SK_Hero.png reference corresponding to SK_Hero_Rig. Reference: grim
angular blackened steel, bevelled silver edges, desaturated armour. New job
SM_KnightShield in jobs.json uses the locked equipment style and explicitly
requests one front-facing heater shield with a peppermint crest, restrained
red/ivory inlays, dull gold/silver rim and rear forearm straps. Target 5,000
faces; hero gear texture budget 2K. No hand, wielder, base or lettering.

Balance before job: Meshy 215 credits. Existing equipment generation group was
already approved; **this new shield is not approved for import/use**. Only this
one job was submitted, through gen_batch.py --only SM_KnightShield after dry-run.
No approve/route command, alternate generator, editor, import or attachment.
Stop after generation/file review for Dylan's explicit shield approval.

Result: succeeded, 1.7 minutes, 30 Meshy credits consumed (185 remaining by
subtraction). ImportSource/Meshy/SM_KnightShield.glb: 8,879,580 bytes, one mesh,
one material, 4,769 triangles, three 2048x2048 texture maps. Front thumbnail reviewed: dark angular gothic
heater outline, silver bevelled rim, gold round peppermint/pinwheel crest with
red/ivory accents, deep red edging. Good stylistic match to hero reference;
this is an observation, **not approval**. Back/straps, front axis, normals and
socket fit remain unverified. Review metadata is SM_KnightShield.review.json.
Preview: ImportSource/Meshy/SM_KnightShield.png; full prompts/task IDs in
ImportSource/Meshy/manifest.jsonl. gen_batch.py declare added exactly one shield
line to Docs/Declared.md. GLB remains a local delivery under the existing
ImportSource/*/*.glb ignore rule; manifest/thumbnail/job/review state are committed.
No self-approval, import or game/editor activity. Await Dylan before further
shield work; actual game use additionally waits for GO.

## Verify at GO

- 6: after Dylan approves the generated shield, CLAUDE imports it, checks actual
  triangle/texture budgets, aligns the front outward and rear straps to the left
  hand/forearm socket, and inspects its silhouette, grip, clipping and animation
  in the game camera. Screenshot review/import remains forbidden during HOLD.
- 4: run backend pytest in its documented environment with FakeModel; require
  all existing tests plus test_proposals.py to pass. Check that proposal cache
  hits do not spend model budget, invalid vocabulary returns 422, foreign output
  returns 503 without caching, and shared limits prevent model calls.
  Run Jam suite; then CLAUDE's stub-backed proposal and 503 fallback game checks.
  Keyboard and gamepad: open/cancel/reopen, submit, select the third plan,
  select authored routes during a request, close the offer during a request,
  confirm late replies cannot reopen it, and check notes still submit normally.
  Confirm text stops at 80 characters; no request to enlarge this cap.
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
