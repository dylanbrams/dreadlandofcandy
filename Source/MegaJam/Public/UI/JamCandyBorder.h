#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "JamCandyBorder.generated.h"

/** Etched pewter frame with peppermint enamel and a faceted sugar jewel. No texture imports. */
UCLASS()
class MEGAJAM_API UJamCandyBorder : public UBorder
{
 GENERATED_BODY()
public:
 /** A seal and etched rules for screen headings, with no filled panel. */
 UPROPERTY(EditAnywhere, Category="Black Sugar") bool bCrest = false;
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
};
