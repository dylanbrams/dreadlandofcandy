#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "JamResolveAttributeSet.generated.h"

#define JAM_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Resolve: a nuisance's capacity to keep being a nuisance. The nonviolent stand-in for hit points.
 * At zero the owner is Pacified (State.Pacified tag, once), never destroyed.
 *
 * Adapted from Spirits of Yendor's USOYHealthAttributeSet: same meta-attribute pattern
 * (effects write IncomingResolveDamage, PostGameplayEffectExecute applies it), with the death
 * broadcast replaced by a once-only pacify and the pacifist-mode knockback intercept removed.
 */
UCLASS()
class MEGAJAM_API UJamResolveAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UJamResolveAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Resolve")
	FGameplayAttributeData Resolve;
	JAM_ATTRIBUTE_ACCESSORS(UJamResolveAttributeSet, Resolve)

	UPROPERTY(BlueprintReadOnly, Category = "Resolve")
	FGameplayAttributeData MaxResolve;
	JAM_ATTRIBUTE_ACCESSORS(UJamResolveAttributeSet, MaxResolve)

	/** Flat amount subtracted from every hit by UJamResolveCalculation. Default 0. */
	UPROPERTY(BlueprintReadOnly, Category = "Resolve")
	FGameplayAttributeData ResolveReduction;
	JAM_ATTRIBUTE_ACCESSORS(UJamResolveAttributeSet, ResolveReduction)

	/** Meta attribute: damage effects write here, PostGameplayEffectExecute consumes and zeros it. */
	UPROPERTY(BlueprintReadOnly, Category = "Resolve")
	FGameplayAttributeData IncomingResolveDamage;
	JAM_ATTRIBUTE_ACCESSORS(UJamResolveAttributeSet, IncomingResolveDamage)

	/** Fires once, when Resolve first reaches zero. Argument is the pacified avatar. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPacified, AActor* /*PacifiedActor*/);
	FOnPacified OnPacified;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
