#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// Native gameplay tags for the ability layer.
// Ported from Dylan's earlier project Spirits of Yendor (SOYGameplayTags), renamed and trimmed. See Docs/Declared.md.
namespace JamGameplayTags
{
	// SetByCaller magnitudes passed into GameplayEffectSpecs at runtime.
	MEGAJAM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Resolve_Base);

	// Team tags, used to filter who an ability may affect.
	MEGAJAM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Hero);
	MEGAJAM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Nuisance);

	// A target whose Resolve reached zero. Granted once; abilities skip targets that have it.
	MEGAJAM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pacified);

	// Granted while the Lecture ability is cooling down (see UJamGE_Cooldown).
	MEGAJAM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Lecture);
}
