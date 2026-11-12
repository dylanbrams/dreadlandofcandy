#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamArtReview.generated.h"

/**
 * -artreview (T20): every generated mesh lined up in a grid at its game height, each with its name, in front of the
 * hero, for checking scale and facing, and for Dylan's approval screenshots. A little red arrow shows +X (forward).
 */
UCLASS()
class MEGAJAM_API AJamArtReview : public AActor
{
	GENERATED_BODY()

public:
	AJamArtReview();
	void Build(const FVector& Origin);
};
