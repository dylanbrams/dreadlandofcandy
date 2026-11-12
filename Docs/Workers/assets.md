# ASSETS handoff

Updated 2026-11-12. **Batch1 complete: 17 models and 34 UI images. Batch2 complete: 8 models and 16 UI images.** Dylan authorized both batches. All 75 batch jobs are done; none failed or remain pending. This is a generation/file-delivery status, not in-engine validation. ASSETS has not opened Unreal. The separate test-stage sword redo remains outside these batches and was not run.

## Cleanup handoff — 2026-11-12

Dylan authorized reprocessing and repair after the decimation review. Use **`ImportSource/_decimated/`** for GAME's next import pass. [Current report, previews, validation and placement notes](../../ImportSource/_decimated/_review/REPORT.md). This supersedes the first decimation review. No editor/content changes or commits were made by the cleanup session.

- **31 clean GLBs:** 30 Hyper3D/Fal plus batch-2 Meshy shard. Closed saved-file topology, finite normals/UVs, original hashes, base-colour pixels and triangle budgets pass. Characters are 25k; most props are 8k; simple repaired accessories/shard are lighter. Raw originals and `_v1` remain intact.
- Removed the knight in the gate, table chairs, clapper limbs/stubs, shard stick, cable and unwanted bases; capped cuts. Rebuilt hollow Hat/HeadMirror/LeafMegaphone and a cable-free lamp lower assembly. Wearables were rendered on Villager. Their openings and five gate passage rays pass.
- Sword blade length/proportions were corrected from the existing source, retaining overall height/paint. No paid redo was submitted; the runner's separate old sword redo status is not a new source model.
- Clean exports have ground/grip/particle/wearable origins and material regions: Goblin Teeth/Skin, Molar Enamel, LollyBug Wings. **12 UCX collision FBX companions** and per-asset policies are in `_decimated/_collision/`. These FBXs have neutral preview materials; retain GLB appearance. Unreal import/camera checks, material instances, socket hookups and gameplay traversal remain GAME's integration work.
- Scale-1 Villager wearable mounts in Blender local metres: Hat `(0, -0.03, 1.898)`; HeadMirror `(0, -0.03, 1.65)`. Gate needs about **7.72×** uniform scale for a 2.20 m high passage; scale collision with it. See report for axes, actor capsule offsets and reproducibility.
- **Batch 2 was already complete** (eight 3D and sixteen UI jobs). Existing meshes were repaired; generation was not repeated and no API credit was spent.

## Current handoff (original generation)

- Approved groups from Dylan's recorded review: character, equipment, NPC, terrain piece, particle mesh and both 2D layers. Effective routes: NPCs on Rodin, terrain pieces on fal, particles on Meshy. Equipment stays on Rodin. No credit fallback was needed.
- Batch1's 3D run: 17 done, 0 failed, 11.1 minutes. Fenella was subsequently rejected and regenerated successfully with Dylan's new direction. Batch2's 3D run: 8 done, 0 failed, 8.0 minutes. Actual service delivery does not establish gameplay suitability.
- **Fenella supersedes the old brief:** a ridiculously thin little flying fairy director, visible pink/gold wings, long skinny limbs, a small angular face, and a handheld digital VIDEO CAMCORDER held at chest height away from her face. Both eyes visible. Voice direction: a tiny high piping voice; no new audio generated here. The old big-head/chunky/no-wings/camera-at-eye version is rejected.
- Updated `SM_Fenella`, both portraits, her billboard and the Director's Cut poster subject in `Assets/Generation/jobs.json`. The new billboard is the character reference for the portraits and poster. Rodin's Fenella job explicitly overrides the generic NPC prompt because its shared suffix still forces chunky shapes and a big head. Her latest manifest records the actual overridden prompt. Other NPCs retain their locked style. The old mesh is `ImportSource/Hyper3D/SM_Fenella_v1.glb`; old portrait PNGs are retained as `_v1`.
- UI assets use built-in Codex image generation, one image per job. Exact job prompts and layer anchors are retained; actual tool requests, including style-reference instructions, are in `ImportSource/_gen/inspection/*.request.json`. The original Lecture attack icon is unchanged. The standalone sword inventory icon was corrected after it copied the Lecture icon's hand/burst. The early paper-backed RESERVED FOR SEQUEL stamp was replaced with an ink-only impression. Some image requests were interrupted by ESC; saved outputs were recovered and completed assets were not regenerated unnecessarily.
- **Dylan's usage rule: always preserve 20% of both the 5-hour and weekly allowance for other tasks.** These counters are not available to ASSETS tools. Use small groups and obtain remaining percentages when needed to protect the reserve; avoid needless repeated questions when there is ample headroom. Latest reported snapshot immediately before the final four UI images: **47% of the 5-hour allowance remaining**; weekly last reported 94%. Earlier 5-hour checkpoints were 69%, 64%, 59% and 55%. This is distinct from generator API credits. Do not check fal's broken balance endpoint.
- Inspection artifacts: `ImportSource/_gen/inspection/batch1_stats.json`, `batch2_stats.json`, labelled `batch1_3d_*.jpg`, `batch2_3d_*.jpg`, `batch1_2d_*.jpg` and `batch2_2d_*.jpg`. The inspection helper accepts `batch1` or `batch2`. The review page is `ImportSource/_gen/review.html`.

## Batch1 model inspection

Every current GLB has a valid header and matching file length. All models have one material slot; independent tooth/plaque/tint regions, pivots, collision and attachment fit need GAME/cleanup work. Rodin previews are untextured geometry; their embedded base-colour atlases were inspected. All six Rodin models are **500,000 triangles** and need reduction. fal outputs are around **40,000 triangles**, also above final game budgets. Meshy's stanchion is **404 triangles**.

| Model | Delivered result / remaining issue |
| --- | --- |
| `SM_Toothbrush` | Rodin; visible pale bristle rows, long exposed shaft and dark wrapped grip. Check hand fit and brush orientation. |
| `SM_Goblin` | Rodin; round mint goblin, cream tunic, open grin with yellow teeth. Teeth are not a separate material slot. |
| `SM_Villager` | Rodin; round cream/peach villager with brown hair and boots, empty hands. Hat/tool attachments need fit checks. |
| `SM_Fenella` | **Replacement** Rodin model: thin limbs and narrow body, visible spread wings, hovering legs, camcorder held at chest height, face clear. Old model retained as `_v1`; review the replacement in the game camera. |
| `SM_LollyBug` | Rodin; pink/cream swirl in its texture, thick wings and small limbs. Adds antennae and an insect abdomen rather than a simple stick body. |
| `SM_Almond` | Rodin; almond silhouette and pale golden texture. Adds arms and a mouth despite the subject's exclusions. |
| `SM_CandyCrop` | fal; two lollipop stems and gumdrops, readable candy silhouette. Adds a brown soil/platform base. 40,012 triangles. |
| `SM_CandyFence` | fal; two candy-striped posts and two rails. Adds small white feet/base pads. |
| `SM_Celery` | fal; pale green stalk/leaves. Adds a face and a broader bundled silhouette. 40,008 triangles. |
| `SM_Molar` | fal; white tooth with crown/roots. Adds a face and pink pedestal/steps. |
| `SM_Signpost` | fal; blank white inset board and wood post. Adds a grassy ground tile. |
| `SM_CardboardBox` | fal; open box/flaps. Adds pale label-like panels and pink tab details; not completely unmarked. |
| `SM_CandyTree` | fal; pale pink faceted canopy lobes and short trunk. Adds grass/soil platform. |
| `SM_Boulder` | fal; pale lavender faceted boulder. Adds a face and a matching pedestal. |
| `SM_Clapperboard` | fal; blank slate and open striped clapper. Adds humanoid arms, gloves, legs and shoes. |
| `SM_DirectorsChair` | fal; open folding wooden chair, blank cream seat/back, no person. Check folding geometry and scale. |
| `SM_Stanchion` | Meshy; broad round base, cream shaft, faceted gold cap. Readable particle-style geometry. |

Dylan reviewed batch1 meshes in the browser and accepted them except the original Fenella (`Docs/STATUS.md`); the table preserves observed cleanup issues rather than reversing his choices.

## Batch2 model inspection

All eight files validate. The seven fal meshes are 39,992 to 40,000 triangles; Meshy's shard is 156 triangles. These have significant prompt deviations to review before importing:

| Model | Delivered result / remaining issue |
| --- | --- |
| `SM_FortGate` | Tooth gate between two pink towers, open arch. Adds a face and a **small knight figure in the arch**; remove the figure and check pass-through dimensions, depth and collision. 39,992 triangles. |
| `SM_LightStand` | Tripod studio lamp with barn doors. Adds a hanging wire; no light beam. |
| `SM_Table` | Pale rectangular table but **adds four chairs**, contrary to table-only prompt. Separate/remove chairs before use. |
| `SM_Hat_Straw` | Pale straw hat and cream band but **includes an entire villager head/neck**. Not an isolated attachable hat yet. 39,998 triangles. |
| `SM_HeadMirror` | Headband and round disc with hole but **includes a villager head/neck**. Not an isolated accessory yet. |
| `SM_Crook` | Curved pale/tan staff. Adds a pink pedestal; remove it. |
| `SM_LeafMegaphone` | Green flared megaphone with leaf veins; adds a handle and a small face-like detail inside. |
| `SM_LollyShard` | Meshy; pink faceted broken candy but **retains a long white stick**, contrary to the shard-only prompt. 156 triangles; remove stick if suitable. |

## UI delivery, credits and final checks

- **50 batch UI assets delivered**, plus the two unchanged approved test anchors = **52 UI assets total**, split evenly between the knight and Fenella visual layers. Final PNGs and raw originals are in `Assets/Incoming/2D/<id>/`; job prompts are in `Assets/Generation/jobs.json`, actual image tool requests in `ImportSource/_gen/inspection/*.request.json`, and ingest provenance in `ImportSource/OpenAI/manifest.jsonl`. All UI was generated with the built-in image tool, not the paid Images API fallback.
- Reviewed every batch PNG in labelled contact sheets. PNGs have the expected transparent backgrounds or opaque paper/poster canvases; frame openings are transparent. Stamps read DIRECTOR'S CUT, RESERVED FOR SEQUEL, PRODUCER'S PICK, EVIDENCE and REFORMED. Both movie-poster titles are correct, with no extra credits or lettering. The Director's Cut poster uses the thin winged Fenella holding a video camcorder; both replacement portraits use the same new sprite as a character reference. The Marshal and Herder portraits use the Farmer portrait as their character reference.
- All five rarity frames and both large iron frames have clear central openings. Their exact crops vary; account for each PNG's dimensions when laying out UI. All skill and loot art is isolated without a separate icon-card frame. The celery loot image was retried once and still looks like an ornate, metal-banded celery bundle rather than a plain single stalk; leave for Dylan's visual review. The damp napkin also received ornate artifact decoration. Passive icons retain muted gold/red accents and may need runtime desaturation for the inactive state.
- The clock face has twelve tick marks, no numerals and no hands. The two separate hands point up with round pivots at their bottom ends. Set their rotation pivots to those round bolts and scale the hour hand shorter when composing the clock; the separate PNGs were each normalized by ingest. Moustache and single wing are separate cutouts. Paper canvases remain opaque and edge-to-edge.
- All **25 batch GLBs** pass header/version/length checks. All batch PNGs pass file checks; opacity/transparency and frame-center measurements are recorded in the inspection JSON. No Unreal import, material setup, collision, rigging, gameplay, screenshot or packaged-build check was performed by ASSETS. The earlier hero rig/animations are unchanged.
- Meshy balance **864 before these batches, 804 after**: 60 credits used for the stanchion and shard. Tripo remains 0 and was not used. Rodin delivered six batch1 jobs plus the Fenella replacement; fal delivered 17 jobs across both batches. Rodin/fal wallet balances are unavailable through the existing checker. No fal balance request was made.
- All generated provenance is declared in `Docs/Declared.md`. The superseded first Fenella model has a separately versioned declaration. Static GLBs remain on this checkout under `ImportSource/{Meshy,Hyper3D,Fal}/` and are ignored by the existing Git rules; UI, previews, prompts, manifests, state, review and inspection artifacts are committed. No push.

**Generation is finished.** GAME/Dylan should review the meshes in the game camera, reduce geometry and remove the unwanted extras listed above before treating them as production-ready. Fenella's high piping voice is direction for AUDIO/GAME, not an audio delivery in this stage.

## Earlier test-stage report (historical)

The following records the original 14-job test delivery before Dylan's approval and the current batches. Its recommendations and stop instructions describe that earlier checkpoint.

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
