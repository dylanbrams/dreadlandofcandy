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
