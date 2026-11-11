#include "AgentHooks.h"
#include "MegaJam.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

bool UAgentHooksSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UAgentHooksSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	FParse::Value(FCommandLine::Get(), TEXT("shotat="), ShotAt);
	FParse::Value(FCommandLine::Get(), TEXT("shotname="), ShotName);
	FParse::Value(FCommandLine::Get(), TEXT("quitat="), QuitAt);
	// The packaged smoke test looks for this line.
	UE_LOG(LogJam, Log, TEXT("World began play: %s"), *InWorld.GetMapName());
}

void UAgentHooksSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const UWorld* World = GetWorld();
	if (!World || !World->HasBegunPlay()) { return; }
	const float Now = World->GetTimeSeconds();

	if (ShotAt > 0.f && Now >= ShotAt)
	{
		ShotAt = -1.f;
		const FString Path = FPaths::ConvertRelativePathToFull(FPaths::ScreenShotDir() / ShotName + TEXT(".png"));
		FScreenshotRequest::RequestScreenshot(Path, true, false);
		UE_LOG(LogJam, Log, TEXT("Screenshot requested: %s"), *Path);
	}
	if (QuitAt > 0.f && Now >= QuitAt)
	{
		QuitAt = -1.f;
		FPlatformMisc::RequestExit(false, TEXT("quitat"));
	}
}

TStatId UAgentHooksSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAgentHooksSubsystem, STATGROUP_Tickables);
}
