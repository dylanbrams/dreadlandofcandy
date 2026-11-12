# BACKEND hand-over

Updated 2026-11-12.

## B1: `POST /v1/director/note`, built and tested, not deployed yet

Code is in its own git repo at `backend/` (inside this checkout, not part of this repo's history). README and runbook: `backend/README.md`, `backend/DEPLOY.md`.

- FastAPI under Gunicorn on `127.0.0.1:8010`, behind the existing nginx at `/v1/director/`. Its own Unix user, Postgres role and database (`director`). OneMore's service, port and database are untouched.
- Contract `Docs/Design/NoteForm.md` v1, unchanged. The server clamps every field to its limits; the game still clamps again (T12a).
- Spend controls: cache first (prompt version, scene, stage, motif, normalised note); a daily model-call budget in Postgres (default 4,000 calls/day, about $1.30 worst case on Haiku 5.5's $0.10/$0.50 per MTok); per-session and per-address hourly limits; an input screen that refuses links, phone numbers, emoji-only notes and plainly off-limits terms without calling the model; model refusals cached.
- Model: `claude-haiku-5-5`, structured output, thinking off, effort low, 4.5 s timeout, no retries.
- Tests: 70 pytest tests, all passing, including 23 canned notes (13 hostile: knob abuse, prompt injection, SQL, markup, links, phone, self-harm, sexual, hateful, too long, blank). They run with a fake model and an in-memory store. The request shape was checked against a local stub of the Messages API.
- **Not yet done:** deploying, and the real-model run of the canned notes. Both need Dylan: SSH to the box, and the API key placed with `sudoedit` off stream. Steps: `backend/DEPLOY.md`, "First deploy". The done-when check is `python3 -I scripts/live_check.py https://<api host>`, which fails any answer over 5 s or outside the contract.

## For GAME (T12b)

- URL once deployed: `https://<OneMore's API host>/v1/director/note` (same host and certificate as OneMore, so packaged builds already trust it).
- Responses: 200 with `source` `model`, `cache`, `refused` (no form) or `budget` (no form); 422 bad request; 429 rate limited; 503 model failed. Everything except a 200 with a form is a silent fallback, as NoteForm.md says.
- `session_id` must be 8 to 64 characters; scene and stage ids 1 to 40 of `A-Z a-z 0-9 _ -`; `set_kinds` lower case.
- **Suggested-note buttons:** pre-warmed with `motif: ""`. For them to be guaranteed cache hits, send `motif: ""` when a button is pressed (carry-forward then applies to typed notes only), or tell BACKEND the motifs to warm as well.

## Requests

- **GAME:** add `backend/` to this repo's `.gitignore`, so the nested repo is never added by accident.
- **GAME or RULES:** confirm the scene and stage ids, set kinds and capacities at each CUT! prompt, and the three suggested notes per prompt. The draft is `backend/deploy/suggested_notes.json` (`Scene2`/`S2_BrushGoblins`, `Scene3`/`S3_Final`). Once they're confirmed, BACKEND re-runs the pre-warm.
- **DESK:** add Claude Haiku 5.5 (Anthropic API, typed-note service) to the tools-used list.

## Next

B1 deploy and live check (with Dylan), pre-warm, then B2 (`/v1/director/premiere`).
