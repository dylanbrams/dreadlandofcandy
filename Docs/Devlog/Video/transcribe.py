"""Word-level timings for Dylan's takes, so the edit can be fitted to the voice.

    python -I transcribe.py                 # every audio/video file in raw/ without a .words.json yet
    python -I transcribe.py raw/08b.mp4     # one file

Writes raw/<name>.words.json: {"duration", "words": [{"w", "s", "e"}], "segments": [{"text", "s", "e"}]}.
Uses faster-whisper small.en on the CPU (int8).
"""
import json, sys
from pathlib import Path
from faster_whisper import WhisperModel

HERE = Path(__file__).parent
EXTS = {".wav", ".mp3", ".m4a", ".flac", ".mp4", ".mov", ".mkv", ".webm"}

def model():
    # CPU: the GPU path crashes on this machine (cudnn_ops64_9.dll is not installed).
    return WhisperModel("small.en", device="cpu", compute_type="int8")

def run(m, path):
    segs, info = m.transcribe(str(path), word_timestamps=True, vad_filter=True, beam_size=5)
    out = {"file": path.name, "duration": info.duration, "words": [], "segments": []}
    for s in segs:
        out["segments"].append({"text": s.text.strip(), "s": round(s.start, 3), "e": round(s.end, 3)})
        for w in s.words or []:
            out["words"].append({"w": w.word.strip(), "s": round(w.start, 3), "e": round(w.end, 3)})
    dest = path.with_suffix(".words.json")
    dest.write_text(json.dumps(out, indent=1), encoding="utf-8")
    print(f"{path.name}: {len(out['words'])} words, {info.duration:.1f}s -> {dest.name}")

if __name__ == "__main__":
    files = [Path(a) for a in sys.argv[1:]] or sorted(p for p in (HERE / "raw").iterdir()
             if p.suffix.lower() in EXTS and not p.with_suffix(".words.json").exists())
    if not files:
        print("nothing to transcribe in raw/"); sys.exit(0)
    m = model()
    for f in files:
        run(m, f)
