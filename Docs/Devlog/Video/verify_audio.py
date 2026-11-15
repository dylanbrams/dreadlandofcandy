"""Check every voice piece ends where its sentence ends (no clipped last word), by transcribing the finished piece.

    python -I verify_audio.py        # writes build/verify_audio.txt, prints the flagged pieces

For take segments the expected last word comes from the source transcript (render_rough's sentence-completion rule);
for pickups the first and last words are printed for a human check. A piece is flagged if its last transcribed words
do not include the expected last word (allowing Whisper's spelling noise).
"""
import json, re, sys
from difflib import SequenceMatcher
from pathlib import Path

HERE = Path(__file__).parent
SEG = HERE / "build" / "seg"
sys.path.insert(0, str(HERE))
import clean_takes as ct  # noqa: E402
from faster_whisper import WhisperModel  # noqa: E402


def norm(w):
    return re.sub(r"[^a-z0-9]", "", w.lower())


def close(a, b):
    return a == b or (len(a) > 2 and SequenceMatcher(None, a, b).ratio() >= 0.7)


def main():
    edit = json.loads((HERE / "edit.json").read_text(encoding="utf-8"))
    names = json.loads((SEG / "names.json").read_text())
    m = WhisperModel("small.en", device="cpu", compute_type="int8")
    lines, flagged, idx = [], [], 1
    for sec in edit:
        for g in sec["segments"]:
            if g["kind"] == "pickup" and not g.get("recorded"):
                continue
            nm = names.get(str(idx))
            w = SEG / f"{nm}.wav"
            if g["kind"] != "blooper" and nm and w.exists():
                segs, _ = m.transcribe(str(w), word_timestamps=True, vad_filter=False, beam_size=5, condition_on_previous_text=False,
                                       initial_prompt="Um, so, uh, I, I mean, like, you know... Uh, okay.")
                got = [x.word.strip() for s in segs for x in (s.words or [])]
                tail = [norm(x) for x in got[-3:]]
                head = " ".join(got[:4])
                exp = None
                if g["kind"] == "take":
                    src = HERE / g["src"]
                    words = json.loads(src.with_suffix(".words.json").read_text(encoding="utf-8"))["words"]
                    sel = ct.select_words(words, g["i"], g["o"])
                    real = [i for i in sel if norm(words[i]["w"]) not in ct.FILLERS] or sel
                    exp = norm(words[real[-1]]["w"])
                ok = exp is None or any(close(exp, t) for t in tail)
                label = g.get("take") or g.get("id")
                lines.append(f"#{idx:<3} {sec['section']:<4}{label:<22}{'OK   ' if ok else 'CHECK'} expected end '{exp}' | starts: {head} | ends: {' '.join(got[-5:])}")
                if not ok:
                    flagged.append(idx)
            idx += 1
    (HERE / "build" / "verify_audio.txt").write_text("\n".join(lines), encoding="utf-8")
    print(f"{len(lines)} voice pieces checked; flagged: {flagged or 'none'}")


if __name__ == "__main__":
    main()
