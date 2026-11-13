# Sword raise wrist correction

The runtime knight's sword is attached to `hand_r` with fixed relative rotation
`FRotator(0, 0, -90)`. This maps blade +Z onto the hand bone's -Y axis. The
generated SwordRaise raises the arm while leaving that axis up to 68.39 degrees
off vertical during the overhead hold.

`Scripts/correct_sword_raise.py` corrects only the `hand_r` rotation track in
Unreal's native bone coordinates. A shortest-arc rotation aligns the blade axis
upward while preserving the original wrist roll. The correction blends in with
smoothstep between 0.15 and 0.65 seconds. Sword attachment remains fixed, so the
hand and sword move together. All other local bone transforms, wrist translation
and scale, sample rate and clip duration remain unchanged.

The default commandlet invocation inspects without saving. Set
`JAM_GRIP_APPLY=1` to apply/save. Run the script using an absolute `-script=`
path. `Scripts/import_hero_rig.py` reapplies it after a source FBX import.
`correct(asset_path=..., apply=True, refresh_source=True)` supports a newly
imported knight at a different asset path. Source FBXs remain unmodified.

The original native package and sampled source keys are retained here for
rollback and reproducibility. A regular rerun uses those original keys so the
correction does not accumulate. `refresh_source=True` samples a newly imported
FBX and retains the previous keys separately.

Validation checks all sampled local tracks against the original: only the wrist
rotation may change. It also checks overhead blade tilt is below 0.1 degree.
See `A_SK_Hero_Rig_SwordRaise_validation.json`. The original open-finger mesh has
no finger bones; this correction changes wrist orientation, not finger curling.
