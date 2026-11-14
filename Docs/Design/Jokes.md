# Jokes

Written 2026-11-11 as design text (declared in `Docs/Declared.md`). Everything here is original; nothing is quoted from another game or film. This is the source for `JamLines` (task T09), the ledger table, loot names, signs and credits. Dylan picks; sessions do not invent replacement jokes without asking.

## How the comedy works (read before adding a line)

Nov 14 plot revision (Dylan): Fenella is filming a reality TV show,
**Heroes of Dread: The Land of Candy**. She assembled the villagers and cast
the hero, and wants huge problems and maximum drama. This introduction plays
automatically on arrival in Sugarborough; talking to her skips ahead to work.
The villagers' concerns remain sincere. Runtime text: `RealityIntro_01..05`.

> Welcome to Heroes of Dread: The Land of Candy! I'm Fenella Flicker, producer, director and the only person here who understands reality television.
>
> I've assembled these villagers and cast YOU as our hero. They bring the problems. You bring the brooding. I'll bring the camera alarmingly close.
>
> I want MASSIVE problems. Impossible odds! Tears before breakfast! The villagers asked for a quiet life. Terrible television. I've requested revisions.
>
> Every little village crisis will become an EPIC catastrophe. Entirely real reactions, naturally. If yours aren't dramatic enough, we'll do another take.
>
> Now come to my golden triangle, hero. Your starring role awaits. Try to look haunted. It's candy country; you'll have to make an effort.

These opening beats use longer reading times than the short ambient captions.
Voice clips for `RealityIntro_01..05` have not been recorded; captions and
Fenella's speaking animation carry the introduction for now.

- **The hero never speaks and never reacts.** Total grim commitment. The hero is the straight man; do not give them a quip. (One exception, set piece 12.)
- **Fenella is cheerful, never bitter.** A producer who is certain she is helping. She is rude about the performance, never about the player.
- **Villagers are sincere.** To them, a rat that won't eat celery is a real emergency and a seven-foot knight is the obvious contractor.
- **Nobody winks.** No character says the situation is silly.
- **Specific beats random.** "Catering: there was no catering, that costs extra" is a joke. "Banana fee" is not.
- **Short.** A caption is on screen for about three seconds: twelve words or fewer. Longer lines are for cards and credits.
- **A line is funny once.** The scheduler must not repeat a line in a run until its pool is exhausted. Pools need depth where the trigger is common (idle, pacified, lecture).
- **Callbacks pay double.** Running threads to return to: celery, "Miscellaneous", level seven, the fourth take she budgeted for, Gary.
- **The second sentence is the joke.** Her pattern: confident claim, then the correction that makes it worse. "I'm paying you by the hour. I'm not. Hurry up."

## Set pieces, ranked by laughs per hour of work

Cost: S under an hour, M a few hours, L half a day.

| # | Set piece | Cost | What happens |
| --- | --- | --- | --- |
| 1 | **The sword fakeout** | M | Enormous sword rises, music swells, hero inhales... and delivers a lecture. Bubbles: "ABSOLUTELY NOT." The music sting cuts off dead on the first word. Never cut this. |
| 2 | **"Perfect. Again."** | S | Scene 1, take 1 goes flawlessly. Fenella: "Perfect! Again." Take 2 begins. "This time the rats know their lines." The rats now run away. |
| 3 | **The budget runs out** | M | Mid-finale: "Small thing. We're out of money. Keep rolling!" Materials drop to grey checkerboard. Rats become boxes labelled RAT. Lecture bubbles become "[SHOUTING]". The music is replaced by Fenella humming it, badly. |
| 4 | **The interrupted narration** | S | Black screen, solemn text: "Beyond the forsaken mountains lies a land of unspeakable sweetness. For a thousand years—" Fenella: "CUT. Nobody reads these. Skip to the rats." Smash cut to the field. |
| 5 | **The itemized ledger** | S | See the ledger table. "Miscellaneous" is always the largest line. |
| 6 | **Plywood houses** | S | From the street the houses are lovely. Walk behind one: bare plywood, struts, a stencil reading THIS SIDE BACK. Caption: "Don't film that side." |
| 7 | **The deleted scene** | M | A door on the street marked DELETED SCENE. Inside: an untextured room, one villager in a T-pose, a floating cube labelled DRAGON (TODO). Fenella: "That was cut for pacing. And for not existing." |
| 8 | **Director's Commentary** | M | Toggle it and she talks over the game about the game. See the commentary pool. |
| 9 | **The call from one metre away** | S | The farmer stands in front of you. The Candy Rock rings. It is the farmer. "Hello? Yes. I'm the one waving." |
| 10 | **Reserved for the sequel** | S | Three loot cards flip over. The best one gets a rubber stamp across it: RESERVED FOR SEQUEL. |
| 11 | **The stunt double** | S | On a failed take, the hero is swapped for a cardboard cutout of the hero, which falls over. "Stunt double's fine. It's cardboard." |
| 12 | **The hero's one word** | S | At the Premiere, asked for a speech, the hero finally speaks. One caption, small: "...Celery." Fenella: "Chills. Cut it." |
| 13 | **Boom in shot** | S | A boom mic on a stick dips into the top of the frame. "Boom's in shot! Who's on boom? ...I'm on boom." It withdraws. |
| 14 | **The late rat** | S | During the slate everyone holds position. One rat sprints in from off-set and takes its mark. |
| 15 | **Rats on break** | S | Between takes, pacified rats stand around a tiny table. Sign: CRAFT SERVICES (CELERY). |
| 16 | **Pause doesn't pause her** | S | Pause menu opens. Caption: "Why did everyone stop? Who called lunch?" |
| 17 | **The extras** | S | Every villager is the same flat. One has a moustache drawn on. Credits list them as "Villager", "Different Villager", "Villager (Moustache)". |
| 18 | **Post-credits pitch** | S | After credits: "Sequel. Same rats. Same field. We call it a return to form." |

If only five ship: 1, 2, 3, 4, 5.

## Fenella's lines, by trigger

Ids are suggestions; one line per id.

**Scene start (ACTION!)**
- Places, everyone! And by everyone I mean you.
- Quiet on set! It's a field. It was already quiet. Well done, all.
- Remember: you're grim, you're haunted, you're holding celery.
- Rolling! Is the red light on? There's no red light. Rolling!
- Scene one, take one. No pressure. It's only my whole career.
- And... ACTION! I love saying that. Ninth time today.
- Hit your mark. Your mark is the dirt. All of it.
- Give me dread. Give me doom. Give me dental hygiene.

**Level seven (once, at the start)**
- You start at level seven. Levels one to six tested poorly.
- Don't ask about the first six levels. Continuity.

**Idle (no progress)**
- Take your time. Film is cheap. It isn't. Hurry up.
- Is this a dramatic pause, or are you buffering?
- I'm paying you by the hour. I'm not. Move anyway.
- Brooding is good. Brooding near the rats is better.
- We'll fix the standing still in post. I can't afford post.
- The audience is bored. I'm the audience.
- Lovely stillness. Very brave. Now do the other thing. Acting.
- I can hear the budget ticking. It's a small sound.

**Strayed from the set**
- This is a closed set! Get back in frame!
- The set ends there. Past it is nothing. I didn't build it.
- Where are you going? The plot is over here.
- That's not in the script. Nothing is, but that especially.
- Come back! I can't fly that far. Union rules. My union.
- If you find a better film out there, don't tell me.

**Lecture cast (rare, one in five)**
- Less violence! More character development!
- The sword is a metaphor! Keep it a metaphor!
- Don't swing it! Do you know what rat insurance costs?
- Louder! The cheap seats are in another village!
- Yes! Disappoint them!

**Target pacified**
- He's learning! That's an arc!
- Hold on his little face. Hold... gorgeous.
- That rat just earned a supporting credit.
- Look at the remorse. You can't direct that. I directed that.
- One down. Emotionally.

**Interact (feeding, brushing)**
- Circular motions! I want to feel the plaque leaving.
- Offer the celery like it's a crown.
- He's chewing! Nobody move. Nobody breathe. Got it.
- Gums too. We're not animals. Well. He is.
- That's the shot. A knight, a goblin, a minty silence.

**Failed take**
- Cut! Beautiful. Unusable, but beautiful.
- That's what take two is for. I've budgeted for four.
- The rats were perfect. We'll go again for you.
- Back to one! That's film talk for "you did it wrong."
- That was a take. Technically. RETAKE!
- Don't apologise. You weren't going to. Again!

**Take 3 and beyond**
- Take three. Still within budget. Barely. Emotionally.
- Take four. This is the last one I budgeted for.
- Take five. We're now spending your cut.
- Take six. I'm telling people this was a choice.

**CUT! pitches (the note)**
- CUT! The rats have no motivation. Options, darling.
- CUT! Needs a stronger antagonist. I'm thinking: bigger goblin.
- CUT! Where's the ticking clock? I brought a clock.
- CUT! Test audiences wanted more rats. I asked two rats.
- CUT! Same scene, but sadder. Lecture them gently.
- CUT! It's good. Make it more. Just... more.

**She gets her way (note taken)**
- YES. This is why I hired you. Found you. Same thing.
- See? Collaboration. I talk, you agree.
- Write that down. "The hero had a great idea." Mine.

**She doesn't (note refused)**
- Fine. Do it your way. It'll be worse. I'll say so in the credits.
- Noted. In my other notebook. The grudge one.
- No, no. You're the artist. I'm only the director.

**Sulking pool (rest of the scene after a refusal)**
- I'm not filming this. I'm filming that bush. The bush takes notes.
- Oh, was that good? I wasn't looking.
- Carry on. Don't mind the director. Nobody does.
- The bush is giving me so much right now.

**Wrap**
- That's a wrap! Everyone gets paid. "Everyone" is a term of art.
- You're going to be a star! My cut is the star part.
- Great work. I'll send you the bill.
- Here's your share. Don't count it. It's not for counting.

**Budget runs out (finale)**
- Small thing. We're out of money. Keep rolling!
- Pretend the box is a rat. I believe in you. I have to.
- This is a stylistic choice now. Say it with me.

**Premiere**
- A triumph. I'd like to thank myself. Thank you.
- Four stars! Out of a number I'll confirm later.

**Pause / quit**
- Why did everyone stop? Who called lunch?
- Walking off set? Your contract has thoughts about that.
- Fine. Go. I'll film the rats. They're easier to light.

## Director's Commentary pool

Plays only when commentary is on; slower cadence, longer lines allowed.

- Fun fact: this rat is played by three smaller rats.
- We shot this scene in one day. We shot everything in one day. Four, legally.
- The talent did all their own standing.
- That house isn't real. None of them are. One is a painting of a house. I won't say which.
- The original cut was nine hours. The studio lost six of them. I know where they are.
- This field was meant to be a volcano. Then I saw the price of a volcano.
- People ask what the film is about. It's about ninety minutes. It's eight. It's about eight minutes.
- Watch the goblin on the left. He's improvising. I docked him for it.
- The celery is real. That's where the money went.
- I wanted rain here. Rain wanted paying.
- Critics called the lecture scene "long." It's a lecture. That's the genre.
- Everything you see was my idea. The things you don't like were notes from the studio.
- There is no studio.
- Gary nearly quit here. Second rat from the left. Total professional. Didn't quit.

## The Production Ledger

Gross on top. Draw four to six deductions per wrap; always include the first two and Miscellaneous last. Amounts are computed so the hero's cut lands just above the floor.

| Line item | Note printed beside it |
| --- | --- |
| Director's fee | Standard. |
| Producer's fee | Also me. |
| Catering | There was no catering. That costs extra. |
| Wing wear and tear | They don't flap themselves. They do. Still. |
| Faerie dust (practical effects) | Organic. Locally sourced. From me. |
| Rat wrangler | The rats wrangled themselves. I supervised. |
| Celery, hero-grade | The good celery. |
| Sword storage | It's your sword. You're welcome. |
| Continuity | You started at level seven. That wasn't free. |
| Megaphone | It's a rolled-up leaf. A premium leaf. |
| Goblin dental plan | Somebody has to. You, apparently. |
| Emotional damages | Mine. From take three. |
| Reshoots | See: you. |
| Rounding | In my favour. For luck. |
| Miscellaneous | Don't worry about it. (Always the largest number.) |

Footer under Hero's Cut, one per wrap:
- "plus exposure"
- "before tax. I am also the tax."
- "non-negotiable. I checked with myself."

Premiere totals card: `FILM GROSS` / `DIRECTOR` / `HERO` / and a last row, `RATS: paid in celery`.

## Loot

Dark-fantasy names, practical stats. Only three stats are real (Lecture Radius, Lecture Cooldown, Move Speed); the rest of each card is flavour.

| Name | Stat | Flavour text |
| --- | --- | --- |
| The Unforgiving Blade of the Last Judgment | +12% Lecture Radius | "It has never been swung. It has never needed to be." |
| Pauldrons of I'm Not Angry | +10% Lecture Radius | "Just disappointed." |
| Greaves of Walking Briskly Toward the Problem | +8% Move Speed | "Running would be undignified." |
| Helm of Respected Queueing | -10% Lecture Cooldown | "No cutting. No exceptions. No mercy." |
| Doomplate of Reasonable Bedtimes | -12% Lecture Cooldown | "Forged in the fires of half past eight." |
| Gauntlets of Firm But Fair | +6% Lecture Radius | "The left one is Firm." |
| Toothbrush of Uncompromising Governance | -8% Lecture Cooldown | "Soft bristles. Hard truths." |
| Cloak of Dramatic Exits | +5% Move Speed | "No exits were found." |
| The Eternally Slightly Damp Napkin | +1% everything | "Nobody knows why. Nobody has asked." |
| Amulet of the Indoor Voice (Cursed) | -5% Lecture Radius | "Whispers the lecture. The rats lean in." |
| Ring of Having Read the Terms | +7% Move Speed | "The only one ever forged." |
| Celery of Unreasonable Authority | +15% Lecture Radius | "Crisp." |

Rarity labels, in order: Common, Uncommon, Rare, Legendary, **Director's Personal Collection** (appears, is always the one she keeps).

Stamp on her pick, one of: RESERVED FOR SEQUEL / PRODUCER'S PICK / EVIDENCE.

## Skills and levels

**The six points already spent** (greyed-out panel, levels 1 to 6, cannot be refunded):

| Passive | Rank | Tooltip |
| --- | --- | --- |
| Brooding | III | "Stand near a cliff 15% more meaningfully." |
| Cape Maintenance | II | "Your cape is 10% less damp." (You have no cape.) |
| Standing | I | "You may now stand." |

Footer on the panel: "Respec unavailable. Levels one to six tested poorly."

**Skill tooltips:**
- The Lecture: "Deals Resolve damage in a cone. Has never once been brief."
- Form A Line!: "Targets in a line stop and reconsider. Queueing is the foundation of civilisation."
- The Disappointed Sigh: "Everyone nearby slows down. They heard it. They know."
- The Last Judgment: "The sword rises. The heavens darken. Then you say what you came to say."

**Rank-up text:** "The Lecture II: now with examples." / "The Lecture III: now with slides." / "The Lecture IV: now with a question-and-answer session nobody may leave."

**Level-up card:** "LEVEL 8. You feel no different. You are, by 4%." Fenella: "A level! That's character growth. I'm billing for it."

**Level 10:** "LEVEL 10. Ultimate unlocked." Fenella: "Save it for the finale. I've already cut the trailer."

**Character sheet stat names:** Lecture Power, Radius, Cooldown Recovery, Move Speed, and one greyed-out row: "Strength: not applicable."

## Lecture bubbles

Default pool (jagged, all caps):
ABSOLUTELY NOT. / WE'VE TALKED ABOUT THIS. / I'M NOT ANGRY. / I'M DISAPPOINTED. / FORM A LINE! / MIND YOUR TEETH! / IS THIS WHO YOU WANT TO BE? / ELBOWS. / INDOOR VOICE! / EXCUSE ME. / PER MY LAST LECTURE / WE DO NOT EAT THE CROPS. / LOOK AT ME WHEN I'M LECTURING.

"Sadder" note pool (round bubbles, lower case): if it's not too much trouble / i just worry / no, you go ahead / i'm sure you had your reasons

Budget-ran-out pool: [SHOUTING] / [LECTURE] / [BUBBLE GOES HERE]

Damage numbers: show guilt, not damage. `+60 GUILT`. A strong hit: `EXCELLENT ADVICE!` Pacified: `REFORMED`.

Nuisance replies (tiny bubbles): ok / sorry / fine. / ...mum? / i'll floss / he started it

## Villagers (via the Candy Rock, while standing next to you)

**Farmer (Scene 1)**
- Offer: "Giant rats are eating my candy crops. I'd handle it, but you're wearing more armour."
- Brief: "Make them eat celery. Don't hurt them. They're terrible, but they're ours."
- Done: "They're chewing. I never thought I'd see it. +7 Approval."
- After: "Don't spend the approval all at once. You can't spend it."

**Dental Marshal (Scene 2)**
- Offer: "Fort Toothache has fallen. To plaque."
- Brief: "They've not brushed since the old king. Go in there and do what must be done. Twice a day."
- Done: "I can see myself in that goblin. Literally. His teeth are that clean."

**Almond Herder (Scene 3)**
- Offer: "The lollypop bugs are distracting my almonds. They're impressionable. They're almonds."
- Brief: "Shoo the bugs. Gather the herd. Count them. One of them lies about being counted."
- Done: "All home. You've the eyes of a killer and the heart of a shepherd."

The rock itself, when it rings: "Incoming rock." / "You have one new rock."

## Signs and set dressing (street between sets)

- SUGARBOROUGH. Population: Extras.
- SET 1: A FIELD (PLAYING ITSELF)
- QUIET PLEASE: RATS REHEARSING
- NO UNLICENSED WHIMSY
- THIS SIDE BACK (on plywood)
- HOT SET. DO NOT LICK.
- CRAFT SERVICES (CELERY)
- FORT TOOTHACHE. Est. whenever. Brushed: never.
- DELETED SCENE. KEEP OUT. (door is ajar)
- A director's chair two inches tall, labelled DIRECTOR. A second, full-sized, labelled TALENT, with a sign: DO NOT SIT.
- A lost-property box: one gauntlet, one celery.

## Production-note captions (corner of the screen, one per walk)

- This walk was restored for the Director's Cut. It was cut for pacing. You can see why.
- The following scene contains 37% more dental hygiene than the theatrical release.
- Colour corrected by a faerie with strong opinions.
- No rats were lectured more than necessary.
- Scene missing. Pretend it was great.

## Cards and menus

**Title screen:** THE DREAD LAND OF CANDY, with DIRECTOR'S CUT rubber-stamped over it at an angle. Subtitle: *A weekend project made by a director who should know better and machines that can't say no.* (Dylan, Nov 13) Buttons: `Roll Camera` (play), `Notes` (options), `Walk Off Set` (quit).

**Casting (title screen, R6):** a `Casting` line under `Roll Camera`, left / right picks who plays the hero; the caption names them. All four use the knight's skills and shout their battle cry on a cast.
- The Knight. Brought his own sword. Has notes about it.
- The Mage. Was promised a spell budget. Read the contract.
- The Archer. Brought her bow. Props still has the arrows.
- The Barbarian. Has been spoken to about his volume.

**Rating card (two seconds before the title):** "This film is rated S for Sugar. Contains mild peril, firm guidance, and one napkin."

**Quit confirm:** "Walk off set? Your contract has thoughts about that." Buttons: `Walk anyway` / `Fine`.

**Options:** the volume slider is labelled "Fenella". Under it, greyed out: "Budget: Low (locked)".

**Slate card detail lines** (under SCENE n - TAKE m): Director: F. Flicker. Camera: F. Flicker. Rats: Themselves.

**RETAKE! card subtitles:** "From the top." / "With feeling this time." / "Same, but good."

**Loading or transition tips:**
- Tip: The sword is for pointing.
- Tip: Rats respond to disappointment, not volume. Use both.
- Tip: Fenella's notes are optional. So is being paid.
- Tip: You are level seven. Nobody remembers why.
- Tip: There is no tip. The tip budget was reallocated.

**Achievement toasts (fake, no system behind them):**
- Stood Where Told
- First Take (on take three)
- Method Actor: stood still for 30 seconds
- Location Scout: walked behind the houses
- Achievement Unlocked: Achievement

## Endings

**THE DIRECTOR'S CUT** (took most notes). Premiere card: "A bold, uncompromising vision. Hers." Review quotes: "'Long.' — a rat" / "'I was in it!' — a different rat" / "'I've seen it nine times. I had to. I made it.' — F. Flicker"

**THE THEATRICAL RELEASE** (refused most notes). Premiere card: "Released against the director's wishes. She has asked that her name be removed." Review quotes: "'Tighter.' — a rat" / "'Where was the bigger goblin?' — F. Flicker, from the lobby" / "'Fine.' — the hero's agent (the hero has no agent)"

## Credits

Scroll, deadpan, real declarations at the end.

- Directed by Fenella Flicker
- Produced by Fenella Flicker
- Executive Producer: F. Flicker
- Written by: "It's more of a feeling"
- Starring: The Hero (as "Talent")
- Rats: Themselves
- Second Rat from the Left: Gary
- Goblins appear courtesy of their dentist
- Villager / Different Villager / Villager (Moustache)
- Catering: none
- Boom Operator: F. Flicker (see incident report)
- Stunt Double: Cardboard
- Celery Wrangler: position vacant
- Best Boy: a good boy
- Special Thanks: Fenella Flicker, accepting on her own behalf
- No rats were lectured more than necessary in the making of this film.
- Any resemblance to a game that existed is coincidental. This one never did.
- (In the Theatrical ending, every "Fenella Flicker" is replaced by "a faerie who wishes to remain nameless" except Boom Operator.)
- Then the real credits: Dylan, tools used, third-party assets, per `Docs/Declared.md`.

## Video cut (30 to 60 seconds, for the submission)

1. Narration interrupted (4 s). 2. Sword fakeout, full (6 s). 3. "Perfect. Again." (5 s). 4. CUT! prompt and the bigger goblin (8 s). 5. Ledger scrolling down to a tiny Hero's Cut (6 s). 6. Budget runs out (8 s). 7. Title stamp (3 s).
