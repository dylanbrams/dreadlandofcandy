# Devlog video pipeline

The picture is fitted to your voice, not the other way round. You record; Claude transcribes each take, places every cut, typing beat and slide change on your word timings, and only then renders.

## 1. Drop your recordings in `raw/`

One file per script section, named by its section number: `00.wav` (cold open voice), `01.mp4`, `02.mp4` … `08b.mp4`, `09.mp4`, `10.mp4`, `11.mp4`. Green-screen video with its audio is ideal; for the cold open (section 0) audio alone is enough. Retakes: `03_b.mp4`, and say which one to use. Gameplay captures go in `raw/broll/` with descriptive names (`river-crossing.mp4`, `knight-placards.mp4`). `raw/` is git-ignored.

## 2. What happens next

1. `python -I transcribe.py` writes `raw/<name>.words.json` with word-level timings (faster-whisper small.en, CPU).
2. Claude reads the transcripts against `../Tomorrow/SCRIPT.md` and writes `edit.json`: for each sentence, CAM, PIP or B-ROLL and which visual, cut on sentence starts, following the picture plan in the script.
3. The cold open is rendered from `../Tomorrow/coldopen.html`, with each beat's Space press timed to land on your spoken prompt (`render_page.mjs --keys`). Its keystrokes come out as events, which become a click track under your voice.
4. Slides, the Gantt chart and screenshots become B-roll clips with slow pans; the music beds from the script table are laid under, ducked beneath your voice.
5. A rough cut goes to `build/`. You watch it and give notes, and Claude adjusts `edit.json` and re-renders only what changed.

## Tools here

- `render_page.mjs`: renders any HTML page to MP4 in deterministic virtual time with headless Edge, no npm packages. `node render_page.mjs <url> <out.mp4> <seconds> [--fps 30] [--keys "1.5:Space,4:Space"]`. About 7 frames a second.
- `transcribe.py`: word timings for takes.

Nothing long has been rendered yet: everything waits for your vocals.
