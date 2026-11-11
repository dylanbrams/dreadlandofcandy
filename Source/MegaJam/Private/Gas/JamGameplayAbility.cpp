#include "Gas/JamGameplayAbility.h"
#include "Gas/JamAoePatternLibrary.h"
#include "Gas/JamGameplayEffects.h"
#include "Gas/JamGameplayTags.h"
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
		Spec.Data->SetSetByCallerMagnitude(JamGameplayTags::SetByCaller_Resolve_Base, BaseDamage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);

		LastHitTargets.Add(Candidate);
		OnTargetHitNative.Broadcast(Candidate);
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
