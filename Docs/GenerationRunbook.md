# Generation runbook (for the agent that runs generation)

Hand this file to the agent (Codex, ChatGPT, or a Claude session) with one line: *"You are the ASSETS worker. Follow Docs/GenerationRunbook.md, stage `<test|batch1|batch2>`."* It is written so the agent can work without Dylan until a step says **stop**.

## Rules

- Work from the project root. Edit only `Assets/Generation/jobs.json`, `Assets/Incoming/`, `ImportSource/`, `Docs/Workers/assets.md` and the `Docs/Declared.md` lines that `gen_batch.py declare` writes. Never open the Unreal editor.
- **The jam is streamed.** Never print, `cat`, open or echo `.env` or any key. The scripts read keys themselves.
- Run Python as `python -I Scripts/<script>.py`.
- Never approve a group yourself. Approval is Dylan looking at the result.
- Commit at the end of each step (`git add Assets/Generation Assets/Incoming ImportSource Docs/Declared.md Docs/Workers/assets.md`). Never commit `Config/DefaultEngine.ini`. Never push.

## The tools

| Command | Does |
| --- | --- |
| `python -I Scripts/check-balances.py` | Credits left on Meshy (and Tripo, fal). Run first. |
| `python -I Scripts/gen_batch.py status` | Every job: done / pending / failed / blocked, and which groups are approved |
| `python -I Scripts/gen_batch.py run --stage S` | Runs all **3D** jobs of stage S **in parallel** (Meshy 8 at a time, Rodin 3, rig 1), retries blips once, stops a service that runs out of credits, writes `ImportSource/_gen/last_run.txt` and the review page. Takes 5 to 30 minutes. Run it in the background. Add `--dry-run` first to see the commands. |
| `python -I Scripts/gen_batch.py images --stage S` | Lists the **2D** jobs that are ready: the exact prompt, the aspect, the style reference to attach, and where to save |
| `python -I Scripts/gen_batch.py ingest <id>` | Takes the image you saved in `Assets/Incoming/2D/_drop/<id>.png`, keys out the background, trims it and marks the job done |
| `python -I Scripts/gen_batch.py review` | Writes `ImportSource/_gen/review.html`, a thumbnail grid of everything made so far |
| `python -I Scripts/gen_batch.py redo <id>` | Makes a job again; the old files are kept as `_v1` |
| `python -I Scripts/gen_batch.py declare` | Adds a `Docs/Declared.md` line for each finished asset (the jam requires it) |

**3D** comes from Meshy (characters, creatures, props, rigging, animations) and Rodin (sword, toothbrush) through their APIs; the runner calls them. **2D** comes from your own image generation (Codex/ChatGPT), on Dylan's subscription. If you can't generate images yourself, `run --stage S --images api` makes them through the OpenAI Images API instead (needs `OPENAI_API_KEY` in `.env`; costs API credit).

## Making a 2D image

For each item `images` lists:

1. Generate one image with the **prompt exactly as written** (don't shorten it; the style words lock the look). Use the listed aspect (square, wide or tall).
2. If a style reference is listed, attach that PNG as the reference.
3. Ask for a transparent background. If the result comes back on a flat colour instead, that's fine: `ingest` keys out a flat background.
4. Save it as PNG to the `save to` path, then run the `then` command.
5. Look at what `ingest` saved (`Assets/Incoming/2D/<id>/<id>.png`). If it has visible text that shouldn't be there (only stamps and posters may carry words), a frame on an icon, several items instead of one, or misspelt stamp words, run `gen_batch.py redo <id>` and generate it again, once. If it fails twice, leave it and note it.

## Stage `test` (the morning of Nov 12)

1. `check-balances.py`. If Meshy shows under about 300 credits, stop and tell Dylan.
2. `gen_batch.py run --stage test --dry-run`, then start `gen_batch.py run --stage test` **in the background**. This makes the hero, the sword, the rat, the house front and the peppermint, then rigs the hero and makes his animations (idle, walk, proud strut, one hand up, plus text-to-motion clips: sword raise, scrub, sigh).
3. While it runs, make the two 2D anchors from `gen_batch.py images --stage test`: `T_Skill_Lecture` (the knight layer) and `T_Card_Clapper` (Fenella's layer).
4. When the run finishes, read `ImportSource/_gen/last_run.txt`. For any failure, read `ImportSource/_gen/logs/<id>.log`. If the cause was transient (timeout, network), re-run the same command, once; it only redoes what isn't done. If it was a bad prompt, leave it for Dylan.
5. Look at every thumbnail (`ImportSource/Meshy/*.png`, `ImportSource/Hyper3D/*`, and the 2D PNGs) and write `Docs/Workers/assets.md`: for each test asset, one line on what it looks like and anything wrong with it (wrong object, wrong style, text baked in, not A-pose, back of the house not flat). Say which of the hero's animations exist, and whether the rig worked at all. That answers the open question of whether the Meshy rig route works.
6. `gen_batch.py review`, `gen_batch.py declare`, commit.
7. **Stop.** Tell Dylan the test set is ready for approval and point him to `Docs/Workers/assets.md` and `ImportSource/_gen/review.html`. The 3D assets get approved in the game camera, which the GAME worker sets up.

## Stage `batch1` (after Dylan approves)

Dylan approves one or more groups: `character`, `equipment`, `npc`, `terrain_piece`, `particle_mesh`, `2d_knight`, `2d_fenella`. Run `gen_batch.py approve <the groups he named>` (only those), then:

1. `gen_batch.py run --stage batch1` in the background. That's about 20 meshes and the toothbrush, in parallel. Unapproved groups are skipped and listed.
2. Meanwhile, work through `gen_batch.py images --stage batch1`, must-priority items first. About 40 images, each attached to its layer's anchor as the style reference.
3. Same failure handling, look-over and `Docs/Workers/assets.md` notes as in the test stage. Then `review`, `declare`, commit.
4. **Stop** and report: what's done, what failed and why, and the credits left (`check-balances.py`).

## Stage `batch2`

Same as batch1 with `--stage batch2`, only when Dylan says so (day 3 or earlier if ahead).

## If Dylan rejects something

He'll say what's wrong. Change the job's `subject` in `Assets/Generation/jobs.json` (or, if the whole look of a group is wrong, the group's words in `Docs/styles.json` / `styles_2d`, which affects everything after), then `gen_batch.py redo <id>` and run again. Note the change in `Docs/Workers/assets.md`.

## Where things land

| What | Where |
| --- | --- |
| Meshes and their preview renders | `ImportSource/Meshy/<id>.glb` + `.png`, `ImportSource/Hyper3D/<id>.glb` |
| The rigged hero and animations (UE5 mannequin skeleton) | `ImportSource/Meshy/SK_Hero_Rig/SK_Hero_Rig.fbx`, `A_SK_Hero_Rig_<Anim>.fbx` |
| 2D | `Assets/Incoming/2D/<id>/<id>.png` (keyed and trimmed), `<id>.raw.png` (as generated) |
| Full prompts actually sent | `manifest.jsonl` beside each output |
| Run state, logs, review page | `ImportSource/_gen/` |
