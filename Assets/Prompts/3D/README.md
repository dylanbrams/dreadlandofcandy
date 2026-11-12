# 3D prompts

Prepared 2026-11-12. Status: **prepared, not generated or approved**.

Text files contain the full style-wrapped prompt for manual use. With the existing clients, pass only the jobs.json subject and --type; passing these full files with --type would duplicate the style. The rig file is a separate animation brief. A reference in this table means a review/style anchor; the 3D runner does not attach an image reference automatically.

See [handoff and review criteria](../README.md).

## test

| Prompt | Tool / group | Priority | Setup | Reference |
| --- | --- | --- | --- | --- |
| [SK_Hero](SK_Hero.txt) | meshy / character | must | --polycount 40000 | None: initial anchor |
| [SK_Hero_Rig](SK_Hero_Rig.txt) | meshy_rig / character | must | rig + animation; see brief | SK_Hero |
| [SM_Sword](SM_Sword.txt) | rodin / equipment | must | --tier Gen-2.5-High | None: initial anchor |
| [SM_Rat](SM_Rat.txt) | meshy / npc | must | --polycount 5000 | None: initial anchor |
| [SM_HouseFront](SM_HouseFront.txt) | meshy / terrain_piece | must | --polycount 4000 | None: initial anchor |
| [SM_Peppermint](SM_Peppermint.txt) | meshy / particle_mesh | must | --polycount 400 | None: initial anchor |

## batch1

| Prompt | Tool / group | Priority | Setup | Reference |
| --- | --- | --- | --- | --- |
| [SM_Toothbrush](SM_Toothbrush.txt) | rodin / equipment | should | --tier Gen-2.5-Medium | SM_Sword |
| [SM_Goblin](SM_Goblin.txt) | meshy / npc | must | --polycount 6000 | SM_Rat |
| [SM_Villager](SM_Villager.txt) | meshy / npc | must | --polycount 6000 | SM_Rat |
| [SM_Fenella](SM_Fenella.txt) | meshy / npc | must | --polycount 5000 | SM_Rat |
| [SM_LollyBug](SM_LollyBug.txt) | meshy / npc | must | --polycount 4000 | SM_Rat |
| [SM_Almond](SM_Almond.txt) | meshy / npc | must | --polycount 2000 | SM_Rat |
| [SM_CandyCrop](SM_CandyCrop.txt) | meshy / terrain_piece | must | --polycount 1500 | SM_HouseFront |
| [SM_CandyFence](SM_CandyFence.txt) | meshy / terrain_piece | must | --polycount 800 | SM_HouseFront |
| [SM_Celery](SM_Celery.txt) | meshy / terrain_piece | must | --polycount 800 | SM_HouseFront |
| [SM_Molar](SM_Molar.txt) | meshy / terrain_piece | must | --polycount 1500 | SM_HouseFront |
| [SM_Signpost](SM_Signpost.txt) | meshy / terrain_piece | must | --polycount 500 | SM_HouseFront |
| [SM_CardboardBox](SM_CardboardBox.txt) | meshy / terrain_piece | must | --polycount 300 | SM_HouseFront |
| [SM_CandyTree](SM_CandyTree.txt) | meshy / terrain_piece | should | --polycount 2000 | SM_HouseFront |
| [SM_Boulder](SM_Boulder.txt) | meshy / terrain_piece | should | --polycount 1000 | SM_HouseFront |
| [SM_Clapperboard](SM_Clapperboard.txt) | meshy / terrain_piece | should | --polycount 800 | SM_HouseFront |
| [SM_DirectorsChair](SM_DirectorsChair.txt) | meshy / terrain_piece | should | --polycount 1000 | SM_HouseFront |
| [SM_Stanchion](SM_Stanchion.txt) | meshy / particle_mesh | should | --polycount 400 | SM_Peppermint |

## batch2

| Prompt | Tool / group | Priority | Setup | Reference |
| --- | --- | --- | --- | --- |
| [SM_FortGate](SM_FortGate.txt) | meshy / terrain_piece | should | --polycount 4000 | SM_HouseFront |
| [SM_LightStand](SM_LightStand.txt) | meshy / terrain_piece | should | --polycount 1000 | SM_HouseFront |
| [SM_Table](SM_Table.txt) | meshy / terrain_piece | should | --polycount 600 | SM_HouseFront |
| [SM_Hat_Straw](SM_Hat_Straw.txt) | meshy / terrain_piece | should | --polycount 800 | SM_HouseFront |
| [SM_HeadMirror](SM_HeadMirror.txt) | meshy / terrain_piece | should | --polycount 800 | SM_HouseFront |
| [SM_Crook](SM_Crook.txt) | meshy / terrain_piece | should | --polycount 500 | SM_HouseFront |
| [SM_LeafMegaphone](SM_LeafMegaphone.txt) | meshy / terrain_piece | stretch | --polycount 300 | SM_HouseFront |
| [SM_LollyShard](SM_LollyShard.txt) | meshy / particle_mesh | stretch | --polycount 150 | SM_Peppermint |
