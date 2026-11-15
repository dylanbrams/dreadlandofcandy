"""Find rendered pieces whose footage windows overlap a Discord toast that was NOT blurred, and mark them for re-render.

    python -I privacy_check.py        # report
    python -I privacy_check.py fix    # delete the .key of unsafe pieces so the next render_rough redoes them

Uses the windows actually used (render_rough.plan_windows, from each piece's voice length) against GameFootage/toasts.json.
Dylan's unscanned recordings are always blurred, so they are checked only for having the mask planned.
"""
import importlib.util, json, sys
from pathlib import Path

HERE = Path(__file__).parent
sys.path.insert(0, str(HERE))
spec = importlib.util.spec_from_file_location("rr", HERE / "render_rough.py")
rr = importlib.util.module_from_spec(spec); spec.loader.exec_module(rr)


def main():
    fix = "fix" in sys.argv
    edit = json.loads((HERE / "edit.json").read_text(encoding="utf-8"))
    names = json.loads((rr.SEG / "names.json").read_text())
    idx, bad, checked = 1, [], 0
    for sec in edit:
        for g in sec["segments"]:
            if g["kind"] == "pickup" and not g.get("recorded"):
                continue
            clips = (g.get("broll") or {}).get("clips") or []
            nm = names.get(str(idx))
            w = rr.SEG / f"{nm}.wav"
            kf = rr.SEG / f"{nm}.key"
            made_by_fixed_code = kf.exists() and kf.read_text().startswith("m2:")
            if clips and nm and w.exists() and g["frame"] in "BP" and not made_by_fixed_code:
                D = rr.dur(w)
                checked += 1
                # the old renderer masked by the planned [i, o] only; this is what the window really covers
                for c, src, start, n in rr.plan_windows(clips, D):
                    need = rr.window_mask(src.name, start, n)
                    had = c.get("mask_bottom_right")
                    if need and not had:
                        bad.append((idx, src.name, round(start, 1), round(start + n, 1), need))
            idx += 1
    print(f"{checked} footage pieces checked; unsafe: {len(bad)}")
    for b in bad:
        print("  #%d %s %.1f-%.1f overlaps toast %s" % b)
        if fix:
            k = rr.SEG / f"{names[str(b[0])]}.key"
            if k.exists():
                k.unlink()
    if fix and bad:
        print("keys removed; re-run render_rough.py to redo those pieces")


if __name__ == "__main__":
    main()
