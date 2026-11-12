#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "JamHUD.generated.h"

class UFont;

/**
 * Plain canvas HUD until the widget HUD exists: scene/take label and REC, objective and progress, the
 * centre card (ACTION!, RETAKE!, wrap), the interact prompt. Reads state only.
 */
UCLASS()
class MEGAJAM_API AJamHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawTextBox(const FString& Text, float X, float Y, UFont* Font, float Scale, const FLinearColor& Colour, bool bCentred);
};
