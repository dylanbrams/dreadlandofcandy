# Dark candy UI — revised direction

Dylan's feedback: the first samples were too kiddie. Explore brooding dark fantasy, with peppermints and gumdrops. This supersedes the bright/cartoon direction. Samples only: no engine, game, build or automation, no runtime source edits.

## C — Saccharine Reliquary

Candy as an ancient sacred object: black cocoa, tarnished brass, chipped peppermint enamel and sugar-crusted garnet gumdrops set like cabochon jewels. Fine, restrained gothic ornament. Ivory serif headings, quiet menu rows, subtle golden selection. Recommended starting point: it makes candy visible in the interface while giving the film jokes a solemn setting.

Proposed sRGB tokens: cocoa surface `#181313`, ivory text `#E8DFCA`, aged brass `#A48A58`, peppermint red `#963B42`, oxidised mint `#82958A`.

## D — Black Sugar / Frostbound

A colder alternative: liquorice-black surfaces, blackened silver, ruby/ivory peppermint inlay and crystalline violet gumdrops. Severe geometry, thin etched rules, moonlit mint selection. Less ornament, more architectural negative space.

Proposed sRGB tokens: liquorice `#10171A`, text `#E1E6DF`, pewter `#849698`, ruby `#813B4A`, frozen mint `#9BC2B7`.

These are proposed tokens, not sampled measurements from generated artwork. Verify contrast in implementation; do not use muted metal colours for small body text. Convert sRGB tokens to linear values correctly in Unreal.

## Shared rules

- Peppermint and gumdrops are materials, heraldry and inset jewels. No frosting clouds, plush shapes, smiling portraits, sprinkle confetti or bouncing buttons. Keep comedy in the authored words and existing gameplay.
- Use a licensed, packaged serif for titles and legible regular text for body/control labels. Suggested sizes at 1920 × 1080: title 64, actions 32, body/subtitles 30, secondary 24. No ultra-thin or widely spaced body text.
- Make menus feel deliberate: flat smoky panels, narrow metal rules, a softly lit selected row plus a small seal/gem pointer. Focus must remain visible without relying on colour. Hover/pressed/disabled remain distinct; keep the hit areas at least 64 px high even when the visible selection rule is thin.
- Keep gameplay sparse. Fine ornament belongs on menu edges and skill frames, not around the entire viewport. Dark translucent surfaces must provide enough separation from both bright daytime candy terrain and dark scenery. Avoid text directly over busy imagery.
- Cooldowns keep a numeral with unit (for example `3s`) and a fill treatment. Dignity retains its real label and value; do not introduce health/mana/stamina bars to imitate another game's mechanics. Keep current key/controller badges and all current stats.
- No continuous shimmer, shake or pulse. Proposed selection transition 100–140 ms; cards fade in over 180 ms. Small material glints can accompany important events once. No new post-process effects or world lighting changes are part of this UI request.

## Application later

The image boards explore mood using dark illustrative scenery. They do not propose replacing terrain, characters, lighting or portraits. Assess the chosen UI over the existing bright candy world as well as dark backdrops before approving implementation. Preserve the real camera, quest behaviour and current film framing.

Use the integration seams in `README.md`: shared style tokens, `JamScreenWidget` factories/selection, `JamHudWidget` card and skill styling, `JamHudView` card colours, and the separate canvas compass in `JamHUD`. Preserve all live menu actions, commentary, nine pause options, recording/timecode, notes, crops/danger, journal/quest branches, character progression and note editor. Any condensed content on the boards is conceptual, not a scope cut.

Build reusable panel/selection primitives first. Small bespoke peppermint seals, gumdrop gems and worn-metal nine-slice frames can follow a chosen direction. Do not import the boards as full-screen textures or bake text into decorative art. Account for disabled states, long quest text, focus and mouse geometry when adapting the current native widget trees.

When engine access is available, use the existing coordinated verification plan in README; add checks over bright daylight terrain, near-white tooth landmarks, and dark scenes. Validate font coverage and small-label contrast, and confirm all nine pause actions fit. No runtime validation is claimed for these concepts.
