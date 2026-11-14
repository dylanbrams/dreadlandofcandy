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
