# STATUS - GAME, Nov 14: batch 1 verified; batch 2 next

Read CLAUDE.md and Docs/Design/RELEASE_PLAN.md (Nov 14 section).
Dylan: finish existing batches before packaging. Do not package or upload yet.
Original wishlist/history: Docs/Archive/STATUS-before-batch0-2026-11-14.md.

## Next task

Batch 2 SHIPs: Q/E camera, right-drag, wheel zoom, F use, R ultimate,
middle mouse signature; read-only controls card in Options and Pause;
numeric XP (disable, brush, quest bonus), about 1.6 levels across three sets.
Character-sheet spending and automatic opening were finished in batch 0.
Quest givers and outer quests stay cut. No Backlot tests or fixes.

## Batch 1

- Item 5: compass target label now 34 design pixels below the strip;
  removed the duplicate line of navigation/camera text underneath.
- Item 15b: travel now builds the skill/XP HUD independently of the shoot;
  previous scene objectives/wraps stay hidden. Travel skills and character
  sheet remain mouse reachable (extended Slate hit-path test).
- Item 13: found the global grey checker replacement and desaturation in
  BudgetOut, not a rain-specific post-process. Open now preserves world and
  gear materials/grade; cardboard enemies, voice and budget effects remain.
- Item 17: archer grip turns 180 degrees about the bow's long axis in the
  left hand, without changing its upright alignment or back carry.
- Item 11 already finished in batch 0; do not repeat it.
- Editor and Development game builds and all 69 Jam tests pass.
- Inspected batch1-travel.png, batch1-archer.png, batch1-budget.png under
  Saved/Screenshots/WindowsEditor. Budget-out world remains coloured.
- Saved/Batch1-Editor-Open.log: PASS mode=open main=3 side=0;
  PASS scenes=3 first=1 takes=4 notes=3/0 ending=directors_cut flees=3
  lines=11 skills=12 hits=1 dignityfails=0 time=107.1.
- No package, ZIP refresh, upload or push (Dylan's instruction).

## Earlier verified work

Batch 0: clickable HUD/notes/prompts/wrap/pause, persistent cursor,
GameAndUI input, level-up character sheet and skill spending, grounded enemy
poses. Open is the no-flag default on L_Sugarborough; -backlot retained.
Manual swapped-button/refocus input, typed directions and cast approval
still need Dylan's play. Backend redeploy is Dylan's task.

## Existing tester ZIP (batch 0, stale relative to source)

Builds/ImaginaryComponent_TheDreadLandOfCandy.zip: 721,385,002 bytes.
Batch 0 no-flag Open packaged smoke passed (Saved/R7Mouse/Packaged-Open.log).
Batch 1 is source/editor verified only until packaging is authorized again.
Itch handoff: Marketing/Itch/LUNA_UPLOAD_HANDOFF.md; explicit go per push.

## Remaining

Finish batch 2; then tester-feedback blockers and R8 declarations/credits/tools.
Final package, Open smoke and ZIP are deferred by Dylan; no git push or upload.
Engine handoff: BUSY, GAME continuing into batch 2.
