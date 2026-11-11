#include "Gas/JamAbilitySystemComponent.h"

UJamAbilitySystemComponent::UJamAbilitySystemComponent()
{
	// UE 5.8 ensures if SetIsReplicated is used during construction (Yendor's 5.5 code did).
	SetIsReplicatedByDefault(false);
}
