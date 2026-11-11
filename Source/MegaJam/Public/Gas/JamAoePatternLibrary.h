#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Gas/JamAoePattern.h"
#include "JamAoePatternLibrary.generated.h"

class AActor;
class UWorld;

// Geometry helpers + actor-collecting queries for AoE patterns.
//
// Footprint query model: each pattern is reduced to a point-containment test (TestPoint), and FindActorsInPattern
// walks a candidate set (via UWorld overlaps for the pattern's bounding circle) and tests each candidate's location.
// This keeps the math in one place and makes adding new shapes a 5-line case in TestPoint.
UCLASS()
class MEGAJAM_API UJamAoePatternLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// True if Point (world XY) is inside Params when stamped at Origin facing Forward (2D math; Z ignored).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Jam|AoE")
	static bool TestPoint(const FJamAoePatternParams& Params, FVector Origin, FVector Forward, FVector Point);

	// Returns the maximum reach of the pattern — used to size the broad-phase overlap sphere.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Jam|AoE")
	static float GetBoundingRadius(const FJamAoePatternParams& Params);

	// Appends ProceduralHoleCount holes into Params.StaticHoles, placed inside the shape with MinHoleSeparation
	// respected. Best-effort: gives up after a fixed attempt budget if the shape is too dense.
	UFUNCTION(BlueprintCallable, Category="Jam|AoE")
	static void ResolveProceduralHoles(UPARAM(ref) FJamAoePatternParams& Params, int32 Seed = 0);

	// Same as TestPoint, but ignores hole rejection. Used by the decal material driver to know if a pixel is
	// inside the gross shape (separate from whether it's inside a hole).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Jam|AoE")
	static bool TestPointIgnoringHoles(const FJamAoePatternParams& Params, FVector Origin, FVector Forward, FVector Point);

	// Collects actors of OptionalClass (or AActor if null) inside the pattern. Excludes Ignored.
	// Caller is responsible for additional filtering (team, tag, line-of-sight, etc.).
	UFUNCTION(BlueprintCallable, Category="Jam|AoE")
	static void FindActorsInPattern(UWorld* World,
	                                const FJamAoePatternParams& Params,
	                                FVector Origin,
	                                FVector Forward,
	                                TSubclassOf<AActor> OptionalClass,
	                                const TArray<AActor*>& Ignored,
	                                TArray<AActor*>& OutActors);
};
