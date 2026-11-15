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
FADE = 0.03

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

SENT_END = re.compile(r"[.?!][\"')\]]*$")
GAP_KEEP = 1.0                 # natural pauses up to this long stay as spoken
PAD_BEFORE, PAD_AFTER = 0.14, 0.22
TAIL_END = 0.30                # extra room after the last word, so a trailing syllable is never clipped


def select_words(words, a, b, complete=True):
    """Words that START inside [a - 0.15, b); then run on to the end of the sentence (a word ending . ? !)."""
    sel = [i for i, w in enumerate(words) if a - 0.15 <= w["s"] < b]
    if not sel or not complete:
        return sel
    last = sel[-1]
    k = last
    while k < len(words) and not SENT_END.search(words[k]["w"]):
        nxt = k + 1
        if nxt >= len(words) or words[nxt]["s"] - words[k]["e"] > 1.5 or words[nxt]["e"] - b > 9:
            break
        k = nxt
        sel.append(k)
    return sel


def spans(words, ranges, drops, complete=True):
    """Keep every word in the ranges, remove um/uh (and anything in `drops`), keep natural pauses up to GAP_KEEP,
    and always finish the sentence. Returns (lo, hi) source spans; cut edges fall in room tone, not on words."""
    out = []
    for a, b in ranges:
        idx = select_words(words, a, b, complete)
        kept = [i for i in idx if norm(words[i]["w"]) not in FILLERS
                and not any(x <= words[i]["s"] and words[i]["e"] <= y for x, y in drops)]
        if not kept:
            continue
        cur = [max(0.0, words[kept[0]]["s"] - PAD_BEFORE), None]
        for n, i in enumerate(kept):
            w = words[i]
            nxt = kept[n + 1] if n + 1 < len(kept) else None
            if nxt is None:
                after = words[i + 1]["s"] - 0.02 if i + 1 < len(words) else w["e"] + TAIL_END
                cur[1] = min(w["e"] + TAIL_END, after)
                break
            gap = words[nxt]["s"] - w["e"]
            hole = any(j not in kept for j in range(i + 1, nxt))        # an um/uh or dropped word sits in the gap
            if hole or gap > GAP_KEEP:
                cut_end = min(w["e"] + PAD_AFTER, words[i + 1]["s"] - 0.02)
                nxt_start = max(words[nxt]["s"] - PAD_BEFORE, words[nxt - 1]["e"] + 0.02)
                if nxt_start > cut_end + 0.06:                           # a real cut; otherwise just keep it natural
                    cur[1] = cut_end
                    out.append(tuple(cur))
                    cur = [nxt_start, None]
        out.append(tuple(cur))
    # merge overlaps between ranges
    merged = []
    for lo, hi in sorted(out):
        if merged and lo <= merged[-1][1] + 0.01:
            merged[-1] = (merged[-1][0], max(merged[-1][1], hi))
        else:
            merged.append((lo, hi))
    return merged


def render(name, folder, take, ranges, drops):
    src = HERE / "raw" / folder / f"2026-11-15 {take}.mp4"
    words = json.loads(src.with_suffix(".words.json").read_text(encoding="utf-8"))["words"]
    sp = spans(words, ranges, drops)
    assert sp, f'{name}: no words selected'
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
