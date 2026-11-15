#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "World/JamTour.h"
#include "JamTourPawn.generated.h"

class UCameraComponent;

/**
 * The stock-footage camera (AJamTourGameMode): flies a closed loop (JamTour) slowly over the open world, above
 * the ground and whatever stands on it, glancing at camps and film sets as it passes. No input, no collision.
 */
UCLASS(NotBlueprintable)
class MEGAJAM_API AJamTourPawn : public APawn
{
	GENERATED_BODY()

public:
	AJamTourPawn();
	virtual void Tick(float DeltaSeconds) override;

	/** Speed in uu/s along the loop, Height in uu above the highest ground just ahead, StartFraction 0..1 of the loop. */
	void Begin(float InSpeed, float InHeight, float StartFraction);

	float GetLoopLength() const { return Path.Length(); }
	int32 GetLaps() const { return Laps; }
	float GetDistance() const { return Distance; }

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	FJamTourPath Path;
	TArray<FJamTourStop> Stops;
	TArray<float> StopGround;   // lazily traced ground height under each stop; NaN until needed
	float Speed = 650.f;
	float Height = 380.f;
	float Distance = 0.f;
	float Age = 0.f;
	float Z = 0.f;
	float Boost = 0.f;          // extra height while something solid is ahead
	float LastGround = 0.f;
	int32 Laps = 0;
	int32 LastLoggedStop = INDEX_NONE;
	bool bStarted = false;
	FRotator View = FRotator::ZeroRotator;

	float GroundAt(const FVector2D& At);
	float StopGroundAt(int32 Index);
};
