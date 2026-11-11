"""Shared helpers for the asset-generation clients (meshy.py, meshy_rig.py, rodin.py,
openai_image.py). Not run directly.

Safety rules every client follows: keys come from the environment or the git-ignored
project-root .env and are never printed; downloads only from allowed hosts over HTTPS,
size-capped, quarantined as .part, validated as a real binary glTF with no external
references, and nothing is ever executed.
"""
import json
import os
import re
import struct
import sys
import time
import urllib.error
import urllib.request
import uuid
from pathlib import Path
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parent.parent
MAX_BYTES = 150 * 1024 * 1024


def env_value(*names):
    vals = dict(os.environ)
    f = ROOT / ".env"
    if f.is_file():
        for line in f.read_text(encoding="utf-8").splitlines():
            m = re.match(r"\s*(?:export\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*?)\s*$", line)
            if m and not line.lstrip().startswith("#"):
                vals[m.group(1)] = m.group(2).strip("\"'")
    for n in names:
        if vals.get(n):
            return vals[n]
    sys.exit(f"No key found (looked for {', '.join(names)} in the environment and project-root .env).")


def request(method, url, key, *, json_body=None, raw_body=None, content_type=None, scheme="Bearer",
            timeout=90, retries=4):
    """Send a request and return parsed JSON. Retries 429 (always) and 5xx (GET only), honouring Retry-After."""
    data = raw_body
    headers = {"Authorization": f"{scheme} {key}", "User-Agent": "jam-asset-client"}
    if json_body is not None:
        data = json.dumps(json_body).encode()
        headers["Content-Type"] = "application/json"
    elif content_type:
        headers["Content-Type"] = content_type
    for attempt in range(retries + 1):
        req = urllib.request.Request(url, data=data, method=method, headers=headers)
        try:
            with urllib.request.urlopen(req, timeout=timeout) as r:
                return json.load(r)
        except urllib.error.HTTPError as e:
            body = e.read().decode(errors="replace")[:400]
            retryable = e.code == 429 or (e.code >= 500 and method == "GET")
            if retryable and attempt < retries:
                wait_s = e.headers.get("Retry-After")
                wait_s = int(wait_s) if wait_s and wait_s.isdigit() else 10 * (attempt + 1)
                print(f"  HTTP {e.code}, retrying in {wait_s}s")
                time.sleep(min(wait_s, 120))
                continue
            sys.exit(f"HTTP {e.code} from {urlparse(url).hostname}: {body}")
        except (urllib.error.URLError, TimeoutError) as e:
            if method == "GET" and attempt < retries:
                print(f"  network error ({e}), retrying")
                time.sleep(10)
                continue
            sys.exit(f"network error talking to {urlparse(url).hostname}: {e}")


def multipart(fields, files=()):
    """Encode multipart/form-data. fields: {name: text}; files: [(field, filename, bytes, mime)].
    Returns (body, content_type)."""
    b = uuid.uuid4().hex
    out = []
    for k, v in fields.items():
        out.append(f'--{b}\r\nContent-Disposition: form-data; name="{k}"\r\n\r\n{v}\r\n'.encode())
    for field, filename, blob, mime in files:
        out.append(f'--{b}\r\nContent-Disposition: form-data; name="{field}"; filename="{filename}"\r\n'
                   f'Content-Type: {mime}\r\n\r\n'.encode() + blob + b"\r\n")
    out.append(f"--{b}--\r\n".encode())
    return b"".join(out), f"multipart/form-data; boundary={b}"


def check_url(url, suffixes, extra_host=None):
    u = urlparse(url)
    allowed = tuple(suffixes) + ((extra_host,) if extra_host else ())
    host = u.hostname or ""
    if u.scheme != "https" or not any(host == s or host.endswith("." + s) for s in allowed):
        sys.exit(f"refusing to download from {host!r}; if that host is right, pass --allow-host {host}")


def validate_glb(path):
    data = path.read_bytes()
    if len(data) < 20 or data[:4] != b"glTF":
        sys.exit("not a binary glTF (bad magic); discarded")
    version, length = struct.unpack_from("<II", data, 4)
    if version != 2 or length != len(data):
        sys.exit("glb header does not match the file; discarded")
    chunk_len, chunk_type = struct.unpack_from("<II", data, 12)
    if chunk_type != 0x4E4F534A or 20 + chunk_len > len(data):
        sys.exit("glb has no valid JSON chunk; discarded")
    doc = json.loads(data[20:20 + chunk_len])
    for item in list(doc.get("buffers", [])) + list(doc.get("images", [])):
        uri = item.get("uri", "")
        if uri and not uri.startswith("data:"):
            sys.exit(f"glb references an external file ({uri[:60]}); discarded")


def validate_fbx(path):
    head = path.read_bytes()[:32]
    if not (head.startswith(b"Kaydara FBX Binary") or head.lstrip().startswith(b"; FBX")):
        sys.exit("not an FBX file; discarded")


def validate_image(path):
    head = path.read_bytes()[:16]
    if not (head.startswith(b"\x89PNG\r\n\x1a\n") or head.startswith(b"\xff\xd8\xff")
            or (head[:4] == b"RIFF" and head[8:12] == b"WEBP")):
        sys.exit("not a PNG, JPEG or WebP image; discarded")


VALIDATORS = {".glb": validate_glb, ".fbx": validate_fbx, ".png": validate_image, ".jpg": validate_image,
              ".webp": validate_image}


def safe_download(url, dest, suffixes, extra_host=None):
    """Download to dest after host, size and format checks. The format check follows dest's
    extension: .glb, .fbx, or an image (.png/.jpg/.webp)."""
    validator = VALIDATORS.get(dest.suffix.lower())
    if not validator:
        sys.exit(f"no validator for {dest.suffix} files")
    check_url(url, suffixes, extra_host)
    dest.parent.mkdir(parents=True, exist_ok=True)
    tmp = dest.with_name(dest.name + ".part")
    n = 0
    with urllib.request.urlopen(url, timeout=120) as r, open(tmp, "wb") as f:
        while chunk := r.read(1 << 16):
            n += len(chunk)
            if n > MAX_BYTES:
                f.close(); tmp.unlink(); sys.exit("download over the size cap; discarded")
            f.write(chunk)
    try:
        validator(tmp)
    except SystemExit:
        tmp.unlink(missing_ok=True)
        raise
    tmp.replace(dest)


def find_manifest(service, name):
    """Last manifest record for a file named <name>.glb, or None."""
    f = ROOT / "ImportSource" / service / "manifest.jsonl"
    if not f.is_file():
        return None
    hit = None
    for line in f.read_text(encoding="utf-8").splitlines():
        try:
            rec = json.loads(line)
        except ValueError:
            continue
        if rec.get("file") == f"{name}.glb":
            hit = rec
    return hit


def log_manifest(service, record):
    d = ROOT / "ImportSource" / service
    d.mkdir(parents=True, exist_ok=True)
    record["date"] = time.strftime("%Y-%m-%d")
    with open(d / "manifest.jsonl", "a", encoding="utf-8") as f:
        f.write(json.dumps(record) + "\n")


def check_name(name):
    if not re.fullmatch(r"[A-Za-z0-9_-]+", name):
        sys.exit("--name: letters, digits, _ and - only")


def apply_style(prompt, kind):
    """Wrap a prompt in the locked style for its type (Docs/styles.json); no-op if unset."""
    if not kind:
        return prompt
    f = ROOT / "Docs" / "styles.json"
    if not f.is_file():
        return prompt
    st = json.loads(f.read_text(encoding="utf-8")).get(kind) or {}
    parts = [x.strip().strip(",").strip() for x in (st.get("prefix", ""), prompt.rstrip(". "), st.get("suffix", ""))]
    return ", ".join(x for x in parts if x)
