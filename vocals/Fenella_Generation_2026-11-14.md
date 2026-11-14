# Fenella voice generation report — 2026-11-14

Generated the scoped JSON batch sequentially with ElevenLabs. The 25 rows include five existing runtime introduction lines and 20 new pickup candidates. No other inventory was rendered.

## Results

- 25/25 MP3 originals and 25/25 mono 48 kHz PCM16 WAV conversions are present under `output/fenella/`.
- All 25 responses have request IDs. WAV validation found no missing files, empty files, wrong sample rates, or non-mono files.
- Total WAV duration: 140.760 seconds (2:20.760); individual clips range from 3.437 to 10.217 seconds.
- Manifest records peak level and clipping counts for every clip. Aggregate clipped sample count: 0.
- Every clip is marked `listening_pending`; technical validation does not establish voice, delivery, or pronunciation approval.
- Voice: Bella (`hpp4J3VqNfWAUOO0d1Us`); model: `eleven_multilingual_v2`; existing dialogue settings retained.

## Quota and spend evidence

The sanitized subscription read reported active Starter tier, 7,325 / 40,000 characters before generation. The post-run subscription read still reported 7,325 / 40,000. The request batch contains 2,279 source text characters. ElevenLabs returned per-request `character-cost` headers for all 25 clips; their sum is 911. This differs from the source text character count, and the subscription counter did not change during the immediate post-run read, so actual account billing/spend cannot be confirmed from these observations. No credit cost beyond the reported response headers is inferred.

## Files and limitations

- Runner: `generate_fenella_pickups.py`
- Separate resumable manifest: `output/fenella_pickups_2026-11-14/manifest.json`
- MP3 and WAV files: `output/fenella/VO_<LineId>.*`

The initial sandboxed connection could not reach the subscription endpoint and stopped before any paid request. The same runner completed after network escalation was approved. No provider errors were returned in the successful run. Listening review remains pending, and the manifest's observed subscription counter did not reflect the returned per-request costs at read time.
