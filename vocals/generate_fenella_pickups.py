"""Sequential, resumable ElevenLabs renderer for the 2026-11-14 Fenella batch."""
import hashlib
import json
import subprocess
import shutil
import sys
import time
import urllib.error
import urllib.request
from datetime import datetime, timezone
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))

import numpy as np
import soundfile as sf

from elevenlabs_config import load_credential

ROOT = Path(__file__).resolve().parent
OUT = ROOT / 'output'
BATCH = OUT / 'fenella_pickups_2026-11-14'
MANIFEST = BATCH / 'manifest.json'
INPUT = ROOT / 'fenella_new_lines_2026-11-14.json'
VOICE_ID = 'hpp4J3VqNfWAUOO0d1Us'
VOICE_NAME = 'Bella'
MODEL = 'eleven_multilingual_v2'
SETTINGS = {'stability': 0.4, 'similarity_boost': 0.75, 'style': 0.45, 'use_speaker_boost': True}
API = 'https://api.elevenlabs.io'


def request(method, path, key, payload=None, output_format=None):
    url = API + path + (('?output_format=' + output_format) if output_format else '')
    data = json.dumps(payload, ensure_ascii=False).encode('utf-8') if payload is not None else None
    headers = {'xi-api-key': key}
    if data is not None:
        headers['Content-Type'] = 'application/json'
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    with urllib.request.urlopen(req, timeout=120) as response:
        return response.read(), response.headers


def save(manifest):
    temp = MANIFEST.with_suffix('.json.part')
    temp.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    temp.replace(MANIFEST)


def main():
    rows = json.loads(INPUT.read_text(encoding='utf-8'))
    if len(rows) != 25 or any(r.get('speaker') != 'fenella' for r in rows):
        raise SystemExit('Input batch validation failed; no audio requested')
    required = sum(len(r['text']) for r in rows)
    key, credential_source = load_credential()
    try:
        raw, _ = request('GET', '/v1/user/subscription', key)
        subscription = json.loads(raw)
    except urllib.error.HTTPError as exc:
        raise SystemExit(f'Subscription check HTTP {exc.code}; no audio requested') from None
    except (urllib.error.URLError, TimeoutError, OSError, ValueError):
        raise SystemExit('Subscription check failed; no audio requested') from None
    safe_subscription = {k: subscription.get(k) for k in ('tier', 'status', 'character_count', 'character_limit')}
    if not all(isinstance(subscription.get(k), int) for k in ('character_count', 'character_limit')):
        raise SystemExit('Subscription quota unavailable; no audio requested')
    remaining = max(0, subscription['character_limit'] - subscription['character_count'])
    if remaining < required:
        raise SystemExit(f'Insufficient included quota: need {required}; remaining {remaining}. No audio requested')
    BATCH.mkdir(parents=True, exist_ok=True)
    manifest = json.loads(MANIFEST.read_text(encoding='utf-8')) if MANIFEST.exists() else {
        'provider': 'ElevenLabs', 'credential_source': credential_source, 'voice_id': VOICE_ID,
        'voice_name': VOICE_NAME, 'model_id': MODEL, 'settings': SETTINGS,
        'batch': INPUT.name, 'subscription_before': safe_subscription, 'required_characters': required,
        'clips': {}
    }
    clips = manifest.setdefault('clips', {})
    for index, row in enumerate(rows, 1):
        ident, text = row['id'], row['text']
        payload = {'text': text, 'model_id': MODEL, 'voice_settings': SETTINGS}
        fingerprint = hashlib.sha256(json.dumps({'text': text, 'voice': VOICE_ID, 'payload': payload}, sort_keys=True, ensure_ascii=False).encode()).hexdigest()
        mp3 = OUT / 'fenella' / f'VO_{ident}.mp3'
        wav = OUT / 'fenella' / f'VO_{ident}.wav'
        prior = clips.get(ident)
        if prior and prior.get('hash') != fingerprint:
            raise SystemExit(f'{ident}: content fingerprint changed; stopped for review')
        if prior and prior.get('status') == 'complete' and mp3.exists() and wav.exists():
            print(f'{index}/25 {ident}: already complete', flush=True)
            continue
        if (mp3.exists() or wav.exists()) and not (prior and prior.get('status') == 'downloaded' and mp3.exists() and not wav.exists()):
            raise SystemExit(f'{ident}: existing take without matching completed manifest; preserved and stopped')
        if prior and prior.get('status') not in {'downloaded'}:
            raise SystemExit(f'{ident}: prior paid request unresolved ({prior.get("status")}); inspect history, no retry')
        rec = prior or {'id': ident, 'speaker': 'fenella', 'text': text, 'voice_id': VOICE_ID,
                        'voice_name': VOICE_NAME, 'model_id': MODEL, 'settings': SETTINGS,
                        'source': row.get('source'), 'runtime_integrated': row.get('runtime_integrated'),
                        'hash': fingerprint, 'status': 'request_started', 'review': 'listening_pending',
                        'started_utc': datetime.now(timezone.utc).isoformat()}
        if prior is None:
            clips[ident] = rec
            save(manifest)
        if rec['status'] != 'downloaded':
            # Persist intent before network activity; ambiguous outcomes are never retried automatically.
            rec['status'] = 'request_started'
            save(manifest)
            data = None
            for attempt in range(3):
                try:
                    data, headers = request('POST', f'/v1/text-to-speech/{VOICE_ID}', key, payload, 'mp3_44100_128')
                    request_id = headers.get('request-id')
                    rec['request_id'] = request_id
                    rec['character_cost'] = headers.get('character-cost')
                    break
                except urllib.error.HTTPError as exc:
                    if exc.code == 429 and attempt < 2:
                        retry_after = exc.headers.get('Retry-After', '2')
                        try:
                            delay = max(1, int(float(retry_after)))
                        except ValueError:
                            delay = 2
                        if delay > 60:
                            rec['status'] = 'rate_limited'
                            save(manifest)
                            raise SystemExit(f'{ident}: Retry-After {delay}s; stopped') from None
                        time.sleep(delay)
                        continue
                    rec['status'] = 'api_error'
                    rec['http_status'] = exc.code
                    save(manifest)
                    raise SystemExit(f'{ident}: API HTTP {exc.code}; stopped') from None
                except (urllib.error.URLError, TimeoutError, OSError):
                    rec['status'] = 'failed_or_ambiguous'
                    save(manifest)
                    raise SystemExit(f'{ident}: request outcome ambiguous; inspect provider history, stopped') from None
            if data is None:
                rec['status'] = 'api_error'
                rec['http_status'] = 429
                save(manifest)
                raise SystemExit(f'{ident}: repeated HTTP 429; stopped after bounded retries')
            if not data:
                rec['status'] = 'failed_or_ambiguous'
                save(manifest)
                raise SystemExit(f'{ident}: empty response; stopped for review')
            mp3.parent.mkdir(parents=True, exist_ok=True)
            part = mp3.with_suffix('.mp3.part')
            part.write_bytes(data)
            part.replace(mp3)
            rec['status'] = 'downloaded'
            rec['mp3'] = str(mp3.relative_to(ROOT))
            save(manifest)
        # A prior downloaded MP3 is converted locally without making another paid request.
        wav_part = wav.with_suffix('.wav.part')
        ffmpeg = shutil.which('ffmpeg') or str(ROOT / 'ffmpeg.exe')
        subprocess.run([ffmpeg, '-v', 'error', '-y', '-i', str(mp3), '-ac', '1', '-ar', '48000', '-c:a', 'pcm_s16le', '-f', 'wav', str(wav_part)], check=True)
        samples, rate = sf.read(wav_part, dtype='int16')
        peak = int(np.max(np.abs(samples.astype(np.int32)))) if samples.size else 0
        if rate != 48000 or samples.ndim != 1 or peak == 0:
            raise SystemExit(f'{ident}: WAV technical validation failed')
        wav_part.replace(wav)
        rec.update(status='complete', mp3=str(mp3.relative_to(ROOT)), wav=str(wav.relative_to(ROOT)),
                   duration_seconds=len(samples) / rate, sample_rate=rate, channels=1,
                   peak_dbfs=float(20 * np.log10(peak / 32768)),
                   clipped_samples=int(np.count_nonzero(np.abs(samples.astype(np.int32)) >= 32767)),
                   review='listening_pending', completed_utc=datetime.now(timezone.utc).isoformat())
        save(manifest)
        print(f'{index}/25 {ident}: complete', flush=True)
    try:
        raw, _ = request('GET', '/v1/user/subscription', key)
        final = json.loads(raw)
        manifest['subscription_after'] = {k: final.get(k) for k in ('tier', 'status', 'character_count', 'character_limit')}
    except (urllib.error.HTTPError, urllib.error.URLError, TimeoutError, OSError, ValueError):
        manifest['subscription_after'] = 'unavailable after generation'
    manifest['completed_count'] = sum(c.get('status') == 'complete' for c in clips.values())
    save(manifest)
    print(json.dumps({'batch_rows': len(rows), 'complete': manifest['completed_count'],
                      'characters_requested': required, 'subscription_before': safe_subscription,
                      'subscription_after': manifest['subscription_after']}))


if __name__ == '__main__':
    main()
