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

## Next
T12a (note clamp and keyword fallback, against `NoteForm.md`), then the Scene 2 and 3 tables with the finishability test.

## Requests to GAME
None yet.
