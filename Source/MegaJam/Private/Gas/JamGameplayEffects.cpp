#include "Gas/JamGameplayEffects.h"
#include "Gas/JamResolveAttributeSet.h"
#include "Gas/JamResolveCalculation.h"
#include "Gas/JamGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UJamGE_ResolveDamage::UJamGE_ResolveDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Mod;
	Mod.Attribute  = UJamResolveAttributeSet::GetIncomingResolveDamageAttribute();
	Mod.ModifierOp = EGameplayModOp::Additive;

	FCustomCalculationBasedFloat Calc;
	Calc.CalculationClassMagnitude = UJamResolveCalculation::StaticClass();
	Calc.Coefficient.SetValue(1.f);
	Calc.PreMultiplyAdditiveValue.SetValue(0.f);
	Calc.PostMultiplyAdditiveValue.SetValue(0.f);
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(Calc);

	Modifiers.Add(Mod);
}

UJamGE_LectureCooldown::UJamGE_LectureCooldown()
{
	DurationPolicy    = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(DurationSeconds));

	UTargetTagsGameplayEffectComponent* TagsComponent =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("GrantedTags"));
	FInheritedTagContainer Granted;
	Granted.Added.AddTag(JamGameplayTags::Cooldown_Lecture);
	TagsComponent->SetAndApplyTargetTagChanges(Granted);
	GEComponents.Add(TagsComponent);
}

void UJamGE_SkillCooldown::GrantTag(const FGameplayTag& Tag)
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.f));   // replaced per cast
	UTargetTagsGameplayEffectComponent* TagsComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("GrantedTags"));
	FInheritedTagContainer Granted;
	Granted.Added.AddTag(Tag);
	TagsComponent->SetAndApplyTargetTagChanges(Granted);
	GEComponents.Add(TagsComponent);
}

UJamGE_LineCooldown::UJamGE_LineCooldown() { GrantTag(JamGameplayTags::Cooldown_Line); }
UJamGE_SighCooldown::UJamGE_SighCooldown() { GrantTag(JamGameplayTags::Cooldown_Sigh); }
UJamGE_JudgmentCooldown::UJamGE_JudgmentCooldown() { GrantTag(JamGameplayTags::Cooldown_Judgment); }
