# Fixes2 progress: CLAUDE (GAME)

HOLD is on (Nov 14): compile only, nothing below is verified. Plan: `Fixes2-plan.md`.

## Rows

| # | State | Commit | Notes |
| --- | --- | --- | --- |
| 1 | code done, compiles (untested) | ac33908 | 20 Fenella pickups in `JamLines.cpp`, hooked once each; captions last the clip's length. Audio import after GO. |
| 4 | contract aa99964; client compiles (untested) | see git log | `Docs/Design/QuestBranches.md` "Proposals". Codex may build the backend. Client: request, parse/clamp, plan route, autoplay `-autopropose=`. **Needs the HUD request below** to be reachable by a player. |
| 5 | compiles (untested) | see git log | Levels Stranger/Acquaintance/Friend/Bestie per giver, multi-step jobs, a repeatable encore per giver after their two authored jobs. |
| 6 | post-GO | | Import + left-hand socket once Codex's shield is approved. |

## Decisions

- **VO path:** the plan says `/Game/Audio/VO/Fenella/`, but the HUD already plays
  `/Game/Audio/VO/VO_<LineId>` by convention and `/Game/Audio` is always cooked. The
  25 new WAVs import to `/Game/Audio/VO/` with their file names (`VO_RealityPickup_01`...).
  88 older clips are already imported; the 25 new are the 5 `RealityIntro` and 20 pickups.
- **Pickup hooks** (all 20, each once per session, spent when shown): listed in
  `Docs/Design/Jokes.md` under "Fenella's lines, by trigger". New triggers sit with the
  story lines (queued, never dropped); `Direct()` still clears the queue, so a pickup
  wiped by a direction is said at its next trigger instead.
- **Caption length:** `ShowCaption` now loads the line's clip and keeps the caption up
  for its duration + 0.4 s (text-length timing if there is no clip).
- **Steps (shared by #4 and #5):** a job is a list of `FJamQuestStep` (verb, target,
  count, camp). The first step stays in the old `FJamVillageQuestDef` fields, more go in
  `Extra`, so the ten authored jobs are unchanged. Each step spawns at its camp when it
  starts (from `TickVillage`, never inside a pacify/treat callback); the HUD hint reads
  "Step 2/2: ...". Feed/brush already include pacifying.
- **Levels (#5):** jobs paid per giver = level (cap 3). Acquaintance (the giver's second
  job): +1 target. Friend: an extra step (`Complement`: gather -> calm the camp's locals;
  pacify rats -> gather celery; goblin brushing -> calm a second goblin camp; bugs ->
  tidy candy shards). Bestie: +1 more and the extra step at a second camp. Bonus
  +25 sprinkles per level. Scaling happens on Accept and is undone on Abandon.
- **Encores (#5):** with only two authored jobs per giver, levels 2-3 needed more work:
  each giver gets a repeatable encore (unlocked by their second job, built at Friend,
  rebuilt bigger after each payment). Encores never count toward the ten, so
  `JamVillageAutoplay: PASS main=3 side=10` and the journal's "N OF 10" are unchanged.
  Encores have one route (no improvise) but can take a proposal.
- **Proposals (#4):** reaction and failure lines go in the quest screen's status line,
  not a caption, because captions hide under menus (#8). One proposal per offer.

## Requests (other threads' files)

- **CODEX, `Game/JamHUD.cpp` (needed for #4 to be playable):**
  1. `AJamHUD::Activate`: `case EJamMenuAction::QuestBranch2: if (GM) { GM->ChooseVillageBranch(2); Selected = 0; } break;`
  2. `case EJamMenuAction::ProposePlan:` open the existing `NoteEditor` in a proposal
     mode: `NoteEditor->Open(TEXT("How would you solve it?"), GM->ProposalSuggestions(), bMenuGamepad)`;
     its submit goes to `GM->RequestVillageProposal(Text)` instead of `HandleTypedNote`.
     `TickNoteEditor` must not close it while in proposal mode and `GM->GetQuestOffer().bVisible`;
     close it (and leave the mode) when the offer closes. Esc/B closes the editor only, back to the offer screen.
  Both enum values exist in `UI/JamScreens.h`; `JamScreens::QuestBranches` already adds the
  options. Until this lands, the "Propose a plan" button does nothing and only
  `-autopropose=` reaches the feature.
- CODEX / whoever owns `Scripts/note-stub.py`: add `POST /v1/director/quest/propose`
  returning the success example from the contract (and honour `--status`), for the
  post-GO local check. If nobody owns it, I'll take it after GO.

## Verify at GO

1. **Row 1, lines and hooks.** Import `vocals/output/fenella/VO_{RealityIntro,RealityPickup,VillagePickup,HeroPickup,ReturnPickup}_*.wav`
   to `/Game/Audio/VO/` (25 clips). Compile; `Scripts\run-tests.ps1` (log `EXIT CODE: 0`;
   `Jam.Film.Lines*`/nag test covers the four new pools, ReturnPickup is once-only so 2 lines pass).
   `Scripts\verify_village_quests.ps1 -Mode offline`: PASS line `JamVillageAutoplay: PASS main=3 side=10`.
   In that log, grep `Fenella: RealityPickup_01`, `_02`, `_03`, `ReturnPickup_01`,
   `VillagePickup_0`, `HeroPickup_0`, `ReturnPickup_02`: each appears once at most, and
   `ReturnPickup_02` after the last `turned in`.
2. **Row 1, sync.** Play to the arrival, listen to the five `RealityIntro` clips with
   captions; caption stays until the voice ends; talking to Fenella cuts both. Packaged
   smoke must still pass (clips are under the cooked `/Game/Audio`).
3. **Row 5, tests.** `Jam.World.Quests.RelationshipLevels` (new), `Jam.World.Quests.GameplayFlow`
   (now expects the level bonus in payouts), `Jam.World.Quests.VariationsAndHooks`
   (15 offers now: 10 + 5 encores). All `Jam.World.Quests.*` pass, total 81+ tests.
4. **Row 5, in game.** Offline village autoplay still `PASS main=3 side=10`; log shows
   `JamQuest: turned in ... level=1` then `level=2` for each giver. Then play: after a
   giver's two jobs, their encore shows two steps; step 1 done moves the marker/compass
   to step 2's camp and spawns there; the hint says "Step 2/2". Screenshot
   `-uiscreen=quests` (journal shows "(Friend)") and the quest screen's kicker shows the level.
5. **Row 4, tests.** `Jam.World.Quests.Proposals` (new): parse, refuse, clamp, merge, plan
   reward, history `council_rats:proposal`.
6. **Row 4, in game (after the HUD request and the stub route).** `Scripts\screenshot.ps1`
   with `-Extra "-uiscreen=questproposal"`: three routes, the plan third, level kicker.
   Stub run: `-noteurl=http://127.0.0.1:8019 -autoplay -autovillage -autopropose="feed them" -quitaftervillage`:
   log `JamQuest: proposal ready` and `accepted ... route=proposal steps=2`, then PASS.
   Stub `--status 503`: `proposal unusable`, authored route taken, PASS. Then by hand:
   Propose a plan with keyboard and with pad suggestions.
