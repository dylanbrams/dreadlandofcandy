# Declared pre-jam work

The jam's FAQ: "You can set up your tools, agents, and MCP ahead of time, but don't start building your game until November 12. Declare any prompts, scripts, or assets you made before the jam."

Everything below was made before Nov 12, 2026. None of it is gameplay. Paste this list into the submission.

## Made before the jam (Nov 7, 2026)

| What | Where | Origin |
| --- | --- | --- |
| Empty UE 5.8 C++ project: one module, engine template map, packaging settings | `MegaJam.uproject`, `Source/`, `Config/` | Written for the jam |
| Agent hooks: `-shotat`, `-shotname`, `-quitat` command-line flags | `Source/MegaJam/*/AgentHooks.*` | Adapted from Dylan's earlier project, Cafeteria Battler |
| One skeleton automation test | `Source/MegaJam/Private/Tests/SkeletonTests.cpp` | Written for the jam |
| Build, test, screenshot, package, packaged smoke test, submission zip and itch.io upload scripts | `Scripts/` | Adapted from Cafeteria Battler |
| Tripo API client (text to 3D, saves .glb + manifest) | `Scripts/tripo.py` | Written for the jam |
| Meshy, Hyper3D Rodin clients, shared helpers, API balance check | `Scripts/meshy.py`, `rodin.py`, `genlib.py`, `check-balances.py` | Written for the jam |
| Agent instructions, including a list of UE 5.8 pitfalls | `CLAUDE.md` | Adapted from Cafeteria Battler |
| Design briefs (the Dread Land of Candy pitch, combat/VFX/quest companion), drafted with ChatGPT | `Docs/Design/` | Written for the jam (Nov 11); design text only, no game code |
| Build spec (scope, architecture, task backlog) and joke/line sheet, written with Claude Code | `Docs/Design/BUILD_SPEC.md`, `Docs/Design/Jokes.md` | Written for the jam (Nov 11); design text only, no game code or assets |
| Style prompt prefixes and suffixes for the mesh generators | `Docs/styles.json` | Written for the jam (Nov 11); prompts only, nothing generated |
| Generation tooling: parallel job runner with approval gate, Meshy rigging/animation client, OpenAI image client and image ingest (key, trim), retries and safer downloads in the shared helpers | `Scripts/gen_batch.py`, `meshy_rig.py`, `openai_image.py`, `genlib.py`, `meshy.py`, `rodin.py` | Written for the jam (Nov 11), with Claude Code; tools only, nothing generated |
| fal.ai 3D client (queue API, Hunyuan3D v3.1 by default) | `Scripts/fal3d.py` | Written Nov 12, before any game asset was generated, with Claude Code; tested with one throwaway model (a stool) that was deleted and is not in the game |
| Generation job list: one prompt subject per planned asset, and the two 2D style paragraphs | `Assets/Generation/jobs.json` | Written for the jam (Nov 11), with Claude Code; prompts only, nothing generated |
| Asset list, generation map and runbook | `Docs/Design/ASSETS.md`, `Docs/GenerationMap.md`, `Docs/GenerationRunbook.md` | Written for the jam (Nov 11), with Claude Code; planning text only |

## Ability layer test template (Nov 11, 2026, before the jam)

A GAS test harness, not gameplay: no map, no art, no quest or class content. Ported from Dylan's own earlier project **Spirits of Yendor** (UE 5.5, `Source/SpiritsOfYendor/*/AbilitySystem`), renamed `SOY*` to `Jam*`, retargeted to UE 5.8 and adapted.

| What | Where | Origin / what changed |
| --- | --- | --- |
| Ability system component, native tags | `Source/MegaJam/*/Gas/JamAbilitySystemComponent.*`, `JamGameplayTags.*` | Yendor `SOYAbilitySystemComponent`, `SOYGameplayTags`; 5.8 replication-default fix; tags trimmed and renamed |
| Resolve attribute set | `JamResolveAttributeSet.*` | Yendor `SOYHealthAttributeSet`; health became Resolve, death became a once-only `State.Pacified`; pacifist-mode knockback removed |
| Damage effect and magnitude calc | `JamGameplayEffects.*`, `JamResolveCalculation.*` | Yendor `SOYGE_Damage`, `SOYDamageCalculation`; renamed |
| Lecture cooldown effect | `JamGameplayEffects.*` | New |
| Area ability base and Lecture cone | `JamGameplayAbility.*` | Yendor `SOYGA_AoeAttack`, `SOYGameplayAbility`; now commits (cooldown), skips pacified targets; Mass damage queue, telegraph footprint actor and Niagara/decal cosmetics removed. `UJamGA_LectureCone` is new |
| AoE shape math | `JamAoePattern.h`, `JamAoePatternLibrary.*` | Yendor `SOYAoePattern`, `SOYAoePatternLibrary`; renamed, VFX slots removed |
| Test pawn, self-running test scene, `-abilitytest` flag | `Source/MegaJam/*/Testing/JamAbilityTestRig.*` | New |
| Automation tests `Jam.Gas.*` | `Source/MegaJam/Private/Tests/GasTests.cpp` | New |

## Copied in during the jam

Add a line here the day anything is brought in from another project (code, prompts, assets), with where it came from.

| Date | What | From |
| --- | --- | --- |

## Third-party assets

Assets we did not make (Fab, engine content, music...), with their licence.

| Asset | Source | Licence |
| --- | --- | --- |
| `Template_Default` map (placeholder) | Unreal Engine 5.8 engine content | Unreal Engine EULA |

## AI tools used

| Tool | Used for |
| --- | --- |
| Claude Code | Code, scripts, docs, the ability layer port and test scene |
| ChatGPT | Pre-jam design briefs in `Docs/Design/` |
| Unreal MCP + PCG and Niagara toolsets (engine plugins, Experimental) | Lets the agent drive the editor, build PCG graphs and Niagara systems |

## Generated during the jam

AI-generated assets, added by `Scripts/gen_batch.py declare`. Full prompts (with the locked style words) are in the `manifest.jsonl` beside each output.

| Asset | Tool | Date | File; prompt |
| --- | --- | --- | --- |
| `SK_Hero` | Meshy text-to-3D | 2026-11-12 | `ImportSource/Meshy/SK_Hero.glb`; prompt in `Assets/Generation/jobs.json` |
| `SK_Hero_Rig` | Meshy rigging + animation (library presets and text-to-motion) | 2026-11-12 | `ImportSource/Meshy/SK_Hero_Rig/SK_Hero_Rig.fbx`; prompt in `Assets/Generation/jobs.json` |
| `SM_Sword` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Sword.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Rat` | Meshy text-to-3D | 2026-11-12 | `ImportSource/Meshy/SM_Rat.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_HouseFront` | Meshy text-to-3D | 2026-11-12 | `ImportSource/Meshy/SM_HouseFront.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Peppermint` | Meshy text-to-3D | 2026-11-12 | `ImportSource/Meshy/SM_Peppermint.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Rat_Rodin` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Rat_Rodin.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_HouseFront_Rodin` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_HouseFront_Rodin.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Peppermint_Rodin` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Peppermint_Rodin.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_HouseFront_Fal` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_HouseFront_Fal.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Peppermint_Fal` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Peppermint_Fal.glb`; prompt in `Assets/Generation/jobs.json` |
| `T_Skill_Lecture` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Skill_Lecture/T_Skill_Lecture.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Card_Clapper` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Card_Clapper/T_Card_Clapper.png`; prompt in `Assets/Generation/jobs.json` |
| `SM_Rat_Fal` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Rat_Fal.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Toothbrush` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Toothbrush.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Goblin` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Goblin.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Villager` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Villager.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Fenella` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Fenella.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_LollyBug` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_LollyBug.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Almond` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Almond.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_CandyCrop` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_CandyCrop.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_CandyFence` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_CandyFence.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Celery` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Celery.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Molar` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Molar.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Signpost` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Signpost.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_CardboardBox` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_CardboardBox.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_CandyTree` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_CandyTree.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Boulder` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Boulder.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Clapperboard` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Clapperboard.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_DirectorsChair` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_DirectorsChair.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Stanchion` | Meshy text-to-3D | 2026-11-12 | `ImportSource/Meshy/SM_Stanchion.glb`; prompt in `Assets/Generation/jobs.json` |
| `T_Skill_FormALine` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Skill_FormALine/T_Skill_FormALine.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Skill_DisappointedSigh` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Skill_DisappointedSigh/T_Skill_DisappointedSigh.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Skill_LastJudgment` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Skill_LastJudgment/T_Skill_LastJudgment.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Sword` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Sword/T_Loot_Sword.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Toothbrush` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Toothbrush/T_Loot_Toothbrush.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Celery` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Celery/T_Loot_Celery.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Frame_Title` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Frame_Title/T_Frame_Title.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Frame_NotesCard` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Frame_NotesCard/T_Frame_NotesCard.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Card_CutBurst` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Card_CutBurst/T_Card_CutBurst.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Card_Retake` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Card_Retake/T_Card_Retake.png`; prompt in `Assets/Generation/jobs.json` |
