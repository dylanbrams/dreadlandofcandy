#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "JamComicBorder.generated.h"

/** Resolution independent comic burst. Uses the border brush's tint and outline colour. */
UCLASS()
class MEGAJAM_API UJamComicBorder : public UBorder
{
	GENERATED_BODY()
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
