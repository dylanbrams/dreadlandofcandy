# Optional backend branches for villager quests

Talking to an available quest giver opens two playable approaches. The player can take either authored route immediately, choose Maybe later, or ask Fenella to improvise. The optional button uses the existing note-service base URL and six-second HTTP timeout. `-nonotes` disables the network option while preserving both authored routes.

The branches change actual gameplay: for example, a lecture-only rat job can become pacification followed by feeding celery; an almond-gathering job can become clearing delivery pests. The recipe retains its stable quest ID, giver, prerequisites, and reward formula. Either route completes that quest and unlocks the giver's authored follow-up. Completed route choices are sent as history on later requests so backend prose can react to how earlier work was solved.

## Endpoint

`POST /v1/director/quest/branches`

The request contains:

- `v`: 1; `session_id`: the existing anonymous run ID.
- `quest_id`, `giver`, `title`, and `pitch`: the offered job.
- `motif`: the current film motif.
- `completed`: stable IDs of handed-in quests.
- `history`: completed route IDs, for example `council_rats:alternative`.
- `routes`: exactly two permitted recipes, with `route` (`original` / `alternative`), `verb`, `target`, `objective`, `pitch`, `count`, `min_count`, `max_count`, and `destinations`.

Example successful response:

```json
{
  "v": 1,
  "source": "model",
  "quest_id": "council_rats",
  "branches": [
    {
      "route": "original",
      "label": "A firm word",
      "pitch": "Pacify the three rats, then return to Mayor Bonbon.",
      "destination": "CampMeadowWest",
      "count": 3
    },
    {
      "route": "alternative",
      "label": "A healthy snack",
      "pitch": "Pacify the rats, feed them celery, then report back.",
      "destination": "CampMeadowWest",
      "count": 3
    }
  ]
}
```

`source: cache` uses the same shape. `source: refused` and `source: budget` have no choices. Rate limiting is HTTP 429, invalid requests 422, and model failure 503. The client keeps authored choices on any unusable result, including an older server returning 404.

## Boundaries and lifecycle

The server checks the request, screens prose before calling the model, looks up a context-aware cache, applies existing session/IP model limits and the shared daily budget, then calls the model once with structured output. A separate `DIRECTOR_SESSION_QUEST_BRANCHES_PER_HOUR` limit defaults to 12. There are no retries. Completed IDs, selected routes, motif, and permitted recipes are in the cache key; a different branch history gets its own response.

The model can write short labels and pitches and choose an allowed camp and bounded count. It cannot invent target classes, quest verbs, rewards, prerequisites, or additional jobs. Server and client both validate the two route IDs and camps. The client reconstructs gameplay from its own recipes and calculates rewards itself. UI objective labels and counts come from those recipes rather than generated prose.

The choice screen stays usable during the request. Selecting an authored route or closing the screen increments a request serial; an old response cannot reopen the screen or rewrite the accepted quest. Each displayed offer makes at most one model request. Accepted, ready, and completed quests remain protected against variation.

## Code and local checks

- Client transport: `UJamNoteClient::RequestQuestBranches`.
- Game lifecycle: `OfferVillageQuest`, `RequestVillageBranches`, `ResolveVillageBranches`, and `ChooseVillageBranch` in `JamVillageFlow.cpp`.
- Contract parsing and canonical routes: `JamVillageQuests::ReadBranches` and `Routes`.
- Server contract/prompt/clamp: `backend/director/quests.py`.
- Server endpoint, model, spend controls: existing `app.py`, `model.py`, and `service.py`.
- Test server: `python -I Scripts/note-stub.py --port 8019 --delay 0.05`; game `-noteurl=http://127.0.0.1:8019`.
- Automated alternate-route run: `-autoplay -autovillage -autobranches -quitaftervillage`.
- Failure check: test server `--status 503`, with the same game arguments. Authored alternate routes remain playable.
- UI previews: `-uiscreen=quests` and `-uiscreen=questbranches`.

The endpoint is implemented in the nested backend repository. Local checks use a fake model or the localhost stub and do not spend production model budget. Deploy the backend changes with its existing deployment procedure to enable the optional button on the public service. The existing nginx `/v1/director/` proxy and database schema cover the new endpoint without route or schema changes.

## Proposals (Fixes2 #4, contract v1, Nov 14)

The player writes their own plan for a villager's job; the model turns it into a
plan made only of things the game can spawn and verbs it can check. GAME owns the
client and this contract; BACKEND (Codex) implements the endpoint next to
`backend/director/quests.py`, with the same screen, limits, budget and cache as
branches, plus tests. Nothing here changes `/quest/branches`.

### Player flow

The quest offer screen gets a fourth button, **Propose a plan**, under the two
authored routes and "Ask Fenella to improvise". It opens the existing note editor
(keyboard or gamepad on-screen keys), 160 characters, placeholder
"How would you solve it?". Submitting sends the request; the screen stays usable
and both authored routes stay selectable while it waits. A usable reply adds a
third route, labelled with the reply's `label`, whose description lists the steps
("Pacify 3 rats at Meadow West, then feed them celery"). Fenella says the reply's
`reaction` as a caption. Choosing it starts the plan; the authored routes remain.
Any failure, refusal, budget stop, 404 or timeout: Fenella says a canned
"Let's stick to the script, darling." style line and the screen shows only the
authored routes. One proposal per offer screen. `-nonotes` (offline) hides the
button. The model never decides rewards, new types, new verbs or new places.

### Endpoint

`POST /v1/director/quest/propose`, same base URL, six-second client timeout,
same anonymous `session_id`.

Request:

```json
{
  "v": 1,
  "session_id": "3f2a...",
  "quest_id": "council_rats",
  "giver": "Mayor Bonbon",
  "title": "A Quiet Constituency",
  "pitch": "Three rats are holding a public meeting in the meadow. Pacify them. We already have a council.",
  "motif": "dental",
  "level": 1,
  "proposal": "Bribe them with snacks so they go home",
  "completed": ["actual_nuts"],
  "history": ["actual_nuts:original"],
  "recipe": { "verb": "pacify", "target": "rat", "count": 3, "destinations": ["CampMeadowWest"] },
  "vocabulary": {
    "steps": [
      { "verb": "pacify", "targets": ["rat", "goblin", "bug"] },
      { "verb": "feed",   "targets": ["rat"] },
      { "verb": "brush",  "targets": ["goblin"] },
      { "verb": "gather", "targets": ["almond", "celery", "candy_shard"] }
    ],
    "camps": [
      { "id": "CampMeadowWest",  "kinds": ["rat"] },
      { "id": "CampMeadowEast",  "kinds": ["rat", "almond", "celery", "candy_shard"] },
      { "id": "CampMeadowFar",   "kinds": ["rat"] },
      { "id": "CampMolarsNear",  "kinds": ["goblin"] },
      { "id": "CampMolarsRidge", "kinds": ["goblin"] },
      { "id": "CampMolarsFar",   "kinds": ["goblin"] },
      { "id": "CampOrchardNear", "kinds": ["bug", "almond", "celery", "candy_shard"] },
      { "id": "CampOrchardGrove","kinds": ["bug"] },
      { "id": "CampOrchardFar",  "kinds": ["bug", "almond", "celery", "candy_shard"] }
    ],
    "limits": { "max_steps": 3, "max_count": 8, "max_total": 12 }
  }
}
```

- `level`: the giver's relationship level, 0 Stranger, 1 Acquaintance, 2 Friend,
  3 Bestie (Fixes2 #5). Higher levels may get longer plans (see limits below).
- `proposal`: the player's text, control characters stripped, at most 160
  characters (server rejects longer with 422). Untrusted: screen it like a note.
- `recipe`: the job's authored shape, for context only.
- `vocabulary`: the closed sets, sent every time so the server prompt and
  validator never drift from the client. The server should still hard-code the
  same sets and reject a request whose vocabulary differs (422), so a modified
  client cannot widen them. `kinds` lists what may appear at a camp; gather
  targets need a camp whose kinds include them.

Success response (`source` is `model` or `cache`):

```json
{
  "v": 1,
  "source": "model",
  "quest_id": "council_rats",
  "label": "Snack diplomacy",
  "pitch": "Calm the rats, then win them over with celery.",
  "reaction": "Bribery! Wonderful. We'll call it a catering subplot.",
  "steps": [
    { "verb": "pacify", "target": "rat",    "count": 3, "destination": "CampMeadowWest" },
    { "verb": "feed",   "target": "rat",    "count": 3, "destination": "CampMeadowWest" }
  ]
}
```

- `label` 1-40 chars, `pitch` 1-240, `reaction` 1-160 (Fenella's voice; one or
  two sentences; no stage directions, no markup).
- `steps`: 1 to `max_steps` items, in order. Each step: `verb` from the
  vocabulary, `target` allowed for that verb, `count` integer 1-`max_count`,
  `destination` a camp whose `kinds` include the target. Sum of counts at most
  `max_total`. At most two distinct destinations.
- Level scaling (server prompt, client clamp): level 0-1 at most 2 steps,
  level 2-3 at most 3. The client clamps, it does not reject, on this one.

Unusable proposals: `{"v":1,"source":"refused","quest_id":"...","reaction":"..."}`
when the proposal is off-topic, unsafe or impossible with the vocabulary
(`reaction` optional, 1-160 chars, the client may show it instead of its canned
line), `{"v":1,"source":"budget","quest_id":"..."}` when the daily budget is
spent. Rate limit 429 (`DIRECTOR_SESSION_QUEST_PROPOSALS_PER_HOUR`, default 6),
invalid request 422, model failure 503. No retries; one model call per request.
Cache key: quest id, level, completed, history, recipe, and the proposal
lower-cased with whitespace collapsed.

### Client validation (the game trusts nothing)

`JamVillageQuests::ReadProposal` parses into an `FJamQuestPlan`. Any of these
fails the whole reply (authored routes only): wrong `v`, source not
`model`/`cache`, mismatched `quest_id`, body over 16 KB, missing or empty
label/pitch/reaction after cleaning, zero steps, unknown verb, target not
allowed for the verb, destination not a camp or not allowed for the target.
These are clamped, not failed: count to 1-8, more steps than the level allows
(extra steps dropped), total over 12 (later counts reduced, a step at 0 is
dropped). Text is cleaned like branches (control chars and `<>` removed,
trimmed, cut to length). A `pacify` step directly followed by `feed`/`brush` of
the same target at the same camp is merged into the feed/brush step (feeding
and brushing already require pacifying first).

### Game semantics

- Steps run in order. Each step spawns its own targets (or pickups) at its
  destination when it starts; the compass and active marker point there; the
  objective line reads "Step 2/3: Rats fed celery 1/3".
- `feed` = existing Interact on rats (celery); `brush` = Interact on goblins;
  `gather` = Collect with the mesh for the target (`SM_Almond`, `SM_Celery`,
  `SM_LollyShard`).
- After the last step the quest is Ready: return to the giver as always.
- Reward: the client's own formula, base reward x total count / base count,
  clamped 50-300, plus 40 XP. A plan earns the same relationship progress as an
  authored route (Fixes2 #5). History records the route as `<quest_id>:proposal`.
- Selecting an authored route or closing the screen bumps the request serial;
  a late reply is dropped.

### Code

- Client: `UJamNoteClient::RequestQuestProposal`, `JamVillageQuests::ReadProposal`,
  `FJamVillageQuestLog::SelectPlan`, `AJamGameMode::RequestVillageProposal`.
- Server: `backend/director/quests.py` (prompt, validation, clamp) next to branches.
- Local check after GO: the note stub gets a `/v1/director/quest/propose` route
  returning the success example; `-autopropose="feed them"` makes autoplay
  submit one proposal and pick the plan.
