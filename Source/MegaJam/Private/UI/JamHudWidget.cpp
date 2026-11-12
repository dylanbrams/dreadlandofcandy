#include "UI/JamHudWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

namespace
{
	const FLinearColor PanelDark(0.02f, 0.02f, 0.025f, 0.68f);
	const FLinearColor Gold(1.f, 0.82f, 0.3f);
	const FLinearColor RecRed(0.95f, 0.12f, 0.1f);
	const FLinearColor Dim(0.75f, 0.75f, 0.72f);

	void SetTextIfChanged(UTextBlock* Block, const FString& Text)
	{
		if (Block && !Block->GetText().ToString().Equals(Text, ESearchCase::CaseSensitive))
		{
			Block->SetText(FText::FromString(Text));
		}
	}

	void Show(UWidget* W, bool bVisible)
	{
		if (!W) { return; }
		const ESlateVisibility V = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (W->GetVisibility() != V) { W->SetVisibility(V); }
	}

	FSlateBrush Rounded(const FLinearColor& Colour, float Radius, const FLinearColor& Outline = FLinearColor::Transparent, float OutlineWidth = 0.f)
	{
		FSlateBrush B;
		B.DrawAs = ESlateBrushDrawType::RoundedBox;
		B.TintColor = FSlateColor(Colour);
		B.OutlineSettings = FSlateBrushOutlineSettings(FVector4(Radius, Radius, Radius, Radius), FSlateColor(Outline), OutlineWidth);
		B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		return B;
	}
}

TSharedRef<SWidget> UJamHudWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildTree();
	}
	return Super::RebuildWidget();
}

UBorder* UJamHudWidget::MakePanel(const FLinearColor& Colour, float Radius, const FMargin& InPadding)
{
	UBorder* B = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	B->SetBrush(Rounded(FLinearColor::White, Radius));
	B->SetBrushColor(Colour);
	B->SetPadding(InPadding);
	B->SetVisibility(ESlateVisibility::HitTestInvisible);
	return B;
}

UTextBlock* UJamHudWidget::MakeText(int32 Size, const FLinearColor& Colour, bool bBold)
{
	UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	T->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size));
	T->SetColorAndOpacity(FSlateColor(Colour));
	T->SetShadowOffset(FVector2D(2.f, 2.f));
	T->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));
	return T;
}

void UJamHudWidget::BuildTree()
{
	Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = Root;
	Root->SetVisibility(ESlateVisibility::HitTestInvisible);

	auto Place = [this](UWidget* W, const FAnchors& Anchors, const FVector2D& Alignment, const FVector2D& Position, int32 Z = 0)
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(W);
		S->SetAnchors(Anchors);
		S->SetAlignment(Alignment);
		S->SetPosition(Position);
		S->SetAutoSize(true);
		S->SetZOrder(Z);
		return S;
	};

	// Bubble layer: fills the screen, under the panels and the card.
	BubbleLayer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	BubbleLayer->SetVisibility(ESlateVisibility::HitTestInvisible);
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(BubbleLayer);
		S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		S->SetOffsets(FMargin(0.f));
		S->SetZOrder(0);
	}

	// Interact prompt, placed over its target.
	PromptPanel = MakePanel(PanelDark, 6.f, FMargin(14.f, 6.f));
	PromptText = MakeText(22, FLinearColor::White, true);
	PromptPanel->SetContent(PromptText);
	{
		UCanvasPanelSlot* S = BubbleLayer->AddChildToCanvas(PromptPanel);
		S->SetAutoSize(true);
		S->SetAlignment(FVector2D(0.5f, 1.f));
		S->SetZOrder(10);
	}

	// Top left: the slate label, take, REC and timecode.
	SlatePanel = MakePanel(PanelDark, 8.f, FMargin(20.f, 12.f));
	{
		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		SceneText = MakeText(18, Dim, true);
		Box->AddChildToVerticalBox(SceneText);
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		TakeText = MakeText(34, FLinearColor::White, true);
		RecText = MakeText(24, RecRed, true);
		TimecodeText = MakeText(24, FLinearColor::White, false);
		Row->AddChildToHorizontalBox(TakeText)->SetVerticalAlignment(VAlign_Center);
		UHorizontalBoxSlot* RecSlot = Row->AddChildToHorizontalBox(RecText);
		RecSlot->SetVerticalAlignment(VAlign_Center);
		RecSlot->SetPadding(FMargin(28.f, 0.f, 10.f, 0.f));
		Row->AddChildToHorizontalBox(TimecodeText)->SetVerticalAlignment(VAlign_Center);
		Box->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
		SlatePanel->SetContent(Box);
	}
	Place(SlatePanel, FAnchors(0.f, 0.f), FVector2D(0.f, 0.f), FVector2D(36.f, 32.f), 5);

	// Top centre: objective, meter, and the hint under them.
	{
		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		ObjectivePanel = MakePanel(PanelDark, 8.f, FMargin(24.f, 12.f));
		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		ObjectiveText = MakeText(28, FLinearColor::White, true);
		Box->AddChildToVerticalBox(ObjectiveText)->SetHorizontalAlignment(HAlign_Center);
		USizeBox* MeterSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		MeterSize->SetWidthOverride(460.f);
		MeterSize->SetHeightOverride(14.f);
		MeterBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		MeterSize->SetContent(MeterBar);
		UVerticalBoxSlot* MeterSlot = Box->AddChildToVerticalBox(MeterSize);
		MeterSlot->SetHorizontalAlignment(HAlign_Center);
		MeterSlot->SetPadding(FMargin(0.f, 8.f, 0.f, 2.f));
		MeterText = MakeText(18, Dim, false);
		Box->AddChildToVerticalBox(MeterText)->SetHorizontalAlignment(HAlign_Center);
		ObjectivePanel->SetContent(Box);
		Column->AddChildToVerticalBox(ObjectivePanel)->SetHorizontalAlignment(HAlign_Center);

		HintPanel = MakePanel(PanelDark, 8.f, FMargin(18.f, 8.f));
		HintText = MakeText(24, Gold, true);
		HintPanel->SetContent(HintText);
		UVerticalBoxSlot* HintSlot = Column->AddChildToVerticalBox(HintPanel);
		HintSlot->SetHorizontalAlignment(HAlign_Center);
		HintSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));
		Place(Column, FAnchors(0.5f, 0.f), FVector2D(0.5f, 0.f), FVector2D(0.f, 32.f), 5);
	}

	// Centre: the card. An accent bar like the top of a clapperboard, a big title, the line under it.
	CardPanel = MakePanel(PanelDark, 10.f, FMargin(56.f, 0.f, 56.f, 28.f));
	{
		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		BarSize->SetHeightOverride(10.f);
		CardAccent = MakePanel(Gold, 3.f, FMargin(0.f));
		BarSize->SetContent(CardAccent);
		UVerticalBoxSlot* BarSlot = Box->AddChildToVerticalBox(BarSize);
		BarSlot->SetHorizontalAlignment(HAlign_Fill);
		BarSlot->SetPadding(FMargin(-40.f, 18.f, -40.f, 10.f));
		CardTitle = MakeText(80, Gold, true);
		CardTitle->SetJustification(ETextJustify::Center);
		CardTitle->SetShadowOffset(FVector2D(4.f, 4.f));
		Box->AddChildToVerticalBox(CardTitle)->SetHorizontalAlignment(HAlign_Center);
		CardSubtitle = MakeText(26, FLinearColor::White, false);
		CardSubtitle->SetJustification(ETextJustify::Center);
		CardSubtitle->SetAutoWrapText(false);
		CardSubtitle->SetWrapTextAt(1200.f);
		CardSubtitle->SetLineHeightPercentage(1.15f);
		Box->AddChildToVerticalBox(CardSubtitle)->SetHorizontalAlignment(HAlign_Center);
		CardPanel->SetContent(Box);
	}
	Place(CardPanel, FAnchors(0.5f, 0.42f), FVector2D(0.5f, 0.5f), FVector2D::ZeroVector, 20);
	CardPanel->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

	Show(PromptPanel, false);
	Show(CardPanel, false);
	Show(HintPanel, false);
}

UBorder* UJamHudWidget::GetBubble(int32 Index)
{
	while (Bubbles.Num() <= Index)
	{
		UBorder* B = MakePanel(FLinearColor::White, 14.f, FMargin(16.f, 10.f));
		UTextBlock* T = MakeText(22, FLinearColor(0.05f, 0.05f, 0.05f), true);
		T->SetShadowOffset(FVector2D::ZeroVector);
		T->SetWrapTextAt(420.f);
		T->SetJustification(ETextJustify::Center);
		B->SetContent(T);
		UCanvasPanelSlot* S = BubbleLayer->AddChildToCanvas(B);
		S->SetAutoSize(true);
		S->SetAlignment(FVector2D(0.5f, 1.f));
		S->SetZOrder(5);
		Bubbles.Add(B);
	}
	return Bubbles[Index];
}

void UJamHudWidget::ApplyView(const FJamHudView& View, float RealSeconds)
{
	if (!Root) { return; }

	// Slate label, REC, timecode.
	Show(SlatePanel, View.bShootVisible);
	if (View.bShootVisible)
	{
		SetTextIfChanged(SceneText, View.SceneLabel);
		SetTextIfChanged(TakeText, FString::Printf(TEXT("TAKE %d"), View.Take));
		const bool bBlinkOn = !View.bRolling || FMath::Fmod(RealSeconds, 1.f) < 0.6f;
		SetTextIfChanged(RecText, View.bRolling ? TEXT("● REC") : TEXT("■ STBY"));
		RecText->SetColorAndOpacity(FSlateColor(View.bRolling ? RecRed : Dim));
		RecText->SetRenderOpacity(bBlinkOn ? 1.f : 0.15f);
		SetTextIfChanged(TimecodeText, JamHud::Timecode(View.FilmSeconds));
	}

	// Objective and meter.
	const bool bObjective = View.bShootVisible && !View.Objective.IsEmpty();
	Show(ObjectivePanel, bObjective);
	if (bObjective)
	{
		SetTextIfChanged(ObjectiveText, View.Objective);
		Show(MeterBar->GetParent(), View.bMeterVisible);
		Show(MeterText, View.bMeterVisible && !View.MeterLabel.IsEmpty());
		if (View.bMeterVisible)
		{
			MeterBar->SetPercent(FMath::Clamp(View.MeterFraction, 0.f, 1.f));
			const bool bPulse = View.bMeterDanger && FMath::Fmod(RealSeconds, 0.5f) < 0.25f;
			MeterBar->SetFillColorAndOpacity(View.bMeterDanger ? (bPulse ? FLinearColor(1.f, 0.6f, 0.5f) : RecRed) : Gold);
			SetTextIfChanged(MeterText, View.MeterLabel);
			MeterText->SetColorAndOpacity(FSlateColor(View.bMeterDanger ? FLinearColor(1.f, 0.55f, 0.45f) : Dim));
		}
	}

	Show(HintPanel, !View.Hint.IsEmpty());
	SetTextIfChanged(HintText, View.Hint);

	// Interact prompt.
	Show(PromptPanel, View.bPromptVisible);
	if (View.bPromptVisible)
	{
		SetTextIfChanged(PromptText, View.Prompt);
		if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(PromptPanel->Slot)) { S->SetPosition(View.PromptPosition); }
	}

	// Bubbles.
	for (int32 i = 0; i < View.Bubbles.Num(); ++i)
	{
		const FJamHudBubble& Data = View.Bubbles[i];
		UBorder* B = GetBubble(i);
		Show(B, true);
		B->SetBrush(Data.bJagged ? Rounded(FLinearColor::White, 2.f, RecRed, 3.f) : Rounded(FLinearColor::White, 18.f, FLinearColor(0.6f, 0.4f, 0.9f), 2.f));
		B->SetBrushColor(FLinearColor::White);
		SetTextIfChanged(Cast<UTextBlock>(B->GetContent()), Data.Text);
		if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(B->Slot)) { S->SetPosition(Data.Position); }
	}
	for (int32 i = View.Bubbles.Num(); i < Bubbles.Num(); ++i) { Show(Bubbles[i], false); }

	// The card, with a short pop when a new one appears.
	const bool bCard = View.CardKind != EJamCardKind::None && !View.CardTitle.IsEmpty();
	Show(CardPanel, bCard);
	if (!bCard)
	{
		ShownCardTitle.Reset();
		return;
	}
	if (!ShownCardTitle.Equals(View.CardTitle, ESearchCase::CaseSensitive))
	{
		ShownCardTitle = View.CardTitle;
		CardShownAt = RealSeconds;
	}
	const FJamCardStyle Style = JamHud::CardStyle(View.CardKind);
	CardPanel->SetBrushColor(Style.Panel);
	CardAccent->SetBrushColor(Style.Accent);
	CardTitle->SetColorAndOpacity(FSlateColor(Style.Accent));
	SetTextIfChanged(CardTitle, View.CardTitle);
	SetTextIfChanged(CardSubtitle, View.CardSubtitle);
	Show(CardSubtitle, !View.CardSubtitle.IsEmpty());
	const float T = FMath::Clamp((RealSeconds - CardShownAt) / 0.18f, 0.f, 1.f);
	const float Scale = T < 1.f ? FMath::Lerp(1.35f, 1.f, FMath::InterpEaseOut(0.f, 1.f, T, 3.f)) : 1.f;
	CardPanel->SetRenderScale(FVector2D(Scale, Scale));
	CardPanel->SetRenderOpacity(FMath::Clamp(T * 2.f, 0.f, 1.f));
}
