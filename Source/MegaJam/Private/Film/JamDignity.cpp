#include "Film/JamDignity.h"

FJamAttackTuning JamAttack::For(EJamTargetKind Kind, bool bBoss)
{
	FJamAttackTuning T;
	switch (Kind)
	{
	case EJamTargetKind::Goblin:
		T.bAttacks = true;
		if (bBoss)
		{
			// Slow, wide, and it hurts. It never flees, so the cooldown is what spaces its charges.
			T.ChargeRange = 650.f;
			T.ChargeSpeedScale = 2.4f;
			T.ChargeSeconds = 1.6f;
			T.HitRange = 120.f;   // plus its size (the game mode adds it)
			T.Damage = 30.f;
			T.Cooldown = 3.5f;
			T.Knockback = 1600.f;
			T.StaggerSeconds = 0.8f;
			T.bUnstoppable = true;
		}
		else
		{
			T.ChargeRange = 450.f;
			T.ChargeSpeedScale = 1.8f;
			T.ChargeSeconds = 1.2f;
			T.HitRange = 110.f;
			T.Damage = 20.f;
			T.Cooldown = 4.f;
			T.Knockback = 1100.f;
			T.StaggerSeconds = 0.5f;
		}
		break;
	case EJamTargetKind::Bug:
		// Quick nips: small, often, more of them.
		T.bAttacks = true;
		T.ChargeRange = 380.f;
		T.ChargeSpeedScale = 1.6f;
		T.ChargeSeconds = 0.9f;
		T.HitRange = 90.f;
		T.Damage = 10.f;
		T.Cooldown = 4.5f;
		T.Knockback = 700.f;
		T.StaggerSeconds = 0.3f;
		break;
	default:
		// Rats know their place. Almonds are nuts.
		break;
	}
	return T;
}

bool JamAttack::MayCharge(const FJamAttackTuning& Tuning, bool bLoose, float CooldownLeft, float HeroDistance, int32 ChargingNow)
{
	return Tuning.bAttacks && bLoose && CooldownLeft <= 0.f && HeroDistance <= Tuning.ChargeRange && ChargingNow < MaxChargers;
}

void FJamDignity::Reset()
{
	Value = Max;
	GraceLeft = 0.f;
	StaggerLeft = 0.f;
	Hits = 0;
}

void FJamDignity::Tick(float DeltaSeconds)
{
	GraceLeft = FMath::Max(0.f, GraceLeft - DeltaSeconds);
	StaggerLeft = FMath::Max(0.f, StaggerLeft - DeltaSeconds);
}

bool FJamDignity::Hit(float Damage, float StaggerSeconds)
{
	if (!CanBeHit() || Damage <= 0.f) { return false; }
	Value = FMath::Max(0.f, Value - Damage);
	StaggerLeft = FMath::Max(StaggerLeft, StaggerSeconds);
	GraceLeft = FMath::Max(JamAttack::HitGraceSeconds, StaggerSeconds);
	++Hits;
	return IsBroken();
}
