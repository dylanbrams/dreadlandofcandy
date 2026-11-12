#include "Game/JamGameState.h"
#include "Engine/World.h"

bool AJamGameState::IsCardVisible() const
{
	if (CardTitle.IsEmpty()) { return false; }
	return CardUntil < 0.f || (GetWorld() && GetWorld()->GetTimeSeconds() < CardUntil);
}
