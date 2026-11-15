#include "World/JamTour.h"
#include "Algo/BinarySearch.h"
#include "Algo/Sort.h"

namespace JamTourRoute   // named, not anonymous: the game target is a unity build
{
	constexpr float EdgeInset = 800.f;        // waypoints stay this far inside the world bounds
	constexpr float MinGap = 500.f;           // two waypoints closer than this are one
	constexpr float MeanderAbove = 8000.f;    // legs longer than this get sideways midpoints
	constexpr float MeanderEvery = 6000.f;
	constexpr float MeanderBulge = 0.12f;     // the bulge's size as a fraction of the leg
	constexpr int32 StepsPerSegment = 48;

	FVector2D Inside(const FBox2D& Bounds, const FVector2D& P)
	{
		return FVector2D(FMath::Clamp(P.X, Bounds.Min.X + EdgeInset, Bounds.Max.X - EdgeInset),
			FMath::Clamp(P.Y, Bounds.Min.Y + EdgeInset, Bounds.Max.Y - EdgeInset));
	}

	void Push(TArray<FVector2D>& Out, const FVector2D& P)
	{
		if (Out.IsEmpty() || FVector2D::Distance(Out.Last(), P) >= MinGap) { Out.Add(P); }
	}

	/** From Out.Last() to To: sideways midpoints on a long leg, then To itself when bAppendTarget. */
	void AddLeg(TArray<FVector2D>& Out, const FBox2D& Bounds, const FVector2D& To, bool bAppendTarget, bool& bFlip)
	{
		const FVector2D From = Out.Last();
		const float Length = FVector2D::Distance(From, To);
		if (Length > MeanderAbove)
		{
			const int32 N = FMath::Max(1, FMath::FloorToInt(Length / MeanderEvery));
			const FVector2D Dir = (To - From) / Length;
			const FVector2D Side(-Dir.Y, Dir.X);
			for (int32 K = 1; K <= N; ++K)
			{
				const float T = float(K) / float(N + 1);
				const float Sign = ((K & 1) ? 1.f : -1.f) * (bFlip ? -1.f : 1.f);
				Push(Out, Inside(Bounds, FMath::Lerp(From, To, T) + Side * (Length * MeanderBulge * Sign * FMath::Sin(PI * T))));
			}
			bFlip = !bFlip;
		}
		if (bAppendTarget) { Push(Out, To); }
	}

	/**
	 * The turnarounds at the far end of each wedge double back almost on themselves, which whips a camera round.
	 * A corner sharper than 70 degrees becomes two points pulled back along its legs, so the turn is a wide swing.
	 * The first corner (the square) stays put.
	 */
	TArray<FVector2D> RoundCorners(const TArray<FVector2D>& In, const FBox2D& Bounds)
	{
		constexpr float MaxSwing = 3400.f, MinSwing = 600.f, SharpCos = 0.5f;   // cos(60 degrees)
		TArray<FVector2D> Out;
		const int32 N = In.Num();
		for (int32 I = 0; I < N; ++I)
		{
			const FVector2D& Prev = In[(I + N - 1) % N];
			const FVector2D& Next = In[(I + 1) % N];
			const FVector2D& Corner = In[I];
			const float InLen = FVector2D::Distance(Prev, Corner), OutLen = FVector2D::Distance(Corner, Next);
			const float Swing = FMath::Min3(MaxSwing, InLen * 0.45f, OutLen * 0.45f);
			if (I == 0 || Swing < MinSwing || FVector2D::DotProduct((Corner - Prev) / InLen, (Next - Corner) / OutLen) >= SharpCos)
			{
				Out.Add(Corner);
				continue;
			}
			Out.Add(Inside(Bounds, Corner - (Corner - Prev) / InLen * Swing));
			Out.Add(Inside(Bounds, Corner + (Next - Corner) / OutLen * Swing));
		}
		// Swings on neighbouring corners can land close together: keep the first of any pair, and the loop's end clear of its start.
		TArray<FVector2D> Spaced;
		for (const FVector2D& P : Out) { Push(Spaced, P); }
		while (Spaced.Num() > 1 && FVector2D::Distance(Spaced.Last(), Spaced[0]) < MinGap) { Spaced.Pop(); }
		return Spaced;
	}

	/** Centripetal Catmull-Rom (alpha 0.5) between P1 and P2, U in 0..1: no loops or cusps on uneven spacing. */
	FVector2D CentripetalCR(const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, float U)
	{
		auto Knot = [](float T, const FVector2D& A, const FVector2D& B) { return T + FMath::Max(FMath::Pow(FVector2D::DistSquared(A, B), 0.25f), 1.e-3f); };
		const float T0 = 0.f, T1 = Knot(T0, P0, P1), T2 = Knot(T1, P1, P2), T3 = Knot(T2, P2, P3);
		const float T = FMath::Lerp(T1, T2, U);
		const FVector2D A1 = P0 * ((T1 - T) / (T1 - T0)) + P1 * ((T - T0) / (T1 - T0));
		const FVector2D A2 = P1 * ((T2 - T) / (T2 - T1)) + P2 * ((T - T1) / (T2 - T1));
		const FVector2D A3 = P2 * ((T3 - T) / (T3 - T2)) + P3 * ((T - T2) / (T3 - T2));
		const FVector2D B1 = A1 * ((T2 - T) / (T2 - T0)) + A2 * ((T - T0) / (T2 - T0));
		const FVector2D B2 = A2 * ((T3 - T) / (T3 - T1)) + A3 * ((T - T1) / (T3 - T1));
		return B1 * ((T2 - T) / (T2 - T1)) + B2 * ((T - T1) / (T2 - T1));
	}
}

void FJamTourPath::Build(const TArray<FVector2D>& Waypoints)
{
	Points.Reset();
	Cumulative.Reset();
	Total = 0.f;
	const int32 N = Waypoints.Num();
	if (N < 3) { return; }
	for (int32 I = 0; I < N; ++I)
	{
		const FVector2D& P0 = Waypoints[(I + N - 1) % N];
		const FVector2D& P1 = Waypoints[I];
		const FVector2D& P2 = Waypoints[(I + 1) % N];
		const FVector2D& P3 = Waypoints[(I + 2) % N];
		for (int32 S = 0; S < JamTourRoute::StepsPerSegment; ++S)
		{
			Points.Add(JamTourRoute::CentripetalCR(P0, P1, P2, P3, float(S) / float(JamTourRoute::StepsPerSegment)));
		}
	}
	const FVector2D First = Points[0];   // a copy: Add(Points[0]) would alias the array it grows
	Points.Add(First);
	Cumulative.Reserve(Points.Num());
	Cumulative.Add(0.f);
	for (int32 I = 1; I < Points.Num(); ++I)
	{
		Total += FVector2D::Distance(Points[I - 1], Points[I]);
		Cumulative.Add(Total);
	}
}

FVector2D FJamTourPath::PointAt(float Distance) const
{
	if (!IsValid()) { return FVector2D::ZeroVector; }
	const float D = FMath::Fmod(Distance, Total) + (Distance < 0.f ? Total : 0.f);
	const int32 Next = FMath::Clamp(Algo::UpperBound(Cumulative, D), 1, Points.Num() - 1);
	const float Span = Cumulative[Next] - Cumulative[Next - 1];
	const float Alpha = Span > UE_SMALL_NUMBER ? (D - Cumulative[Next - 1]) / Span : 0.f;
	return FMath::Lerp(Points[Next - 1], Points[Next], FMath::Clamp(Alpha, 0.f, 1.f));
}

FVector2D FJamTourPath::DirectionAt(float Distance) const
{
	return (PointAt(Distance + 60.f) - PointAt(Distance - 60.f)).GetSafeNormal();
}

namespace JamTour
{
	TArray<FVector2D> Waypoints(const FJamWorldDef& World)
	{
		TArray<FVector2D> Out;
		FVector2D Square = FVector2D::ZeroVector;
		for (const FJamLocation& L : World.Locations) { if (L.Kind == EJamLocationKind::Village) { Square = L.Position; break; } }
		Out.Add(Square);
		bool bFlip = false;

		TArray<const FJamBiomeDef*> Wild;
		for (const FJamBiomeDef& B : World.Biomes) { if (B.Biome != EJamBiome::Village) { Wild.Add(&B); } }
		for (int32 W = 0; W < Wild.Num(); ++W)
		{
			const FJamBiomeDef& Biome = *Wild[W];
			// Out on the side of the wedge's centre line below it, back on the other side, each in order of distance.
			TArray<const FJamLocation*> Outbound, Inbound;
			for (const FJamLocation& L : World.Locations)
			{
				if (L.Biome != Biome.Biome || (L.Kind != EJamLocationKind::Camp && L.Kind != EJamLocationKind::FilmSet && L.Kind != EJamLocationKind::Giver)) { continue; }
				const float Bearing = FMath::RadiansToDegrees(FMath::Atan2(L.Position.Y - Square.Y, L.Position.X - Square.X));
				(FMath::FindDeltaAngleDegrees(Biome.WedgeCentreYaw, Bearing) <= 0.f ? Outbound : Inbound).Add(&L);
			}
			Algo::Sort(Outbound, [&Square](const FJamLocation* A, const FJamLocation* B) { return FVector2D::Distance(A->Position, Square) < FVector2D::Distance(B->Position, Square); });
			Algo::Sort(Inbound, [&Square](const FJamLocation* A, const FJamLocation* B) { return FVector2D::Distance(A->Position, Square) > FVector2D::Distance(B->Position, Square); });
			for (const FJamLocation* L : Outbound) { JamTourRoute::AddLeg(Out, World.Bounds, L->Position, true, bFlip); }
			for (const FJamLocation* L : Inbound) { JamTourRoute::AddLeg(Out, World.Bounds, L->Position, true, bFlip); }
			// Back to the square between wedges; after the last wedge the loop closes onto the first point.
			JamTourRoute::AddLeg(Out, World.Bounds, Square, W + 1 < Wild.Num(), bFlip);
		}
		return JamTourRoute::RoundCorners(Out, World.Bounds);
	}

	TArray<FJamTourStop> Stops(const FJamWorldDef& World)
	{
		TArray<FJamTourStop> Out;
		for (const FJamLocation& L : World.Locations)
		{
			const bool bWildGiver = L.Kind == EJamLocationKind::Giver && L.Biome != EJamBiome::Village;
			if (L.Kind != EJamLocationKind::Village && L.Kind != EJamLocationKind::Camp && L.Kind != EJamLocationKind::FilmSet && !bWildGiver) { continue; }
			FJamTourStop S;
			S.Id = L.Id; S.Position = L.Position; S.Radius = L.Radius; S.bVillage = L.Kind == EJamLocationKind::Village;
			Out.Add(S);
		}
		return Out;
	}

	float SpeedScale(const TArray<FJamTourStop>& Stops, const FVector2D& At)
	{
		float Scale = 1.f;
		for (const FJamTourStop& S : Stops)
		{
			const float Edge = FMath::Max(0.f, FVector2D::Distance(At, S.Position) - S.Radius);
			Scale = FMath::Min(Scale, FMath::Lerp(0.5f, 1.f, FMath::Clamp(Edge / 2500.f, 0.f, 1.f)));
		}
		return Scale;
	}

	float GazeWeight(const TArray<FJamTourStop>& Stops, const FVector2D& At, int32& OutStop)
	{
		OutStop = INDEX_NONE;
		float Best = 0.f;
		for (int32 I = 0; I < Stops.Num(); ++I)
		{
			const FJamTourStop& S = Stops[I];
			if (S.bVillage) { continue; }
			const float Dist = FVector2D::Distance(At, S.Position);
			// Full strength a little outside the stop, fading out far away and, so the view does not stare at its feet, right on top of it.
			float W = 1.f - FMath::Clamp((Dist - S.Radius * 0.3f) / 2500.f, 0.f, 1.f);
			W *= FMath::Clamp(Dist / (S.Radius * 0.8f + 300.f), 0.f, 1.f);
			if (W > Best) { Best = W; OutStop = I; }
		}
		return Best;
	}
}
