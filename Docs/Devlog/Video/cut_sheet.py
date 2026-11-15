"""Plain-text cut sheet of the rough cut: build/cut_sheet.txt

    python -I cut_sheet.py

One line per segment, in video order: number, start time in the video, length, section, framing (CAM / PIP / B-roll),
source take and range, and the first words spoken. So notes can say "drop #34" or "retake #51".
"""
import json, sys, wave
from pathlib import Path

HERE = Path(__file__).parent
SEG = HERE / "build" / "seg"
sys.path.insert(0, str(HERE))
import clean_takes as ct  # noqa: E402


def wav_len(p):
    with wave.open(str(p)) as w:
        return w.getnframes() / w.getframerate()


def first_words(g, n=9):
    if g["kind"] == "take":
        src = HERE / g["src"]
        words = json.loads(src.with_suffix(".words.json").read_text(encoding="utf-8"))["words"]
        sel = ct.select_words(words, g["i"], g["o"])
        return " ".join(words[i]["w"] for i in sel[:n])
    if g["kind"] == "pickup":
        return " ".join(g["text"].split()[:n])
    return g.get("text", "")[:40]


def main():
    edit = json.loads((HERE / "edit.json").read_text(encoding="utf-8"))
    names = json.loads((SEG / "names.json").read_text())
    lines, idx, t = [], 1, 0.0
    cold = json.loads((SEG / "coldopen.plan.json").read_text())["total"] if (SEG / "coldopen.plan.json").exists() else 48
    for sec in edit:
        for g in sec["segments"]:
            if g["kind"] == "pickup" and not g.get("recorded"):
                continue
            if sec["section"] == "00":
                if idx == 1:
                    lines.append(f"      {0:>3}:{0:04.1f}  {cold:5.1f}s  00    COLD OPEN (screen render; voice lines below are inside it)")
                    t = cold
                lines.append(f"#{idx:<3}  (in cold open)        00    voice  {g.get('take')}  {g.get('i')}-{g.get('o')}   {first_words(g)}")
                idx += 1
                continue
            nm = names.get(str(idx)); d = wav_len(SEG / f"{nm}.wav") if nm and (SEG / f"{nm}.wav").exists() else 0
            kind = {"C": "CAM", "P": "PIP", "B": "B-roll"}[g["frame"]]
            src = (f"{g['take']} {g['i']:.0f}-{g['o']:.0f}s" if g["kind"] == "take"
                   else f"{g['id']}" if g["kind"] == "pickup" else g["kind"])
            lines.append(f"#{idx:<3} {int(t // 60):>3}:{t % 60:04.1f}  {d:5.1f}s  {sec['section']:<5} {kind:<7} {src:<26} {first_words(g)}")
            t += d
            idx += 1
        if sec["section"] == "10":
            lines.append(f"      {int(t // 60):>3}:{t % 60:04.1f}   15.0s  10c   credits roll (footage under credits music)")
            t += 15
    lines.append(f"TOTAL {int(t // 60)}:{t % 60:04.1f}")
    (HERE / "build" / "cut_sheet.txt").write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote build/cut_sheet.txt ({idx - 1} segments, {t / 60:.1f} min)")


if __name__ == "__main__":
    main()
