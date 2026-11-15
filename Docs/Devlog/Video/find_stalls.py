"""Find game stalls (the capture froze while the game hitched) in GameFootage: build/stalls.json

    python -I find_stalls.py

A stall = at least 0.5 s of exactly repeated frames. Natural 8-10 fps duplication gives runs of 3-4 frames, never 15.
render_rough cuts stalls out of B-roll windows (voice-over, so nobody misses them).
"""
import json
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

import cv2
import numpy as np

HERE = Path(__file__).parent
FOOT = HERE.parent / "GameFootage"
MIN_RUN = 15


def stalls(path):
    c = cv2.VideoCapture(str(path))
    fps = c.get(cv2.CAP_PROP_FPS) or 30.0
    prev, runs, run, start, i = None, [], 0, 0, 0
    while True:
        ok, f = c.read()
        if not ok:
            break
        g = cv2.cvtColor(cv2.resize(f, (320, 180)), cv2.COLOR_BGR2GRAY).astype(np.float32)
        same = prev is not None and float(np.abs(g - prev).mean()) < 0.02
        if same:
            if run == 0:
                start = i - 1
            run += 1
        else:
            if run >= MIN_RUN:
                runs.append([round(start / fps, 2), round(i / fps, 2)])
            run = 0
        prev = g
        i += 1
    if run >= MIN_RUN:
        runs.append([round(start / fps, 2), round(i / fps, 2)])
    return path.name, runs


def main():
    files = sorted(FOOT.glob("*.mp4"))
    out = {}
    with ProcessPoolExecutor(max_workers=4) as ex:
        for name, runs in ex.map(stalls, files):
            out[name] = runs
            total = sum(b - a for a, b in runs)
            print(f"{name:<28}{len(runs):3d} stalls, {total:5.1f}s frozen", flush=True)
    (HERE / "build" / "stalls.json").write_text(json.dumps(out), encoding="utf-8")


if __name__ == "__main__":
    main()
