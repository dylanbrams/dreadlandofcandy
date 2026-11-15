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

## Sunday morning, Nov 15 05:00–11:00 (added 11:10)

Times are git commit stamps (+0200). The DEVLOG's "~08:30 JST" heading is the same 08:28 commit; the JST label is a slip. Commits without a Claude trailer (devlog kit, UI concepts, candy environment, marshmallows, Black Sugar UI) were made by ChatGPT/Codex sessions; the DEVLOG names ChatGPT for the candy environment only, so confirm the rest before crediting by name.

| Time | Event | Evidence | Limits |
| --- | --- | --- | --- |
| 05:08–05:16 | Each villager gets 2–4 encores, then afterglow; review of the Fixes2/3 range fixed seven gameplay bugs. | `cac48cf`, `e92d51f`, DEVLOG ~05:30 | Not live-tested. |
| 07:46 | ChatGPT devlog kit (this folder). | `0d5e107` | |
| 08:28 | Repackage with the night's fixes: smoke PASS (0 errors), ZIP 737.7 MB, full village autoplay 3 main + 10 side; archer bow to left hand. | `634ec22`, DEVLOG | Last smoked package; everything after this is unpackaged or unsmoked. |
| 08:38 | Submission zip excludes Saved; package stages the prerequisites installer. | `fc565e5` | |
| 08:39–08:44 | Four UI directions (Candy Arcade, Frosted Stickerbook, Saccharine Reliquary, Black Sugar). | `57b3406`, `e2ccf3b`, `Docs/Design/CandyUI/` | |
| 08:46 | 15 fal PATINA PBR sets reviewed and picked; paving, mint ground, hot chocolate river, 36 trees, 73 shrubs, 23 backdrop mountains, 30 steam cards. | `084030d`, `../CandyPBR/`, DEVLOG | Editor captures; packaged delivery and performance unverified. |
| 08:49 | Encore EXP and levels past 10 (+5% power each). | `247af6f` | Not seen live. |
| 09:17 | Three generated, Blender-cleaned marshmallows replace 13 cylinders; drift 45 cm/s with bob. | `4cc1ac3`, `../Marshmallows/` | |
| 09:36 | Black Sugar UI: menus, HUD, compass, OFL fonts, crest, cooldown numbers; 82 tests. | `05e5721`, `Docs/Design/CandyUI/IMPLEMENTATION.md` | Six screens reviewed; 25-screen sweep and gamepad playtest skipped to save usage. |
| 10:22 | Fixes4 #1–8 from Dylan's playtest notes (`Docs/Feedback/Fixes4.txt`). 85 tests. | `8a4ece6` | Packaged, not smoked or zipped. |
| 10:39 | Fixes4 #9–10: river channel, bridge, 1,432 road instances removed, buildings off paths, ~1,930 brush instances. | `6da7023` | Review captures; not packaged. |
| 11:00 | Fixes4 #11: camera lifts over terrain, buildings, canopies; walkable slope 52°. | `1dd9094` | Not packaged. |

Before/after captures for the deck are copied into `assets/morning/` from `Saved/Screenshots/WindowsEditor/` (river-before = `marshmallow-flow-18.png`, river-after = `fixes4b-L_RiverReview.png`).

Quest-length note in PersonalNotes: the supporting research is the IKEA effect (Norton, Mochon & Ariely, 2011, Harvard DASH), where effort raised valuation only when the task was completed. No study found on quest length and attachment in games; the script presents it as Dylan's hunch.
