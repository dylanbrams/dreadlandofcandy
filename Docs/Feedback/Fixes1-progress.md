# Fixes1 progress

## 1. Wrap screen and comic buttons - implemented

A reusable C++ Slate/UMG comic border now gives the wrap action, note choices, menus, and quest choices visible jagged edges, generous click targets, and hover feedback. The final open-world wrap says FILM COMPLETE and returns the player to the village. The Premiere remains available through Fenella. Backlot map verification is excluded at Dylan's request: the enlarged mountain obstructs the third arena. Its results are not used as evidence for this patch.

Wide serrated buttons use cached polygon triangulation so translucent fills do not overlap or produce crossed stripes. The wrap remains visible when the compass switches to the next quest. Holding F to talk cannot auto-confirm a new choice through repeated key events.

## 2. Recruitment and quest markers - implemented

The hero starts looking into Sugarborough. Fenella waits there under a glowing, bouncing golden triangle and joins the hero only after conversation. Entering the first quest area before recruitment does not start it. Her explicit captions direct every main stage and transition. The active quest's clearing has the same marker, alongside the compass and travel hint.

Ground placement ignores nonblocking world volumes; this prevents Fenella and other quest actors from being placed above invisible geometry. Arrival and the quest UI were inspected in engine at 1280 x 800.

## 3. Village follow-up quests - implemented and expanded

After Fenella's three quests, five villagers offer ten linked jobs: Mayor Bonbon, Baker Crumb, Gardener Sprig, Dr. Molar, and Scout Taffy. Their jobs cover pacifying, gathering different items, and pacifying then feeding/brushing. The villagers reuse existing art with role props. Their golden markers reflect availability and reward readiness.

Each giver has a follow-up unlocked by returning the first job. Rewards require a return visit, cannot be claimed twice, and pay the displayed sprinkles plus 40 quest XP. Pause > Quest Journal shows progress and rewards and can set aside an errand. Fenella accompanies the hero throughout; watching the Premiere and credits returns to the same village session.

## Dynamic hooks and backend branches

Recipes have stable IDs, prerequisites, seeded counts and camps, bounded variations, transactional versioned JSON overrides, and lifecycle events. See [Village_QuestHooks.md](../Design/Village_QuestHooks.md) and the example `quest-offers.example.json`.

Every quest offers two playable routes. Ask Fenella to improvise optionally calls the backend for contextual branch choices. Selected route history accompanies later requests. The game and backend both validate choices; cancellation, timeout, refusal, invalid responses, and older servers preserve authored routes. See [QuestBranches.md](../Design/QuestBranches.md). The new endpoint is implemented locally and still needs deployment to the public backend.

## Validation

Editor builds pass. The final complete suites passed 79 Unreal tests and 117 backend tests. Full terrain autoplay passed the three main quests and all ten original village routes; a second run received localhost HTTP branch choices and completed all ten alternate routes (`JamVillageAutoplay: PASS main=3 side=10`). A third run completed all ten alternate routes with every backend request returning HTTP 503, proving the authored fallback. Wide-border geometry and deliberate-choice input regression checks pass; the corrected route UI was inspected in engine.

New dialogue is captioned. Progress is session-local. No packaged build has been produced for this patch.
