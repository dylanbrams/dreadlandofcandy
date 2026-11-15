#pragma once

#include "CoreMinimal.h"
#include "Gas/JamAoePattern.h"
#include "Film/JamProgression.h"

/**
 * Fixes3 #3: each class hits in its own shape. Applied to the ability's resolved pattern (what is hit) and to the
 * ground flash (what is shown), so the two always agree. Cast: 0 knight, 1 mage, 2 archer, 3 barbarian.
 */
namespace JamClassShape
{
inline void Adjust(int32 Cast, EJamSkill Skill, FJamAoePatternParams& P)
{
	if (Skill == EJamSkill::Lecture)
	{
		if (Cast == 1) { P.Shape = EJamAoeShape::Cone; P.ConeHalfAngleDeg = 60.f; P.Radius *= 0.75f; }   // sugar spray: wide, close
		else if (Cast == 2) { P.Shape = EJamAoeShape::Line; P.Radius *= 1.4f; P.Width = 60.f; }         // celery shot: long and precise
		else if (Cast == 3) { P.Shape = EJamAoeShape::Circle; P.Radius *= 0.65f; }                      // kneading: everything around him
	}
	else if (Skill == EJamSkill::FormALine)
	{
		if (Cast == 1) { P.Shape = EJamAoeShape::Cone; P.ConeHalfAngleDeg = 25.f; }                       // soap jet
		else if (Cast == 2) { P.Radius *= 1.3f; P.Width *= 0.6f; }                                         // floss: longer, thinner
		else if (Cast == 3) { P.Shape = EJamAoeShape::Cone; P.ConeHalfAngleDeg = 70.f; P.Radius *= 0.7f; } // bellow: wide blast
	}
	else if (Skill == EJamSkill::LastJudgment && Cast == 1) { P.Radius *= 1.2f; }                         // the peppermint shower
}
}
