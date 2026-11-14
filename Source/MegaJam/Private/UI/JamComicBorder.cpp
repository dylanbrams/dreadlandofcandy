#include "UI/JamComicBorder.h"
#include "UI/JamComicGeometry.h"
#include "Components/BorderSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Widgets/Layout/SBorder.h"

class SJamComicBorder : public SBorder
{
public:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& Cull,
		FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool bEnabled) const override
	{
		const FVector2f Size(Geometry.GetLocalSize());
        if (Size.X < 24.f || Size.Y < 24.f) { return SCompoundWidget::OnPaint(Args, Geometry, Cull, Elements, Layer, Style, bEnabled); }
        if (Size != CachedSize)
        {
            CachedSize = Size;
            CachedPoints = JamComicGeometry::Outline(Size);
            CachedIndices.Reset();
            for (int32 Index : JamComicGeometry::Triangulate(CachedPoints)) { CachedIndices.Add(SlateIndex(Index)); }
        }

		const bool bActive = bEnabled && IsEnabled();
		const ESlateDrawEffect Effect = bActive ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		FLinearColor Fill = GetBorderBackgroundColor().GetColor(Style);
		if (IsHovered() && bActive) { Fill = FMath::Lerp(Fill, FLinearColor::White, 0.18f); }
		const FColor Ink = (Fill * Style.GetColorAndOpacityTint()).ToFColor(true);
		TArray<FSlateVertex> Vertices;
		auto Vertex = [&](FVector2f P) { return FSlateVertex::Make<ESlateVertexRounding::Disabled>(Geometry.GetAccumulatedRenderTransform(), P, FVector2f(0.5f, 0.5f), Ink); };
		for (const FVector2f& P : CachedPoints) { Vertices.Add(Vertex(P)); }
		const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
		FSlateDrawElement::MakeCustomVerts(Elements, Layer, FSlateApplication::Get().GetRenderer()->GetResourceHandle(*White), Vertices, CachedIndices, nullptr, 0, 0, Effect);
		TArray<FVector2f> Points = CachedPoints;
		const FVector2f FirstPoint = Points[0];
		Points.Add(FirstPoint);
		const FLinearColor EdgeColour = GetBorderImage()->OutlineSettings.Color.GetSpecifiedColor() * Style.GetColorAndOpacityTint();
		FSlateDrawElement::MakeLines(Elements, Layer + 1, Geometry.ToPaintGeometry(), Points, Effect, EdgeColour, true, IsHovered() ? 4.f : 3.f);
		return SCompoundWidget::OnPaint(Args, Geometry, Cull, Elements, Layer + 2, Style, bEnabled);
	}
private:
    mutable FVector2f CachedSize = FVector2f(-1.f, -1.f);
    mutable TArray<FVector2f> CachedPoints;
    mutable TArray<SlateIndex> CachedIndices;
};

TSharedRef<SWidget> UJamComicBorder::RebuildWidget()
{
	MyBorder = SNew(SJamComicBorder);
	if (GetChildrenCount() > 0) { CastChecked<UBorderSlot>(GetContentSlot())->BuildSlot(MyBorder.ToSharedRef()); }
	return MyBorder.ToSharedRef();
}
