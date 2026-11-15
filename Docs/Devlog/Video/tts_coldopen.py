"""Voice the AI responses of the cold open (coldopen.html) with ElevenLabs.

    python -I tts_coldopen.py            # generate what is missing (bounded; no automatic retries)
    python -I tts_coldopen.py --list     # show the lines and which are done

Claude's lines use the voice below ("River"). ChatGPT's line waits for the voice Dylan picks: set CHATGPT_VOICE_ID.
The credential is loaded by vocals/elevenlabs_config.load_credential (read into memory, never printed).
Output: build/tts/<id>.wav (mono 48 kHz) and <id>.mp3; build/tts/manifest.json records characters and request ids.
"""
import json, subprocess, sys, urllib.error, urllib.request
from pathlib import Path

HERE = Path(__file__).parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / "vocals"))
from elevenlabs_config import load_credential   # noqa: E402

OUT = HERE / "build" / "tts"
CLAUDE_VOICE_ID = "SAz9YH"   # prefix only; resolved against the account's voice list below ("River")
CHATGPT_VOICE_ID = "CwhRBW"   # prefix; "Roger" (picked by ChatGPT via Dylan)

# id -> (speaker, text). Beat numbers are coldopen.html's beats (0-based): 0 and 3 are Claude, 1 is ChatGPT, 5-9 Claude.
LINES = {
    "co_b0_claude": ("claude", "That's completely unrealistic. Let's build something rational. Like a spreadsheet."),
    "co_b1_gpt": ("gpt", "That's completely unrealistic, maybe we can build a design doc for one in 4 days."),
    "co_b3_claude": ("claude", "Forty thousand tokens. Wonderful. How about one room and a goblin?"),
    "co_b5_claude": ("claude", "...Fine."),
    "co_b6_claude": ("claude", "Also fine."),
    "co_b7_claude": ("claude", "We cut those."),
    "co_b8_claude": ("claude", "Five villagers. Ten linked jobs. I'd like it noted that I said no."),
    "co_b9_claude": ("claude", "Of course there are marshmallows."),
}
SETTINGS = {"stability": 0.55, "similarity_boost": 0.75, "style": 0.2, "use_speaker_boost": True}  # dry, even


def api(key, method, path, payload=None):
    req = urllib.request.Request("https://api.elevenlabs.io" + path, method=method,
                                 data=json.dumps(payload, ensure_ascii=False).encode() if payload else None,
                                 headers={"xi-api-key": key, "Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=120) as r:
        return r.read(), r.headers


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    manifest_path = OUT / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8")) if manifest_path.exists() else {}
    if "--list" in sys.argv:
        for k, (who, text) in LINES.items():
            print(f"{k:<14}{who:<7}{'done' if (OUT / (k + '.wav')).exists() else '-':<5}{text}")
        return
    key, _ = load_credential()
    voices = json.loads(api(key, "GET", "/v2/voices?page_size=100")[0])["voices"]
    ids = {"claude": next(v["voice_id"] for v in voices if v["voice_id"].startswith(CLAUDE_VOICE_ID)),
           "gpt": next(v["voice_id"] for v in voices if v["voice_id"].startswith(CHATGPT_VOICE_ID))}
    for k, (who, text) in LINES.items():
        wav = OUT / f"{k}.wav"
        if wav.exists():
            continue
        if not ids[who]:
            print(f"{k}: no voice chosen for {who} yet; skipped")
            continue
        try:
            data, hdr = api(key, "POST", f"/v1/text-to-speech/{ids[who]}?output_format=mp3_44100_128",
                            {"text": text, "model_id": "eleven_multilingual_v2", "voice_settings": SETTINGS})
        except (urllib.error.URLError, OSError) as e:
            raise SystemExit(f"{k}: generation stopped (HTTP {getattr(e, 'code', '?')}); inspect before retrying") from None
        if not data:
            raise SystemExit(f"{k}: empty audio")
        (OUT / f"{k}.mp3").write_bytes(data)
        subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(OUT / f"{k}.mp3"), "-ac", "1", "-ar", "48000",
                        "-c:a", "pcm_s16le", str(wav)], check=True)
        manifest[k] = {"voice": who, "chars": len(text), "request_id": hdr.get("request-id"), "text": text}
        manifest_path.write_text(json.dumps(manifest, indent=1, ensure_ascii=False), encoding="utf-8")
        d = float(subprocess.run(["ffprobe", "-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", str(wav)],
                                 capture_output=True, text=True).stdout)
        print(f"{k}: {d:.2f}s")


if __name__ == "__main__":
    main()
