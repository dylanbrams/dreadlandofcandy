"""Audit the rendered segments for freezes and low real framerate.

    python -I audit_video.py        # reads build/seg/NNN.mp4 + edit.json, writes build/audit.txt, prints a summary

Per segment: % of frames that exactly repeat the previous one, the longest freeze (s), and the 'effective fps'
(distinct frames per second). Static cards/stills are listed separately (they are meant to be still).
Flags: gameplay/CAM/PIP with effective fps < 24 or a freeze > 0.4 s.
"""
import json
from pathlib import Path

import cv2
import numpy as np

HERE = Path(__file__).parent
SEG = HERE / "build" / "seg"
REPEAT = 0.02          # mean abs grey difference (0-255) below this = the same frame


def analyse(path):
    c = cv2.VideoCapture(str(path))
    prev, diffs = None, []
    while True:
        ok, f = c.read()
        if not ok:
            break
        g = cv2.cvtColor(cv2.resize(f, (320, 180)), cv2.COLOR_BGR2GRAY).astype(np.float32)
        if prev is not None:
            diffs.append(float(np.abs(g - prev).mean()))
        prev = g
    d = np.array(diffs)
    if len(d) < 3:
        return None
    rep = d < REPEAT
    longest = run = 0
    for r in rep:
        run = run + 1 if r else 0
        longest = max(longest, run)
    n = len(d) + 1
    return dict(frames=n, repeat_pct=100 * rep.mean(), longest_s=longest / 30.0, eff_fps=30.0 * (1 - rep.mean()))


def main():
    edit = json.loads((HERE / "edit.json").read_text(encoding="utf-8"))
    names = json.loads((SEG / "names.json").read_text())
    rows, idx = [], 1
    for sec in edit:
        for g in sec["segments"]:
            if g["kind"] == "pickup" and not g.get("recorded"):
                continue
            p = SEG / f"{names.get(str(idx), 'none')}.mp4"
            if p.exists() and p.stat().st_size > 1000:
                a = analyse(p)
                if a:
                    b = g.get("broll") or {}
                    kind = ("CAM" if g["frame"] == "C" else "PIP" if g["frame"] == "P" else "B")
                    static = (g["frame"] in "BP" and not b.get("clips") and "slide" not in "")  # cards / stills
                    rows.append((idx, sec["section"], kind, (g.get("visual") or g.get("take") or g.get("id") or "")[:46], static, a))
            idx += 1
    lines, flagged = [], []
    for i, s, k, v, static, a in rows:
        flag = "" if static else ("  <-- CHECK" if (a["eff_fps"] < 24 or a["longest_s"] > 0.4) else "")
        if flag:
            flagged.append(i)
        lines.append(f"#{i:<3} {s:<4}{k:<4}{'still' if static else 'live ':<6}eff {a['eff_fps']:5.1f} fps  repeat {a['repeat_pct']:4.0f}%  "
                     f"longest freeze {a['longest_s']:.2f}s  {v}{flag}")
    (HERE / "build" / "audit.txt").write_text("\n".join(lines), encoding="utf-8")
    live = [r for r in rows if not r[4]]
    print(f"{len(rows)} segments audited ({len(live)} live). Flagged: {flagged or 'none'}")
    print(f"worst effective fps among live segments: " + ", ".join(f"#{r[0]} {r[5]['eff_fps']:.0f}" for r in sorted(live, key=lambda r: r[5]['eff_fps'])[:6]))


if __name__ == "__main__":
    main()
