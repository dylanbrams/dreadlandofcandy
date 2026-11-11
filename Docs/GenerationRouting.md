# Which tool for which asset

Researched Nov 11 from vendor docs; quality claims are the vendors' own and untested by us. Re-rank after the
first test generations (Nov 12). Working clients: `Scripts/meshy.py`, `Scripts/rodin.py`, `Scripts/tripo.py`
(Tripo API balance is 0, so web app only). Balances: `Scripts/check-balances.py`.

## Ranking per type

| Type | 1st | 2nd | 3rd | Notes |
| --- | --- | --- | --- | --- |
| **Character** (hero, animated, humanoid) | **Meshy**, then its rigging + animation API | **Rodin Gen-2.5 with the pose option** | Tripo web app | Meshy takes a *textured humanoid* `.glb`, returns a rigged one with 500+ preset animations (GLB/FBX). Rodin has no skeleton endpoint, but its `TAPose` option forces a clean T/A pose so a rigger (Meshy's, or UE retargeting) can bind it. Untextured, non-humanoid or unclear-limbed models are rejected by Meshy rigging. |
| **NPC** (many, cheap) | **Meshy** (same rig pipeline, several tasks at once on Pro) | **Rodin** at `Gen-2.5-Low` / `Extreme-Low` | Hunyuan3D v3.1 Rapid on fal | Variety is cheaper from shared skeleton + swapped meshes/textures than from unique rigs. Static NPCs with procedural idle need no rig at all. |
| **Equipment** (weapons, armor, props, rigid) | **Rodin** | **Meshy** (`--polycount` for budget) | Hunyuan3D v3.1 Pro on fal, or Tripo web (smart low-poly) | Rigid and static, so quality per credit matters more than rigging. Fix pivot and orientation in Blender so it attaches to a socket. |
| **Terrain** | **Unreal itself**: Landscape / PCG, driven through MCP | **PBRgen** for seamless PBR landscape materials | Meshy/Rodin for rock, cliff and ruin kit pieces and landmarks | No 3D generator makes a good *terrain*: they output single objects. Generate the surface and the set-dressing separately. A grayscale heightmap from an image model works, but 8-bit gives banding; smooth it in UE or use noise. I could not confirm what PBRgen outputs; check its UE compatibility before relying on it. |
| **Niagara particle** | **Niagara itself** (built-in templates, authored through MCP) | **Flipbook texture from an image model** (Codex), assembled into a SubUV grid | **Niagara Flipbook Baker** to bake an existing sim into a flipbook | Not a 3D-model job. Debris/shard meshes can come from Meshy. A paid Fab plugin ("AI Flipbook Generator") exists; skipped. |
| **2D item** | **Codex** (your subscription) | - | - | See prompts below. |

Reallusion Character Creator 5 (iClone Personal perk) is the high-quality character route with UE5 skeleton
export, but it is a big install and learning curve for 3.5 days. Not planned.

## Running in parallel

Separate lanes, so nothing waits on anything else:

| Lane | Tool | Takes |
| --- | --- | --- |
| A | Meshy (`meshy.py`) | characters, NPCs, then rigging/animation |
| B | Rodin (`rodin.py`) | equipment, hero props, anything that must count for the Hyper3D prize |
| C | fal (client not written yet) | overflow and bulk; Hunyuan3D / Meshy / Rodin / Tripo models are all hosted there |
| D | Codex | all 2D |
| E | Unreal + MCP | terrain, Niagara, materials |

Meshy Pro allows 10 concurrent tasks; Rodin's limit is undocumented (it returns HTTP 429 with `Retry-After`).

## Things to check on Nov 12 before committing

- Whether Meshy's rigging endpoint accepts a Rodin `.glb` (by URL or upload), which would give Rodin quality plus Meshy rigging.
- Whether Rodin through fal counts for the Hyper3D prize; direct use via `rodin.py` surely does.
- That the fal key can generate (it lacks permission for the billing endpoint, which says nothing about generation).
- Licence per service in `Docs/Declared.md`.

## 2D items: Codex prompt template

Fill in `{...}` once the theme is known. One sheet per batch keeps style consistent; slice it with a script after.

```
Create a 4x4 sheet of {16} distinct {item type, e.g. inventory icons} for a video game, {art style}.
Each item sits centred in its own equal square cell, same scale and same camera angle,
{lighting, e.g. soft top-left light}, bold readable silhouette, no text, no labels, no borders.
Palette: {5-6 colours or 'matching the attached reference'}.
Background: one flat, solid {#00FF00} colour with no gradient or shadow (so it can be keyed out).
Items: 1. {...} 2. {...} ... 16. {...}
```

- Ask for a flat key colour and remove it with a script; do not rely on Codex producing real transparency.
- Attach the first good sheet as a reference to every later one.
- Size the cells for the UI (icons usually 256 px) and keep a separate list of what was generated for the AI-tools declaration.

## One style per type

Generators drift: the same words give different looks on different days and across tools. Lock each type once,
early, and reuse the lock.

1. **Pick one look per type after the theme reveal.** One sentence each, concrete (shape language, finish,
   palette, proportions). Example: "chunky low-poly, hand-painted flat colours, no baked lighting, big readable shapes".
2. **Put it in `Docs/styles.json`.** Each type has a `prefix` and `suffix`; `meshy.py` and `rodin.py` add them
   around the prompt when you pass `--type character|npc|equipment|terrain_piece|particle_mesh`. The file is empty
   now, so nothing changes until it is filled in. Keep the style words identical within a type; vary only the subject.
3. **Make one test asset per type and approve it** before batching. Any later asset is judged against it.
4. **Use one tool per type where you can**, because two tools give two looks. This is why the ranking above puts one
   first for each type: characters and NPCs both from Meshy, equipment from Rodin. Only mix tools when the first
   is out of credits.
5. **Normalise in Unreal, not in the generators.** Large wins come from shared treatment: one master material (or a
   few material instances) with a shared palette/gradient, the same texture size per type, the same triangle budget
   per type, flat or smooth shading chosen once, one lighting setup. Strip or overwrite baked lighting.
6. **Same trick for 2D**: put the style paragraph at the top of every Codex prompt and attach the first approved
   sheet as a reference. Terrain and particles take their look from the shared material palette and colour grade.

| Type | Lock these |
| --- | --- |
| Character | proportions (head-to-body ratio), polycount, one rig, one skin material |
| NPC | same as character but cheaper: same proportions and palette, reduced polycount, shared skeleton |
| Equipment | scale reference (e.g. a one-metre grip), edge treatment, one palette, same texture size |
| Terrain | one material palette, one rock/vegetation kit, one fog/colour grade |
| Niagara particle | one colour ramp, one softness (hard or soft edges), one frame count and grid size |
| 2D item | line weight, palette, perspective, background colour for keying |
