#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Gas/JamAoePattern.h"
#include "Film/JamProgression.h"
#include "JamGameplayAbility.generated.h"

class UGameplayEffect;

// Project base for abilities. Instanced per actor, local-only (solo-first); see UJamAbilitySystemComponent.
// Ported from Spirits of Yendor (USOYGameplayAbility).
UCLASS(Abstract)
class MEGAJAM_API UJamGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UJamGameplayAbility();
};

/**
 * Single-shot area ability: stamp a pattern at the caster, find the nuisances inside it, apply a Resolve
 * damage effect to each. Skips anything already Pacified. Commits the ability first, so cooldown applies.
 *
 * Ported from Spirits of Yendor's USOYGA_AoeAttack with the telegraph/footprint-actor path, the Mass
 * grunt-damage queue and the Niagara/decal cosmetics removed.
 */
UCLASS(Abstract)
class MEGAJAM_API UJamGA_AoeAttack : public UJamGameplayAbility
{
	GENERATED_BODY()

public:
	UJamGA_AoeAttack();

	/** The shape to stamp. Subclasses may instead fill DynamicParams and set bUseDynamicParams. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AoE")
	TObjectPtr<UJamAoePatternDataAsset> Pattern;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AoE")
	FJamAoePatternParams DynamicParams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AoE")
	bool bUseDynamicParams = false;

	/** Applied to each target; must write IncomingResolveDamage (UJamGE_ResolveDamage). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AoE")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** SetByCaller Resolve damage per target, before the target's ResolveReduction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AoE")
	float BaseDamage = 25.f;

	/** Only targets whose ASC carries this tag are affected. Empty = no team filter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AoE")
	FGameplayTag RequiredTargetTag;

	/** Fired once per target actually hit, on the ability instance (for combat feedback hooks). */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetHit, AActor* /*Target*/);
	FOnTargetHit OnTargetHitNative;

	/** Targets hit by the most recent activation. Read by tests and the debug rig. */
	TArray<TWeakObjectPtr<AActor>> LastHitTargets;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

protected:
	virtual FJamAoePatternParams ResolveParams() const;
	virtual float ResolveDamage() const { return BaseDamage; }
	void Strike(const FGameplayAbilityActorInfo* ActorInfo, const FVector& Origin, const FVector& Forward);
};

/**
 * Prototype of the knight's Lecture: a wide frontal cone that costs Resolve instead of health.
 * Cone 800 uu long, 35 degree half angle, 60 Resolve per hit, 1.5 s cooldown.
 * The VFX, sword fakeout and comic bubbles hang off this later.
 */
UCLASS()
class MEGAJAM_API UJamGA_LectureCone : public UJamGA_AoeAttack
{
	GENERATED_BODY()

public:
	UJamGA_LectureCone();

protected:
	/** Radius and power come from the owner's progression (rank and loot) when it has one. */
	virtual FJamAoePatternParams ResolveParams() const override;
	virtual float ResolveDamage() const override;
	/** The cooldown's length comes from the progression too (rank, loot, Cooldown Recovery). */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
};

/**
 * Skills 2 to 4 (BUILD_SPEC 3c, T05a). Shape, damage, status and cooldown come from the owner's progression at
 * the skill's current rank; the ability refuses until the skill is unlocked. Hits apply the skill's status to
 * nuisances: Form A Line stops them (Flustered), the Sigh slows them.
 */
UCLASS(Abstract)
class MEGAJAM_API UJamGA_Skill : public UJamGA_AoeAttack
{
	GENERATED_BODY()

public:
	EJamSkill Skill = EJamSkill::FormALine;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual FJamAoePatternParams ResolveParams() const override;
	virtual float ResolveDamage() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	FJamSkillRank Numbers() const;
};

UCLASS()
class MEGAJAM_API UJamGA_FormALine : public UJamGA_Skill
{
	GENERATED_BODY()
public:
	UJamGA_FormALine();
};

UCLASS()
class MEGAJAM_API UJamGA_DisappointedSigh : public UJamGA_Skill
{
	GENERATED_BODY()
public:
	UJamGA_DisappointedSigh();
};

UCLASS()
class MEGAJAM_API UJamGA_LastJudgment : public UJamGA_Skill
{
	GENERATED_BODY()
public:
	UJamGA_LastJudgment();
};
