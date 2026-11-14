# Devlog checkpoint 1 (Nov 14, 10:00)

Start here, ChatGPT: this folder is the seed of the post-jam devlog. Claude wrote the Claude side; add yours and keep the same formats.

| File | What it is |
| --- | --- |
| `CHECKPOINT.md` | Snapshot of what exists, timeline, numbers, what went wrong |
| `OUTLINE.md` | Devlog / video outline, with `[ ]` gaps and where to find sources |
| `deck/TheDreadLandOfCandy_Devlog.pptx` | 21-slide starter deck (built by `deck/build_deck.py`) |
| `gantt/work_log.csv` | Gantt data: one row per piece of work |
| `gantt/chatgpt_candidates.csv` | Guessed ChatGPT/Codex rows from untrailered commits; `confidence=unconfirmed` |
| `gantt/gantt.png` / `gantt_slide.png` | Charts (full / compact for the deck) |
| `gantt/gantt_legend.md` | Id to task table, regenerated |

## Adding ChatGPT work to the Gantt

1. Add rows to `gantt/work_log.csv`: `agent` = `ChatGPT`, `start`/`end` as `YYYY-MM-DD HH:MM`, `lane` one of Design, Setup, Assets, Terrain, FX, Audio, Art, Rules, Backend, Game, Docs (new lanes are fine), `confidence` = `commit-times`, `devlog`, `approx` or `confirmed`. Use ids `G01`, `G02`... Move confirmed rows out of `chatgpt_candidates.csv`.
2. `cd gantt; python build_gantt.py; python build_gantt.py --slide` (needs `pip install matplotlib`). `--candidates` draws the unconfirmed rows hatched.
3. `cd ../deck; python build_deck.py` (needs `pip install python-pptx`) rebuilds the deck. It overwrites the .pptx, so either edit the `SLIDES` list in the script or edit the .pptx by hand and stop rebuilding.

## Attribution rule used

Claude rows come from `Co-Authored-By: Claude ...` trailers in `git log` plus DEVLOG entries. Commits with no trailer (29 of 142) are not assumed to be ChatGPT; `chatgpt_candidates.csv` marks them unconfirmed. Start times are guesses (the end of the previous commit or DEVLOG timestamps); only end times are real commit stamps. Nov 14 batches 0 to 2 are labelled GAME in DEVLOG but carry no Claude trailer.

## Recompute the numbers

```
git log --oneline | wc -l
git log --format='%ad|%(trailers:key=Co-Authored-By,valueonly)' --date=format:'%m-%d' | awk -F'|' '{c[$1" "($2~/Claude/?"claude":"other")]++} END{for(k in c)print k,c[k]}'
```

## Codex attribution review

The chart and deck include documented ChatGPT/Codex work and hatched candidates. Rebuild with `python build_gantt.py --candidates` and `python build_gantt.py --slide --candidates`, then rebuild the deck. See the review in CHECKPOINT.md for attribution disputes and the date-only placeholder bars. The existing Claude bars and the candidate estimates have not been independently verified.
