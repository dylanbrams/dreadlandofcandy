#include "UI/JamUiStyle.h"
#include "Fonts/CompositeFont.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"

namespace
{
 FLinearColor Colour(const TCHAR* Hex) { return FLinearColor::FromSRGBColor(FColor::FromHex(Hex)); }
}

FLinearColor JamUi::Surface(float Alpha) { return Colour(TEXT("10171A")).CopyWithNewOpacity(Alpha); }
FLinearColor JamUi::Ink() { return Colour(TEXT("E1E6DF")); }
FLinearColor JamUi::Dim() { return Colour(TEXT("A9B6B4")); }
FLinearColor JamUi::Silver() { return Colour(TEXT("849698")); }
FLinearColor JamUi::Mint() { return Colour(TEXT("9BC2B7")); }
FLinearColor JamUi::Frost() { return Colour(TEXT("C1D3D8")); }
FLinearColor JamUi::Ruby() { return Colour(TEXT("D38C9C")); }
FLinearColor JamUi::Enamel() { return Colour(TEXT("813B4A")); }
FLinearColor JamUi::Amethyst() { return Colour(TEXT("B19CC6")); }
FLinearColor JamUi::Selection() { return Colour(TEXT("293D40")); }

FSlateFontInfo JamUi::Font(int32 Size, bool bEmphasis, bool bHeading)
{
 // Raw, fixed-weight TTFs are NonUFS runtime dependencies, so this works in cooked builds too.
 const TCHAR* File = bHeading ? TEXT("Cinzel-Medium.ttf") : bEmphasis ? TEXT("CormorantGaramond-SemiBold.ttf") : TEXT("CormorantGaramond-Medium.ttf");
 const FString Path = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("UI/Fonts"), File);
 // Small functional labels keep the engine font's larger counters and symbol coverage.
 if (Size < 24 || !IFileManager::Get().FileExists(*Path)) { return FCoreStyle::GetDefaultFontStyle(bEmphasis ? "Bold" : "Regular", Size); }
 // Keep shared composite fonts alive and use the supported UE 5.8 font constructor.
 // UMG builds/updates these styles on the game thread; the cache has only three paths.
 static TMap<FString, TSharedPtr<const FCompositeFont>> Fonts;
 TSharedPtr<const FCompositeFont>& Composite = Fonts.FindOrAdd(Path);
 if (!Composite.IsValid())
 {
  Composite = MakeShared<FCompositeFont>(FName(TEXT("Regular")), Path, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
 }
 return FSlateFontInfo(Composite, Size, FName(TEXT("Regular")));
}

FSlateBrush JamUi::Frame(const FLinearColor& Edge, float Width)
{
 FSlateBrush Brush;
 Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
 Brush.TintColor = FSlateColor(FLinearColor::White);
 Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(2.f), FSlateColor(Edge), Width);
 Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
 return Brush;
}
