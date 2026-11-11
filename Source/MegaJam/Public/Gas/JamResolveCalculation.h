#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "JamResolveCalculation.generated.h"

/**
 * Magnitude calculation for the Resolve damage effect:
 *   max(0, SetByCaller(SetByCaller.Resolve.Base) - Target.ResolveReduction)
 * Ported from Spirits of Yendor's USOYDamageCalculation.
 */
UCLASS()
class MEGAJAM_API UJamResolveCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UJamResolveCalculation();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition ResolveReductionDef;
};
