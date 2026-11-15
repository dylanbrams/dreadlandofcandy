# Shard expansion

135 additional 1024px transparent RGBA comic cards: three cries for each of
Manners, Brush and Vitamins in 15 locales. Existing art is preserved.

All outer and inner colored borders are explicit straight polygon segments.
Six seeded asymmetric bursts use alternating long blades and deep notches;
there is no added central lettering panel. Fonts retain their natural curves.

Rebuild from ComicCards: `python Expansion/build.py`. Requires Pillow,
fontTools, installed Windows fonts and the existing project angular renderer.
Roster, manifest and numbered contact sheets are generated beside the PNGs.
The manifest includes glyph containment, font sizes, alpha bounds and SHA256.

Language scope follows October 2026 Steam combined-platform survey top 15:
English, Simplified Chinese, Russian, Spain Spanish, Brazilian Portuguese,
German, Japanese, French, Polish, Korean, Turkish, Traditional Chinese,
Latin American Spanish, Thai and Ukrainian. Steam language settings are not
necessarily users' first languages. Sources:
https://store.steampowered.com/hwsurvey/
https://gaugedb.com/hardware-survey?category=language&item=english&month=2026-10&platform=combined

Translations are short thematic adaptations and need native-speaker review.
These are source assets only. Existing gameplay still needs its registry
expanded and the new textures imported; no Unreal session was run here.
Use authored RGB, sRGB, straight alpha, clamp sampling and mipmaps as in the
parent handoff. Preview JPGs are review artifacts, not game textures.
