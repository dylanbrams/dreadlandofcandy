#include "Gas/JamResolveAttributeSet.h"
#include "Gas/JamGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Actor.h"

UJamResolveAttributeSet::UJamResolveAttributeSet()
{
	InitResolve(100.f);
	InitMaxResolve(100.f);
	InitResolveReduction(0.f);
	InitIncomingResolveDamage(0.f);
}

void UJamResolveAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxResolveAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetResolveAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxResolve());
	}
	else if (Attribute == GetResolveReductionAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UJamResolveAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingResolveDamageAttribute())
	{
		const float Damage = GetIncomingResolveDamage();
		SetIncomingResolveDamage(0.f);
		if (Damage <= 0.f)
		{
			return;
		}

		SetResolve(FMath::Clamp(GetResolve() - Damage, 0.f, GetMaxResolve()));

		// Pacify exactly once: the tag is the "already done" marker.
		UAbilitySystemComponent& TargetASC = Data.Target;
		if (GetResolve() <= 0.f && !TargetASC.HasMatchingGameplayTag(JamGameplayTags::State_Pacified))
		{
			TargetASC.AddLooseGameplayTag(JamGameplayTags::State_Pacified);
			AActor* Avatar = TargetASC.GetAvatarActor();
			OnPacified.Broadcast(Avatar);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetMaxResolveAttribute())
	{
		SetResolve(FMath::Clamp(GetResolve(), 0.f, GetMaxResolve()));
	}
}
