"""Resume skill animation delivery on existing Meshy rigs, without regenerating models.

Run with --project PATH. Saves task IDs through the project's meshy_rig client.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys

MOTIONS = {
    "FormALine": "A standing armoured adult gives a firm commanding gesture: left arm extends forward at chest height and sweeps inward to order a queue, then returns to ready. Right hand holds an imaginary sword still, feet planted, no walking, no props, no attack swing. Complete the gesture in the first second.",
    "LastJudgment": "A standing armoured adult delivers a grand final verdict: left arm rises dramatically overhead, palm open, then sweeps down authoritatively to chest height in the first second. Right hand holds an imaginary sword still at the side. Feet planted, no jump, no props, no weapon swing, then return to ready.",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, required=True)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()
    root = args.project.resolve()
    rig_root = root / "ImportSource/Meshy"
    # Establish the two new motions once on Knight, then reuse their motion IDs.
    for who in ("Knight", "Mage", "Archer", "Barbarian"):
        name = f"SK_{who}_Rig"
        state_path = rig_root / name / "rig.json"
        state = json.loads(state_path.read_text(encoding="utf-8"))
        assert state.get("rig_task") and (state_path.parent / f"{name}.fbx").is_file(), who
        if who != "Knight" and not args.dry_run:
            source = json.loads((rig_root / "SK_Knight_Rig/rig.json").read_text(encoding="utf-8"))
            for label in ("SwordRaise", "Sigh", "Scrub", *MOTIONS):
                rec = source["anims"][label]
                assert rec.get("motion_task"), label
                state.setdefault("anims", {}).setdefault(label, {
                    key: rec[key] for key in ("motion_task", "prompt") if key in rec
                })
            state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
        command = [sys.executable, "-I", str(root / "Scripts/meshy_rig.py"),
                   "--task-id", "existing-rig", "--name", name]
        motions = dict(MOTIONS)
        if who != "Knight":
            knight = json.loads((rig_root / "SK_Knight_Rig/rig.json").read_text(encoding="utf-8"))
            motions.update({label: knight["anims"][label]["prompt"]
                            for label in ("SwordRaise", "Sigh", "Scrub")})
        for label, prompt in motions.items():
            command.extend(["--motion", f"{label}=2:{prompt}"])
        if args.dry_run:
            command.append("--dry-run")
        print(f"Delivering {who}: {', '.join(motions)}", flush=True)
        subprocess.run(command, cwd=root, check=True)
    print("All four casts delivered.", flush=True)


if __name__ == "__main__":
    main()
