#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "JamGameplayEffects.generated.h"

/**
 * Instant Resolve damage. The caster sets the base amount with
 * SetByCaller(JamGameplayTags::SetByCaller_Resolve_Base); the target's UJamResolveAttributeSet applies it.
 * Ported from Spirits of Yendor's USOYGE_Damage.
 */
UCLASS()
class MEGAJAM_API UJamGE_ResolveDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UJamGE_ResolveDamage();
};

/**
 * Lecture cooldown: a timed effect that grants Cooldown.Ability.Lecture to the caster.
 * New for the jam (Yendor's AoE ability never committed, so it had no cooldown).
 */
UCLASS()
class MEGAJAM_API UJamGE_LectureCooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UJamGE_LectureCooldown();

	static constexpr float DurationSeconds = 1.5f;
};

/** Cooldowns of skills 2 to 4 (T05a). Length is set per cast from the rank table; each grants its own tag. */
UCLASS(Abstract)
class MEGAJAM_API UJamGE_SkillCooldown : public UGameplayEffect
{
	GENERATED_BODY()

protected:
	void GrantTag(const FGameplayTag& Tag);
};

UCLASS()
class MEGAJAM_API UJamGE_LineCooldown : public UJamGE_SkillCooldown
{
	GENERATED_BODY()
public:
	UJamGE_LineCooldown();
};

UCLASS()
class MEGAJAM_API UJamGE_SighCooldown : public UJamGE_SkillCooldown
{
	GENERATED_BODY()
public:
	UJamGE_SighCooldown();
};

UCLASS()
class MEGAJAM_API UJamGE_JudgmentCooldown : public UJamGE_SkillCooldown
{
	GENERATED_BODY()
public:
	UJamGE_JudgmentCooldown();
};
