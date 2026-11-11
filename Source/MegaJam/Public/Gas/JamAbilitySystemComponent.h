#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "JamAbilitySystemComponent.generated.h"

// Project ASC. Solo-first: not replicated. If co-op happens, this is the one place to switch it on
// (SetIsReplicated(true) + a replication mode) and the abilities' net policies move with it.
// Ported from Spirits of Yendor (USOYAbilitySystemComponent). See Docs/Declared.md.
UCLASS()
class MEGAJAM_API UJamAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UJamAbilitySystemComponent();
};
