#include "Game/JamGameState.h"
#include "Engine/World.h"

bool AJamGameState::IsCardVisible() const
{
	if (CardTitle.IsEmpty()) { return false; }
	return CardUntil < 0.f || (GetWorld() && GetWorld()->GetTimeSeconds() < CardUntil);
}

bool AJamGameState::IsWrapVisible() const
{
	if (!Wrap.bVisible) { return false; }
	return WrapUntil < 0.f || (GetWorld() && GetWorld()->GetTimeSeconds() < WrapUntil);
}

bool AJamGameState::IsCaptionVisible() const
{
	return !CaptionLineId.IsNone() && GetWorld() && GetWorld()->GetTimeSeconds() < CaptionUntil;
}
