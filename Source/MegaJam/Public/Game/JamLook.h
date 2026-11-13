#pragma once

#include "CoreMinimal.h"

class UWorld;

/**
 * The film's look (T20 lighting and grade). The map is still the engine template, so this adjusts its lights
 * at runtime and adds one unbound post-process volume: warmer, a little more saturated and contrasted, shadows
 * lifted toward candy violet, soft bloom, a vignette like an old lens. `-nograde` leaves the template look.
 */
namespace JamLook
{
	MEGAJAM_API void Apply(UWorld* World, bool bOpen = false);
	/** T21: the money runs out, and so does the colour grading. */
	MEGAJAM_API void BudgetOut(UWorld* World);
}
