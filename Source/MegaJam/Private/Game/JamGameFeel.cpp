// Feedback6 Group B: combat feel. Hit-stop, slow motion, camera kicks and Fenella's combo line.
// New functions only, so this file never collides with other workers' edits to JamGameMode.cpp.
#include "Game/JamGameMode.h"
#include "Game/JamHero.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "TimerManager.h"

void AJamGameMode::PushTimeDilation(float Dilation, float RealSeconds)
{
	AWorldSettings* Settings = GetWorldSettings();
	if (!Settings) { return; }
	if (!bDilationHeld) { BaseDilation = Settings->TimeDilation; bDilationHeld = true; }
	// A slower request wins while another is running; the timer restarts for the longer of the two.
	const float Target = FMath::Clamp(FMath::Min(Dilation, Settings->TimeDilation), 0.01f, 1.f);
	Settings->SetTimeDilation(Target);
	// Timers run in dilated game time, so real seconds times the dilation is the game time to wait.
	const float GameSeconds = FMath::Max(RealSeconds * Target, 0.001f);
	const float Remaining = GetWorldTimerManager().GetTimerRemaining(DilationTimer);
	GetWorldTimerManager().SetTimer(DilationTimer, this, &AJamGameMode::RestoreTimeDilation, FMath::Max(GameSeconds, Remaining), false);
}

void AJamGameMode::RestoreTimeDilation()
{
	if (!bDilationHeld) { return; }
	bDilationHeld = false;
	if (AWorldSettings* Settings = GetWorldSettings()) { Settings->SetTimeDilation(BaseDilation); }
}

void AJamGameMode::PlayFinisherFeel()
{
	const double Now = GetWorld()->GetRealTimeSeconds();
	// B1/B2: a short freeze and a kick on the pacify that finishes a nuisance; longer and bigger inside an ultimate.
	const bool bUltimate = Now - LastUltimateRealAt < 2.5;
	PushTimeDilation(0.05f, bUltimate ? 0.12f : 0.05f);
	if (Hero.IsValid()) { Hero->AddShake(bUltimate ? 16.f : 5.f); }

	// B6: three pacifies within four seconds earns a line from Fenella (pool mirrored in Docs/Design/Jokes.md).
	RecentPacifyTimes.Add(Now);
	RecentPacifyTimes.RemoveAll([Now](double T) { return Now - T > 4.0; });
	if (RecentPacifyTimes.Num() >= 3 && Now - LastComboLineAt > 12.0)
	{
		static const TCHAR* ComboLines[] = {
			TEXT("Three in four seconds! Do that again, but slower, and with feeling."),
			TEXT("A montage! I did not ask for a montage, and I am delighted."),
			TEXT("Please stop being so efficient. The audience needs time to gasp."),
			TEXT("That is what I call a combo. I call everything that, but this time it is true."),
		};
		LastComboLineAt = Now;
		RecentPacifyTimes.Reset();
		Direct(ComboLines[FMath::RandRange(0, UE_ARRAY_COUNT(ComboLines) - 1)]);
	}
}

void AJamGameMode::BeginUltimateOpening()
{
	// B4: 0.3 s of slow motion at launch, then the card ring bursts out at full speed.
	LastUltimateRealAt = GetWorld()->GetRealTimeSeconds();
	PushTimeDilation(UltimateSlowDilation, UltimateSlowRealSeconds);
}
