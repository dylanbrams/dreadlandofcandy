# Evidence and timeline additions

Checkpoint assembled from records available through Nov 15, approximately 01:45 project time. These are snapshots, not a claim that later work or submission has happened. Git timestamps are +0200; the deadline is stated separately in PST.

| Date / point | Event | Evidence | Limits |
| --- | --- | --- | --- |
| Nov 14 09:44 | Blocker batch addressed compass, travel HUD, grey-screen effect and bow orientation. | `Docs/DEVLOG.md`, GAME batch 1 | Historical checkpoint. |
| Nov 14 10:00 | Controls and XP batch; 72 tests and three-scene Open autoplay reported. | `Docs/DEVLOG.md`, GAME batch 2 | Historical count. |
| Nov 14 14:45 | Dylan restored village quest givers as a core feature: five villagers and ten linked jobs. | `Docs/DEVLOG.md` Fixes1; `Docs/Design/RELEASE_PLAN.md` Nov 14 afternoon | Supersedes earlier cut. |
| Nov 14 | P1 package: three main quests and ten village jobs in packaged smoke; 79 tests; 688 MB ZIP. | `Docs/DEVLOG.md`, P1 package | No upload recorded in that entry. |
| Nov 14 evening | Codex took over Fixes2 after Claude quota ended. Village offline, proposal and fallback runs later completed ten jobs; 82 tests reported. | `Docs/DEVLOG.md`, Fixes2 entries; `Docs/Workers/codex-fixes2.md` | Manual interaction and audio checks remained. |
| Nov 14 late | Riverlands expansion, bridge, village buildings and foliage; logged playable area increase of 58.3%. | `Docs/DEVLOG.md`, Sugarborough and Riverlands redesign | Manual crossing and cooked delivery were unverified at that checkpoint. |
| Nov 15 00:45–01:25 | Candy foliage mix, class skill sounds and attack FX; 82 tests reported. | `Docs/DEVLOG.md` Nov 15 entries; commits `690505d`, `d1b054f`, `0cd724b` | Effects and sounds had not been judged live. Commit stamps are snapshots, not work duration. |
| Nov 15, latest recorded checkpoint | Fresh package, basic smoke and ZIP succeeded; 737.7 MB. | `Docs/STATUS.md`; `Saved/pkg-nov15b.log`, `Saved/smoke-nov15b.log`, `Saved/zip-nov15b.log` | Dylan's hands-on playtest remains outstanding; this is not evidence of jam submission. |

## Story leads to recover

- Find the initial MMO exchange before quoting it. Dylan's MMO history and hills joke are in `../PersonalNotes.txt`.
- Compare the original film with the decision to make the village and quest givers core. Use dated revisions of the release plan.
- The failed package / old executable and memory pressure are documented in `../Checkpoint1/CHECKPOINT.md`; recover primary logs before making a detailed claim.
- Credits and pre-jam disclosure: `Docs/Declared.md`; worker histories: `Docs/Workers/`; chronology: `Docs/DEVLOG.md` and git.
- Capture actual gameplay, voice, director notes, village, class attacks and river crossing. Confirm what is in the final build.
- Record Dylan's firsthand decisions, final submission and reception when known.

## Asset comparisons

`assets/generation-comparison.jpg` shows three original rat candidates rendered from `ImportSource/Meshy/SM_Rat.glb`, `ImportSource/Hyper3D/SM_Rat_Rodin.glb` and `ImportSource/Fal/SM_Rat_Fal.glb`, and a cleaned Tripo candy apple tree from `Assets/DylanGenerationFoliageTwo/Clean/SM_CandyAppleTree.glb`. See `Docs/Declared.md`, `Docs/Workers/assets.md` and the foliage delivery README. The rat prompt for fal/Hunyuan was revised after moderation; the tree is a different subject from a later batch. These are source-asset Blender renders with normalized framing, not in-engine screenshots or a controlled four-way benchmark. The initial selection notes say Dylan picked the Rodin rats, Meshy hero and peppermint, and fal houses (`Docs/DEVLOG.md`, Nov 12 07:25). A later user-provided Tripo foliage delivery is documented in `Docs/Declared.md` and its own README; early records describing Tripo as unused predate it.

## Gantt handoff

`timeline.csv` adds evidence-backed later event markers. They are points, not invented durations or exclusive authorship claims. The existing `../Checkpoint1/gantt/work_log.csv` and charts are historical estimates; do not add bar lengths to infer effort. Integrate these markers into a revised Gantt once session start/end evidence is recovered.
