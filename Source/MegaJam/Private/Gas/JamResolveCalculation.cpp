#include "Gas/JamResolveCalculation.h"
#include "Gas/JamResolveAttributeSet.h"
#include "Gas/JamGameplayTags.h"

UJamResolveCalculation::UJamResolveCalculation()
{
	ResolveReductionDef.AttributeToCapture = UJamResolveAttributeSet::GetResolveReductionAttribute();
	ResolveReductionDef.AttributeSource    = EGameplayEffectAttributeCaptureSource::Target;
	ResolveReductionDef.bSnapshot          = false;

	RelevantAttributesToCapture.Add(ResolveReductionDef);
}

float UJamResolveCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Base = Spec.GetSetByCallerMagnitude(JamGameplayTags::SetByCaller_Resolve_Base, /*WarnIfNotFound*/ false, 0.f);

	float Reduction = 0.f;
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	GetCapturedAttributeMagnitude(ResolveReductionDef, Spec, EvalParams, Reduction);

	return FMath::Max(0.f, Base - Reduction);
}
