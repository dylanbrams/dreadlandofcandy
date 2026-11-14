# Angular action-card expansion

24 additional transparent 1024 x 1024 PNG cards, using four new AI-created
angular burst templates. This set preserves the templates' spike contours with
transparent canvas padding. No rectangle, octagon, rounded bubble or separate
body polygon is added underneath the lettering. The first 12-card set is kept
in the parent directory for comparison.

`preview-a.jpg` and `preview-b.jpg` show 12 cards each. Both use MANNERS, BRUSH
and VITAMINS with the parent set's eight languages. Variants change the burst
shape, colored rim, pale fill, ink color and lettering angle. The second dozen
use colored lettering. No POW/SMACK artwork is included.

Edit `roster.json` and run `python Scripts/build_angular_comic_cards.py` from
the project root. Requires Pillow, fontTools and installed Windows fonts as
described in the parent handoff. Text is composited programmatically; template
art is generated with built-in image_gen. Exact generation prompts and original
paths are in `Templates/provenance.json`. Letter placement is validated using
the actual rotated glyph silhouette, allowing larger type within angular shapes
without forcing a rectangular text panel. Font binaries are not redistributed.

`manifest.json` records all final filenames, locales, font sizes, text bounds,
colors, SHA-256 hashes and suggested Unreal object paths. These are PNG sources,
not imported Unreal assets or a runtime registry. Use individual `T_Comic_*.png`
files as sRGB textures, preserving alpha and clamp sampling. See the parent
`HANDOFF.md` for existing Niagara card layers and material integration details.
Keep authored RGB colors rather than multiplying them by the current preset's
orange/blue tint. Review JPEGs are not effect textures.

Checks cover font glyph availability, transparent padding and rotated text
containment. Translations inherit the parent set and have not had native-speaker
review. Flight, gameplay-camera readability, material setup, impact and cooking
remain the effect implementation process's work.
