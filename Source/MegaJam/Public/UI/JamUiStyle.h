#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"

/** Black Sugar: shared UI colours are authored in sRGB, then converted for Slate. */
namespace JamUi
{
 MEGAJAM_API FLinearColor Surface(float Alpha = 0.98f);
 MEGAJAM_API FLinearColor Ink();
 MEGAJAM_API FLinearColor Dim();
 MEGAJAM_API FLinearColor Silver();
 MEGAJAM_API FLinearColor Mint();
 MEGAJAM_API FLinearColor Frost();
 MEGAJAM_API FLinearColor Ruby();
 MEGAJAM_API FLinearColor Enamel();
 MEGAJAM_API FLinearColor Amethyst();
 MEGAJAM_API FLinearColor Selection();
 MEGAJAM_API FSlateFontInfo Font(int32 Size, bool bEmphasis = false, bool bHeading = false);
 MEGAJAM_API FSlateBrush Frame(const FLinearColor& Edge, float Width = 1.f);
}
