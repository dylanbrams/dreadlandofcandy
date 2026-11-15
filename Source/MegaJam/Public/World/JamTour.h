#pragma once

#include "CoreMinimal.h"
#include "World/JamWorld.h"

/**
 * The flight plan for the stock-footage tour (AJamTourGameMode). Headless: no UObjects, no world.
 * A closed loop that leaves the village square, sweeps out along each biome wedge on one side of its centre line
 * and back on the other, and returns to the square between wedges, so every camp, film set and biome giver is
 * passed within a camp radius or two. Long legs get gentle sideways meanders so the camera sees country between
 * places, not a ruler line.
 */

struct MEGAJAM_API FJamTourStop
{
	FName Id;
	FVector2D Position = FVector2D::ZeroVector;
	float Radius = 0.f;
	bool bVillage = false;
};

/** A closed centripetal Catmull-Rom curve through the waypoints, sampled by arc length (uu). */
class MEGAJAM_API FJamTourPath
{
public:
	void Build(const TArray<FVector2D>& Waypoints);
	bool IsValid() const { return Points.Num() > 2 && Total > 1.f; }
	float Length() const { return Total; }
	/** Distance wraps: 0 and Length() are the same point, negative distances count back from the end. */
	FVector2D PointAt(float Distance) const;
	FVector2D DirectionAt(float Distance) const;
	const TArray<FVector2D>& Samples() const { return Points; }

private:
	TArray<FVector2D> Points;   // dense polyline, last sample == first
	TArray<float> Cumulative;   // arc length at each sample
	float Total = 0.f;
};

namespace JamTour
{
	/** The loop's corners, village square first. Inside the world bounds (inset), no two in a row closer than 500 uu. */
	MEGAJAM_API TArray<FVector2D> Waypoints(const FJamWorldDef& World);
	/** Places the camera slows for and glances at: the village, every camp, film set and biome giver. */
	MEGAJAM_API TArray<FJamTourStop> Stops(const FJamWorldDef& World);
	/** 0.5 right at a stop (and anywhere in the village) rising to 1 at 2500 uu from the edge of the nearest one. */
	MEGAJAM_API float SpeedScale(const TArray<FJamTourStop>& Stops, const FVector2D& At);
	/** 0..1: how strongly the camera should glance at the stop (not the village) nearest to At. Index is that stop, or INDEX_NONE. */
	MEGAJAM_API float GazeWeight(const TArray<FJamTourStop>& Stops, const FVector2D& At, int32& OutStop);
}
