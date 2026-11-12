# ASSETS handoff

Updated 2026-11-12. **Test stage finished: all 14 jobs done; ready for Dylan's review.** The fal rat succeeded after Dylan authorized a simpler subject. This session's instruction to run the test stage supersedes the previous prompt-only hold. No groups are approved, no routes changed, and no batch1/batch2 generation ran. Unreal was never opened.

Review grid: [review.html](../../ImportSource/_gen/review.html). GAME should put the meshes in the game camera for Dylan's approval. Observations below come from every generator thumbnail, the ingested UI PNGs, GLB structure and Rodin base-color atlases; they are not in-engine acceptance tests.

## Test asset inspection

| Asset | Result and visual notes |
| --- | --- |
| `SK_Hero` | Done, Meshy. Dark angular plate armour, closed pointed helm, broad shoulders, empty open hands, separated legs and arms lowered away from torso in an apparent A-pose. Good dark-fantasy identity; **cape-like cloth behind the left arm despite "no cape"**, fairly clean tabard rather than visibly scorched/tattered. No visible lettering or candy. 41,148 triangles. |
| `SK_Hero_Rig` | Done. Rigged FBX/GLB plus all seven requested animation FBXs and BasicWalk exist. Rig service succeeded with requested `ue5_mannequin` template; GLB has one skin and 28 joints. Finger bones absent from that GLB skeleton. Deformation, grip, retarget compatibility and motion quality need GAME review. |
| `SM_Sword` | Done, Rodin. Straight pointed blade, angular crossguard, wrapped long grip and pommel, no wielder/base/visible inscriptions. Grey thumbnail; embedded texture atlas contains worn steel and dark brown leather. Blade looks narrower and less comically oversized than requested; scale and hand fit remain to check. **500,000 triangles**, reduction needed. |
| `SM_Rat` | Done, Meshy. Cream upright rat, large pink ears/nose, raised paws, thin pink toes, long whiskers, curled tail. Friendly but more anatomical/detailed than chunky cartoon brief; paws forward rather than neutral A-pose. Thin parts may read poorly at game scale. No text/base. 5,175 triangles. |
| `SM_Rat_Rodin` | Done. Rounder toy-like mouse/rat, big head, simple face, stubby feet/paws, thick upward-curled tail. Closer cartoon silhouette than Meshy, though tail rises behind head and paws held inward. Grey thumbnail; atlas confirms cream/pink and simple dark eyes. **500,000 triangles**. |
| `SM_Rat_Fal` | **Done after simplifying subject.** Cream/pink faceted toy rat, very large round ears, simple black eyes, pink paws/feet and thick curled tail. Friendly chunky cartoon silhouette, somewhat mouse-like. Arms close to sides rather than neutral A-pose; tail protrudes away from torso. No visible lettering or pedestal. 40,000 triangles, one material/texture; reduction still needed. Original subject rejected twice; revised subject succeeded on first attempt. |
| `SM_HouseFront` | Done, Meshy. Bright orange gingerbread front, white icing, red door/windows, chunky low-poly surfaces. **Broad ground slab and substantial roof depth**, contrary to flat film-set facade brief. Front preview does not establish planar back. No visible text. 4,107 triangles. |
| `SM_HouseFront_Rodin` | Done. Shallow peaked house-shaped slab, raised door/window/icing details, no visible ground platform. Best facade silhouette available. Grey thumbnail; atlas has orange gingerbread, red, white and blue accents. **Back planarity unverified** from front/side render. **500,000 triangles**. |
| `SM_HouseFront_Fal` | Done. Cheerful orange/white/pink candy cottage, clean cartoon shapes. **Full side walls, deep roof, green ground platform and small vegetation**, fails shallow front-only requirement. No visible lettering. 40,000 triangles. |
| `SM_Peppermint` | Done, Meshy. Strong red/white pinwheel disc, two pale chunky twisted wrapper ends. Readable faceted silhouette; slightly glossy rather than matte. No base/lettering. **418 triangles**, close to 400 target and most suitable particle starting point. |
| `SM_Peppermint_Rodin` | Done. Flattened candy with spiral relief but **four diagonal wrapper tips**, not two. Grey thumbnail; atlas has red/white candy and cream wrapper. Too elaborate for tiny particle. **500,000 triangles**. |
| `SM_Peppermint_Fal` | Done. Bright red/white swirl, faceted cream wrap, two chunky ends. **Adds red oval pedestal** and thick bowl-like rim; less suitable particle silhouette. 40,000 triangles. |
| `T_Skill_Lecture` | Done, built-in Codex image generation; exact runner prompt, no reference. Raised dark steel/gold greatsword held by gauntlet, blank cream comic burst, red accents, torn wrist cloth. Adds arm/cloth and prominent red blade streaks; check small-size readability. Single composite icon, no visible text or external frame. Transparent PNG trimmed to **881 x 1024**; square raw original retained. |
| `T_Card_Clapper` | Done, built-in Codex image generation; exact runner prompt, no reference. Open black/white clapper, exposed corrugated cardboard, cream paper writing area, yellow tape. Good handmade film-prop appearance; paper fully blank. Single item, no visible text or external frame. Transparent PNG trimmed to **943 x 1024**; square raw original retained. |

## Comparison recommendations for Dylan

Review preferences only; no approval or routing written.

| Group / trio | Closest available result and tradeoff |
| --- | --- |
| Rat / NPC | **fal now looks closest** to the bright low-poly cartoon brief: faceted chunky toy body, simple face and thick appendages. Rodin is also friendly but smoother and 500,000 triangles; Meshy's thin feet/whiskers look more anatomical. fal is still 40,000 triangles versus Meshy's 5,175, and its tail protrudes. **fal used a revised simpler subject, so this is not an identical-prompt comparison.** |
| House / terrain piece | **Rodin** for shallow standalone facade without ground slab. Meshy gives useful low-poly candy look but added ground/roof depth; fal made full house. Check Rodin back before approval. |
| Peppermint / particle mesh | **Meshy** for clear swirl, two wrapper ends, no pedestal and 418-triangle geometry. Rodin has four tips/excessive geometry; fal has pedestal and 40,000 triangles. |

## Rig and animation delivery

Files in `ImportSource/Meshy/SK_Hero_Rig/`:

- `SK_Hero_Rig.fbx` and `SK_Hero_Rig.glb`: rig succeeded. GLB joint names include pelvis, spine, clavicles, arms, hands, thighs, calves, feet and balls of feet.
- `A_SK_Hero_Rig_Idle.fbx`, `A_SK_Hero_Rig_Walk.fbx`, `A_SK_Hero_Rig_ProudStrut.fbx`, `A_SK_Hero_Rig_OneHandUp.fbx`: all saved successfully.
- `A_SK_Hero_Rig_SwordRaise.fbx`, `A_SK_Hero_Rig_Scrub.fbx`, `A_SK_Hero_Rig_Sigh.fbx`: text-to-motion and subsequent animation tasks succeeded and saved FBXs.
- `A_SK_Hero_Rig_BasicWalk.fbx`: additional walk supplied with rig. `rig.json` retains task IDs for resuming.

All nine FBXs are nonempty binary FBX version 7400. All eleven static GLBs have valid headers and matching declared file sizes. Triangle/material/texture counts and position bounds: `ImportSource/_gen/inspection/mesh_stats.json`. Every static mesh has **one material slot**: independent tint regions, grips, pivots, dimensions and collision need cleanup/review. Rodin thumbnails are untextured geometry previews, not proof of missing textures: all four GLBs contain three texture references and their base-color atlases were inspected. Extracted atlases are under `ImportSource/_gen/inspection/`.

The rig route works through file delivery. SwordRaise's 0.3-second raise/hold, toothbrush alignment, deformation and motion quality remain untested until GAME previews them. Both UI anchors inspected after ingest: transparency retained, no lettering, duplicate items or decorative icon frames. No 2D redo needed. Raw images are 1254 x 1254; ingest trims and caps the long side to 1024.

## Execution, credits and handoff limits

- Meshy balance: **1,100 before, 864 after** (236 used including interrupted work). Tripo: **0**. Rodin has no balance endpoint in checker. fal balance endpoint returns HTTP 403 despite successful house/peppermint generation.
- Server restart interrupted first runner after five meshes completed. Confirmed no generation Python process remained, removed stale lock, retried only six unfinished jobs once. Completed meshes kept. Old remote tasks may have continued, so some credits may cover undownloaded interrupted results; manifests are not a complete billing ledger.
- After the six-job recovery, Dylan requested one unchanged-prompt rat retry at 07:20: the same HTTP 422 rejection. Dylan then authorized a prompt change. At 07:22 the simpler toy-rat subject succeeded on its first attempt, keeping the shared NPC style and 40,000-face request unchanged. No other assets regenerated. `last_run.txt` now describes the successful revised rat (1 done, 0 failed), **not whole stage**. Final `gen_batch.py status --stage test`: **14 done**, none failed/pending. Attempt history remains in `ImportSource/_gen/logs/SM_Rat_Fal.log`; full old/new requests in `ImportSource/_gen/SM_Rat_Fal_prompt_history.json`. Meshy balance last checked at 864.
- fal only reported a generic content-checker rejection (`content_policy_violation`, category `model_moderated`), without identifying a word or specific reason. Each job sends one prompt string (shared style prefix + subject + suffix) and `face_count=40000`, with no conversation history. The house and peppermint were separate successful fal prompts. The simplified rat success does not establish which wording triggered the earlier rejection.
- **Dylan's preference: do not check fal balances; its balance endpoint does not work.** No further fal balance requests after that instruction.
- Review page regenerated; **14 finished-asset declarations** added via `gen_batch.py declare`. Full submitted prompts in manifests; only `SM_Rat_Fal.subject` changed in `Assets/Generation/jobs.json`. Revised subject: "a friendly cartoon toy rat figurine, cream fur, round pink ears, pink nose, black eyes, short paws and a thick curved tail, chunky toy proportions, standing upright alone". Earlier prompt pack remains at `Assets/Prompts/README.md`.
- Static GLBs at `ImportSource/*/*.glb` are ignored by existing `.gitignore`; mesh files remain on this checkout for GAME. FBXs, nested rigged GLB, previews, UI PNGs, manifests, state and review artifacts are included in the scoped commit. Logs/drop scratch remain ignored.

**Stop here for Dylan's approval.** GAME can prepare camera comparisons. Remaining issues: hero cape-like cloth, facade geometry, particle cleanup, mesh reduction and animation/camera validation. Successful generation is not asset approval.
