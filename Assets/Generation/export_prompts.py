"""Export prompt text locally. No generation clients, credentials or network access.

Run from any directory: python -I Assets/Generation/export_prompts.py
"""
import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Assets/Prompts"
STAGES = ("test", "batch1", "batch2")


def main():
    doc = json.loads((ROOT / "Assets/Generation/jobs.json").read_text(encoding="utf-8"))
    styles = json.loads((ROOT / "Docs/styles.json").read_text(encoding="utf-8"))
    jobs = doc["jobs"]
    by_id = {j["id"]: j for j in jobs}
    assert len(by_id) == len(jobs), "Duplicate job IDs"
    anchors = {j["group"]: j["id"] for j in jobs
               if j["stage"] == "test" and j["tool"] != "meshy_rig"}
    records = []
    for j in jobs:
        assert j["stage"] in STAGES
        assert all(dep in by_id for dep in j.get("needs", []))
        lane = "2D" if j["tool"] == "image" else "3D"
        dest = OUT / lane / f"{j['id']}.txt"
        if j["tool"] == "image":
            prompt = " ".join((doc["styles_2d"][j["group"]], j["subject"],
                               doc["rules_2d"][j.get("kind", "icon")]))
            ref = None if j["stage"] == "test" else anchors[j["group"]]
            setup = j.get("size", "1024x1024")
            setup += "; opaque" if j.get("kind") == "full" else "; transparent"
        elif j["tool"] == "meshy_rig":
            args = j["args"]
            actions = [args[i + 1] for i, a in enumerate(args) if a == "--action"]
            motions = [args[i + 1] for i, a in enumerate(args) if a == "--motion"]
            prompt = ("RIG AND ANIMATION BRIEF (not a text-to-3D prompt)\n\n"
                      "Input: SK_Hero textured humanoid mesh. Target height: 2.1 m.\n"
                      "Requested export: UE5 mannequin skeleton via the existing rig client.\n"
                      "Preset actions (id:name):\n" + "\n".join(actions)
                      + "\n\nText-to-motion requests (name=duration:prompt):\n"
                      + "\n".join(motions)
                      + "\n\nMust: idle and walk. Sword raise is optional; code can rotate the sword.\n"
                      "Keep feet planted for raise, scrub and sigh. No combat swings or jumps.\n"
                      "Separate hand props are attached in Unreal, not generated into the body.\n"
                      "Review grip alignment and timing in the game camera.\n"
                      "Rig success is untested; retain the two-hour day-2 time-box and fallback.\n")
            ref = "SK_Hero"
            setup = "rig + animation; see brief"
        else:
            st = styles[j["group"]]
            parts = [x.strip().strip(",").strip() for x in
                     (st["prefix"], j["subject"].rstrip(". "), st["suffix"])]
            prompt = ", ".join(x for x in parts if x)
            if j["tool"] == "meshy":
                assert len(prompt) <= 800, f"{j['id']}: {len(prompt)} > 800 characters"
            ref = None if j["stage"] == "test" else anchors[j["group"]]
            setup = " ".join(j.get("args", []))
        records.append({"id": j["id"], "file": dest.relative_to(OUT).as_posix(),
                        "stage": j["stage"], "priority": j["pri"], "tool": j["tool"],
                        "group": j["group"], "setup": setup, "reference": ref,
                        "prompt": prompt})

    # Validate everything before writing any exports.
    for r in records:
        dest = OUT / r["file"]
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_text(r["prompt"].rstrip() + "\n", encoding="utf-8")
    for lane in ("2D", "3D"):
        intro = ("One self-contained prompt per asset; no sheets or slicing. The text files contain "
                 "only the full prompt to paste. Sizes and references are below. Use only an approved "
                 "reference from the matching layer. Opaque paper/poster jobs fill the canvas; all "
                 "other images require transparency.\n" if lane == "2D" else
                 "Text files contain the full style-wrapped prompt for manual use. With the existing "
                 "clients, pass only the jobs.json subject and --type; passing these full files with "
                 "--type would duplicate the style. The rig file is a separate animation brief. "
                 "A reference in this table means a review/style anchor; the 3D runner does not "
                 "attach an image reference automatically.\n")
        lines = [f"# {lane} prompts\n", "Prepared 2026-11-12. Status: **prepared, not generated or approved**.\n",
                 intro, "See [handoff and review criteria](../README.md).\n"]
        for stage in STAGES:
            lines += [f"## {stage}\n", "| Prompt | Tool / group | Priority | Setup | Reference |",
                      "| --- | --- | --- | --- | --- |"]
            for r in records:
                if not r["file"].startswith(lane + "/") or r["stage"] != stage:
                    continue
                ref = r["reference"] or "None: initial anchor"
                lines.append(f"| [{r['id']}]({r['id']}.txt) | {r['tool']} / {r['group']} | "
                             f"{r['priority']} | {r['setup']} | {ref} |")
            lines.append("")
        (OUT / lane / "README.md").write_text("\n".join(lines), encoding="utf-8")
    (OUT / "prompts.json").write_text(json.dumps(records, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Exported {len(records)} prompt files: {dict(Counter(r['tool'] for r in records))}")
    print("No assets generated; no generation state, approvals or declarations changed.")


if __name__ == "__main__":
    main()
