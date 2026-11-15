"""Trailer announcer lines via ElevenLabs.
  python -I generate_announcer.py audition          one test line in each candidate voice
  python -I generate_announcer.py lines <voice_id>  every line in LINES with one voice
Writes output/announcer/<name>.mp3 and skips files that already exist (no repeat spend)."""
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from elevenlabs_config import load_credential
from generate_fenella_pickups import request

OUT = Path(__file__).parent / 'output' / 'announcer'
MODEL = 'eleven_multilingual_v2'
SETTINGS = {'stability': 0.45, 'similarity_boost': 0.8, 'style': 0.35, 'use_speaker_boost': True}
CHOSEN = 'laTmoAT4stwoPzthxjZT'  # David - The Movie Trailer (Dylan's pick, Nov 15)
CANDIDATES = {
    'david_trailer': 'laTmoAT4stwoPzthxjZT',
    'don_trailer': 'JJCR1UICgHnHljtvu5uF',
    'desmond_trailer': '2cW6cjkWenOhW7QrnJbJ',
    'brian_premade': 'nPczCjzI2devNBz1zQrb',
}
AUDITION = 'In a land of unspeakable sweetness... one hero stands alone.'
LINES = {
    'A01_land': 'In a land of unspeakable sweetness...',
    'A02_hero': '...one hero stands alone.',
    'A03_armed': 'Armed with nothing but a sword... and a very stern lecture.',
    'A04_nothingdies': 'This time... nothing dies.',
    'A05_manners': 'They will learn manners.',
    'A06_vegetables': 'They will eat their vegetables.',
    'A07_openworld': 'An open world. Of chocolate.',
    'A08_quests': 'Quests that rewrite themselves.',
    'A09_title': 'The Dread Land of Candy.',
    'A10_fight': 'And fight for Truth, Toothbrushes, Vegetables, and Table Manners.',  # Dylan, Nov 15
}


def tts(key, voice, text, path):
    if path.exists():
        print('skip', path.name)
        return
    data, _ = request('POST', f'/v1/text-to-speech/{voice}', key,
                      {'text': text, 'model_id': MODEL, 'voice_settings': SETTINGS}, 'mp3_44100_128')
    path.write_bytes(data)
    print('wrote', path.name, len(text), 'chars')


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    key, _ = load_credential()
    if sys.argv[1] == 'audition':
        for name, voice in CANDIDATES.items():
            try:
                tts(key, voice, AUDITION, OUT / f'audition_{name}.mp3')
            except Exception as exc:  # a shared voice may need adding to the library first
                print('failed', name, type(exc).__name__, getattr(exc, 'code', ''))
    else:
        voice = sys.argv[2]
        for name, text in LINES.items():
            tts(key, voice, text, OUT / f'{name}.mp3')
    (OUT / 'lines.json').write_text(json.dumps({'model': MODEL, 'settings': SETTINGS, 'lines': LINES}, indent=1), encoding='utf-8')


if __name__ == '__main__':
    main()
