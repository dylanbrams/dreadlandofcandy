# RULES worker notes

## Done
- T03: done by GAME on `main` (merged into `rules`).
- **T04 ledger** (commit on `rules`): `Public/Film/JamLedger.h`, `Private/Film/JamLedger.cpp`, tests `Jam.Film.Ledger.*` (Floor, Shape, Loot, Deterministic, Idempotent, Table). 14/14 `Jam.Film` pass.
  - `JamLedger::Compute({Gross, Floor, Seed, LootRarities})` -> lines (4 to 6: Director's fee, Producer's fee, seeded middle items, Miscellaneous last and largest), `DeductionsTotal`, `HeroCut`, `KeptLootIndex`, `FooterIndex`.
  - Hero's cut = floor + about 5% of gross (clamped to gross). Deductions fill the rest, so cut + deductions = gross.
  - She keeps the highest rarity, first on ties; INDEX_NONE if fewer than two drops.
  - `FJamLedgerCommit::Commit` computes once and returns the stored result thereafter.
  - GAME: seed with scene index and take count; text (item, note) is in the result, footers via `JamLedger::Footers()`.

## Next
T09 (lines table from Jokes.md + nag scheduler), then T04a, T12a.

## Requests to GAME
None yet.
