"""Check ElevenLabs access without printing credentials or spending TTS credits."""
import json
import re
import urllib.error
import urllib.request
from pathlib import Path
from elevenlabs_config import load_credential

ROOT = Path(__file__).resolve().parent


def api_key():
    return load_credential()[0]


def get(path, key):
    request = urllib.request.Request('https://api.elevenlabs.io' + path,
                                     headers={'xi-api-key': key})
    try:
        with urllib.request.urlopen(request, timeout=25) as response:
            return json.load(response)
    except urllib.error.HTTPError as error:
        result = {'error_http_status': error.code}
        # Provider messages can contain sensitive context. Only expose known enums.
        try:
            detail = json.load(error).get('detail', {})
            status = detail.get('status') if isinstance(detail, dict) else None
            if status in {'invalid_api_key', 'missing_permissions', 'quota_exceeded',
                          'subscription_required', 'insufficient_credits'}:
                result['error'] = status
        except (ValueError, AttributeError, OSError):
            pass
        return result
    except (urllib.error.URLError, TimeoutError, OSError):
        return {'error': 'Network connection failed'}
    except ValueError:
        return {'error': 'Invalid JSON response'}


def inventory():
    source = (ROOT.parent / 'Source/MegaJam/Private/Film/JamLines.cpp').read_text(encoding='utf-8-sig')
    pools = source.split('TArray<FPool> BuildPools()', 1)[1].split('const TCHAR* const GNames', 1)[0]
    rows = []
    for match in re.finditer(r'EJamLineTrigger::(\w+),\s*\{(.*?)\}\s*\}', pools, re.S):
        trigger, body = match.groups()
        for number, literal in enumerate(re.findall(r'TEXT\("((?:\\.|[^"\\])*)"\)', body), 1):
            text = json.loads('"' + literal + '"')
            speaker = 'fenella' if trigger not in {'BubbleDefault', 'BubbleSadder', 'BubbleBudget', 'NuisanceReply', 'ProductionNote'} else 'unassigned'
            rows.append({'id': f'{trigger}_{number:02}', 'text': text, 'speaker': speaker,
                         'source': 'JamLines.cpp', 'asset_name': f'VO_{trigger}_{number:02}'})
    for line in (ROOT / 'VoiceLines.md').read_text(encoding='utf-8-sig').splitlines():
        match = re.match(r'- ([A-Z]+_\d+): (.*)', line)
        if match:
            line_id, text = match.groups()
            rows.append({'id': line_id, 'text': text, 'speaker': line_id.split('_')[0].lower(),
                         'source': 'VoiceLines.md', 'asset_name': f'VO_{line_id}',
                         'status': 'hold_silent_hero' if line_id.startswith('HERO_') else 'candidate'})
    (ROOT / 'line_inventory.json').write_text(json.dumps(rows, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    fenella = [r for r in rows if r['speaker'] == 'fenella']
    candidates = [r for r in rows if r['source'] == 'VoiceLines.md' and r['status'] == 'candidate']
    return {'fenella_lines': len(fenella), 'fenella_characters': sum(len(r['text']) for r in fenella),
            'other_candidate_lines': len(candidates), 'other_candidate_characters': sum(len(r['text']) for r in candidates),
            'held_hero_lines': sum(r.get('status') == 'hold_silent_hero' for r in rows)}


if __name__ == '__main__':
    print(json.dumps({'inventory': inventory()}))
    key, credential_source = load_credential()
    print(json.dumps({'credential_source': credential_source}))
    subscription = get('/v1/user/subscription', key)
    print(json.dumps({'subscription': {k: subscription[k] for k in
          ('tier', 'status', 'character_count', 'character_limit', 'can_extend_character_limit', 'error_http_status', 'error') if k in subscription}}))
    if all(isinstance(subscription.get(k), int) for k in ('character_limit', 'character_count')):
        print(json.dumps({'remaining_included_credits': max(0, subscription['character_limit'] - subscription['character_count'])}))
    voices = get('/v2/voices?page_size=100', key)
    print(json.dumps({'voices_available': len(voices['voices']) if 'voices' in voices else None,
                      'has_more': voices.get('has_more'),
                      'error_http_status': voices.get('error_http_status'), 'error': voices.get('error')}))
    models = get('/v1/models', key)
    if isinstance(models, list):
        print(json.dumps({'tts_models': [{'id': m['model_id'], 'language_count': len(m.get('languages', []))}
                                         for m in models if m.get('can_do_text_to_speech')]}))
    else:
        print(json.dumps({'models': models}))
    checks_ok = (not any(k in subscription for k in ('error', 'error_http_status'))
                 and 'voices' in voices and isinstance(models, list))
    print(json.dumps({'read_only_checks_passed': checks_ok,
                      'tts_generation_permission_verified': False,
                      'audio_generated': False}))
    raise SystemExit(0 if checks_ok else 1)
