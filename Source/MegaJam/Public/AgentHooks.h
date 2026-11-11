#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AgentHooks.generated.h"

/**
 * Command-line hooks so an agent can see the game without capturing the desktop.
 * Runs in every game world (editor -game and packaged builds), never in the editor's own world.
 *   -shotat=<seconds> [-shotname=<name>]  one in-engine screenshot (with UI) to Saved/Screenshots/
 *   -quitat=<seconds>                     close the game
 * Game-specific flags (-autoplay, -uiscreen=...) belong in the game mode once the game exists.
 */
UCLASS()
class MEGAJAM_API UAgentHooksSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableInEditor() const override { return false; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	float ShotAt = -1.f;
	float QuitAt = -1.f;
	FString ShotName = TEXT("autoshot");
};
