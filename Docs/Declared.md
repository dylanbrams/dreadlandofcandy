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
| `SM_Fenella_v1` (superseded) | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Fenella_v1.glb`; original prompt in `ImportSource/Hyper3D/manifest.jsonl`; replaced with thin winged fairy and video camcorder |
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
| `SM_Fenella` | Hyper3D Rodin Gen-2.5 | 2026-11-12 | `ImportSource/Hyper3D/SM_Fenella.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_FortGate` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_FortGate.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_LightStand` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_LightStand.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Table` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Table.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Hat_Straw` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Hat_Straw.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_HeadMirror` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_HeadMirror.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_Crook` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_Crook.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_LeafMegaphone` | fal.ai (Hunyuan3D v3.1 Pro text-to-3D) | 2026-11-12 | `ImportSource/Fal/SM_LeafMegaphone.glb`; prompt in `Assets/Generation/jobs.json` |
| `SM_LollyShard` | Meshy text-to-3D | 2026-11-12 | `ImportSource/Meshy/SM_LollyShard.glb`; prompt in `Assets/Generation/jobs.json` |
| `T_Passive_Brooding` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Passive_Brooding/T_Passive_Brooding.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Passive_CapeMaintenance` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Passive_CapeMaintenance/T_Passive_CapeMaintenance.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Passive_Standing` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Passive_Standing/T_Passive_Standing.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Pauldrons` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Pauldrons/T_Loot_Pauldrons.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Greaves` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Greaves/T_Loot_Greaves.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Helm` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Helm/T_Loot_Helm.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Doomplate` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Doomplate/T_Loot_Doomplate.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Gauntlets` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Gauntlets/T_Loot_Gauntlets.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Cloak` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Cloak/T_Loot_Cloak.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_DampNapkin` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_DampNapkin/T_Loot_DampNapkin.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Amulet` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Amulet/T_Loot_Amulet.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Loot_Ring` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Loot_Ring/T_Loot_Ring.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Card_CallSheet` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Card_CallSheet/T_Card_CallSheet.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Stamp_DirectorsCut` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Stamp_DirectorsCut/T_Stamp_DirectorsCut.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Stamp_ReservedForSequel` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Stamp_ReservedForSequel/T_Stamp_ReservedForSequel.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Stamp_ProducersPick` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Stamp_ProducersPick/T_Stamp_ProducersPick.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Stamp_Evidence` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Stamp_Evidence/T_Stamp_Evidence.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Stamp_Reformed` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Stamp_Reformed/T_Stamp_Reformed.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Portrait_Fenella` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Portrait_Fenella/T_Portrait_Fenella.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Portrait_Fenella_Sulk` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Portrait_Fenella_Sulk/T_Portrait_Fenella_Sulk.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Fenella_Billboard` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Fenella_Billboard/T_Fenella_Billboard.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Fenella_Wing` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Fenella_Wing/T_Fenella_Wing.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Paper_NotesCard` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Paper_NotesCard/T_Paper_NotesCard.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Paper_Ledger` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Paper_Ledger/T_Paper_Ledger.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Frame_Rarity_Common` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Frame_Rarity_Common/T_Frame_Rarity_Common.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Frame_Rarity_Uncommon` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Frame_Rarity_Uncommon/T_Frame_Rarity_Uncommon.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Frame_Rarity_Rare` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Frame_Rarity_Rare/T_Frame_Rarity_Rare.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Frame_Rarity_Legendary` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Frame_Rarity_Legendary/T_Frame_Rarity_Legendary.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Frame_Rarity_DirectorsCollection` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Frame_Rarity_DirectorsCollection/T_Frame_Rarity_DirectorsCollection.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Poster_Marquee` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Poster_Marquee/T_Poster_Marquee.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Poster_DirectorsCut` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Poster_DirectorsCut/T_Poster_DirectorsCut.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Portrait_Farmer` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Portrait_Farmer/T_Portrait_Farmer.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Portrait_Marshal` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Portrait_Marshal/T_Portrait_Marshal.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Portrait_Herder` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Portrait_Herder/T_Portrait_Herder.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Frame_CandyRockPhone` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Frame_CandyRockPhone/T_Frame_CandyRockPhone.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Clock_Face` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Clock_Face/T_Clock_Face.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Poster_TheatricalRelease` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Poster_TheatricalRelease/T_Poster_TheatricalRelease.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Clock_HourHand` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Clock_HourHand/T_Clock_HourHand.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Clock_MinuteHand` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Clock_MinuteHand/T_Clock_MinuteHand.png`; prompt in `Assets/Generation/jobs.json` |
| `T_Villager_Moustache` | ChatGPT image generation (Codex) | 2026-11-12 | `Assets/Incoming/2D/T_Villager_Moustache/T_Villager_Moustache.png`; prompt in `Assets/Generation/jobs.json` |

### Derived mesh cleanup — 2026-11-12

The 30 current Hyper3D/Fal meshes above and batch-2 Meshy `SM_LollyShard` were processed in Blender 3.3 with new jam-authored `decimate.py`, `ImportSource/_decimated/cleanup.py`, and companion validation/preview/collision scripts. No cleanup code was copied from another project and no extra generator/API requests were made. Derived GLBs live under `ImportSource/_decimated/{Hyper3D,Fal,Meshy}`; original generator files and provenance remain intact.

Work includes welding before Collapse, removal of unwanted attachments/bases, cap repairs, normals/pivots, blade proportion adjustment, material regions and UCX collision companions. Hat, HeadMirror and LeafMegaphone were procedurally rebuilt to remove fused heads/interior ornaments while retaining their original embedded base-colour atlases as paint; lamp tripod and clapper slate were rebuilt around retained textured parts. See `ImportSource/_decimated/_review/REPORT.md`, `cleanup_results.json` and `validation.json` for per-file actions and verification. These derivatives retain the provenance and licence obligations of their source assets; Unreal import/gameplay acceptance was not performed by ASSETS.

### Additional terrain foliage generated during the jam - 2026-11-12

Requested by Dylan and generated through Hyper3D Rodin Gen-2.5-Low. Each model
consumed 0.5 credits (four total). Raw originals, full prompts, task identifiers,
derived GLBs/FBXs and texture/geometry checks are in `ImportSource/Terrain/Foliage/`.
Preparation and previews use new jam-authored Blender/Python scripts in that
folder. These assets have not yet been imported into Unreal or placed in the map.

| Asset | Tool | Date | Source and provenance |
| --- | --- | --- | --- |
| `SM_Rodin_PeppermintBirch` | Hyper3D Rodin Gen-2.5-Low | 2026-11-12 | `ImportSource/Terrain/Foliage/Raw/SM_Rodin_PeppermintBirch.glb`; `generation_manifest.json` |
| `SM_Rodin_LicoriceWillow` | Hyper3D Rodin Gen-2.5-Low | 2026-11-12 | `ImportSource/Terrain/Foliage/Raw/SM_Rodin_LicoriceWillow.glb`; `generation_manifest.json` |
| `SM_Rodin_CandyCaneFir` | Hyper3D Rodin Gen-2.5-Low | 2026-11-12 | `ImportSource/Terrain/Foliage/Raw/SM_Rodin_CandyCaneFir.glb`; `generation_manifest.json` |
| `SM_Rodin_BubblegumBlossom` | Hyper3D Rodin Gen-2.5-Low | 2026-11-12 | `ImportSource/Terrain/Foliage/Raw/SM_Rodin_BubblegumBlossom.glb`; `generation_manifest.json` |
| `SM_Rodin_JellybeanBush` | Hyper3D Rodin Gen-2.5-Low | 2026-11-12 | `ImportSource/Terrain/Foliage/Raw/SM_Rodin_JellybeanBush.glb`; `generation_manifest.json` |
| `SM_Rodin_SugarFern` | Hyper3D Rodin Gen-2.5-Low | 2026-11-12 | `ImportSource/Terrain/Foliage/Raw/SM_Rodin_SugarFern.glb`; `generation_manifest.json` |
| `SM_Rodin_CandyFlowerClump` | Hyper3D Rodin Gen-2.5-Low | 2026-11-12 | `ImportSource/Terrain/Foliage/Raw/SM_Rodin_CandyFlowerClump.glb`; `generation_manifest.json` |
| `SM_Rodin_GummySucculent` | Hyper3D Rodin Gen-2.5-Low | 2026-11-12 | `ImportSource/Terrain/Foliage/Raw/SM_Rodin_GummySucculent.glb`; `generation_manifest.json` |

### Dense terrain ecology - authored during the jam, Nov 12–13

New original meshes `SM_CandyGrassMint`, `SM_CandyGrassRose`,
`SM_CandyGrassIce`, `SM_CandyflossThicket` and `SM_SugarSnowflake`, plus
grass-wind/snowfall materials, a powdered-sugar ground-atlas variant and a
PCG/HISM ecology graph were authored with Codex in
`ImportSource/Terrain/Dense/` and `Content/World/Dense/`. Blender 4.5 exports
and Unreal Python perform the build. Seven of the previously declared Rodin
foliage models are reused; the willow is held out. No new paid model generation
or copied third-party code/assets were used. The existing engine daylight
ambient cubemap is referenced for lighting.

## 2026-11-13 — expansion character generation

At Dylan's request, Codex authored prompts and used the Meshy text-to-3D API
to generate and PBR-texture `SK_Mage`, `SK_Archer` and `SK_Barbarian`.
Meshy auto-rigging targets the UE5 mannequin skeleton, with library Idle (0),
Walk (30) and the supplied basic walk exported as FBX alongside rigged FBX/GLB.
Prompts and the resumable runner are in `Assets/Generation/expansion_characters.*`;
deliveries and provenance are under `ImportSource/Meshy/`. A first Barbarian
attempt was missing lower legs despite its full-body prompt; that model and
its rig were preserved under
`ImportSource/_gen/expansion_characters/rejected_barbarian_v1/` and replaced
with a newly generated complete body. Blender 3.3 was used for solid geometry
and posed FBX review renders. These are new jam-generated assets, not imported
pre-jam work. Unreal import and gameplay integration are not part of this batch.

## 2026-11-13 — knight continuity refresh

Dylan preferred the new cast's smoother aesthetic and requested a matching
knight. Codex authored the `SK_Knight` prompt and generated a new textured
model through Meshy, followed by UE5 mannequin auto-rigging and library
animations. The original knight's SwordRaise, Scrub and Sigh text-to-motion
task IDs were reused and retargeted onto the new rig. Source/configuration:
`Assets/Generation/knight_refresh.json`; outputs:
`ImportSource/Meshy/SK_Knight*`. Blender 3.3 is used for geometry and posed
FBX review. The original `SK_Hero` assets are retained. No pre-jam assets or
third-party code were copied; Unreal integration remains a GAME task.

## 2026-11-13 — knight sword-raise wrist correction

Codex authored an Unreal Python correction for the Meshy-generated knight's
right-hand animation track. It turns the wrist with the existing fixed sword
attachment, preserves other tracks and timing, and is reapplied after source
FBX import. The existing runtime animation was updated in place; original native
package/source keys and in-game comparison captures are retained in
`ImportSource/RigCorrections/`. No new model or motion generation was performed.

## 2026-11-13 — theme song (R2)

Dylan supplied `music/theme-song.mp3` (2:18) during the jam. Its lyrics are an
adjusted version of "Big Rock Candy Mountain" (Harry McClintock, recorded 1928),
kept in `Docs/BigRockCandyMountainAdjustedLyrics.txt`. The song plays only over
the end credits (`/Game/Audio/Music/M_theme_song_take1`) and in the trailer.
**Dylan: add which tool made the recording.** Also imported the same day:
`village_take1/2` from `music/` (same source as the other score cues).

## 2026-11-13 — the cast in game (R6)

GAME imported `SK_Knight`, `SK_Mage`, `SK_Archer` and `SK_Barbarian` (the
Meshy rigs declared above) to `/Game/Art/Cast/<Who>/` with the new
`Scripts/import_cast.py`, and the eleven English battle cries
(`vocals/output/attacks/en/VO_{K,M,A,B}0*_en_epic.wav`, ElevenLabs
`eleven_multilingual_v2` text-to-speech, generated during the jam on Nov 13;
lines, voices and request ids in `vocals/output/attacks/manifest.json`) to
`/Game/Audio/VO/Cast/VO_Cry_*`. The knight's SwordRaise wrist correction above
is reapplied on import. Nothing copied from another project.

## 2026-11-13: cast skill animations and weapons

Meshy text-to-motion generated FormALine and LastJudgment; those and the
existing SwordRaise, Sigh and Scrub motion tasks were animated onto the four
existing rigs. Prompts/task IDs: `Scripts/complete_cast_animations.py` and
`ImportSource/Meshy/SK_*_Rig/rig.json`. Meshy generated the staff and bow
(and one unused rejected rolling pin): source prompts, task IDs and provenance
are in `Scripts/generate_cast_weapons.py`, `ImportSource/_gen/cast_weapons/`
and the Meshy manifest. The final rolling pin is original procedural Blender
geometry, built by `Scripts/prepare_cast_weapons.py`; no third-party models.
Existing original Niagara meshes/materials were reused for particles.
