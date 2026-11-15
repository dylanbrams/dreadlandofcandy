"""Cut chosen passages out of raw takes, removing um/uh and long pauses, to build/clean/<id>.mp4.

    python -I clean_takes.py            # every pick below
    python -I clean_takes.py 08-pbr     # one

PICKS: id -> (folder under raw/, take id, [(in, out), ...], [drop (a, b) ranges]).
Within each range the words are kept, um/uh-type words are removed, and silences longer than GAP are cut down
(a cut keeps PAD seconds of room either side so words are never clipped). Video and audio are cut together
(jump cuts on CAM; the picture plan covers them with PIP/B-ROLL). Output 1280x720 30 fps, AAC.
"""
import json, re, subprocess, sys
from pathlib import Path

HERE = Path(__file__).parent
OUT = HERE / "build" / "clean"
FILLERS = {"um", "uh", "uhm", "umm", "uhh", "er", "erm", "ah", "hmm", "mm", "hm", "eh"}
GAP, PAD, FADE = 0.35, 0.07, 0.01

P = "pickups"
PICKS = {
    # gap pickups
    "02-encores": (P, "14-59-12", [(1.4, 10.95)], []),
    "02-filmshoot-partial": (P, "14-59-37", [(5.9, 12.1)], []),
    "02-usabletake": (P, "14-59-53", [(0.95, 4.05)], []),
    "03-declared": (P, "15-01-15", [(0.25, 3.55)], []),
    "03b-warning": (P, "15-01-23", [(0.55, 3.55)], []),
    "06-playtest": (P, "15-01-39", [(0.75, 13.35)], []),
    "08-three": (P, "15-02-09", [(0.85, 7.65)], []),
    "08-pbr": (P, "15-02-27", [(0.55, 7.45)], []),
    "08-marshmallows": (P, "15-02-55", [(0.65, 6.55)], []),
    "08-blacksugar-a": (P, "15-03-11", [(0.55, 8.95)], []),
    "08-blacksugar-b": (P, "15-03-31", [(1.05, 16.15)], []),
    "08b-fixed": (P, "15-03-59", [(0.45, 27.95)], [(9.3, 9.65)]),
    "09-lookatwhat": (P, "15-04-43", [(0.65, 2.05)], []),
    "09-green": (P, "15-04-50", [(0.45, 3.85)], []),
    "10-ending": (P, "15-05-07", [(0.55, 14.15)], []),
    "11-theme": (P, "15-05-28", [(1.15, 3.15)], []),
    # new material (not in the script)
    "09-miyazaki": (P, "14-58-21", [(0.35, 31.55)], []),
    # retakes in his own words
    "retake-A-dream": (P, "15-05-41", [(3.1, 34.05)], []),
    "retake-B-sunday": (P, "15-06-27", [(0.65, 70.7), (76.0, 84.05), (109.85, 112.65)], []),
    "retake-C-gamejams": (P, "15-10-46", [(0.35, 70.75)], []),
    "retake-D-qa-people": (P, "15-12-13", [(0.45, 81.75)], []),
    "retake-E-gantt-usage": (P, "15-14-00", [(0.75, 45.55)], []),
    "retake-F-swg": (P, "15-15-01", [(0.95, 21.05)], []),
}

def norm(w):
    return re.sub(r"[^a-z]", "", w.lower())

def spans(words, ranges, drops):
    """Words to keep -> list of (start, end) source spans, merged across short gaps."""
    keep = []
    for w in words:
        if norm(w["w"]) in FILLERS:
            continue
        if not any(a <= w["s"] and w["e"] <= b for a, b in ranges):
            continue
        if any(a <= w["s"] and w["e"] <= b for a, b in drops):
            continue
        keep.append(w)
    out = []
    for w in keep:
        s, e = w["s"], w["e"]
        if out and s - out[-1][1] <= GAP:
            out[-1][1] = e
        else:
            out.append([s, e])
    # pad, without overlapping a neighbour's pad
    res = []
    for i, (s, e) in enumerate(out):
        lo = max(0.0, s - PAD)
        hi = e + PAD
        if res and lo < res[-1][1]:
            lo = res[-1][1]
        if i + 1 < len(out) and hi > out[i + 1][0] - PAD:
            hi = (e + out[i + 1][0]) / 2
        res.append((lo, hi))
    return res

def render(name, folder, take, ranges, drops):
    src = HERE / "raw" / folder / f"2026-11-15 {take}.mp4"
    words = json.loads(src.with_suffix(".words.json").read_text(encoding="utf-8"))["words"]
    sp = spans(words, ranges, drops)
    parts, ins = [], ""
    for i, (s, e) in enumerate(sp):
        d = e - s
        parts.append(f"[0:v]trim=start={s:.3f}:end={e:.3f},setpts=PTS-STARTPTS,fps=30[v{i}];"
                     f"[0:a]atrim=start={s:.3f}:end={e:.3f},asetpts=PTS-STARTPTS,"
                     f"afade=t=in:d={FADE},afade=t=out:st={max(d - FADE, 0):.3f}:d={FADE}[a{i}]")
        ins += f"[v{i}][a{i}]"
    fc = ";".join(parts) + f";{ins}concat=n={len(sp)}:v=1:a=1[v][a]"
    OUT.mkdir(parents=True, exist_ok=True)
    dest = OUT / f"{name}.mp4"
    scr = OUT / f"{name}.filter.txt"
    scr.write_text(fc, encoding="utf-8")
    cmd = ["ffmpeg", "-v", "error", "-y", "-i", str(src), "-/filter_complex", str(scr),
           "-map", "[v]", "-map", "[a]", "-c:v", "libx264", "-crf", "18", "-preset", "veryfast",
           "-pix_fmt", "yuv420p", "-c:a", "aac", "-b:a", "192k", str(dest)]
    subprocess.run(cmd, check=True)
    scr.unlink()
    dur = sum(e - s for s, e in sp)
    src_dur = sum(b - a for a, b in ranges)
    print(f"{name}: {len(sp)} cuts, {src_dur:.1f}s -> {dur:.1f}s")

def main():
    names = sys.argv[1:] or list(PICKS)
    for n in names:
        render(n, *PICKS[n])

if __name__ == "__main__":
    main()
