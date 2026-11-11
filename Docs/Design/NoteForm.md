# Note form: the contract between the game and the note service

Shared by RULES (`JamNoteClamp`, task T12a) and BACKEND (tasks B1, B2). Version 1. Change this file first, then both sides. The game clamps everything again on arrival; the server validating is a courtesy, not the safety.

## POST /v1/director/note

Request:

```json
{
  "v": 1,
  "session_id": "random uuid per run, no player identity",
  "scene": "fort_toothache",
  "stage": "brush_goblins",
  "set_kinds": ["goblin", "rat"],
  "set_capacity": 8,
  "motif": "",
  "note": "make the goblins enormous and ashamed"
}
```

`note`: at most 80 characters. `motif`: empty on a player's first note, otherwise the motif from their previous response.

Response:

```json
{
  "v": 1,
  "source": "cache | model | budget | refused",
  "form": {
    "count": 3,
    "resolve_mult": 1.5,
    "speed_mult": 0.8,
    "scale": 2.5,
    "time_limit_s": 0,
    "target_kind": "keep",
    "tint": "none",
    "weather": "none",
    "mood": "sad",
    "motif": "shame",
    "scene_title": "The Remorse of Fort Toothache",
    "fenella_reaction": "Enormous AND ashamed. That's range. Rolling!",
    "objective": "Brush the very large, very sorry goblins",
    "lecture_bubbles": ["LOOK AT YOURSELVES.", "SO BIG. SO UNBRUSHED."],
    "replies": ["we know", "sorry"],
    "loot_name": "Toothbrush of Considerable Regret",
    "loot_flavour": "Sized for a larger conscience."
  }
}
```

`source` is `budget` (daily limit reached, cache miss) or `refused` (abusive or off-limits note): `form` is absent and the game falls back to its keyword matcher and authored notes.

## Field limits (the game clamps to these; anything else is dropped)

| Field | Limit |
| --- | --- |
| `count` | 1 to `set_capacity`, and never above 8 |
| `resolve_mult` | 0.5 to 3.0 |
| `speed_mult` | 0.5 to 2.0 |
| `scale` | 0.5 to 3.0 |
| `time_limit_s` | 0 (none) or 30 to 120 |
| `target_kind` | `keep`, or one of the request's `set_kinds` |
| `tint` | `none`, `pink`, `mint`, `gold`, `grey`, `purple` |
| `weather` | `none`, `rain`, `sprinkle_snow`, `fog` |
| `mood` | `cheerful`, `epic`, `sad` |
| `motif` | 24 characters, three words |
| `scene_title` | 40 characters |
| `fenella_reaction` | 90 characters |
| `objective` | 60 characters |
| `lecture_bubbles` | 3 to 5 items, 24 characters each |
| `replies` | 2 to 3 items, 16 characters each |
| `loot_name` | 48 characters |
| `loot_flavour` | 80 characters |

A form is rejected outright (fallback used) if any text field is empty after truncation or the combination makes the stage unfinishable: `count` times `resolve_mult` so high that the Lecture alone cannot finish inside `time_limit_s`. The exact check is `JamNoteClamp`'s to define and test.

The model never sets gross, rewards, XP or completion. The game computes gross from the knobs.

## Cache key (server)

`v`, prompt version, `scene`, `stage`, `motif`, and `note` normalised: lower case, trimmed, inner whitespace collapsed, punctuation removed. A hit returns the stored `form` with `source: "cache"` and makes no model call.

## POST /v1/director/premiere

Request:

```json
{
  "v": 1,
  "session_id": "same uuid",
  "motif": "shame",
  "notes": ["make the goblins enormous and ashamed"],
  "stats": { "takes": 7, "notes_taken": 1, "notes_refused": 1, "notes_typed": 1 },
  "ending": "directors_cut | theatrical"
}
```

`notes`: at most 6 items of 80 characters.

Response:

```json
{
  "v": 1,
  "source": "model | budget | refused",
  "title": "The Dread Land of Candy: A Study in Shame",
  "tagline": "They were big. They were sorry.",
  "reviews": [
    { "quote": "I felt seen. I did not enjoy it.", "by": "a goblin" }
  ]
}
```

Limits: `title` 48, `tagline` 80, exactly 3 `reviews`, `quote` 70, `by` 30. On anything else the game shows its authored ending.

## Timeouts and failure

The game waits six seconds, then falls back. Any non-200, malformed JSON, wrong `v` or missing field is a fallback, silently; the player never sees an error.
