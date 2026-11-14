# Village quests and dynamic offer hooks

The open-world adventure begins by recruiting Fenella in Sugarborough. Her three film quests unlock ten village quests. Fenella remains the companion, offers directions, and can show the Premiere in town between errands. Credits return to the same village session.

## Quest catalogue

| Giver | First job | Follow-up | Gameplay |
| --- | --- | --- | --- |
| Mayor Bonbon | A Quiet Constituency | A More Representative Goblin | Pacify rats; pacify and brush goblins |
| Baker Crumb | Contains Actual Nuts | The Cake Has Teeth | Gather almonds; gather candy shards |
| Gardener Sprig | Celery, Actually | Crop Circles Are Rude | Gather celery; pacify bugs |
| Dr. Molar | Open Wide, Shut Up | Brushing Up On Diplomacy | Pacify and brush goblins; pacify and feed rats |
| Scout Taffy | Uncredited Extras | Nuts About The Horizon | Pacify bugs; gather almonds |

Each giver offers their next unfinished job. The follow-up unlocks when the first job is handed in. Only one errand is active at a time. Finishing objectives makes a reward ready; returning and talking to the giver pays it once. Quest completion pays 40 XP plus the displayed sprinkles; individual actions also give their usual XP. The journal is under Pause > Quest Journal. Setting aside a job removes its spawned targets and pickups without granting its reward and makes it available again.

## Definitions and seeded variation

`Source/MegaJam/Private/World/JamVillageQuests.cpp` owns recipes. `JamVillageQuests::Givers()` owns the five physical NPCs separately from their jobs; adding a follow-up does not spawn a duplicate villager.

A recipe has a stable ID, giver location, prerequisite ID, objective verb, target kind, pickup mesh, count range, eligible camps, display text, and base payment. References are checked by automation tests. A new recipe should only refer to existing target kinds and imported meshes.

`FJamVillageQuestLog(Seed)` resolves counts and camps once per session. The first two jobs stay authored by default; the rest vary reproducibly. Use `-questseed=42` to reproduce an offer set. Resolved offers remain stable during accepted work. Changing the seed does not require asset generation, a server, or network access.

## JSON offer hook

A tool or a future generation service can produce versioned overrides. The game can read a file at startup with `-questoffers="D:/Source/Unreal/MegaJam2026/Docs/Feedback/quest-offers.example.json"`.

```json
{
  "v": 1,
  "offers": [
    {
      "id": "celery_actually",
      "destination": "CampMeadowEast",
      "count": 4,
      "pitch": "The village needs a fresh celery delivery."
    }
  ]
}
```

All fields except `id` are optional. The matching C++ API is `FJamVillageQuestLog::ApplyOverridesJson(Json, Error)`; individual typed changes use `ApplyVariation(Index, Variation)`.

The parser bounds the document to 32K characters and the override count to the catalogue size. IDs must exist and appear once. Destination changes must name one of the recipe's camps. Counts must be positive integers and are clamped to the recipe's range. Rewards are calculated from the authored rate and capped at 300 sprinkles. Pitch text is cleaned and limited to 240 characters. Extra gameplay fields, including reward, target class, verb, and prerequisites, are rejected. Accepted, ready, and completed jobs cannot be rewritten. A malformed batch leaves every existing offer unchanged; an invalid startup file keeps the seeded recipes.

This file hook remains local. Optional live branching is documented in [QuestBranches.md](QuestBranches.md); the player requests it explicitly from the quest offer screen.

## Event hook

`FJamVillageQuestLog::OnChanged` is a multicast C++ delegate with a copied event payload: event type, stable quest ID, progress, and required count. It emits recruitment, main completion, acceptance, progress, reward-ready, turn-in, and abandonment events. Narrative, analytics, or a future reactive world system can subscribe without changing the gameplay rules. Listeners should observe events rather than mutate the quest log inside the callback.

The game mode alone owns spawned actors, proximity checks, XP, and currency. Offer text and generation hooks cannot award rewards or report progress themselves. If a future server supplies prose or variations, pass its response through the same validator before accepting an offer.

## Validation and remaining limits

Automation covers recruitment gating, nonblocking-volume ground placement, every recipe's world references, seeded variation, prerequisite order, reward bounds, accepted-offer immutability, transactional JSON rejection, abandonment cleanup, all objective types, turn-in range, duplicate rewards, the journal summary, and returning from the Premiere.

`-autoplay -autovillage -quitaftervillage -nonotes` plays the main adventure and all ten jobs. Look for `JamVillageAutoplay: PASS main=3 side=10` in the log. Backlot map runs are excluded from verification at Dylan's request because the enlarged mountain blocks the third arena. Use the open-world terrain flow and isolated automation tests for this work.

Progress is session-local, matching the existing game. New directions and quest dialogue are captions; this change does not generate new voice recordings. The editor target is the verification target; cook/package validation is a separate release check.
