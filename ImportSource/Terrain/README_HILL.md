# Molar Hills elevation checkpoint

The saved main map now climbs 900 cm through the Molar Hills wedge, from just
outside Sugarborough to the level Fort Toothache set. Other film sets and the
village retain their reserved pads. `hill_profile.py` defines the uplift;
`build_sources.py` adds it after the original road/pad flattening.

`hill_sources.json` records the nine edited OBJ/FBX tiles and existing source
validation. `export_hill.py` re-exports only those tiles in Blender. The Unreal
script `apply_hill_unreal.py` imports them while preserving current polished
materials/collision, then lifts existing scenery and HISM instance roots by
the exact triangulated uplift. It preserves XY, scale, rotation and instance
count. Its map tag prevents a second lift; mesh hashes avoid redundant imports.
Current native verification is `hill_validation.json`; ground traces ignore
scenery such as the arch roof and verify the actual road surface below it.

This is an incremental upgrade of the previously baked map. Original ecology
JSON files remain the pre-hill placement baseline. If regenerating the whole
world and ecology using the new height function, those new placements already
contain the uplift: do not lift them again with this incremental script. Legacy
review-map dressing is pre-hill; use the fresh camera-only copies
`/Game/World/Review/L_MolarHillApproach` and `L_MolarHillOverview` for this pass.

The previous flat, horizontal capsule approach test is superseded for this road
by native ground samples and actual Open-mode packaged autoplay traversal.
All original source reservation slope checks still pass.

`import_skill_icons.py` imports the original four delivered images (three active
slots now use them) and authors `/Game/Art/UI/Materials/M_SkillCooldown`. This
UI material reveals color clockwise from twelve o'clock over a grayscale icon.
Runtime parameters: IconTexture, Cooldown (1 just cast / 0 ready), Unlocked.
New class-specific icon imports can replace the data paths independently.
