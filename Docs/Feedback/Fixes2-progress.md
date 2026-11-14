# Fixes2 progress: CLAUDE (GAME)

HOLD is on (Nov 14): compile only, nothing below is verified. Plan: `Fixes2-plan.md`.

## Rows

| # | State | Commit | Notes |
| --- | --- | --- | --- |
| 1 | code done, compiles (untested) | see git log | 20 Fenella pickups in `JamLines.cpp`, hooked once each; captions last the clip's length. Audio import after GO. |
| 4 | contract written | | `Docs/Design/QuestBranches.md` "Proposals". Codex may start the backend. Client next. |
| 5 | not started | | |
| 6 | post-GO | | Import + left-hand socket once Codex's shield is approved. |

## Decisions

- **VO path:** the plan says `/Game/Audio/VO/Fenella/`, but the HUD already plays
  `/Game/Audio/VO/VO_<LineId>` by convention and `/Game/Audio` is always cooked. The
  25 WAVs import to `/Game/Audio/VO/` with their file names (`VO_RealityPickup_01`...).
  Only the 20 pickups and 5 `RealityIntro` clips are new; the older 84 already exist.
- **Pickup hooks** (all 20, each once per session, spent when shown): listed in
  `Docs/Design/Jokes.md` under "Fenella's lines, by trigger". New triggers sit with the
  story lines (queued, never dropped); `Direct()` still clears the queue, so a pickup
  wiped by a direction is said at its next trigger instead.
- **Caption length:** `ShowCaption` now loads the line's clip and keeps the caption up
  for its duration + 0.4 s (text-length timing if there is no clip).

## Requests (other threads' files)

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
2. **Row 1, sync.** Screenshot `-uiscreen` not enough: play to the arrival, listen to the
   five `RealityIntro` clips with captions; caption stays until the voice ends; talking
   to Fenella cuts both. Packaged smoke must still pass (clips are under the cooked `/Game/Audio`).
