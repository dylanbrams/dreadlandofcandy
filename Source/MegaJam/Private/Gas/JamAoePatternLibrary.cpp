#include "Gas/JamAoePatternLibrary.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"

namespace
{
	// Express Point in the pattern's local frame: X = forward, Y = right. Z is ignored.
	FVector2D ToLocal(const FVector& Origin, const FVector& Forward, const FVector& Point)
	{
		const FVector Delta = Point - Origin;
		FVector Fwd = Forward; Fwd.Z = 0.f;
		if (!Fwd.Normalize()) { Fwd = FVector::ForwardVector; }
		const FVector Right(-Fwd.Y, Fwd.X, 0.f);
		return FVector2D(FVector::DotProduct(Delta, Fwd), FVector::DotProduct(Delta, Right));
	}

	// Returns true if LocalPoint falls inside any of Holes.
	bool InsideAnyHole(const FVector2D& LocalPoint, const TArray<FJamAoeHole>& Holes)
	{
		for (const FJamAoeHole& H : Holes)
		{
			if (FVector2D::DistSquared(LocalPoint, H.LocalOffset) <= H.Radius * H.Radius) { return true; }
		}
		return false;
	}
}

float UJamAoePatternLibrary::GetBoundingRadius(const FJamAoePatternParams& P)
{
	switch (P.Shape)
	{
	case EJamAoeShape::Circle:
	case EJamAoeShape::Ring:
	case EJamAoeShape::Cone:
	case EJamAoeShape::Hex:
	case EJamAoeShape::Spiral:
	case EJamAoeShape::Cross:
	case EJamAoeShape::Diagonal:
		return FMath::Max(P.Radius, P.Width);
	case EJamAoeShape::Line:
		return FMath::Sqrt(P.Radius * P.Radius + P.Width * P.Width);
	case EJamAoeShape::Plus:
		return P.TileSize * 1.5f;
	case EJamAoeShape::Square:
		return P.SquareSize * 0.7071f + 1.f;  // half-diagonal
	case EJamAoeShape::Custom:
	{
		float Max = 0.f;
		for (const FIntPoint& C : P.CustomGridCells)
		{
			Max = FMath::Max(Max, FMath::Sqrt((float)(C.X*C.X + C.Y*C.Y)));
		}
		return Max * P.TileSize + P.TileSize * 0.5f;
	}
	}
	return FMath::Max(1.f, P.Radius);
}

bool UJamAoePatternLibrary::TestPoint(const FJamAoePatternParams& P, FVector Origin, FVector Forward, FVector Point)
{
	if (!TestPointIgnoringHoles(P, Origin, Forward, Point)) { return false; }
	if (P.StaticHoles.Num() == 0)                           { return true; }
	return !InsideAnyHole(ToLocal(Origin, Forward, Point), P.StaticHoles);
}

bool UJamAoePatternLibrary::TestPointIgnoringHoles(const FJamAoePatternParams& P, FVector Origin, FVector Forward, FVector Point)
{
	const FVector2D L = ToLocal(Origin, Forward, Point);
	const float D2 = L.X * L.X + L.Y * L.Y;

	switch (P.Shape)
	{
	case EJamAoeShape::Circle:
		return D2 <= P.Radius * P.Radius;

	case EJamAoeShape::Ring:
		return D2 <= P.Radius * P.Radius && D2 >= P.InnerRadius * P.InnerRadius;

	case EJamAoeShape::Cone:
	{
		if (D2 > P.Radius * P.Radius || D2 < KINDA_SMALL_NUMBER) { return D2 < KINDA_SMALL_NUMBER; }
		const float Cos = L.X / FMath::Sqrt(D2);
		return Cos >= FMath::Cos(FMath::DegreesToRadians(P.ConeHalfAngleDeg));
	}

	case EJamAoeShape::Line:
		// Forward rectangle in front of the caster: 0 <= X <= Radius, |Y| <= Width.
		return L.X >= 0.f && L.X <= P.Radius && FMath::Abs(L.Y) <= P.Width;

	case EJamAoeShape::Cross:
		// Two perpendicular bars centered on origin.
		return (FMath::Abs(L.X) <= P.Radius && FMath::Abs(L.Y) <= P.Width)
		    || (FMath::Abs(L.Y) <= P.Radius && FMath::Abs(L.X) <= P.Width);

	case EJamAoeShape::Diagonal:
	{
		// Rotated cross at 45°.
		const float U = (L.X + L.Y) * 0.7071f;
		const float V = (L.X - L.Y) * 0.7071f;
		return (FMath::Abs(U) <= P.Radius && FMath::Abs(V) <= P.Width)
		    || (FMath::Abs(V) <= P.Radius && FMath::Abs(U) <= P.Width);
	}

	case EJamAoeShape::Plus:
	{
		// Origin tile + 4 cardinal-adjacent tiles.
		const float Half = P.TileSize * 0.5f;
		const float Cx = FMath::Abs(L.X), Cy = FMath::Abs(L.Y);
		const bool bCenter   = Cx <= Half && Cy <= Half;
		const bool bForward  = L.X >= Half && L.X <= P.TileSize * 1.5f && Cy <= Half;
		const bool bBackward = L.X <= -Half && L.X >= -P.TileSize * 1.5f && Cy <= Half;
		const bool bRight    = L.Y >= Half && L.Y <= P.TileSize * 1.5f && Cx <= Half;
		const bool bLeft     = L.Y <= -Half && L.Y >= -P.TileSize * 1.5f && Cx <= Half;
		return bCenter || bForward || bBackward || bRight || bLeft;
	}

	case EJamAoeShape::Square:
	{
		const float Half = P.SquareSize * 0.5f;
		return FMath::Abs(L.X) <= Half && FMath::Abs(L.Y) <= Half;
	}

	case EJamAoeShape::Hex:
	{
		// Six tiles ringing the origin at radius P.Radius. Tolerance = P.Width (treat as point-tile size).
		for (int32 k = 0; k < 6; ++k)
		{
			const float Theta = (PI / 3.f) * k;
			const FVector2D HC(FMath::Cos(Theta) * P.Radius, FMath::Sin(Theta) * P.Radius);
			if (FVector2D::DistSquared(L, HC) <= P.Width * P.Width) { return true; }
		}
		return false;
	}

	case EJamAoeShape::Spiral:
	{
		// Sample-time check: ring at radius proportional to current ability stack/time would normally be supplied
		// by the ability via Radius. Here we treat as a thin ring of half-width=Width around Radius.
		const float D = FMath::Sqrt(D2);
		return FMath::Abs(D - P.Radius) <= P.Width;
	}

	case EJamAoeShape::Custom:
	{
		const float Tile = FMath::Max(1.f, P.TileSize);
		// Map to nearest grid cell in local coords.
		const int32 Gx = FMath::RoundToInt(L.X / Tile);
		const int32 Gy = FMath::RoundToInt(L.Y / Tile);
		const FIntPoint Cell(Gx, Gy);
		// Point must also be within the cell's half-tile box around the cell center.
		if (FMath::Abs(L.X - Gx * Tile) > Tile * 0.5f) { return false; }
		if (FMath::Abs(L.Y - Gy * Tile) > Tile * 0.5f) { return false; }
		return P.CustomGridCells.Contains(Cell);
	}
	}
	return false;
}

void UJamAoePatternLibrary::ResolveProceduralHoles(FJamAoePatternParams& P, int32 Seed)
{
	if (P.ProceduralHoleCount <= 0) { return; }

	const float Bound = GetBoundingRadius(P);
	const float HoleR = FMath::Max(1.f, P.ProceduralHoleRadius);
	const float MinSepSq = P.MinHoleSeparation * P.MinHoleSeparation;
	const FVector Origin  = FVector::ZeroVector;
	const FVector Forward = FVector::ForwardVector;

	FRandomStream R(Seed != 0 ? Seed : FMath::Rand());
	const int32 Attempts = P.ProceduralHoleCount * 25;
	int32 Added = 0;

	for (int32 i = 0; i < Attempts && Added < P.ProceduralHoleCount; ++i)
	{
		// Uniform-area sample inside the bounding circle.
		const float Theta = R.FRandRange(0.f, 2.f * PI);
		const float Rad   = Bound * FMath::Sqrt(R.FRand());
		const FVector2D Local(FMath::Cos(Theta) * Rad, FMath::Sin(Theta) * Rad);

		// Convert local back to world for the shape test (origin at world zero, forward = +X here).
		const FVector WorldPt(Local.X, Local.Y, 0.f);
		if (!TestPointIgnoringHoles(P, Origin, Forward, WorldPt)) { continue; }

		// Separation check against existing holes.
		bool bClear = true;
		for (const FJamAoeHole& H : P.StaticHoles)
		{
			if (FVector2D::DistSquared(Local, H.LocalOffset) < MinSepSq) { bClear = false; break; }
		}
		if (!bClear) { continue; }

		FJamAoeHole NewHole;
		NewHole.LocalOffset = Local;
		NewHole.Radius      = HoleR;
		P.StaticHoles.Add(NewHole);
		++Added;
	}
}

void UJamAoePatternLibrary::FindActorsInPattern(UWorld* World, const FJamAoePatternParams& P,
                                                FVector Origin, FVector Forward, TSubclassOf<AActor> OptionalClass,
                                                const TArray<AActor*>& Ignored, TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (!World) { return; }

	const float Bound = GetBoundingRadius(P);
	if (Bound <= 0.f) { return; }

	// Broad phase: dynamic Pawn overlap. Project-specific channels can subclass this if needed.
	FCollisionQueryParams Q(SCENE_QUERY_STAT(JamAoePattern), /*bTraceComplex*/ false);
	for (AActor* A : Ignored) { if (A) { Q.AddIgnoredActor(A); } }

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps, Origin, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Bound), Q);

	TSet<AActor*> Seen;
	for (const FOverlapResult& R : Overlaps)
	{
		AActor* A = R.GetActor();
		if (!A || Seen.Contains(A)) { continue; }
		if (OptionalClass && !A->IsA(OptionalClass)) { continue; }
		if (!TestPoint(P, Origin, Forward, A->GetActorLocation())) { continue; }
		Seen.Add(A);
		OutActors.Add(A);
	}
}
