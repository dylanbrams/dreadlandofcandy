"""Check that every piece's picture and audio are the same length, and that the joined video matches the joined audio.

    python -I check_sync.py

Per-piece differences accumulate along the timeline (a 60 ms mismatch in 40 pieces is 2.4 s of drift), so this prints the
cumulative drift and the first pieces that differ by more than one frame. Expected output: 0 mismatched pieces, drift 0.00 s.
"""
import json, subprocess, wave
from pathlib import Path

HERE = Path(__file__).parent
SEG = HERE / "build" / "seg"


def nframes(p):
    r = subprocess.run(["ffprobe", "-v", "error", "-select_streams", "v:0", "-count_frames", "-show_entries",
                        "stream=nb_read_frames", "-of", "csv=p=0", str(p)], capture_output=True, text=True)
    return int(r.stdout.strip().split(",")[0])


def wav(p):
    with wave.open(str(p)) as w:
        return w.getnframes() / w.getframerate()


def dur(p, stream):
    r = subprocess.run(["ffprobe", "-v", "error", "-select_streams", stream, "-show_entries", "stream=duration", "-of", "csv=p=0", str(p)],
                       capture_output=True, text=True)
    return float(r.stdout.strip().split(",")[0])


def main():
    names = json.loads((SEG / "names.json").read_text())
    edit = json.loads((HERE / "edit.json").read_text(encoding="utf-8"))
    parts, idx = [("cold open", SEG / "000.mp4", SEG / "000.wav")], 1
    for s in edit:
        for g in s["segments"]:
            if g["kind"] == "pickup" and not g.get("recorded"):
                continue
            if s["section"] != "00":
                n = names[str(idx)]
                parts.append((f"#{idx} {s['section']}", SEG / f"{n}.mp4", SEG / f"{n}.wav"))
            idx += 1
        if s["section"] == "10":
            parts.append(("credits", SEG / "credits.mp4", SEG / "credits.wav"))
    bad, drift, t = [], 0.0, 0.0
    for label, v, a in parts:
        fv, fa = nframes(v) / 30.0, wav(a)
        d = fv - fa
        drift += d
        if abs(d) > 1 / 30 + 1e-4:
            bad.append((label, t / 60, fv, fa, d))
        t += fa
    print(f"{len(parts)} pieces, {t / 60:.2f} min | pieces off by more than a frame: {len(bad)} | cumulative drift {drift:+.3f} s")
    for b in bad[:8]:
        print("  %-12s at %5.2f min  picture %.3fs  audio %.3fs  diff %+.3fs" % b)
    final = HERE / "build" / "devlog_rough_v1.mp4"
    if final.exists():
        print(f"final file: video stream {dur(final, 'v:0'):.2f} s, audio stream {dur(final, 'a:0'):.2f} s "
              f"(difference {dur(final, 'v:0') - dur(final, 'a:0'):+.2f} s)")


if __name__ == "__main__":
    main()
