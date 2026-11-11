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
| Claude Code | Code, scripts, docs |
| Unreal MCP + PCG and Niagara toolsets (engine plugins, Experimental) | Lets the agent drive the editor, build PCG graphs and Niagara systems |
