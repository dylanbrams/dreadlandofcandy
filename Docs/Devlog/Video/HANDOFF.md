# Devlog video assembly: handoff (Nov 15, ~12:30)

You are assembling Dylan's devlog video for The Dread Land of Candy. You are a docs/video worker: edit only `Docs/Devlog/` (mostly `Docs/Devlog/Video/`). Don't open the Unreal editor or build the game; the GAME session owns those. The jam is streamed: never print credentials or config files with tokens. Commit your scripts and docs at green checkpoints; `raw/` and `build/` are git-ignored. Never push.

## Read first

1. `Docs/Devlog/Tomorrow/SCRIPT.md`: the script. Read the music table, then the **picture plan** (the CAM / PIP / B-ROLL table and its rules), then sections 0–11. Dylan's recordings follow it, loosely.
2. `Docs/Devlog/Video/README.md`: the voice-first pipeline.
3. `Docs/Devlog/Tomorrow/coldopen.html`: the cold-open prompt screen (section 0). Edit its `beats` list if Dylan's wording differs.

## Inputs

- **Dylan's takes:** recorded on a green screen. Ask Dylan where they are if `raw/` is empty, then copy (don't move) them into `raw/`, named by section (`00.wav`, `01.mp4` … `08b.mp4` … `11.mp4`). He recorded "most of the dialogue": list which sections are missing.
- **Music beds:** `music/*.mp3` (ACE Studio score), assigned per section in the script's table. **Never use `theme-song.mp3` or anything from `D:\AudacityRecordings\BigRockCandyMountain\` except the blooper below**: its guitar is the 1928 McClintock recording, still under US copyright.
- **Post-credits blooper:** `Docs/Devlog/Tomorrow/assets/blooper/BigRockCandyMountain_Blooper_8s.mp3`, the only theme-song audio allowed.
- **Graphics:**
  - `Docs/Devlog/Tomorrow/index.html` + `slides.js`: an HTML slide deck. Add a `?slide=N&rec` URL parameter so one slide can be rendered without its UI chrome.
  - `Docs/Devlog/Checkpoint1/gantt/gantt_slide.png`: the updated Gantt.
  - `Docs/Devlog/Tomorrow/assets/` (`morning/` before/after shots, the rat comparison), `Docs/Devlog/CandyPBR/contact.jpg`, `Docs/Devlog/Marshmallows/Marshmallow_contact_sheet.jpg`, `Docs/Design/CandyUI/*.png`, `Docs/Devlog/Screenshots/`, `Saved/Screenshots/WindowsEditor/`.
- **Voice clips from the game** (for Fenella beats): `vocals/output/fenella/<asset>.wav`, with their text in `vocals/line_inventory.json`.
- **Gameplay B-roll:** probably none recorded yet. If Dylan has none, cover those beats with stills and slides, and list the missing gameplay shots for him (the trailer's capture list in `Docs/Devlog/Trailer/TRAILER.md` overlaps). If you capture gameplay yourself, run the packaged build (`Builds/Win64-Development`), record only the game window (ffmpeg `gdigrab` with `title=`), never the desktop, and ask Dylan first.

## Tools (already verified on this machine)

- `ffmpeg` 9 with `chromakey`, `despill`, `overlay`, `zoompan`, `sidechaincompress`, `loudnorm`.
- `render_page.mjs`: HTML to MP4 in deterministic virtual time via headless Edge; no npm packages. About 7 fps to render. `--keys "t:Space,..."` presses keys at given times. Lines the page logs as `console.log('EVT …')` are saved to `<out>.events.json`: coldopen.html logs `EVT key` per typed character, to drive a click track.
- `transcribe.py`: faster-whisper small.en on the **CPU** (the GPU path crashes: cuDNN missing). Writes `raw/<name>.words.json` with word timings.
- Python 3.12 with numpy, cv2, matplotlib. **Package installs are blocked** (pip install was denied), so use what's here.

## Workflow

1. Transcribe everything in `raw/`. Summarize per section: duration, what he said vs the script, flubs and retakes, and give Dylan that summary.
2. Write `edit.json`: an ordered list of segments, each with a source take and in/out times (cut on sentence starts, drop flubs and dead air), a framing (CAM / PIP / B-ROLL), a visual (file, slide number, or rendered page), and the music bed. Follow the picture plan: no more than 6–8 s on CAM, then at least 10 s away. Reorganize where his takes run in a different order from the script.
3. **Fit visuals to the voice.** Render the cold open with `--keys` timed so each beat starts as he says that prompt; render slides and pans to the lengths of their segments. Render nothing long before the edit list exists.
4. Green screen: tune `chromakey` + `despill` on one frame per take, and show Dylan a keyed still before batch-processing. If the key is ragged, use straight CAM cuts, without PIP.
5. Audio: voice normalized (`loudnorm`, about −16 LUFS), beds at −18 to −22 dB, ducked under the voice with `sidechaincompress`, swelling in gaps. A click track from coldopen's events (synthesize short filtered noise bursts with numpy). No music under section 0 or the post-credits blooper.
6. Assemble the segments as 1920×1080, 30 fps intermediates, then concat and mix audio last. Output `build/devlog_rough_v1.mp4`, and extract a few frames to check it yourself before telling Dylan.
7. Iterate on Dylan's notes by editing `edit.json` and re-rendering only the changed segments.

## Constraints and judgement

- Usage matters to Dylan. Keep transcripts and logs out of your context: read summaries and use small scripts. Delegate long, mechanical runs (transcription, batch keying, renders) to background commands or haiku subagents that report back only a summary.
- The jam deadline is Nov 15, 5:59 PM PST (03:59 Nov 16 on this machine's +02:00 clock); the video isn't part of the submission. If the GAME session needs the machine for packaging, pause heavy renders.
- Don't invent facts in on-screen text. Captions and cards come from `SCRIPT.md`, `Docs/DEVLOG.md` or Dylan.
