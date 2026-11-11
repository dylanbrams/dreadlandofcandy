# CLAUDE.md

2026 Epic AI MegaJam entry, UE 5.8 C++. Jam: build Nov 12 to Nov 15, deadline **Nov 15, 5:59 PM PST**. Submit a packaged **Windows Development** build as `TeamName_GameName.zip`, under 1 GB, through the jam's itch.io page (https://itch.io/jam/2026-epic-ai-megajam). Unpackaged projects are not accepted.

Commit at the end of every task once the build and tests pass, and at green checkpoints inside a long task; never push unless Dylan asks. The whole jam is streamed: never `cat`, print or open credentials, config with tokens, or anything outside this folder that could hold secrets.

## Start here

Read `Docs/STATUS.md` (what exists, what is broken, next task), then `Docs/Design/BUILD_SPEC.md`, which governs: the game we ship, the architecture, the task backlog, the cut order and the session protocol. Authored text comes from `Docs/Design/Jokes.md`. The three older briefs in `Docs/Design/` are reference only (the master brief is about 40k tokens and mostly describes an MMO we are not building); open one only when a task names a section. None of these documents is evidence that anything works.

Every session ends by rewriting `Docs/STATUS.md` and appending a few lines to `Docs/DEVLOG.md`. Only one session builds at a time.

## Declared pre-jam work

Everything that existed before Nov 12 is listed in `Docs/Declared.md`. Anything copied in from another project during the jam gets a line there too, the same day.

## Scripts (PowerShell, from the project root)

Each finds the project from the `.uproject`, so renaming the project needs no script edits.

| Script | Does |
| --- | --- |
| `Scripts\build.ps1` | Build the editor target (`-Game -Config Shipping` for the game target) |
| `Scripts\run-tests.ps1` | Build, then run the `Jam.*` automation tests headless (`-Filter`, `-NoBuild`, `-Game`) |
| `Scripts\screenshot.ps1` | Run the game (`-game`), save one in-engine screenshot to `Saved\Screenshots\`: `-At 6 -Name x -ResX -ResY`, `-Extra "-flag"` |
| `Scripts\package.ps1` | Package Win64 Development to `Builds\Win64-Development` |
| `Scripts\smoke-packaged.ps1` | Launch the packaged build, check it stays up and its log (add a check per cooked-only failure found) |
| `Scripts\zip-submission.ps1` | `-TeamName -GameName`: zip the packaged build as the jam wants, fail if 1 GB or more |
| `Scripts\upload-itch.ps1` | Push to itch.io with butler (asks first; never run it without Dylan's go-ahead) |

Game flags: `-shotat=<s> -shotname=<n>` (in-engine screenshot), `-quitat=<s>` (`UAgentHooksSubsystem`). Add `-autoplay` (play the game unattended) and `-uiscreen=<screen>` (open one screen) to the game mode on day one, so screenshots and the smoke test can reach real gameplay. Use these to see the game instead of capturing the desktop.

## How to work

- Small steps, each ending with: build, tests passing, a screenshot looked at. Package and smoke-test at least once a day; leave the last day for polish and submission, not features.
- Headless first: put rules in plain C++ that tests can run without a world or renderer.
- Tests go under `Jam.` (`IMPLEMENT_SIMPLE_AUTOMATION_TEST`, `EditorContext | ClientContext | EngineFilter`).
- UI: layout in `/Game/UI/WBP_*` reparented to a C++ screen class, behaviour in C++, widgets bound by name with `BindWidget`. 1920x1080 design canvas with ScaleToFit (already set). Every screen usable with a gamepad.
- The map is a placeholder (`/Engine/Maps/Templates/Template_Default`); change `GameDefaultMap` in `Config/DefaultEngine.ini` once the game has one.

## UE 5.8 traps (found on Cafeteria Battler)

- `FString::Printf` needs a literal format string (checked at compile time); wrap it in a macro, not a function.
- The engine's Fab plugin breaks UBT on this install; it is disabled in the `.uproject`.
- Anything loaded by path at runtime (DataTables, `/Engine/BasicShapes`, widget classes) must be in `DirectoriesToAlwaysCook` (`Config/DefaultGame.ini`), or it is missing from packaged builds. The smoke test is how you find out.
- CommonUI types pull in GameplayTags: any module using them needs the `GameplayTags` dependency.
- `UCommonButtonBase` wraps whatever root exists when it initializes: build button content in `Initialize()` before calling `Super`.
- A WBP made outside the designer needs every widget's GUID in `WidgetVariableNameToGuidMap` and bound widgets marked `bIsVariable`.
- A native `UUserWidget` child inside a WBP is saved with the layout, so anything it builds for itself must be `UPROPERTY(Transient)`.
- Screen shortcuts (Space, Enter) go in a preview key handler, or the focused button eats them first.
- Mass: fragment base types live in `MassCore` (link it with `MassEntity`); `FMassEntityManager::Initialize()` needs `FMassEntityManager_InitParams_Concurrent`; never create, destroy or tag entities inside `ForEachEntityChunk`, queue them.
- Subsystems need a `UGameInstance` outer, even in tests.
- Save structs: every field must be a `UPROPERTY`, or loading silently drops it.
- PowerShell 5.1 `Get-Content` reads UTF-8 files without a BOM as ANSI; writing them back garbles non-ASCII text. Edit text files with the Edit tool or Python.
- GAS: 5.8 ensures on `SetIsReplicated` in a component constructor, use `SetIsReplicatedByDefault`. `FGameplayAbilitySpecHandle` is in `GameplayAbilitySpecHandle.h`. Needs the `GameplayAbilities` plugin in the `.uproject` and `GameplayAbilities`, `GameplayTags`, `GameplayTasks` in `Build.cs`. Attribute sets made as default subobjects register with the ASC automatically.
- `IMPLEMENT_SIMPLE_AUTOMATION_TEST` flags must be an inline expression or a macro; a `constexpr EAutomationTestFlags::Type` does not compile.
- The editor writes an Android file server `SecurityToken` into `Config/DefaultEngine.ini` when it runs. Never commit that diff.
