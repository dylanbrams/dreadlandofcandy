#pragma once

#include "CoreMinimal.h"
#include "Film/JamFilmTypes.h"

/**
 * R4b, "they hit back" (plain C++, tested headless). Goblins, bugs and the boss goblin charge the hero when he is
 * near; rats and almonds never do. A hit knocks him back, staggers him and drains his Dignity; at zero the take
 * fails through the normal failed-take path, and a retake restores it.
 */
struct MEGAJAM_API FJamAttackTuning
{
	bool bAttacks = false;
	/** Starts a charge when the hero is this close (2D, uu). */
	float ChargeRange = 0.f;
	/** Charge speed as a multiple of the kind's own speed. */
	float ChargeSpeedScale = 1.f;
	/** Gives up a charge that has not connected after this long. */
	float ChargeSeconds = 1.2f;
	/** Connects when this close (centre to centre, plus its extra size). */
	float HitRange = 110.f;
	/** Dignity lost per hit. */
	float Damage = 0.f;
	/** After a charge (hit or miss) it goes back to fleeing or wandering for this long. */
	float Cooldown = 3.f;
	/** The hero's knockback speed (uu/s) and stagger time. */
	float Knockback = 0.f;
	float StaggerSeconds = 0.f;
	/** A skill hit does not stop its charge (the boss). Others are flustered out of it: that is the counter. */
	bool bUnstoppable = false;
};

namespace JamAttack
{
	/** BUILD_SPEC 10 style numbers per kind. bBoss: the note's very large goblin (Resolve x3 and up). */
	MEGAJAM_API FJamAttackTuning For(EJamTargetKind Kind, bool bBoss);

	/** Charges allowed at once (the rest keep fleeing), so a crowd of bugs cannot stun-lock him. */
	constexpr int32 MaxChargers = 2;
	/** After a hit he cannot be hit again for this long (stagger included). */
	constexpr float HitGraceSeconds = 1.4f;

	/**
	 * May this attacker start a charge now? It must attack, be loose (not pacified, treated or flustered), be ready
	 * (cooldown over), have the hero in range, and there must be room among the chargers. Fleeing does not stop it:
	 * the charge is the moment it turns round.
	 */
	MEGAJAM_API bool MayCharge(const FJamAttackTuning& Tuning, bool bLoose, float CooldownLeft, float HeroDistance, int32 ChargingNow);
}

/** The hero's Dignity for one take. */
class MEGAJAM_API FJamDignity
{
public:
	static constexpr float Max = 100.f;

	/** A take starts (or is retaken): full, no grace, no stagger. */
	void Reset();
	/** Time passes (grace and stagger run down). */
	void Tick(float DeltaSeconds);

	/** Can a hit land now? False during the grace after the last hit, or once broken. */
	bool CanBeHit() const { return !IsBroken() && GraceLeft <= 0.f; }
	/**
	 * A hit lands: drains Damage, starts the stagger and the grace. Returns true if this hit broke him (Dignity
	 * reached zero); false otherwise, including when the hit was ignored (grace, already broken).
	 */
	bool Hit(float Damage, float StaggerSeconds);

	float GetValue() const { return Value; }
	float GetFraction() const { return Value / Max; }
	bool IsBroken() const { return Value <= 0.f; }
	bool IsStaggered() const { return StaggerLeft > 0.f; }
	int32 GetHits() const { return Hits; }

private:
	float Value = Max;
	float GraceLeft = 0.f;
	float StaggerLeft = 0.f;
	int32 Hits = 0;
};
