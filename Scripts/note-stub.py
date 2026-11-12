"""A local stand-in for the director-note service (Docs/Design/NoteForm.md v1), for testing T12b without BACKEND.

    python -I Scripts/note-stub.py [--port 8011] [--delay 1.5] [--status 200]
    game:  -noteurl=http://127.0.0.1:8011

Answers /v1/director/note with a fixed, valid form shaped a little by the note's words, and
/v1/director/premiere with a fixed title. --delay tests the reading state, --delay 8 the six-second timeout,
--status 503 the fallback. No model, no keys, no network beyond localhost.
"""
import argparse
import json
import time
from http.server import BaseHTTPRequestHandler, HTTPServer


def note_form(req):
    note = req.get("note", "").lower()
    kinds = req.get("set_kinds", [])
    cap = int(req.get("set_capacity", 3))
    big = any(w in note for w in ("big", "enormous", "huge", "giant"))
    form = {
        "count": max(1, min(cap, 2 if big else 4)),
        "resolve_mult": 1.5 if big else 1.0,
        "speed_mult": 0.8 if big else 1.2,
        "scale": 2.5 if big else 1.0,
        "time_limit_s": 0,
        "target_kind": "keep",
        "tint": "gold" if "gold" in note else ("purple" if "taxes" in note else "pink"),
        "weather": "rain" if "rain" in note else ("fog" if "fog" in note else ("sprinkle_snow" if "snow" in note else "none")),
        "mood": "sad" if ("sad" in note or "ashamed" in note) else "epic",
        "motif": "shame" if "ashamed" in note else "taxes",
        "scene_title": "The Remorse of the Stub",
        "fenella_reaction": "Bold. Wrong. I love it. Rolling!",
        "objective": "Deal with the very large, very sorry ones",
        "lecture_bubbles": ["LOOK AT YOURSELVES.", "SO BIG. SO UNBRUSHED.", "FILE YOUR RETURNS."],
        "replies": ["we know", "sorry"],
        "loot_name": "Toothbrush of Considerable Regret",
        "loot_flavour": "Sized for a larger conscience.",
    }
    if kinds and "target_kind" in form and form["target_kind"] not in ["keep"] + kinds:
        form["target_kind"] = "keep"
    return {"v": 1, "source": "model", "form": form}


def premiere():
    return {
        "v": 1, "source": "model",
        "title": "The Dread Land of Candy: A Study in Shame",
        "tagline": "They were big. They were sorry.",
        "reviews": [
            {"quote": "I felt seen. I did not enjoy it.", "by": "a goblin"},
            {"quote": "Ninety minutes. It was eight.", "by": "The Sugarborough Bugle"},
            {"quote": "The rain was real. So was my pain.", "by": "a rat"},
        ],
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=8011)
    ap.add_argument("--delay", type=float, default=1.5)
    ap.add_argument("--status", type=int, default=200)
    args = ap.parse_args()

    class Handler(BaseHTTPRequestHandler):
        def do_POST(self):
            length = int(self.headers.get("Content-Length", 0))
            try:
                req = json.loads(self.rfile.read(length) or b"{}")
            except ValueError:
                req = {}
            print("POST", self.path, json.dumps(req), flush=True)
            time.sleep(args.delay)
            if args.status != 200:
                self.send_response(args.status)
                self.end_headers()
                return
            if self.path.endswith("/note"):
                body = note_form(req)
            elif self.path.endswith("/premiere"):
                body = premiere()
            else:
                self.send_response(404)
                self.end_headers()
                return
            data = json.dumps(body).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)

        def log_message(self, *a):
            pass

    print(f"note stub on http://127.0.0.1:{args.port} (delay {args.delay}s, status {args.status})", flush=True)
    HTTPServer(("127.0.0.1", args.port), Handler).serve_forever()


if __name__ == "__main__":
    main()
