# Flying action cards: source-art delivery

The newer [angular expansion](Angular/HANDOFF.md) adds 24 cards using four
new spike-only silhouettes without this first set's central polygon. Review
`Angular/preview-a.jpg` and `Angular/preview-b.jpg` when selecting effect art.

User direction: sharp, spiky vintage superhero action bursts, with thematic
words such as MANNERS, BRUSH and VITAMINS. No POW/SMACK lettering, speech tails,
rounded speech balloons or long reprimand sentences in the delivered set.

## Files

- Twelve `T_Comic_*.png` files: 1024 x 1024 straight-alpha RGBA, transparent
  outside the burst, ivory interiors, black lettering, varied colored borders.
- `preview.jpg`: contact sheet for human review; do not import as an effect texture.
- `roster.json`: editable UTF-8 words, locales, template selection and colors.
- `Templates/`: three reusable blank AI-created spiky action-burst templates.
  Exact generation prompts and original paths are in `Templates/provenance.json`.
  The rebuild script broadens their central body with an editable polygon so
  the final lettering can stay large, while retaining the generated spike shapes.
- `manifest.json`: final filenames, checksums, text bounds, font sizes and
  suggested Unreal paths. Text and colors are applied deterministically.

Rebuild from the project root: `python Scripts/build_comic_cards.py`.
Requires Pillow and fontTools plus the named installed Windows fonts. Font
files are not copied into the project. Latin uses Impact; Japanese uses Yu Gothic
Bold, Korean uses Malgun Gothic Bold, Chinese uses Microsoft YaHei Bold.
Change the roster's font filename to another installed font if desired.

The three meanings each have four variants:

| Meaning | Locales | Suggested use |
| --- | --- | --- |
| Manners | English, German, French, Japanese | K02 Stern Instruction / K03 Good Manners |
| Brush (teeth) | English, Spanish, Korean, simplified Chinese | Hygiene effects / brushing |
| Vitamins | English, German, French, Brazilian Portuguese | B03 Aggressive Encouragement / nutrition effects |

Translations are short thematic adaptations; native-speaker review has not
been performed. French manners uses `LES BONNES MANIÈRES !`; Korean and Spanish
brush explicitly mean brushing one's teeth. The script checks every glyph is
present in the selected font and that the text fits inside the burst interior.

## Effect implementation boundary

This delivery contains source art, not imported Unreal assets or a gameplay change.
The second process should import the individual PNGs into
`/Game/FX/Attacks/Textures/` (suggested object paths are in the manifest), preserve
alpha, enable sRGB and clamp sampling. Use a two-sided unlit card material:
texture RGB for the authored colors, texture alpha for opacity/masking, and
particle alpha for lifetime fade. Avoid multiplying RGB by the current orange
or blue preset tint: that would recolor the ivory fill and multilingual lettering.
Retain mipmaps for moving world particles and check lettering at gameplay distance.

Choose cards by meaning/locale, or mix locales intentionally. Bake a padded atlas
or texture array if useful; this delivery deliberately supplies individual PNGs
so it does not dictate the Niagara selection mechanism. No runtime fonts,
translation, TTS or text layout are required.

Existing spawn point: `Source/MegaJam/Private/Game/JamAttackParticles.cpp` loads
K02 and B03 including `_Layer1`. Their current card recipe is `CARDS` in
`Scripts/build_niagara_attacks.py`: generic `SM_FX_Card`, fixed orientation,
six-card cap, 0.65-second lifetime. The importer currently assigns that mesh the
generic `M_FX_Mesh`; replacing only a PNG will not show this art. Supply the card
material/texture binding, camera-facing release, tumble and fade/impact behavior.
Keep cast-owned cleanup and cosmetic-only behavior. Do not rebuild the generated
Niagara library blindly over another process's hand edits.

Validation here covers PNG alpha, glyph coverage and layout, with a reviewed
contact sheet. In-engine readability, atlas filtering, material setup, flight,
impact and cooking remain for the effect implementation process.
