# Testing handoff: village quests and backend branches

Workspace: `D:\Source\Unreal\MegaJam2026`. Shell: PowerShell. Engine: `D:\Program Files\Epic\UE_5.8`.

## Task for the next thread

Verify the village quest work using the existing checks. Start with the evidence below; avoid repeating expensive checks that already passed unless code or binaries changed. Keep fixes limited to actual failures and preserve unrelated working-tree edits. No deployment, packaging, or commits are needed for this testing task.

**Do not run the backlot map or backlot autoplay. Dylan says the enlarged mountain blocks entry to the third arena. Backlot results are excluded from validation. Use `/Game/World/L_Sugarborough` and isolated automation tests.** Backlot modes have been removed from the new verification scripts. Isolated rules/geometry tests do not launch that map.

## What changed

- Fenella waits in Sugarborough for recruitment, then follows the hero and gives stage directions. Glowing, bouncing golden triangles mark her and active quest locations.
- The final main-quest wrap returns to the village; five villagers have ten linked quests. Objectives include gathering, pacifying, and pacifying then feeding/brushing. Rewards require returning to the giver and cannot repeat.
- Pause > Quest Journal shows progress and can set aside a quest. Fenella offers the Premiere in town; credits return to the same village session.
- Every villager job offers two playable routes. Optional **Ask Fenella to improvise** calls `POST /v1/director/quest/branches`. Choices can change the actual objective, count, and camp. Completed route history informs subsequent requests.
- Authored routes remain usable offline, during a request, and on backend failure. Cancelled/stale responses cannot alter accepted quests. Server/client validate against canonical recipes and calculate rewards locally.
- Comic buttons have large click targets, hover feedback, and proper concave-polygon fills. Held talk keys and camera mouse clicks cannot accidentally confirm a branch.

## Already verified

- Latest editor build succeeded.
- **79 Unreal automation tests passed**, including complete quest lifecycle, all objective types, rewards, JSON variations, backend branch parsing, stale responses, geometry, and deliberate-choice input.
- **117 backend tests passed**, with a fake model and no production model spend.
- Full terrain autoplay completed all three main quests and all ten original village quests.
- A localhost HTTP run received backend choices and completed all ten alternate routes.
- A second localhost run completed all ten alternate routes with **every branch request returning HTTP 503**.
- Arrival, branch-choice screen, and quest journal were visually inspected at 1280 x 800. Wide comic fill artifacts were corrected.
- No packaged build was made. New dialogue is captioned; progress remains session-local.

Evidence:

- `Saved/Automation/RunTests.log` — final 79-test run; look for `TEST COMPLETE. EXIT CODE: 0`.
- `Saved/Automation/Fixes1-village.log` — authored-route playthrough.
- `Saved/Automation/Fixes1-backend.log` — successful HTTP branches and all ten alternate quests.
- `Saved/Automation/Fixes1-fallback.log` — HTTP 503 fallbacks and all ten alternate quests.
- Successful complete terrain runs end with `JamVillageAutoplay: PASS main=3 side=10`.
- Screenshots: `Saved/Screenshots/WindowsEditor/fixes1-arrival.png`, `fixes1-questbranches.png`, and `village-journal.png`.

## Commands, only as needed

```powershell
# Build only if source changed or the binary is stale.
& .\Scripts\build.ps1

# Isolated Unreal suite (expected: 79 passed).
& .\Scripts\run-tests.ps1 -NoBuild

# Backend suite (expected: 117 passed; no network/key required).
& .\backend\.venv\Scripts\python.exe -m pytest -q backend/tests

# Actual terrain flow. Each complete run takes roughly 1–3 minutes.
& .\Scripts\verify_village_quests.ps1 -Mode offline
& .\Scripts\verify_village_quests.ps1 -Mode backend
& .\Scripts\verify_village_quests.ps1 -Mode fallback

# Optional UI captures, rather than another gameplay run.
& .\Scripts\verify_village_quests.ps1 -Mode arrival
& .\Scripts\verify_village_quests.ps1 -Mode journal
& .\Scripts\verify_village_quests.ps1 -Mode choices
```

The reusable script launches hidden temporary instances and cleans up its own processes. Backend/fallback modes launch the local stub on port 8019; use `-Port 8020` if occupied. They override the service URL to localhost. **Do not point tests at the production API or read API-key files.** Other than the journal mode, the final reusable script's modes are equivalent to the earlier successful `Saved/verify_fixes1.ps1` runs, but have not all been rerun under their new script name.

Check both process completion and logs: the older `run-tests.ps1` can report a partial pass count after a crash. Require the expected count, final exit-code marker, and no assertion/fatal error. Do not trust only its shell exit code.

## Backend and code locations

`backend/` is a separate, git-ignored nested repository. Its changes are local and **the new endpoint has not been deployed to the public service**. An older public backend returning 404 must preserve authored routes. No SQL/nginx migration is needed; deployment itself is outside this testing handoff.

- Gameplay: `Source/MegaJam/Private/Game/JamVillageFlow.cpp`.
- Recipes/rules/parser: `Source/MegaJam/Private/World/JamVillageQuests.cpp`.
- Transport: `JamNoteClient.cpp`; UI: `JamScreens.cpp`, `JamScreenWidget.cpp`, `JamHudWidget.cpp`.
- Server: `backend/director/quests.py`, `service.py`, `model.py`, `app.py`.
- Tests: `VillageQuestTests.cpp`, `UiHudInputTests.cpp`, `backend/tests/test_quests.py`.
- Contracts: `Docs/Design/QuestBranches.md`, `Docs/Design/Village_QuestHooks.md`.
- Overall progress: `Docs/Feedback/Fixes1-progress.md`.

Finish with a short report of which checks were reused/run, failures found, and any remaining limits. Do not restart feature development just to consume quota.
