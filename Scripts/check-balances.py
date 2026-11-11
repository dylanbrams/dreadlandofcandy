#!/usr/bin/env python3
"""Check which asset-generation API keys in the project-root .env work, and their credit balance.

Prints variable NAMES and results only, never key values. Free calls (no credits are spent).
"""
import json
import os
import re
import urllib.error
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CANDIDATES = {
    "tripo":   ["TRIPO_API_KEY"],
    "meshy":   ["MESHY_API_KEY"],
    "hyper3d": ["HYPER3D_API_KEY", "RODIN_API_KEY", "HYPER3D_KEY"],
    "fal":     ["FAL3D_API_KEY", "FAL_KEY", "FAL_API_KEY"],
}


def env_values():
    vals = dict(os.environ)
    f = ROOT / ".env"
    if f.is_file():
        for line in f.read_text(encoding="utf-8").splitlines():
            m = re.match(r"\s*(?:export\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*?)\s*$", line)
            if m and not line.lstrip().startswith("#"):
                vals[m.group(1)] = m.group(2).strip("\"'")
    return vals


def get(url, auth):
    req = urllib.request.Request(url, headers={"Authorization": auth, "User-Agent": "jam-balance-check"})
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            return r.status, json.load(r)
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode(errors="replace")[:200]
    except Exception as e:
        return None, type(e).__name__


def main():
    vals = env_values()
    for svc, names in CANDIDATES.items():
        name = next((n for n in names if vals.get(n)), None)
        if not name:
            print(f"{svc:8} no key found (looked for {', '.join(names)})")
            continue
        key = vals[name]
        if svc == "tripo":
            code, body = get("https://openapi.tripo3d.ai/v3/account/balance", "Bearer " + key)
            out = body.get("data") if code == 200 else body
        elif svc == "meshy":
            code, body = get("https://api.meshy.ai/openapi/v1/balance", "Bearer " + key)
            out = body if code == 200 else body
        elif svc == "fal":
            code, body = get("https://api.fal.ai/v1/account/billing?expand=credits", "Key " + key)
            out = (body.get("credits") if code == 200 and isinstance(body, dict) else body)
        else:
            out = "no documented balance endpoint; check the wallet in the hyper3d.ai web app"
            code = "-"
        print(f"{svc:8} {name}: HTTP {code} -> {out}")


if __name__ == "__main__":
    main()
