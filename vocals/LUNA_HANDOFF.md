# Voice production handoff for Luna

User request: produce Fenella (director fairy), villagers and other cast, and attack battlecries in roughly 15 commercially important Steam languages. The user requested this handoff to conserve assistant usage. No audio has been generated in this session.

**Latest user direction: this is a game jam. Translate and generate now; fluent-speaker review happens retrospectively. Do not wait for translation approval, casting approval or a sample sign-off. Choose suitable voices, draft natural translations, do quick self-checks and proceed with the batch. The user authorizes the sample and full-batch speech generation needed for this task using available ElevenLabs credits. Keep uncertain translations marked for later review without blocking generation.**

## Start here

- **Access fix verified 2026-11-13:** `python preflight.py` now uses `elevenlabs_config.load_credential()`: local `.env.local` first (`ELEVENLABS_API_KEY` preferred), parent `.env` only if no nonempty local key exists. An API failure must stop the batch, never silently switch keys/accounts. Reuse this loader in the batch runner; no batch runner exists in this folder yet. Load named values into memory only; do not display either credential file.
- **Sample generation succeeded 2026-11-13:** `generate_samples.py` produced K02 in English, German and Japanese using Harry / `SOYHLrjzK2X1ezoPC6cr` with `eleven_multilingual_v2`, plus `SFX_Lecture_Impact_v01` with `eleven_text_to_sound_v2`. See `output/samples/manifest.json` for exact texts/settings, request IDs where returned, durations and technical checks. Both TTS and SFX access now confirmed. Four nonempty decodable originals plus mono 48 kHz 16-bit WAVs; zero clipped samples. Listening and fluent review remain pending. Response `character-cost` headers total 23; subscription counter still returned zero immediately afterward, so do not treat that counter as proof of no spending. This is only a four-clip smoke test, not the full batch runner or final casting.
- Read-only checks with the local key succeeded: active Starter subscription, 0 of 40,000 included credits consumed, overage unavailable, 21 voices, and models accessible. These are a snapshot; rerun before generation. No TTS request was made, so generation permission still needs confirmation by the first already-authorized sample. The older parent's 401 results do not describe the onboarded key.
- Work in `D:\Source\Unreal\MegaJam2026\vocals`. Read `..\CLAUDE.md`, `..\Docs\Design\WORKERS.md`, and relevant parts of `..\Docs\Design\BUILD_SPEC.md`. Work as AUDIO; leave game code, editor and imports to GAME.
- Existing credential: `ELEVENLABS_KEY` in `..\.env`. Never print credentials or put them in commands, source, manifests or logs. Load only the named key into memory. SDK convention is `ELEVENLABS_API_KEY`; map the existing name in memory if needed.
- User explicitly requested `npx -y @elevenlabs/cli@latest onboard init`. It succeeded after network escalation. `onboard connect --env-path .env.local` also SUCCEEDED and created a new project key. `onboard check --answered --env-path .env.local` completed and returned `next: write_code`, with the key sourced from that file. SDK is not installed/imported yet. Do not open `.env.local`; use the CLI to inspect status. Local `.gitignore` now ignores `.env` and `.env.*`.
- Next implementation step: install the appropriate SDK (`pip install elevenlabs` for Python, declare the dependency), load `ELEVENLABS_API_KEY` from local `.env.local` without printing it, and implement the offline batch workflow. The parent key is a fallback, not the newly onboarded credential. CLI also suggested reading official ElevenLabs skills before integration code: https://github.com/elevenlabs/skills . Download references to a temporary directory, not this project. No SDK or generation code was added because the user requested a handoff. The user's latest direction authorizes proceeding with speech generation; do not request another sample approval.
- `preflight.py` inventories lines and checks subscription, voices and models without generating audio. It prints the credential source filename and sanitized results, never the key. Failed voice checks report `null`, not zero; failed checks exit nonzero. Network access may require escalation in the sandbox.

## Sources and exact scope

1. **Fenella:** `..\Source\MegaJam\Private\Film\JamLines.cpp` is the actual runtime table. 88 Fenella lines, 4,416 text characters. Preserve text and IDs exactly. IDs are `<Trigger>_<two-digit index starting at 01>`. `IsFenella` includes SceneStart through Commentary. Cheerful, certain she is helping; a film producer directing an epic, never sneering at the player. Cast a clear, lively adult feminine voice with comic timing; avoid a squeaky novelty fairy voice.
2. **Villagers/opponents/audition heroes:** local `VoiceLines.md`. 117 non-knight candidates, 6,867 characters. Includes general villagers, farmer, Dental Marshal, almond herder, audition extras, rats, goblins, lollypop bugs and pacified replies. Preserve IDs. These are candidates, not runtime-integrated lines. Villagers sincerely mean what they say; enemies have directions in the file. Use a small reusable cast with distinct character assignments rather than a new voice for every line.
3. **Hold 19 HERO_* lines:** `Jokes.md` says the knight never speaks. They are explicitly conditional candidates. User's current attack request authorizes producing the newer attack cries, but does not authorize adding idle quips or changing game behavior. Do not silently record these 19 lines.
4. **Attacks:** `..\Docs\Design\The_Dread_Land_of_Candy_Attacks_and_Effects_Pipeline.md`, sections 2 and 8, is the newer locked roster. The older `The_Dread_Land_of_Candy_Attacks_and_VFX_Design.txt`, section VI, explains delivery. All four classes need distinct, consistent voices. Utterly serious epic delivery of mundane domestic commands; no mock accents.

`line_inventory.json` contains exact extracted runtime lines and local candidates, source, proposed filename and speaker/status. Unassigned bubble/production-note rows are NOT Fenella dialogue. Never read `[SHOUTING]`, `[LECTURE]` or `[BUBBLE GOES HERE]` aloud. Do not voice all rows indiscriminately.

| Attack | Locked English cry |
| --- | --- |
| K01 | EXCUSE ME! |
| K02 | ABSOLUTELY NOT! |
| K03 | MIND YOUR MANNERS! |
| B01 | BAKE THIS! |
| B02 | UNDECIDED: hold; do not invent a line |
| B03 | EAT YOUR VEGETABLES! |
| M01 | A SPOONFUL OF SUGAR! |
| M02 | ONE AFTER DINNER! |
| M03 | WASH YOUR HANDS! |
| A01 | INCOMING GREENERY! |
| A02 | THESE ARE RICH IN VITAMINS! |
| A03 | DID YOU FLOSS?! |

Also produce the B03 rare exhausted-parent performance: **Please. Just eat them.** Quiet, emotionally spent, domestic anticlimax. Not an epic whisper. Keep meaning the same in translation. B01's previous cry is retired. Optional M02 alternatives are lower priority.

## Language selection

Do not claim a verified top-15 Steam revenue ranking: one was not established. Steam client language share is not revenue share, and countries are not languages. Proposed practical shortlist (NOT ranked): English, German, French, Japanese, Korean, Mandarin Chinese (Simplified text), Brazilian Portuguese, Spanish, Italian, Polish, Russian, Turkish, Ukrainian, Czech, Thai. Record this assumption for the user; adjust if they provide actual sales data or a preferred list.

Use locale IDs `en`, `de`, `fr`, `ja`, `ko`, `zh-CN`, `pt-BR`, `es-ES`, `it`, `pl`, `ru`, `tr`, `uk`, `cs`, `th`. Provider language codes may differ (e.g. `zh`, `pt`, `es`). **Do not send `language_code` with `eleven_multilingual_v2`: the official convert API explicitly says this parameter is unsupported for that model.** It infers language from the translated text; short cries need listening checks for language drift. If explicit language enforcement is needed, choose a model supporting that parameter and verify its locale coverage before spending. Written Traditional Chinese is not an additional spoken language by itself.

Prepare a translation table before rendering: attack ID, locale, source English, translated line, literal back-translation, delivery, review status. Prefer natural parental commands retaining the exact meaning. Self-check meaning and obvious errors, then generate immediately. Mark translations `fluent_review_pending`; fluent-speaker review is retrospective and MUST NOT gate audio generation. Do not claim native QA based on back-translation alone. Do not spend on unsupported locales silently.

11 locked cries x 15 locales = **165 primary attack clips**. Rare B03 in all 15 would add 15 (or start with English and mark remaining 14 pending). Plus 88 Fenella and 117 cast candidates = **385 clips** if all candidates and all rare variants are selected, before alternate takes. B02 and HERO_* excluded. Fenella and other dialogue are English initially; only attack cries requested multilingual.

## Production procedure

1. Finish onboarding; verify access, available voices, models and remaining credits. Use current official docs. No automatic paid overage or plan changes. TTS is billed separately from assistant usage.
2. Choose suitable voices and record a casting sheet. Generate a small sample set for a quick technical/listening check, then continue directly into the bulk batch without waiting for user or fluent-speaker approval. Avoid cloning or imitating recognizable actors. Record voice IDs, model IDs, provider, settings, date and applicable license/attribution.
3. Prepare manifests and character estimates, then render selected rows. Prioritize frequent Fenella pools, 11 English cries, then multilingual cries, then cast candidates. Do not change authored jokes to reduce costs.
4. Use a resumable runner: stable IDs, text/settings hash, per-clip status and provider request ID; skip completed matching files, atomic writes, bounded retries for rate limits/network errors. Stop on auth/quota failures. A timed-out paid request may already have completed: inspect history before blindly regenerating. No API key in logs.
5. Generate separate clips, never a long recording split on guessed silence. Save provider originals and import-ready mono PCM WAV (e.g. 48 kHz, 16-bit). Confirm supported API output format and convert when required. No music, reverb tails or SFX baked into dialogue. Gentle trim/padding; avoid clipping and harsh normalization. Match perceived loudness across cast; cries can be theatrical without digital clipping.
6. Validate nonempty decodable audio, duration, sample rate/channels, clipping, text-to-file mapping and missing/duplicate IDs. Listen to samples per voice, language and delivery; flag pronunciation doubts. Provide a local review page or CSV with playback paths and approval status.
7. Keep offline generation separate from the shipped game. No runtime ElevenLabs calls or embedded keys. GAME handles Unreal import and playback.

## Delivery and integration

- Fenella WAVs: `output/fenella/VO_<LineId>.wav`. GAME imports as `/Game/Audio/VO/VO_<LineId>`; existing caption playback already supports that convention and the Fenella volume slider.
- Other cast: `output/cast/<speaker>/VO_<LineId>.wav`; runtime hooks remain GAME work.
- Attacks: `output/attacks/<locale>/VO_<AttackID>_<locale>_<variant>.wav`, with `epic` and `parent` variants as applicable. These names are a proposed handoff convention, not an existing engine contract.
- Manifest columns: line ID, attack ID if relevant, speaker, locale, exact spoken text, English source, voice/model/settings, output path, duration, generation/review status, provenance/license. No secrets.
- GAME needs locale/variant selection and voice concurrency for attacks; producing clips does not implement those systems. Choose one prerecorded language per cast. Basic attacks use cries occasionally, not every hit; preserve rare-parent timing and avoid overlapping voices.
- Leave summary and import requests locally in this folder. Changes to `Docs/Declared.md`, runtime source/cooking, and worker notes outside the writable folder must be handed to the owning session or require filesystem approval. Do not stage unrelated files or push.

Official references: https://elevenlabs.io/docs/api-reference/text-to-speech/convert ; https://elevenlabs.io/docs/api-reference/voices/search ; https://elevenlabs.io/docs/api-reference/user/subscription/get ; https://elevenlabs.io/docs/overview/models ; https://partner.steamgames.com/doc/store/localization/languages
