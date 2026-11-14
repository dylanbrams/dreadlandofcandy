#pragma once

#include "CoreMinimal.h"

namespace JamComicGeometry
{
inline TArray<FVector2f> Outline(const FVector2f& Size)
{
	TArray<FVector2f> Points;
	const FVector2f Corners[] = { { 3, 3 }, { Size.X - 3, 3 }, { Size.X - 3, Size.Y - 3 }, { 3, Size.Y - 3 } };
	for (int32 Edge = 0; Edge < 4; ++Edge)
	{
		const FVector2f A = Corners[Edge], B = Corners[(Edge + 1) % 4], D = B - A;
		const FVector2f Inward = FVector2f(-D.Y, D.X).GetSafeNormal();
		const int32 Teeth = FMath::Clamp(FMath::RoundToInt(D.Size() / 45.f), 2, 24);
		for (int32 Tooth = 0; Tooth < Teeth; ++Tooth)
		{
			Points.Add(A + D * (float(Tooth) / Teeth));
			Points.Add(A + D * ((Tooth + 0.65f) / Teeth) + Inward * (Tooth % 2 ? 7.f : 10.f));
		}
	}
	return Points;
}

inline float Cross(const FVector2f& A, const FVector2f& B) { return A.X * B.Y - A.Y * B.X; }

/** Ear clipping: a wide serrated border is concave and cannot safely use a centre triangle fan. */
inline TArray<int32> Triangulate(const TArray<FVector2f>& Points)
{
	TArray<int32> Remaining, Triangles;
	for (int32 i = 0; i < Points.Num(); ++i) { Remaining.Add(i); }
	while (Remaining.Num() > 3)
	{
		bool bClipped = false;
		for (int32 i = 0; i < Remaining.Num(); ++i)
		{
			const int32 A = Remaining[(i + Remaining.Num() - 1) % Remaining.Num()], B = Remaining[i], C = Remaining[(i + 1) % Remaining.Num()];
			const float Area = Cross(Points[B] - Points[A], Points[C] - Points[A]);
			if (FMath::Abs(Area) < 0.001f) { Remaining.RemoveAt(i); bClipped = true; break; }
			if (Area < 0.f) { continue; }
			bool bContainsPoint = false;
			for (int32 P : Remaining)
			{
				if (P == A || P == B || P == C) { continue; }
				if (Cross(Points[B] - Points[A], Points[P] - Points[A]) >= -0.001f
					&& Cross(Points[C] - Points[B], Points[P] - Points[B]) >= -0.001f
					&& Cross(Points[A] - Points[C], Points[P] - Points[C]) >= -0.001f) { bContainsPoint = true; break; }
			}
			if (bContainsPoint) { continue; }
			Triangles.Append({ A, B, C }); Remaining.RemoveAt(i); bClipped = true; break;
		}
		if (!bClipped) { return {}; }
	}
	if (Remaining.Num() == 3) { Triangles.Append(Remaining); }
	return Triangles;
}
}
