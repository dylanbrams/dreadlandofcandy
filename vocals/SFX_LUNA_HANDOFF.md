# Sound effects handoff for Luna

Direction from Dylan, 2026-11-13: **sound effects should be ridiculously over the top, like everything else in the game that's not the theme.** Produce epic fantasy catastrophe for embarrassingly domestic actions. Enormous scale, theatrical anticipation, absurdly specific materials, microscopic anticlimax. Never settle for a polite stock-game click or a realistic vegetable crunch.

This is a generation list, not a request to generate audio in this planning session. No SFX have been generated here. Luna can use the prompts below in ElevenLabs Sound Effects. Speech and battle cries are already covered by `LUNA_HANDOFF.md`; do not duplicate them here. These effects are shared across languages.

## Scope and order

The inspected `Docs/STATUS.md` reports no SFX yet. `BUILD_SPEC.md` names eight core sounds; those are P0 below. The shipped slice also has Form A Line, Disappointed Sigh and Last Judgment. The newer twelve-attack design is an expansion and has separate P2 coverage: do not assume those attacks are integrated. Reuse shared effects instead of producing twelve complete cinematic recordings.

1. **P0: eight core effects.** Make one good take each first. These establish the sonic identity.
2. **P1: nineteen supporting effects** for the current slice and proposed open world. Current-slice skill cues come first; open-world and ambient cues follow when their hooks exist.
3. **P2: twenty expansion effects**, only as the corresponding attacks are being integrated.

Total: **47 distinct effects**, before variations. Stop after P0 if time or credits are tight. For frequent sounds, later make three distinct takes of lecture impact, bubble, pacify, UI click, knead, sugar impact and celery impact. For footsteps, four isolated steps per surface is enough initially. Do not multiply the entire list into variants.

## Prompt instructions

Prepend this direction to every row's prompt:

> Ridiculously over-the-top theatrical fantasy game sound effect. Treat this tiny domestic action as a world-ending event, with total dramatic sincerity. Huge perceived scale and exaggerated physical textures. Clean isolated sound, no intelligible speech, no music, no background soundtrack, no clipping. Keep the transient clear and the ending clean. Follow the specified action exactly.

The durations are **creative targets**, not fixed animation contracts or claims about provider limits. Keep repeatable effects short; scale comes from timbre and layers, not long tails or excessive volume. Loops need clean boundaries and no opening or closing flourish. Generate starts, loops and stops separately. Do not bake the epic music, attack cry, or all timing into one file.

## P0 — the eight core sounds

| Proposed ID | Target | Prompt after shared direction | Playback purpose |
| --- | --- | --- | --- |
| SFX_Hero_Inhale | 0.6–0.9 s | A single gigantic intake of breath through a knight's helmet, like a cathedral pipe organ drawing air before passing sentence; rising metal resonance and immense suction, abruptly held. Wordless breath only. | Anticipation before lecture; avoid covering the cry. |
| SFX_Lecture_Impact | 0.35–0.6 s | One thunderous blast of parental disapproval: compressed air cannon, colossal book slammed onto an oak desk, a short metallic shockwave. A physical WHUMP with no spoken voice. | Lecture contact. |
| SFX_Bubble_Pop | 0.12–0.25 s | One comic speech balloon punching into existence with a wildly oversized rubber snap and tiny paper slap; absurdly forceful, crisp, extremely short. | Bubble appearance; throttle repeated bubbles. |
| SFX_Pacify | 0.4–0.7 s | An enormous inflated ego collapses into one tiny embarrassed rubber squeak, descending balloon pressure ending in a meek toy chirp. No creature voice or words. | Resolve exhausted; pacification, never death. |
| SFX_Clapper | 0.3–0.5 s | A movie clapperboard snaps shut with the authority of a castle drawbridge crashing down: explosive wooden crack, gigantic short room resonance, instant finish. | Slate / ACTION; distinct from Fenella VO. |
| SFX_Cut_Horn | 0.5–0.8 s | One outrageously authoritarian film-set air horn, part ocean liner and part royal herald's horn, a sharp blast that stops the universe; abrupt decisive cutoff. Nonmusical warning blast. | CUT card arrival. |
| SFX_Coin_Trickle | 1–1.5 s | A ludicrously grand cash-register mechanism opens for a pathetically small payout: a few tiny coins trickle down an enormous metal chute, diminishing to one lonely coin. No fanfare. | Hero's Cut reveal, once per ledger. |
| SFX_UI_Click | 0.08–0.18 s | A miniature imperial rubber stamp slams down with absurd bureaucratic force: compressed wooden clack, paper punch, tiny metallic latch. One crisp compact event. | Confirm / activate; fast enough for menus. |

## P1 — current skills, feedback, movement and world

| Proposed ID | Target | Prompt after shared direction | Playback purpose |
| --- | --- | --- | --- |
| SFX_Sword_Raise | 0.6–1 s | An impossibly heavy ceremonial sword rises: tectonic steel scrape, armor groan and rising air pressure, building toward an imminent apocalypse, then cutting dead BEFORE any strike. No sword hit. | Sword fakeout. Separate from the music sting. |
| SFX_FormLine_Contact | 0.4–0.7 s | A gigantic ruler strikes a desk and a row of steel filing drawers snaps into military alignment, one rapid straight-line chain of precise percussive clacks. | Form A Line contact. |
| SFX_DisappointedSigh | 0.8–1.2 s | One wordless exhausted exhalation magnified into an ancient bellows emptying an entire cathedral; deep air pressure rolls outward and dwindles in crushing disappointment. No words or melody. | Disappointed Sigh skill. |
| SFX_LastJudgment_Contact | 0.8–1.2 s | A courthouse-sized book slams shut, a colossal air shockwave radiates in all directions, metal fittings tremble, and one tiny pencil drops at the end. Absolutely no blade strike. | Ultimate contact, after fakeout pause. |
| SFX_Flustered | 0.2–0.4 s | A proud spring mechanism suddenly loses its place: explosive tiny clockwork stumble, two panicked wooden clacks and a rising rubber wobble. Wordless, no cartoon speech. | Brief fluster / interrupted enemy posture. |
| SFX_Interact_Complete | 0.3–0.5 s | A monumental bureaucratic stamp certifies a tiny task: paper slap, vault-sized lock engaging, one immaculate crystalline ping. | Treat / collect completion, not every held-interact tick. |
| SFX_Quest_Accept | 0.6–0.9 s | A parchment unfurls like a siege banner, an enormous wax seal crashes down, and a mechanical ratchet locks with ludicrous finality. | Quest acceptance. |
| SFX_Stage_Complete | 0.5–0.8 s | A huge theatrical stage mechanism locks into place with three escalating wooden clunks and one brilliant glass chime, pompous and triumphant without music. | Stage completion; avoid stacking with wrap. |
| SFX_LevelUp | 0.8–1.2 s | A massive ascending clockwork elevator of ratchets rockets upward, opens an enormous celestial latch, then resolves into one tiny immaculate bell. No melody. | Level-up card; not a musical sting. |
| SFX_Ledger_Deduction | 0.15–0.3 s | A merciless accountant's heavy stamp punches paper while a cash drawer snaps the wrong way, one sharp mechanical confiscation. | Each deduction row, rate limited. |
| SFX_Budget_Warning | 0.5–0.8 s | A giant studio light's electrical ballast sputters dangerously and a huge spring-loaded meter knocks against its stop twice. Alarmingly theatrical, no horn. | Budget threshold once; not a constant alarm. |
| SFX_Budget_Empty | 1–1.5 s | The entire movie studio powers down: giant breaker clack, descending mechanical whine, stage-light fizz, one pathetic final relay tick. | Budget exhausted transition. |
| SFX_Retake_Rewind | 0.6–1 s | A gigantic film reel spins backward at impossible speed, sprockets chatter, then the mechanism brakes with a rubbery squeal and decisive latch. | Retake reset. |
| SFX_UI_Back | 0.12–0.25 s | A ridiculously heavy official stamp is sucked backward off its paperwork, short vacuum pop followed by one tiny latch snapping shut. | Cancel / back. |
| SFX_UI_Unavailable | 0.2–0.35 s | A monumental locked filing cabinet refuses to open: one metal thunk, strained spring wobble, offended little mechanical rattle. | Unavailable ability or invalid UI action; throttle. |
| SFX_Step_Stone | 0.15–0.3 s | One armored boot plants on village cobbles like a miniature siege engine, heavy iron clonk, stone tick and comically weighty armor rattle, very short. | Isolated footstep, later four variations. |
| SFX_Step_CandySoil | 0.15–0.3 s | One heavy armored boot compresses sugary dirt: exaggerated biscuit crunch, sticky sugar pull and weighty armor clonk, crisp short finish. | Field / biome step, later four variations. |
| SFX_Fenella_Fly_Loop | 3–5 s loop | Continuous absurdly industrious tiny fairy wings, like a miniature studio helicopter made of silk and clockwork, light flutter with rapid mechanical detail. Steady texture, no passes or approach. | Low in mix, near fairy only; loop candidate. |
| SFX_Backlot_Ambience_Loop | 8–12 s loop | Quiet continuous outdoor film-backlot air, breeze through flimsy wooden scenery, faint canvas tension creaks and distant loose rigging rattles with impossibly oversized wood resonance. No people, voices, footsteps or distinct foreground event. | Subtle exploration bed; comedy in the material, spare room for music. |

## P2 — expansion attack material

Reuse P0 lecture and bubble sounds and P1 sword raise for K02. Do not regenerate those under new names. P2 sounds support the design; attack timings still belong to GAME/FX.

| Proposed ID | Attack / target | Prompt after shared direction |
| --- | --- | --- |
| SFX_Shield_Bash | K01, 0.3–0.5 s | A shield gives one politely devastating shove: huge bronze door knocker, compact pressure blast and short armor rattle. Firm impact, no cutting flesh. |
| SFX_Manners_Dome_Open | K03, 0.6–1 s | A castle-sized transparent ceremonial dome unfolds from a shield, gigantic resonant bronze panels locking together around an airy crystalline shell. No choir. |
| SFX_Manners_Dome_Rebuke | K03, 0.2–0.4 s | An invisible royal boundary repels one intruder: taut glass boing, massive doorknocker snap, one tiny offended porcelain tick. |
| SFX_Manners_Dome_Close | K03, 0.4–0.7 s | A monumental protective glass dome folds into a tiny perfectly mannered shield latch, descending panel clicks ending in a delicate single tick. |
| SFX_Knead_Impact | B01, 0.25–0.45 s | Two gigantic fists knead an impossibly heavy lump of dough in ONE double-fisted strike: thunderous wet dough THWUMP, sticky compression and an explosive flour puff. No gore. |
| SFX_RollingPin_Cyclone_Loop | B02, 2–4 s loop | Continuous storm of colossal wooden rolling pins orbiting at hurricane speed, deep rushing air, dense controlled wood clatter and flour hiss. Steady sustained middle only, no intro or finale. |
| SFX_RollingPin_Finale | B02, 0.8–1.2 s | One absurd outward explosion of heavy wooden rolling pins and flour, rapid wooden collisions, ending cleanly. No lingering individual roll. |
| SFX_RollingPin_Stray | B02, 0.6–1 s | After an unseen catastrophe, one lonely wooden rolling pin falls, bounces twice, rolls a short distance and stops with a pathetic little tap. Otherwise silent. |
| SFX_Encouragement_Blast | B03, 0.5–0.8 s | A catastrophic blast of compressed air launches a storm of stiff paper placards: enormous nonvocal pressure wave, sharp cardboard slaps, short crumpling finish. No shouting, voices or words. |
| SFX_Sugar_Volley | M01, 0.2–0.35 s | A gigantic silver spoon flick launches a tiny fast spray of sugar cubes, impossibly grand polished-metal snap with several brilliant crystalline ticks and a short air zip. No impact. |
| SFX_Sugar_Impact | M01, 0.2–0.4 s | One sugar cube lands with the force of a crystal monument: sharp tink into massive compressed crunch, tiny sparkling fragments, very short clean finish. |
| SFX_Peppermint_Fall | M02, 0.5–0.9 s | One enormous peppermint meteor descends, spiraling whistle and dense rushing air building to impending candy catastrophe, no impact at the end. |
| SFX_Peppermint_Impact | M02, 0.5–0.8 s | One giant hard peppermint hits like siege artillery: deep compact thud, spectacular brittle candy fracture and sparkling sugar debris. No rubble avalanche or long tail. |
| SFX_Soap_Torrent_Loop | M03, 2–4 s loop | Continuous apocalyptic pressure washer pumping thick magical soap, roaring viscous spray, ferocious foam churn and tiny bright bubbles. Huge steady energy, no speech, no start or stop. |
| SFX_Soap_Finish | M03, 0.5–0.8 s | A colossal soap cannon shuts off with a pressure-valve gulp and collapsing foam, then one tiny sanitizer-dispenser plip and immaculate glass squeak. |
| SFX_Celery_Launch | A01/A02, 0.2–0.35 s | An epic war bow releases a celery stalk like a ballista bolt: monumental taut-string snap, leafy whip and extremely fast air zip. No impact. |
| SFX_Celery_Impact | A01/A02, 0.2–0.4 s | One celery stalk strikes with siege-weapon force, gigantic wet fibrous CRUNCH, snapping vegetable ribs and a tiny leaf flutter. No gore, one compact hit. |
| SFX_Floss_Launch | A03, 0.3 s | A siege-bow tension snap pulls absurdly taut dental thread from a tiny dispenser, huge string resonance with a sharp plastic click. No impact. |
| SFX_Floss_Snare | A03, 0.5 s | An explosive dental-floss spool unravels into several gigantic elastic strand twangs, binding a target with ludicrous tensile force, ending taut. |
| SFX_Floss_Retract | A03, 0.25 s | An outrageously fast dental-floss dispenser retracts its thread, violent elastic zip ending in one tiny plastic click. |

M02's finale can initially use its impact with extra layering; A02's late stalk uses an isolated celery impact. No separate rain recording needed: schedule individual impacts to actual contacts.

## Delivery and game handoff

- Save provider originals and import-ready mono PCM WAV, preferably 48 kHz / 16-bit, under `output/sfx/<ID>_v01.wav`. Keep stereo ambience originals if useful. No actual file exists merely because it is listed here.
- Manifest: ID, priority, attack/trigger, prompt, requested and actual duration, loop flag, model/settings, provider request ID, original/output paths, generation status, listening-review status and provenance. Never log credentials. Use resumable generation and inspect provider history before retrying an ambiguous paid timeout.
- Listen for accidental voices/music, flat or realistic delivery, muddy attacks, clipping, chopped tails and loop seams. A failed prompt gets revised; extreme volume does not rescue a weak effect.
- Keep the ridiculous scale but leave headroom. Match perceived level, keep frequent sounds compact, and preserve voice intelligibility. Critical feedback and local attacks need concurrency limits; no individual sound per decorative projectile or paper scrap.
- For the sword joke: raise / anticipation, deliberate silence, then lecture contact. GAME cuts or ducks the music at the fakeout; this SFX file cannot perform that operation itself. Last Judgment gets the grandest treatment. The rare quiet B03 voice still uses the full catastrophic blast.
- Starts / loops / stops attach to actual attack events. Stop loops on cancel and scene reset. No full-length prerecorded sequence should dictate authoritative contact timing.
- GAME owns Unreal imports, playback hooks, attenuation, volume controls, concurrency and cooking. Proposed destination `/Game/Audio/SFX/`; these IDs are a handoff convention, not an existing engine contract. No editor work or game source changes in this task.
- Record generated-asset provenance for the jam declaration and credits. Do not change plans or enable paid overage automatically. No audio generation or spending is performed by this document.

Sources inspected locally: `Docs/Design/BUILD_SPEC.md` (skills and eight-sound baseline), `Docs/Design/The_Dread_Land_of_Candy_Attacks_and_Effects_Pipeline.md` (twelve expansion concepts), `Docs/STATUS.md` (reported audio state), and `vocals/LUNA_HANDOFF.md` (existing speech scope).
