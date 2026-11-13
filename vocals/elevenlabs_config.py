"""Shared credential loading for preflight and future offline batch tools.

Read only named values into memory; never print credential files or values.
Do not switch accounts after an authentication/permission failure.
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def load_credential(root=ROOT):
    """Prefer local onboarding; fall back only when no local key is present."""
    for path, names, label in (
        (root / '.env.local', ('ELEVENLABS_API_KEY', 'ELEVENLABS_KEY'), '.env.local'),
        (root.parent / '.env', ('ELEVENLABS_API_KEY', 'ELEVENLABS_KEY'), '../.env'),
    ):
        try:
            lines = path.read_text(encoding='utf-8-sig').splitlines()
        except FileNotFoundError:
            continue
        except OSError:
            raise SystemExit(f'Cannot read {label}; credential contents withheld') from None
        values = {}
        for line in lines:
            match = re.fullmatch(r'\s*(?:export\s+)?(ELEVENLABS_API_KEY|ELEVENLABS_KEY)\s*=\s*(.*)', line)
            if not match:
                continue
            value = match[2].strip()
            if value.startswith(('"', "'")):
                quote = value[0]
                end = value.find(quote, 1)
                if end < 0:
                    raise SystemExit(f'Malformed named credential in {label}; contents withheld')
                value = value[1:end]
            else:
                value = '' if value.startswith('#') else re.split(r'\s+#', value, maxsplit=1)[0].strip()
            values[match[1]] = value
        for name in names:
            if values.get(name):
                return values[name], label
    raise SystemExit('No ElevenLabs credential found in .env.local or ../.env')
