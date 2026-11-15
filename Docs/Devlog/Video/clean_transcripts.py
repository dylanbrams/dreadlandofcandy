"""Clean transcripts from the words.json files.

    python -I clean_transcripts.py

Reads raw/original/*.words.json. Writes, in transcripts/:
  <stem>.txt     filler-free text, one line per sentence-ish chunk, with [start-end] seconds
  fillers.json   {stem: [[start, end, word], ...]} um/uh spans, for the edit to trim
  _all.txt       every take in time order, one block each
  _digest.txt    one line per take: duration, words, fillers, first/last words
"""
import json, re
from pathlib import Path

HERE = Path(__file__).parent
SRC = HERE / "raw" / "original"
OUT = HERE / "transcripts"
FILLERS = {"um", "uh", "uhm", "umm", "uhh", "er", "erm", "ah", "hmm", "mm", "hm", "eh"}
GAP = 0.8  # seconds of silence that starts a new line

def norm(w):
    return re.sub(r"[^a-z]", "", w.lower())

def main():
    OUT.mkdir(exist_ok=True)
    fillers, digest, allt = {}, [], []
    for f in sorted(SRC.glob("*.words.json")):
        stem = f.name.replace(".words.json", "").replace("2026-11-15 ", "")
        d = json.loads(f.read_text(encoding="utf-8"))
        keep, fl = [], []
        for w in d["words"]:
            if norm(w["w"]) in FILLERS:
                fl.append([w["s"], w["e"], norm(w["w"])])
            else:
                keep.append(w)
        fillers[stem] = fl
        lines, cur = [], []
        for w in keep:
            if cur and (w["s"] - cur[-1]["e"] > GAP or re.search(r"[.?!]$", cur[-1]["w"])):
                lines.append(cur); cur = []
            cur.append(w)
        if cur: lines.append(cur)
        text = "\n".join(f"[{l[0]['s']:.1f}-{l[-1]['e']:.1f}] " + " ".join(x["w"] for x in l) for l in lines)
        (OUT / f"{stem}.txt").write_text(f"# {stem}  ({d['duration']:.0f}s)\n{text}\n", encoding="utf-8")
        allt.append(f"## {stem} ({d['duration']:.0f}s, {len(fl)} fillers)\n{text}\n")
        ws = [x["w"] for x in keep]
        digest.append(f"{stem} {d['duration']:5.0f}s {len(ws):4d}w {len(fl):3d}f | {' '.join(ws[:12])} ... {' '.join(ws[-6:])}")
    (OUT / "fillers.json").write_text(json.dumps(fillers), encoding="utf-8")
    (OUT / "_all.txt").write_text("\n".join(allt), encoding="utf-8")
    (OUT / "_digest.txt").write_text("\n".join(digest), encoding="utf-8")
    print(f"{len(digest)} takes, {sum(len(v) for v in fillers.values())} fillers; {(OUT/'_all.txt').stat().st_size//1024} KB all")

main()
