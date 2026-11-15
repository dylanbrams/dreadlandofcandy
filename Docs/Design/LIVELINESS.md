# Liveliness pass — Fixes5 #5–7

Implementation: movement-driven nuisance scurry/skitter/wobble, villager weight shifts,
breathing and conversational nods, shared motion for their role props, floating/turning
collectibles, and recycled drifting rooftop steam. All cosmetic animation leaves gameplay
actor positions and collision unchanged. Steam is capped at 20 buildings / 100 puffs,
updates at 30 Hz and hides beyond 60 m. Existing world materials and meshes are reused.

NPC skeletal pipeline: `Scripts/meshy_rig.py` generates rigs and Idle/Walk clips from
the approved original meshes. `Scripts/import_npc_rigs.py` imports each with its own
skeleton under `/Game/Art/NPC`. Runtime requires both compatible clips before replacing
the static mesh; procedural animation remains the fallback. Art's existing cook directory
covers the new assets. Villager, goblin and upright rat are candidates; humanoid rigging
is inappropriate for the flying lolly bug and almond.

## Further suggestions

- Tiny wing flutters and antenna motion for lolly bugs, using a dedicated insect rig.
- Footfall dust or sugar crumbs that change with the terrain and stop when standing still.
- Nearby villagers acknowledge the hero with a look and wave, then cheer on quest turn-in.
- A bakery chimney with flour puffs and a warm oven glow; dentist window glints; laundry
  or shop signs swinging gently. Building-specific placements would improve on roof-level steam.
- Candy-tree crown sway and fluttering grass from a shared wind material. Keep trunks
  fixed and use instance bounds that include the motion.
- A few butterflies or lazy fireflies around flowers and riverbanks, with distance culling.
- An expanding ring and short sprinkle shower on quest completion, coordinated with the
  existing sound and caption rather than a full-screen flash.
- Small water ripples where floating marshmallows pass bridge piers.

Prefer brief, readable action reactions and quiet ambient movement between fights.
Keep the ground under enemies clear so Resolve bars, attack shapes and objectives stay readable.
