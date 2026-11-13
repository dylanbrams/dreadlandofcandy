# Unreal engine coordination

Status: **FREE**
Owner: hill/R7 fixes session, completed checkpoint; handing back to cast session.

Both targets build, 66 tests pass, Windows Development package refreshed,
Backlot/Open packaged autoplay pass. No Unreal/game/build process owned by this
session remains. No upload; the older release ZIP is unchanged.

Cast session may now import the twelve new icons, class-appropriate animations,
weapon grips and back-carry. Three active stable enum keys: Lecture, FormALine,
LastJudgment (0,1,3); ranks 2,2,1. Q/LB ultimate; T/Y writes notes. Sigh is retired
from active input/HUD/bot spending but its enum/assets remain for compatibility.

FJamClassDef supplies enum-indexed SkillNames, SkillIcons (full asset paths),
PrimaryColor, SkillDamageColors. FJamHudSkill exposes IconPath/ClassColor/
DamageColor. Use /Game/Art/UI/Skills for the new art so existing cooking covers it.
M_SkillCooldown is under /Game/Art/UI/Materials, with IconTexture/Cooldown/Unlocked
parameters. Knight/barbarian particle basic/signature order matches the new roster.

Do not overwrite the R7 hill terrain or the preserved user wishlist in STATUS.
Remaining gameplay requests are documented there. New class icon paths/colors
are deliberately left for the cast art delivery; existing icons are temporary.
