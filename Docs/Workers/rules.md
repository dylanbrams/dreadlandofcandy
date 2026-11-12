# RULES worker notes

## Done
- T03: done by GAME on `main` (merged into `rules`).
- **T04 ledger** (commit on `rules`): `Public/Film/JamLedger.h`, `Private/Film/JamLedger.cpp`, tests `Jam.Film.Ledger.*` (Floor, Shape, Loot, Deterministic, Idempotent, Table). 14/14 `Jam.Film` pass.
  - `JamLedger::Compute({Gross, Floor, Seed, LootRarities})` -> lines (4 to 6: Director's fee, Producer's fee, seeded middle items, Miscellaneous last and largest), `DeductionsTotal`, `HeroCut`, `KeptLootIndex`, `FooterIndex`.
  - Hero's cut = floor + about 5% of gross (clamped to gross). Deductions fill the rest, so cut + deductions = gross.
  - She keeps the highest rarity, first on ties; INDEX_NONE if fewer than two drops.
  - `FJamLedgerCommit::Commit` computes once and returns the stored result thereafter.
  - GAME: seed with scene index and take count; text (item, note) is in the result, footers via `JamLedger::Footers()`.

- **T09 lines + nag scheduler**: `Public/Film/JamLines.h`, `JamNagScheduler.h` (+ `.cpp` in `Private/Film/`), tests `Jam.Film.Nag.*` (Table, GlobalGapAndCooldown, SilentAndStory, NoRepeat, Deterministic, SulkCommentaryOnce, PollThresholds). 21/21 `Jam.Film` pass.
  - `JamLines::All()` / `ForTrigger(t)` / `Find(id)`. Ids are `<Trigger>_<nn>` in table order (e.g. `Idle_03`), the VO convention. Append only once clips exist. Covers every Fenella pool in Jokes.md plus the commentary and sulking pools (T22's text is in), default/sadder/budget lecture bubbles, nuisance replies and production notes.
  - `FJamNagScheduler(seed)`: `Request(trigger, snapshot)` for events, `Poll(snapshot)` about once a second (derives strayed after 6 s, idle after 20 s, commentary), `PickFromPool(trigger)` for bubbles, replies and production notes (ungated, no repeats until exhausted), `Reset(seed)` per run.
  - Story triggers (SceneStart, Level7Intro, FailedTake, Take3..6, CutPitch, NoteTaken/Refused, ForcedRetake, NarrationCut, Wrap, BudgetOut, Premiere, Pause) always speak. Ambient ones obey gap 8 s (commentary 25 s), per-trigger cooldown 20 s, `bSilenced`. `bSulking` turns ambient reactions into the Sulking pool. Level7Intro and Take3..6 are once per run. `JamLines::TriggerForTake(take)` picks SceneStart / Take3..6.
  - GAME T11: build a `FJamNagSnapshot` from game state each poll; set `bSilenced` for dialogue, menus and the CUT! prompt. The scheduler never touches rules.

- **T04a progression**: `Public/Film/JamProgression.h`, `Private/Film/JamProgression.cpp`, tests `Jam.Film.Progression.*` (Start, LevelsLandOnCards, PointsNeverExceedLevels, StatsTotal, Tables, RollLoot). 27/27 `Jam.Film` pass.
  - `FJamProgression` (put one on the PlayerState): starts level 7, 0 points. `AddXp(source)` (Pacify 8, Treat 12, Collect 6) fills the bar but is held at `XpForLevel(next)-1`; **only `ReachMilestone(Scene1Wrap | Scene2Wrap | Scene3FinalNote)` raises the level** (8 / 9 / 10; thresholds 100 / 250 / 450). Returns levels gained (0 if repeated), so call it at the wrap card and the Scene 3 CUT!. Level 10 unlocks The Last Judgment by itself.
  - `SpendPoint(skill)` / `CanSpendPoint`: buys rank 1 of Form A Line / Sigh, or raises any rank (Lecture to 4, others to 3). Never the ultimate. Invariant (tested under random play): `6 passives + spent + points == level - 1`.
  - `GetStats()`: Lecture Power, Radius, Cooldown Recovery, Move Speed (base 60 / 800 / 1.0 / 600 at rank 1, then rank and loot). `GetSkillNumbers(skill)` is the rank row with loot applied (damage, radius, line length and width, stop/slow seconds, slow factor, cooldown): feed the GAS abilities from it. Rank data also at `JamProgression::RankData`.
  - Loot: `JamProgression::LootTable()` (the 12 items of Jokes.md, with stat, signed percent, rarity 0 to 3), `RollLoot(seed, 3)` gives three distinct indices ordered worst to best, so the last card is the one she keeps. `AddLoot(index)` stacks (napkin +1% helps every stat incl. cooldown). For the ledger pass `LootRarities` from the same rolls; `KeptLootIndex` is then the best card. Show `RarityName(rarity, bDirectorsPick)`; passives for the pre-spent panel in `Passives()`.
  - Numbers for ranks and XP are my placeholders (BUILD_SPEC 10 does not set them); tune in `JamProgression.cpp`.

- **T12a note clamp + keyword fallback**: `Public/Film/JamNoteClamp.h`, `Private/Film/JamNoteClamp.cpp`, tests `Jam.Film.NoteClamp.*` (ValidResponse, GarbageJson, OutOfRangeKnobs, OverLongText, RejectsAndDropsUnknown, Unfinishable, Keywords, GrossFromKnobs). 35/35 `Jam.Film` pass.
  - `JamNoteClamp::FromResponse(body, ctx, out, reason)`: whole response body in, `FJamClampedNote` out (`Def` = `FJamNoteDef` with the replacement stage and a client-computed `GrossMultiplier`; `Cosmetics` = scale, tint, weather, mood, motif, scene title, Fenella's reaction, bubbles, replies, loot name/flavour). False means use `FromKeywords`, then the scene's authored note. Rules as NoteForm.md: v must be 1; source must be cache/model; every field present with the right type; knobs clamped (not rejected); text cleaned (control chars, whitespace) and cut; unknown fields dropped; unknown tint/weather/mood become none/none/cheerful; unknown `target_kind` becomes keep; bubbles 3 to 5, replies 2 to 3; time limit 0 or 30 to 120; must be finishable.
  - `FJamNoteContext`: build from the stage the note replaces: `BaseStage`, `Kinds` (kind, capacity) the set spawns, Lecture damage/cooldown (defaults 60 / 1.5). For the request: `set_kinds` and `set_capacity` = `MaxCapacity()`. The note's verb is always the base stage's.
  - Finishable: Count x (2 s travel + hits x cooldown, or +0.6 s for interact) must fit in 60% of the time limit; no clock is always finishable. Authored notes can use `JamNoteClamp::IsFinishable` in their tests.
  - `JamNoteClamp::SanitizeNote(typed)`: what the client sends and shows (80 chars, no control characters).
  - `FromKeywords(typed, ctx, seed, out)`: false if it understands nothing. Words: bigger/smaller, more/fewer, faster/slower, tough/weak, sad/epic/happy, rain/snow/fog, clock, five tints. Text comes from Jokes.md pools (`NoteTaken` reaction, default or sadder bubbles, nuisance replies). **Fenella's reaction is set; `SceneTitle`, `LootName` and `LootFlavour` are left empty and `Motif` is empty: GAME keeps the scene's own.**
  - **Request to GAME:** I wrote a private mini JSON reader (`Private/Film/JamMiniJson.*`) because `MegaJam.Build.cs` has no `Json` module. For the HTTP client (T12b) you will want `Json`/`HTTP` in Build.cs anyway; the clamp takes the body as a string, so it does not care.

- **Scenes 2 and 3 as data** (`JamScenes::Scene2()`, `Scene3()`, `All()` in `Private/Film/JamShoot.cpp`; `SetCapacity` extended). Tests `Jam.Film.Scenes.Finishable` (every stage and every authored note fits its set and is finishable with the Lecture alone) and `Jam.Film.Scenes.Playthrough` (each scene driven to its wrap once, with the note taken and refused). 36/36 `Jam.Film` pass.
  - Scene 2 "Fort Toothache": pacify 3 goblins, then brush 3 (60 s countdown). CUT! before the brush stage, pitch `CutPitch_02` ("stronger antagonist"): one boss goblin (Resolve x3.2 = 480, speed x0.714 = 250), brush it, 60 s, gross x1.5.
  - Scene 3 "The Almond Affair": disperse 5 bugs (meter fails at 10), gather 4 almonds, final stage 5 bugs (meter 10). CUT! before the final stage, pitch `CutPitch_06` ("make it more"): 8 bugs, speed x1.2, gross x1.5. The budget-out beat itself is GAME's (T21).
  - **Set capacities GAME must provide** (the game mode's spawn markers): Scene 1 rats 3 (as built); Scene 2 goblins 6; Scene 3 bugs 8 and almonds 4. The typed-note clamp uses these as `set_kinds`/`set_capacity`: `JamNoteClamp::MakeContext(scene, stageIndex)` builds the context for a request. Change a capacity in `JamScenes::SetCapacity` and the tests re-check everything.
  - An Interact stage with a changed Resolve multiplier (the boss) spawns fresh targets that need pacifying before the brush; the finishability model counts that. The boss must therefore be spawned untreated and un-pacified when the note stage starts.
  - `PitchLineId` of the notes is a `JamLines` id (`CutPitch_02`, `CutPitch_06`); the authored note-taken/refused reactions are the `NoteTaken` / `NoteRefused` pools (use `Request(EJamLineTrigger::NoteTaken, ...)`).

## Next
Day 1 and day 2 RULES backlog is done. Waiting on GAME requests (rule fixes, the T22 commentary toggle is already in `JamLines` as `Commentary` + `FJamNagSnapshot::bCommentary`).

## Requests to GAME
- Merge `rules` into `main` (it is ahead; builds and passes `Jam.Film` in this worktree). New headers: `Film/JamLedger.h`, `JamLines.h`, `JamNagScheduler.h`, `JamProgression.h`, `JamNoteClamp.h`.
- Add `Json` and `HTTP` to `MegaJam.Build.cs` for T12b if you want the engine's JSON; not needed for the clamp.
- Provide the Scene 2 and Scene 3 set capacities above.
