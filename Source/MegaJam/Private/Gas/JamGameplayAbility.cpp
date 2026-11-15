#include "Gas/JamGameplayAbility.h"
#include "Gas/JamAoePatternLibrary.h"
#include "Gas/JamClassShape.h"
#include "Gas/JamGameplayEffects.h"
#include "Gas/JamGameplayTags.h"
#include "Game/JamPlayerState.h"
#include "Game/JamNuisance.h"
#include "Game/JamGameMode.h"
#include "Game/JamHero.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Engine/World.h"

UJamGameplayAbility::UJamGameplayAbility()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	NetSecurityPolicy  = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

UJamGA_AoeAttack::UJamGA_AoeAttack()
{
	RequiredTargetTag = JamGameplayTags::Team_Nuisance;
	DamageEffect      = UJamGE_ResolveDamage::StaticClass();
}

FJamAoePatternParams UJamGA_AoeAttack::ResolveParams() const
{
	if (bUseDynamicParams) { return DynamicParams; }
	if (Pattern)           { return Pattern->Params; }
	return FJamAoePatternParams();
}

void UJamGA_AoeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo,
                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !DamageEffect)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Commit applies the cooldown (and cost, if any) exactly once per activation.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Strike(ActorInfo, Avatar->GetActorLocation(), Avatar->GetActorForwardVector());
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UJamGA_AoeAttack::Strike(const FGameplayAbilityActorInfo* ActorInfo, const FVector& Origin, const FVector& Forward)
{
	LastHitTargets.Reset();

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	if (!Avatar || !SourceASC) { return; }

	const FJamAoePatternParams Params = ResolveParams();

	TArray<AActor*> Ignored;
	Ignored.Add(Avatar);
	TArray<AActor*> Candidates;
	UJamAoePatternLibrary::FindActorsInPattern(GetWorld(), Params, Origin, Forward, nullptr, Ignored, Candidates);

	for (AActor* Candidate : Candidates)
	{
		const IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(Candidate);
		UAbilitySystemComponent* TargetASC = Interface ? Interface->GetAbilitySystemComponent() : nullptr;
		if (!TargetASC) { continue; }
		if (RequiredTargetTag.IsValid() && !TargetASC->HasMatchingGameplayTag(RequiredTargetTag)) { continue; }
		if (TargetASC->HasMatchingGameplayTag(JamGameplayTags::State_Pacified)) { continue; }

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddInstigator(Avatar, Avatar);
		FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffect, GetAbilityLevel(), Context);
		if (!Spec.IsValid()) { continue; }
		Spec.Data->SetSetByCallerMagnitude(JamGameplayTags::SetByCaller_Resolve_Base, ResolveDamage());
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);

		LastHitTargets.Add(Candidate);
		OnTargetHitNative.Broadcast(Candidate);
		if (AJamNuisance* Nuisance = Cast<AJamNuisance>(Candidate))
		{
			Nuisance->Recoil(Origin);
			if (AJamGameMode* GM = Avatar->GetWorld() ? Avatar->GetWorld()->GetAuthGameMode<AJamGameMode>() : nullptr) { GM->NotifyAttackHit(Cast<AJamHero>(Avatar), Nuisance); }
		}
	}
}

UJamGA_LectureCone::UJamGA_LectureCone()
{
	bUseDynamicParams = true;
	DynamicParams.Shape            = EJamAoeShape::Cone;
	DynamicParams.Radius           = 800.f;
	DynamicParams.ConeHalfAngleDeg = 35.f;
	BaseDamage = 60.f;
	CooldownGameplayEffectClass = UJamGE_LectureCooldown::StaticClass();
}

namespace
{
	int32 OwnerCast(const UGameplayAbility* Ability)
	{
		const AJamHero* Hero = Cast<AJamHero>(Ability->GetAvatarActorFromActorInfo());
		return Hero ? Hero->GetCastIndex() : 0;   // the test rig casts as the knight
	}

	const FJamProgression* OwnerProgression(const UGameplayAbility* Ability)
	{
		const AJamPlayerState* PS = Cast<AJamPlayerState>(Ability->GetOwningActorFromActorInfo());
		return PS ? &PS->GetProgression() : nullptr;
	}
}

FJamAoePatternParams UJamGA_LectureCone::ResolveParams() const
{
	FJamAoePatternParams Params = Super::ResolveParams();
	if (const FJamProgression* Prog = OwnerProgression(this))
	{
		const float Radius = Prog->GetSkillNumbers(EJamSkill::Lecture).Radius;
		if (Radius > 0.f) { Params.Radius = Radius; }
	}
	JamClassShape::Adjust(OwnerCast(this), EJamSkill::Lecture, Params);
	return Params;
}

float UJamGA_LectureCone::ResolveDamage() const
{
	if (const FJamProgression* Prog = OwnerProgression(this))
	{
		const float Damage = Prog->GetSkillNumbers(EJamSkill::Lecture).Damage;
		if (Damage > 0.f) { return Damage; }
	}
	return BaseDamage;
}

void UJamGA_LectureCone::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const FJamProgression* Prog = OwnerProgression(this);
	const float Seconds = Prog ? Prog->GetSkillNumbers(EJamSkill::Lecture).Cooldown : 0.f;
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!Prog || Seconds <= 0.f || !CooldownGE)
	{
		Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		return;
	}
	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CooldownGE->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (Spec.IsValid())
	{
		Spec.Data->SetDuration(Seconds, true);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
	}
}

// ---- Skills 2 to 4 (T05a) ---------------------------------------------------------------------------------------

FJamSkillRank UJamGA_Skill::Numbers() const
{
	const FJamProgression* Prog = OwnerProgression(this);
	return Prog ? Prog->GetSkillNumbers(Skill) : JamProgression::RankData(Skill, 1);
}

bool UJamGA_Skill::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) { return false; }
	const AJamPlayerState* PS = ActorInfo ? Cast<AJamPlayerState>(ActorInfo->OwnerActor.Get()) : nullptr;
	return !PS || PS->GetProgression().IsUnlocked(Skill);   // the rig (no PlayerState) may cast anything
}

FJamAoePatternParams UJamGA_Skill::ResolveParams() const
{
	const FJamSkillRank N = Numbers();
	FJamAoePatternParams P;
	if (Skill == EJamSkill::FormALine)
	{
		P.Shape = EJamAoeShape::Line;
		P.Radius = N.Length;
		P.Width = N.Width * 0.5f;   // the table's width is the whole line; the shape takes half
	}
	else
	{
		P.Shape = EJamAoeShape::Circle;   // the Sigh and the Judgment ring the hero
		P.Radius = N.Radius;
	}
	JamClassShape::Adjust(OwnerCast(this), Skill, P);
	return P;
}

float UJamGA_Skill::ResolveDamage() const
{
	return Numbers().Damage;
}

void UJamGA_Skill::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	const float Seconds = Numbers().Cooldown;
	if (!CooldownGE || Seconds <= 0.f) { return; }
	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CooldownGE->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (Spec.IsValid())
	{
		Spec.Data->SetDuration(Seconds, true);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
	}
}

void UJamGA_Skill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	// The status rides on the hit: the line stops them, the sigh slows them. Pacified targets keep sitting.
	const FJamSkillRank N = Numbers();
	for (const TWeakObjectPtr<AActor>& Hit : LastHitTargets)
	{
		AJamNuisance* Target = Cast<AJamNuisance>(Hit.Get());
		if (!Target || N.StatusSeconds <= 0.f) { continue; }
		if (Skill == EJamSkill::FormALine) { Target->Fluster(N.StatusSeconds); }
		else if (Skill == EJamSkill::DisappointedSigh) { Target->Slow(N.SlowFactor, N.StatusSeconds); }
	}
}

UJamGA_FormALine::UJamGA_FormALine()
{
	Skill = EJamSkill::FormALine;
	CooldownGameplayEffectClass = UJamGE_LineCooldown::StaticClass();
}

UJamGA_DisappointedSigh::UJamGA_DisappointedSigh()
{
	Skill = EJamSkill::DisappointedSigh;
	CooldownGameplayEffectClass = UJamGE_SighCooldown::StaticClass();
}

UJamGA_LastJudgment::UJamGA_LastJudgment()
{
	Skill = EJamSkill::LastJudgment;
	CooldownGameplayEffectClass = UJamGE_JudgmentCooldown::StaticClass();
}
