#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "JamHUD.generated.h"

/**
 * Plain canvas HUD until the widget HUD exists (T06 onward): the interact prompt over the nuisance the
 * hero would treat. Reads state only.
 */
UCLASS()
class MEGAJAM_API AJamHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
