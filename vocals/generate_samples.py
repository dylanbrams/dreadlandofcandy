"""Small, explicitly bounded ElevenLabs smoke test. No automatic paid retries."""
import argparse
import hashlib
import json
import subprocess
import urllib.error
import urllib.request
from datetime import datetime, timezone
from pathlib import Path

from elevenlabs_config import load_credential
from preflight import get

ROOT = Path(__file__).resolve().parent
OUT = ROOT / 'output' / 'samples'


def save_manifest(manifest):
    temporary = OUT / 'manifest.json.part'
    temporary.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    temporary.replace(OUT / 'manifest.json')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--generate', action='store_true')
    parser.add_argument('--voice-id')
    args = parser.parse_args()
    key, source = load_credential()
    if not args.generate:
        result = get('/v2/voices?page_size=100', key)
        if 'voices' not in result:
            raise SystemExit(json.dumps(result))
        print(json.dumps([{'id': v['voice_id'], 'name': v['name'], 'labels': v.get('labels', {})}
                          for v in result['voices']]))
        return
    if not args.voice_id:
        parser.error('--voice-id is required to generate')
    subscription = get('/v1/user/subscription', key)
    if subscription.get('character_limit', 0) - subscription.get('character_count', 0) < 1000:
        raise SystemExit('Could not verify at least 1000 included credits; stopped')
    jobs = []
    for locale, text, back in (
        ('en', 'ABSOLUTELY NOT!', 'Absolutely not!'),
        ('de', 'Auf gar keinen Fall!', 'Under no circumstances!'),
        ('ja', '絶対にダメだ！', 'Absolutely not / absolutely forbidden!'),
    ):
        jobs.append({'id': f'VO_K02_{locale}_epic', 'locale': locale,
                     'english_source': 'ABSOLUTELY NOT!', 'back_translation': back,
                     'delivery': 'Firm theatrical refusal; provisional knight casting',
                     'review': 'listening_pending' if locale == 'en' else 'fluent_review_pending',
                     'endpoint': f'/v1/text-to-speech/{args.voice_id}',
                     'payload': {'text': text, 'model_id': 'eleven_multilingual_v2',
                                 'voice_settings': {'stability': 0.35, 'similarity_boost': 0.75,
                                                    'style': 0.65, 'use_speaker_boost': True}}})
    jobs.append({'id': 'SFX_Lecture_Impact_v01', 'review': 'listening_pending',
                 'endpoint': '/v1/sound-generation',
                 'payload': {'text': 'Ridiculously over-the-top theatrical fantasy game sound effect. '
                             'Treat this tiny domestic action as a world-ending event, with total dramatic sincerity. '
                             'One thunderous blast of parental disapproval: compressed air cannon, '
                             'colossal book slammed onto an oak desk, a short metallic shockwave. '
                             'A physical WHUMP with no spoken voice. Huge perceived scale, crisp transient, '
                             'clean isolated sound, clean ending, no music, no background, no clipping.',
                             'model_id': 'eleven_text_to_sound_v2', 'duration_seconds': 0.6,
                             'prompt_influence': 0.5}})
    OUT.mkdir(parents=True, exist_ok=True)
    manifest_path = OUT / 'manifest.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8')) if manifest_path.exists() else {
        'provider': 'ElevenLabs', 'credential_source': source,
        'subscription_before': subscription['character_count'], 'clips': {}}
    for job in jobs:
        clip_id = job['id']
        fingerprint = hashlib.sha256(json.dumps(job, sort_keys=True).encode()).hexdigest()
        previous = manifest['clips'].get(clip_id)
        original = OUT / f'{clip_id}.mp3'
        wav = OUT / f'{clip_id}.wav'
        if previous:
            if previous['hash'] != fingerprint:
                raise SystemExit(f'{clip_id}: settings changed; inspect existing sample before spending')
            if previous['status'] == 'complete' and original.exists() and wav.exists():
                print(f'{clip_id}: already complete', flush=True)
                continue
            if previous['status'] != 'downloaded' or not original.exists():
                raise SystemExit(f'{clip_id}: previous request unresolved; inspect provider history before retrying')
        else:
            record = {**job, 'hash': fingerprint, 'status': 'request_started',
                      'date': datetime.now(timezone.utc).isoformat()}
            manifest['clips'][clip_id] = record
            save_manifest(manifest)
            request = urllib.request.Request('https://api.elevenlabs.io' + job['endpoint'] + '?output_format=mp3_44100_128',
                       data=json.dumps(job['payload'], ensure_ascii=False).encode('utf-8'),
                       headers={'xi-api-key': key, 'Content-Type': 'application/json'}, method='POST')
            try:
                with urllib.request.urlopen(request, timeout=120) as response:
                    record['request_id'] = response.headers.get('request-id') or response.headers.get('x-request-id')
                    record['character_cost'] = response.headers.get('character-cost')
                    data = response.read()
                if not data:
                    raise ValueError('empty audio')
                part = original.with_suffix('.mp3.part')
                part.write_bytes(data)
                part.replace(original)
                record['status'] = 'downloaded'
                save_manifest(manifest)
            except (urllib.error.URLError, OSError, ValueError) as error:
                record['status'] = 'failed_or_ambiguous'
                record['http_status'] = getattr(error, 'code', None)
                save_manifest(manifest)
                raise SystemExit(f'{clip_id}: generation stopped; HTTP {record["http_status"]}; inspect history before retry') from None
        record = manifest['clips'][clip_id]
        subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i', str(original), '-ac', '1',
                        '-ar', '48000', '-c:a', 'pcm_s16le', str(wav)], check=True)
        import soundfile as sf
        import numpy as np
        samples, rate = sf.read(wav)
        peak = float(np.max(np.abs(samples)))
        if not len(samples) or not peak:
            raise SystemExit(f'{clip_id}: empty or silent decoded audio')
        record.update(status='complete', original=str(original.relative_to(ROOT)),
                      wav=str(wav.relative_to(ROOT)), duration_seconds=len(samples) / rate,
                      sample_rate=rate, channels=1, peak_dbfs=float(20 * np.log10(peak)),
                      clipped_samples=int(np.count_nonzero(np.abs(samples) >= 32767 / 32768)))
        save_manifest(manifest)
        print(json.dumps({'id': clip_id, 'duration': record['duration_seconds'],
                          'peak_dbfs': record['peak_dbfs'], 'clipped_samples': record['clipped_samples']}), flush=True)
    after = get('/v1/user/subscription', key)
    manifest['subscription_after'] = {k: after[k] for k in ('character_count', 'character_limit', 'error', 'error_http_status') if k in after}
    save_manifest(manifest)
    print(json.dumps({'subscription_after': manifest['subscription_after']}))


if __name__ == '__main__':
    main()
