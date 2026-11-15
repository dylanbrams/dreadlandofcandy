#include "UI/JamCandyBorder.h"
#include "UI/JamUiStyle.h"
#include "Components/BorderSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

class SJamCandyBorder : public SBorder
{
public:
 bool bCrest = false;
 virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& G, const FSlateRect& Cull,
  FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool bEnabled) const override
 {
  const FVector2f Size(G.GetLocalSize());
  if (Size.X < 48.f || Size.Y < 32.f) { return SBorder::OnPaint(Args, G, Cull, Elements, Layer, Style, bEnabled); }
  const ESlateDrawEffect Effect = bEnabled && IsEnabled() ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
  const FLinearColor Tint = Style.GetColorAndOpacityTint();
  const FLinearColor Edge = GetBorderImage()->OutlineSettings.Color.GetSpecifiedColor();
  const FLinearColor Fill = GetBorderBackgroundColor().GetColor(Style);
  const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
  const auto Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*White);
  TArray<FSlateVertex> FillVertices, DetailVertices;
  TArray<SlateIndex> FillIndices, DetailIndices;
  auto Polygon = [&](const TArray<FVector2f>& Points, FLinearColor Colour, int32 At)
  {
   TArray<FSlateVertex>& Vertices = At == Layer ? FillVertices : DetailVertices;
   TArray<SlateIndex>& Indices = At == Layer ? FillIndices : DetailIndices;
   const int32 Base = Vertices.Num();
   const FColor Ink = (Colour * Tint).ToFColor(true);
   for (const FVector2f& P : Points) { Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(), P, FVector2f(0.5f, 0.5f), Ink)); }
   for (int32 i = 1; i + 1 < Points.Num(); ++i) { Indices.Add(SlateIndex(Base)); Indices.Add(SlateIndex(Base+i)); Indices.Add(SlateIndex(Base+i+1)); }
  };
  auto Flush = [&]()
  {
   if (!FillIndices.IsEmpty()) { FSlateDrawElement::MakeCustomVerts(Elements, Layer, Handle, FillVertices, FillIndices, nullptr, 0, 0, Effect); }
   if (!DetailIndices.IsEmpty()) { FSlateDrawElement::MakeCustomVerts(Elements, Layer+2, Handle, DetailVertices, DetailIndices, nullptr, 0, 0, Effect); }
  };
  auto Line = [&](TArray<FVector2f> Points, FLinearColor Colour, float Width = 1.f)
  {
   FSlateDrawElement::MakeLines(Elements, Layer + 1, G.ToPaintGeometry(), Points, Effect, Colour * Tint, true, Width);
  };
  if (bCrest)
  {
   const FVector2f Centre(Size.X * 0.5f, Size.Y * 0.5f);
   const float R = FMath::Min(24.f, Size.Y * 0.38f);
   Line({{12.f,Centre.Y},{Centre.X-R-12.f,Centre.Y}}, JamUi::Silver());
   Line({{Centre.X+R+12.f,Centre.Y},{Size.X-12.f,Centre.Y}}, JamUi::Silver());
   Line({{32.f,Centre.Y+4.f},{Centre.X-R-24.f,Centre.Y+4.f}}, JamUi::Silver().CopyWithNewOpacity(0.3f));
   Line({{Centre.X+R+24.f,Centre.Y+4.f},{Size.X-32.f,Centre.Y+4.f}}, JamUi::Silver().CopyWithNewOpacity(0.3f));
   TArray<FVector2f> Ring;
   for (int32 i = 0; i < 48; ++i)
   {
    const float A = i * 2.f*PI/48.f, B = (i+1)*2.f*PI/48.f;
    const FVector2f P = Centre+FVector2f(FMath::Cos(A),FMath::Sin(A))*R;
    Polygon({Centre,P,Centre+FVector2f(FMath::Cos(B),FMath::Sin(B))*R}, (i/6)%2 == 0 ? JamUi::Enamel() : JamUi::Ink(), Layer+2);
    Ring.Add(Centre+FVector2f(FMath::Cos(A),FMath::Sin(A))*(R+3.f));
   }
   const FVector2f RingStart = Ring[0];
   Ring.Add(RingStart);
   Line(Ring, JamUi::Silver(), 1.5f);
   for (float Offset : {-R-16.f, R+16.f})
   {
    const FVector2f Gem = Centre+FVector2f(Offset,0);
    Polygon({Gem+FVector2f(0,-6),Gem+FVector2f(5,0),Gem+FVector2f(0,6),Gem+FVector2f(-5,0)}, JamUi::Amethyst(), Layer+2);
    Polygon({Gem+FVector2f(0,-4),Gem+FVector2f(0,3),Gem+FVector2f(-3,0)}, JamUi::Ink().CopyWithNewOpacity(0.6f), Layer+2);
   }
   Flush();
   return SCompoundWidget::OnPaint(Args,G,Cull,Elements,Layer+3,Style,bEnabled);
  }
  const float X = Size.X, Y = Size.Y, C = 7.f;
  TArray<FVector2f> Rim = {{C, 1.f}, {X-C, 1.f}, {X-1.f, C}, {X-1.f,Y-C}, {X-C,Y-1.f}, {C,Y-1.f}, {1.f,Y-C}, {1.f,C}};
  Polygon(Rim, IsHovered() ? FMath::Lerp(Fill, JamUi::Selection(), 0.25f) : Fill, Layer);
  const FVector2f First = Rim[0];
  Rim.Add(First);
  const float Stroke = FMath::Max(1.f, GetBorderImage()->OutlineSettings.Width);
  Line(Rim, Edge, Stroke + (IsHovered() ? 0.5f : 0.f));
  Line({{15.f,5.f},{X-15.f,5.f}}, JamUi::Silver().CopyWithNewOpacity(0.45f));
  Line({{15.f,Y-5.f},{X-15.f,Y-5.f}}, JamUi::Silver().CopyWithNewOpacity(0.25f));
  // A small garnet / ivory enamel inlay on the left; content is padded away from it.
  const FVector2f Centre(11.f, Y * 0.5f);
  constexpr float R = 5.f;
  for (int32 i = 0; i < 24; ++i)
  {
   const float A = i * 2.f * PI / 24.f, B = (i+1) * 2.f * PI / 24.f;
   Polygon({Centre, Centre + FVector2f(FMath::Cos(A), FMath::Sin(A))*R, Centre + FVector2f(FMath::Cos(B), FMath::Sin(B))*R}, (i / 3) % 2 == 0 ? JamUi::Enamel() : JamUi::Ink(), Layer+2);
  }
  const FVector2f Gem(X-10.f, Y*0.5f);
  Polygon({Gem+FVector2f(0,-4),Gem+FVector2f(4,0),Gem+FVector2f(0,4),Gem+FVector2f(-4,0)}, JamUi::Amethyst(), Layer+2);
  Polygon({Gem+FVector2f(0,-3),Gem+FVector2f(0,2),Gem+FVector2f(-3,0)}, JamUi::Ink().CopyWithNewOpacity(0.65f), Layer+2);
  Flush();
  return SCompoundWidget::OnPaint(Args, G, Cull, Elements, Layer+3, Style, bEnabled);
 }
};

TSharedRef<SWidget> UJamCandyBorder::RebuildWidget()
{
 TSharedRef<SJamCandyBorder> Candy = SNew(SJamCandyBorder);
 Candy->bCrest = bCrest;
 MyBorder = Candy;
 if (GetChildrenCount() > 0) { CastChecked<UBorderSlot>(GetContentSlot())->BuildSlot(MyBorder.ToSharedRef()); }
 return MyBorder.ToSharedRef();
}
