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
