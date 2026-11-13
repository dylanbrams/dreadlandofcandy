#include "UI/JamHudWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Spacer.h"
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

	// Failure flash: a red film-burn over the world, under the panels.
	FlashLayer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	FlashLayer->SetBrushColor(FLinearColor(0.9f, 0.15f, 0.05f, 1.f));
	FlashLayer->SetVisibility(ESlateVisibility::Collapsed);
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(FlashLayer);
		S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		S->SetOffsets(FMargin(0.f));
		S->SetZOrder(1);   // tints the world, not the panels
	}

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
	{
		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Box->AddChildToVerticalBox(PromptText);
		USizeBox* FillSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		FillSize->SetHeightOverride(5.f);
		PromptFill = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		PromptFill->SetFillColorAndOpacity(Gold);
		FillSize->SetContent(PromptFill);
		Box->AddChildToVerticalBox(FillSize)->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
		PromptPanel->SetContent(Box);
	}
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
		NotesText = MakeText(16, Dim, false);
		Box->AddChildToVerticalBox(NotesText)->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
		// R4b: Dignity, a label and a bar under the take.
		UHorizontalBox* DRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		DignityLabel = MakeText(16, Dim, true);
		DignityLabel->SetText(FText::FromString(TEXT("DIGNITY")));
		DRow->AddChildToHorizontalBox(DignityLabel)->SetVerticalAlignment(VAlign_Center);
		USizeBox* DSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		DSize->SetWidthOverride(220.f);
		DSize->SetHeightOverride(12.f);
		DignityBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		{
			// A dark track, so an empty bar reads as empty.
			FProgressBarStyle Style = DignityBar->GetWidgetStyle();
			Style.BackgroundImage.TintColor = FSlateColor(FLinearColor(0.08f, 0.08f, 0.09f, 0.9f));
			DignityBar->SetWidgetStyle(Style);
		}
		DSize->SetContent(DignityBar);
		UHorizontalBoxSlot* DBarSlot = DRow->AddChildToHorizontalBox(DSize);
		DBarSlot->SetVerticalAlignment(VAlign_Center);
		DBarSlot->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));
		DignityRow = DRow;
		Box->AddChildToVerticalBox(DRow)->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));
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

	// Bottom centre: Fenella's caption, film-subtitle style, her name in her colour.
	CaptionPanel = MakePanel(FLinearColor(0.f, 0.f, 0.f, 0.72f), 6.f, FMargin(22.f, 10.f));
	{
		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		CaptionSpeaker = MakeText(16, FLinearColor(0.8f, 0.6f, 1.f), true);
		CaptionSpeaker->SetText(FText::FromString(TEXT("FENELLA")));
		Box->AddChildToVerticalBox(CaptionSpeaker)->SetHorizontalAlignment(HAlign_Center);
		CaptionText = MakeText(28, FLinearColor::White, false);
		CaptionText->SetJustification(ETextJustify::Center);
		CaptionText->SetWrapTextAt(980.f);
		Box->AddChildToVerticalBox(CaptionText)->SetHorizontalAlignment(HAlign_Center);
		CaptionPanel->SetContent(Box);
	}
	Place(CaptionPanel, FAnchors(0.5f, 1.f), FVector2D(0.5f, 1.f), FVector2D(0.f, -60.f), 15);

	// CUT!: the note prompt. Her pitch, then two tiles: take (hold) or refuse.
	NotePanel = MakePanel(FLinearColor(0.07f, 0.01f, 0.01f, 0.92f), 10.f, FMargin(48.f, 0.f, 48.f, 32.f));
	{
		const FJamCardStyle CutStyle = JamHud::CardStyle(EJamCardKind::Cut);
		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		BarSize->SetHeightOverride(10.f);
		BarSize->SetContent(MakePanel(CutStyle.Accent, 3.f, FMargin(0.f)));
		UVerticalBoxSlot* BarSlot = Box->AddChildToVerticalBox(BarSize);
		BarSlot->SetHorizontalAlignment(HAlign_Fill);
		BarSlot->SetPadding(FMargin(-32.f, 18.f, -32.f, 6.f));
		UTextBlock* Cut = MakeText(72, CutStyle.Accent, true);
		Cut->SetText(FText::FromString(TEXT("CUT!")));
		Cut->SetShadowOffset(FVector2D(4.f, 4.f));
		Box->AddChildToVerticalBox(Cut)->SetHorizontalAlignment(HAlign_Center);
		UTextBlock* Has = MakeText(18, FLinearColor(0.8f, 0.6f, 1.f), true);
		Has->SetText(FText::FromString(TEXT("FENELLA HAS A NOTE")));
		Box->AddChildToVerticalBox(Has)->SetHorizontalAlignment(HAlign_Center);
		NotePitchText = MakeText(30, FLinearColor::White, false);
		NotePitchText->SetJustification(ETextJustify::Center);
		NotePitchText->SetWrapTextAt(980.f);
		UVerticalBoxSlot* PitchSlot = Box->AddChildToVerticalBox(NotePitchText);
		PitchSlot->SetHorizontalAlignment(HAlign_Center);
		PitchSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 22.f));

		UHorizontalBox* Tiles = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTextBlock* TakeKey = nullptr;
		UTextBlock* TakeSub = nullptr;
		UProgressBar* Fill = nullptr;
		UTextBlock* RefuseKey = nullptr;
		UTextBlock* RefuseSub = nullptr;
		UTextBlock* WriteKey = nullptr;
		UTextBlock* WriteSub = nullptr;
		UBorder* TakeTile = MakeChoiceTile(FLinearColor(0.45f, 1.f, 0.55f), TEXT("TAKE THE NOTE"), TakeKey, TakeSub, &Fill);
		UBorder* RefuseTile = MakeChoiceTile(FLinearColor(1.f, 0.55f, 0.3f), TEXT("REFUSE"), RefuseKey, RefuseSub, nullptr);
		UBorder* WriteTile = MakeChoiceTile(FLinearColor(0.75f, 0.55f, 1.f), TEXT("YOUR OWN NOTE"), WriteKey, WriteSub, nullptr);
		WriteKeyText = WriteKey;
		WriteSub->SetText(FText::FromString(TEXT("Type anything. She'll shoot it.\nProbably.")));
		TakeKeyText = TakeKey;
		TakeSubText = TakeSub;
		TakeFill = Fill;
		RefuseKeyText = RefuseKey;
		RefuseSub->SetText(FText::FromString(TEXT("Your film, your way.\nShe will sulk. Audibly.")));
		Tiles->AddChildToHorizontalBox(TakeTile)->SetPadding(FMargin(0.f, 0.f, 14.f, 0.f));
		Tiles->AddChildToHorizontalBox(WriteTile)->SetPadding(FMargin(14.f, 0.f, 14.f, 0.f));
		Tiles->AddChildToHorizontalBox(RefuseTile)->SetPadding(FMargin(14.f, 0.f, 0.f, 0.f));
		Box->AddChildToVerticalBox(Tiles)->SetHorizontalAlignment(HAlign_Center);
		NoteTiles = Tiles;
		NoteReadingText = MakeText(28, FLinearColor(0.8f, 0.6f, 1.f), true);
		NoteReadingText->SetJustification(ETextJustify::Center);
		NoteReadingText->SetWrapTextAt(1100.f);
		UVerticalBoxSlot* ReadingSlot = Box->AddChildToVerticalBox(NoteReadingText);
		ReadingSlot->SetHorizontalAlignment(HAlign_Center);
		ReadingSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 10.f));
		NotePanel->SetContent(Box);
	}
	Place(NotePanel, FAnchors(0.5f, 0.45f), FVector2D(0.5f, 0.5f), FVector2D::ZeroVector, 21);
	NotePanel->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

	BuildWrapPanel();
	BuildSkillBar();

	Show(NotePanel, false);
	Show(CaptionPanel, false);
	Show(PromptPanel, false);
	Show(CardPanel, false);
	Show(HintPanel, false);
}

UBorder* UJamHudWidget::MakeChoiceTile(const FLinearColor& Accent, const TCHAR* Title, UTextBlock*& OutKey, UTextBlock*& OutSub, UProgressBar** OutFill)
{
	UBorder* Tile = MakePanel(FLinearColor(0.f, 0.f, 0.f, 0.55f), 8.f, FMargin(24.f, 16.f));
	Tile->SetBrush(Rounded(FLinearColor::White, 8.f, Accent, 2.f));
	Tile->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.55f));
	USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Size->SetWidthOverride(400.f);
	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	OutKey = MakeText(20, Accent, true);
	Box->AddChildToVerticalBox(OutKey)->SetHorizontalAlignment(HAlign_Center);
	UTextBlock* Head = MakeText(34, FLinearColor::White, true);
	Head->SetText(FText::FromString(Title));
	Box->AddChildToVerticalBox(Head)->SetHorizontalAlignment(HAlign_Center);
	OutSub = MakeText(19, Dim, false);
	OutSub->SetJustification(ETextJustify::Center);
	OutSub->SetWrapTextAt(360.f);
	Box->AddChildToVerticalBox(OutSub)->SetHorizontalAlignment(HAlign_Center);
	if (OutFill)
	{
		USizeBox* FillSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		FillSize->SetHeightOverride(8.f);
		*OutFill = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		(*OutFill)->SetFillColorAndOpacity(Accent);
		FillSize->SetContent(*OutFill);
		Box->AddChildToVerticalBox(FillSize)->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));
	}
	Size->SetContent(Box);
	Tile->SetContent(Size);
	return Tile;
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

	Show(FlashLayer, View.FailFlash > 0.f);
	if (View.FailFlash > 0.f) { FlashLayer->SetRenderOpacity(0.55f * View.FailFlash * View.FailFlash); }

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
		const bool bNotes = View.NotesTaken + View.NotesRefused > 0;
		Show(NotesText, bNotes);
		if (bNotes) { SetTextIfChanged(NotesText, FString::Printf(TEXT("Notes taken %d   refused %d"), View.NotesTaken, View.NotesRefused)); }
		Show(DignityRow, View.bDignityVisible);
		if (View.bDignityVisible)
		{
			DignityBar->SetPercent(FMath::Clamp(View.DignityFraction, 0.f, 1.f));
			const bool bLow = View.DignityFraction <= 0.4f;
			const bool bFlash = View.DignityHit > 0.f && FMath::Fmod(RealSeconds, 0.16f) < 0.08f;
			DignityBar->SetFillColorAndOpacity(bFlash ? FLinearColor::White : (bLow ? RecRed : Gold));
			DignityLabel->SetColorAndOpacity(FSlateColor(bLow ? FLinearColor(1.f, 0.55f, 0.45f) : Dim));
		}
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
		PromptFill->SetPercent(View.HoldFraction);
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

	Show(CaptionPanel, !View.Caption.IsEmpty());
	SetTextIfChanged(CaptionText, View.Caption);
	SetTextIfChanged(CaptionSpeaker, View.bSulking ? TEXT("FENELLA (SULKING)") : TEXT("FENELLA"));

	Show(NotePanel, View.bNotePrompt);
	if (View.bNotePrompt)
	{
		// The pitch lines open with "CUT!", which the panel already shouts.
		FString Pitch = View.NotePitch;
		if (Pitch.StartsWith(TEXT("CUT!"))) { Pitch.RightChopInline(4); Pitch.TrimStartInline(); }
		SetTextIfChanged(NotePitchText, FString::Printf(TEXT("\u201C%s\u201D"), *Pitch));
		SetTextIfChanged(TakeKeyText, View.bGamepad ? TEXT("HOLD (A)") : TEXT("HOLD [E]"));
		SetTextIfChanged(RefuseKeyText, View.bGamepad ? TEXT("PRESS (B)") : TEXT("PRESS [Q]"));
		SetTextIfChanged(WriteKeyText, View.bGamepad ? TEXT("PRESS (Y)") : TEXT("PRESS [T]"));
		Show(NoteTiles, !View.bReadingNote);
		Show(NoteReadingText, View.bReadingNote);
		if (View.bReadingNote)
		{
			// Dots that count up while she reads.
			const int32 Dots = 1 + (int32)(RealSeconds * 2.f) % 3;
			SetTextIfChanged(NoteReadingText, FString::Printf(TEXT("“%s”\n\nShe is reading it%s"), *View.ReadingNote, *FString::ChrN(Dots, TEXT('.'))));
		}
		SetTextIfChanged(TakeSubText, FString::Printf(TEXT("Her way. Box office x%.1f.\nThe next shot changes."), View.NoteGross));
		TakeFill->SetPercent(View.HoldFraction);
	}

	ApplySkills(View, RealSeconds);
	ApplyWrap(View.Wrap);
	if (View.Wrap.bVisible)
	{
		if (!bWrapShown) { bWrapShown = true; WrapShownAt = RealSeconds; }
		const float W = FMath::Clamp((RealSeconds - WrapShownAt) / 0.22f, 0.f, 1.f);
		const float WS = FMath::Lerp(1.15f, 1.f, FMath::InterpEaseOut(0.f, 1.f, W, 3.f));
		WrapPanel->SetRenderScale(FVector2D(WS, WS));
		WrapPanel->SetRenderOpacity(FMath::Clamp(W * 2.f, 0.f, 1.f));
	}
	else { bWrapShown = false; }

	// The card, with a short pop when a new one appears. The note prompt and the wrap panel take its place.
	const bool bCard = !View.bNotePrompt && !View.Wrap.bVisible && View.CardKind != EJamCardKind::None && !View.CardTitle.IsEmpty();
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

// ---- Wrap panel (T14) -------------------------------------------------------------------------------------------

namespace
{
	constexpr int32 MaxLedgerRows = 8;
	constexpr int32 LootSlots = 3;

	FLinearColor RarityColour(int32 Rarity)
	{
		switch (Rarity)
		{
		case 0:  return FLinearColor(0.75f, 0.75f, 0.72f);
		case 1:  return FLinearColor(0.4f, 0.95f, 0.45f);
		case 2:  return FLinearColor(0.35f, 0.65f, 1.f);
		case 3:  return FLinearColor(1.f, 0.6f, 0.15f);
		default: return FLinearColor(0.95f, 0.35f, 0.9f);   // Director's Personal Collection
		}
	}

	FString Sprinkles(int32 Amount) { return FString::Printf(TEXT("%d"), Amount); }
}

void UJamHudWidget::BuildWrapPanel()
{
	const FJamCardStyle Style = JamHud::CardStyle(EJamCardKind::Wrap);
	WrapPanel = MakePanel(FLinearColor(0.025f, 0.025f, 0.03f, 0.95f), 12.f, FMargin(44.f, 0.f, 44.f, 26.f));
	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

	USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	BarSize->SetHeightOverride(10.f);
	BarSize->SetContent(MakePanel(Style.Accent, 3.f, FMargin(0.f)));
	UVerticalBoxSlot* BarSlot = Box->AddChildToVerticalBox(BarSize);
	BarSlot->SetHorizontalAlignment(HAlign_Fill);
	BarSlot->SetPadding(FMargin(-28.f, 18.f, -28.f, 4.f));

	UTextBlock* Title = MakeText(60, Style.Accent, true);
	Title->SetText(FText::FromString(TEXT("THAT'S A WRAP!")));
	Title->SetShadowOffset(FVector2D(4.f, 4.f));
	Box->AddChildToVerticalBox(Title)->SetHorizontalAlignment(HAlign_Center);
	WrapScene = MakeText(20, Dim, false);
	UVerticalBoxSlot* SceneSlot = Box->AddChildToVerticalBox(WrapScene);
	SceneSlot->SetHorizontalAlignment(HAlign_Center);
	SceneSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 18.f));

	UHorizontalBox* Columns = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	Box->AddChildToVerticalBox(Columns);

	// Left: the ledger.
	USizeBox* LedgerSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	LedgerSize->SetWidthOverride(620.f);
	UVerticalBox* Ledger = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	LedgerSize->SetContent(Ledger);
	Columns->AddChildToHorizontalBox(LedgerSize)->SetVerticalAlignment(VAlign_Top);

	auto AmountRow = [this](UVerticalBox* Into, const TCHAR* Label, int32 Size, const FLinearColor& Colour, UTextBlock*& OutAmount)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTextBlock* L = MakeText(Size, Colour, true);
		L->SetText(FText::FromString(Label));
		UHorizontalBoxSlot* LS = Row->AddChildToHorizontalBox(L);
		LS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LS->SetVerticalAlignment(VAlign_Center);
		OutAmount = MakeText(Size, Colour, true);
		Row->AddChildToHorizontalBox(OutAmount)->SetVerticalAlignment(VAlign_Center);
		return Into->AddChildToVerticalBox(Row);
	};
	UTextBlock* GrossAmount = nullptr;
	UTextBlock* LedgerHead = MakeText(15, Dim, true);
	LedgerHead->SetText(FText::FromString(TEXT("THE PRODUCTION LEDGER  (sprinkles)")));
	Ledger->AddChildToVerticalBox(LedgerHead)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
	AmountRow(Ledger, TEXT("GROSS"), 26, FLinearColor::White, GrossAmount)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
	WrapGross = GrossAmount;

	for (int32 i = 0; i < MaxLedgerRows; ++i)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBox* Words = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Item = MakeText(20, FLinearColor::White, false);
		UTextBlock* Note = MakeText(15, Dim, false);
		Words->AddChildToVerticalBox(Item);
		Words->AddChildToVerticalBox(Note);
		UHorizontalBoxSlot* WS = Row->AddChildToHorizontalBox(Words);
		WS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		UTextBlock* Amount = MakeText(20, FLinearColor(1.f, 0.55f, 0.45f), true);
		Row->AddChildToHorizontalBox(Amount)->SetVerticalAlignment(VAlign_Center);
		Ledger->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 3.f));
		LedgerRows.Add(Row);
		LedgerItems.Add(Item);
		LedgerNotes.Add(Note);
		LedgerAmounts.Add(Amount);
	}
	USizeBox* Rule = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Rule->SetHeightOverride(2.f);
	Rule->SetContent(MakePanel(FLinearColor(1.f, 1.f, 1.f, 0.4f), 0.f, FMargin(0.f)));
	Ledger->AddChildToVerticalBox(Rule)->SetPadding(FMargin(0.f, 10.f, 0.f, 8.f));
	UTextBlock* CutAmount = nullptr;
	AmountRow(Ledger, TEXT("HERO'S CUT"), 34, Gold, CutAmount);
	WrapCut = CutAmount;
	WrapFooter = MakeText(17, Dim, false);
	Ledger->AddChildToVerticalBox(WrapFooter)->SetHorizontalAlignment(HAlign_Right);

	Columns->AddChildToHorizontalBox(WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass()))->SetPadding(FMargin(22.f, 0.f));

	// Right: three loot cards, then the character sheet.
	UVerticalBox* Right = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Columns->AddChildToHorizontalBox(Right)->SetVerticalAlignment(VAlign_Top);
	UTextBlock* LootHead = MakeText(15, Dim, true);
	LootHead->SetText(FText::FromString(TEXT("LOOT")));
	Right->AddChildToVerticalBox(LootHead)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
	UHorizontalBox* Cards = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	Right->AddChildToVerticalBox(Cards);
	for (int32 i = 0; i < LootSlots; ++i)
	{
		UOverlay* Stack = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		UBorder* Card = MakePanel(FLinearColor(0.f, 0.f, 0.f, 0.6f), 8.f, FMargin(14.f, 12.f));
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(232.f);
		Size->SetMinDesiredHeight(250.f);
		UVerticalBox* V = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Rarity = MakeText(14, Dim, true);
		Rarity->SetWrapTextAt(204.f);
		UTextBlock* Name = MakeText(20, FLinearColor::White, true);
		Name->SetWrapTextAt(204.f);
		UTextBlock* Flavour = MakeText(15, Dim, false);
		Flavour->SetWrapTextAt(204.f);
		UTextBlock* Stat = MakeText(18, Gold, true);
		Stat->SetWrapTextAt(204.f);
		UTextBlock* Tag = MakeText(16, Gold, true);
		V->AddChildToVerticalBox(Rarity);
		V->AddChildToVerticalBox(Name)->SetPadding(FMargin(0.f, 4.f));
		V->AddChildToVerticalBox(Flavour)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		V->AddChildToVerticalBox(Stat);
		V->AddChildToVerticalBox(Tag)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		Size->SetContent(V);
		Card->SetContent(Size);
		UOverlaySlot* CardSlot = Stack->AddChildToOverlay(Card);
		CardSlot->SetHorizontalAlignment(HAlign_Fill);
		CardSlot->SetVerticalAlignment(VAlign_Fill);

		// Her stamp, slapped across the card at an angle.
		UBorder* Stamp = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Stamp->SetBrush(Rounded(FLinearColor::White, 4.f, FLinearColor(0.95f, 0.15f, 0.1f), 4.f));
		Stamp->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.35f));
		Stamp->SetPadding(FMargin(10.f, 4.f));
		UTextBlock* StampText = MakeText(22, FLinearColor(1.f, 0.2f, 0.12f), true);
		StampText->SetJustification(ETextJustify::Center);
		StampText->SetWrapTextAt(200.f);
		Stamp->SetContent(StampText);
		Stamp->SetRenderTransformAngle(-14.f);
		Stamp->SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot* StampSlot = Stack->AddChildToOverlay(Stamp);
		StampSlot->SetHorizontalAlignment(HAlign_Center);
		StampSlot->SetVerticalAlignment(VAlign_Bottom);
		StampSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 44.f));   // over her card's flavour, clear of the name

		Cards->AddChildToHorizontalBox(Stack)->SetPadding(FMargin(i == 0 ? 0.f : 8.f, 0.f, 0.f, 0.f));
		LootCards.Add(Card);
		LootRarity.Add(Rarity);
		LootName.Add(Name);
		LootFlavour.Add(Flavour);
		LootStat.Add(Stat);
		LootTag.Add(Tag);
		LootStamp.Add(Stamp);
	}

	UTextBlock* SheetHead = MakeText(15, Dim, true);
	SheetHead->SetText(FText::FromString(TEXT("CHARACTER SHEET")));
	Right->AddChildToVerticalBox(SheetHead)->SetPadding(FMargin(0.f, 16.f, 0.f, 4.f));
	for (int32 i = 0; i < 4; ++i)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTextBlock* N = MakeText(19, FLinearColor::White, false);
		UTextBlock* Val = MakeText(19, FLinearColor::White, true);
		UHorizontalBoxSlot* NS = Row->AddChildToHorizontalBox(N);
		NS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		Row->AddChildToHorizontalBox(Val);
		Right->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 2.f));
		StatNames.Add(N);
		StatValues.Add(Val);
	}
	WrapLevel = MakeText(24, Gold, true);
	Right->AddChildToVerticalBox(WrapLevel)->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));

	WrapContinue = MakeText(18, Dim, true);
	UVerticalBoxSlot* ContSlot = Box->AddChildToVerticalBox(WrapContinue);
	ContSlot->SetHorizontalAlignment(HAlign_Center);
	ContSlot->SetPadding(FMargin(0.f, 18.f, 0.f, 0.f));

	WrapPanel->SetContent(Box);
	UCanvasPanelSlot* S = Root->AddChildToCanvas(WrapPanel);
	S->SetAnchors(FAnchors(0.5f, 0.5f));
	S->SetAlignment(FVector2D(0.5f, 0.5f));
	S->SetPosition(FVector2D(0.f, 10.f));
	S->SetAutoSize(true);
	S->SetZOrder(22);
	WrapPanel->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Show(WrapPanel, false);
}

void UJamHudWidget::ApplyWrap(const FJamWrapView& Wrap)
{
	Show(WrapPanel, Wrap.bVisible);
	if (!Wrap.bVisible) { return; }

	SetTextIfChanged(WrapScene, FString::Printf(TEXT("\u201C%s\u201D"), *Wrap.SceneTitle));
	SetTextIfChanged(WrapGross, Sprinkles(Wrap.Gross));
	for (int32 i = 0; i < LedgerRows.Num(); ++i)
	{
		const bool bRow = Wrap.Lines.IsValidIndex(i);
		Show(LedgerRows[i], bRow);
		if (!bRow) { continue; }
		SetTextIfChanged(LedgerItems[i], Wrap.Lines[i].Item);
		SetTextIfChanged(LedgerNotes[i], Wrap.Lines[i].Note);
		SetTextIfChanged(LedgerAmounts[i], FString::Printf(TEXT("-%d"), Wrap.Lines[i].Amount));
	}
	SetTextIfChanged(WrapCut, Sprinkles(Wrap.HeroCut));
	SetTextIfChanged(WrapFooter, Wrap.Footer);

	for (int32 i = 0; i < LootCards.Num(); ++i)
	{
		const bool bCard = Wrap.Loot.IsValidIndex(i);
		Show(LootCards[i]->GetParent(), bCard);
		if (!bCard) { continue; }
		const FJamWrapLoot& L = Wrap.Loot[i];
		const FLinearColor Colour = RarityColour(L.RarityLevel);
		LootCards[i]->SetBrush(Rounded(FLinearColor::White, 8.f, L.bEquipped ? Gold : Colour, L.bEquipped ? 3.f : 1.5f));
		LootCards[i]->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, L.bKept ? 0.75f : 0.6f));
		SetTextIfChanged(LootRarity[i], L.Rarity.ToUpper());
		LootRarity[i]->SetColorAndOpacity(FSlateColor(Colour));
		SetTextIfChanged(LootName[i], L.Name);
		SetTextIfChanged(LootFlavour[i], FString::Printf(TEXT("\u201C%s\u201D"), *L.Flavour.TrimQuotes()));
		SetTextIfChanged(LootStat[i], L.Stat);
		SetTextIfChanged(LootTag[i], L.bEquipped ? TEXT("EQUIPPED") : FString());
		Show(LootStamp[i], L.bKept && !L.Stamp.IsEmpty());
		if (L.bKept) { SetTextIfChanged(Cast<UTextBlock>(LootStamp[i]->GetContent()), L.Stamp); }
	}

	for (int32 i = 0; i < StatNames.Num(); ++i)
	{
		const bool bRow = Wrap.Stats.IsValidIndex(i);
		Show(StatNames[i], bRow);
		Show(StatValues[i], bRow);
		if (!bRow) { continue; }
		const FJamWrapStat& St = Wrap.Stats[i];
		const bool bFraction = St.Unit == TEXT("x");
		auto Fmt = [bFraction, &St](float V) { return bFraction ? FString::Printf(TEXT("%.2f%s"), V, *St.Unit) : FString::Printf(TEXT("%.0f%s"), V, *St.Unit); };
		SetTextIfChanged(StatNames[i], St.Name);
		const bool bChanged = !FMath::IsNearlyEqual(St.Before, St.After, 0.005f);
		SetTextIfChanged(StatValues[i], bChanged ? FString::Printf(TEXT("%s  \u2192  %s"), *Fmt(St.Before), *Fmt(St.After)) : Fmt(St.After));
		const bool bBetter = St.After > St.Before;
		StatValues[i]->SetColorAndOpacity(FSlateColor(!bChanged ? FLinearColor::White : (bBetter ? FLinearColor(0.45f, 1.f, 0.55f) : FLinearColor(1.f, 0.5f, 0.4f))));
	}
	Show(WrapLevel, !Wrap.LevelLine.IsEmpty());
	SetTextIfChanged(WrapLevel, Wrap.LevelLine);
	Show(WrapContinue, !Wrap.ContinueHint.IsEmpty());
	SetTextIfChanged(WrapContinue, Wrap.ContinueHint);
}

// ---- Skill bar (T14a) -------------------------------------------------------------------------------------------

void UJamHudWidget::BuildSkillBar()
{
	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UBorder* Panel = MakePanel(PanelDark, 8.f, FMargin(12.f, 10.f));
	Panel->SetContent(Box);
	SkillBar = Panel;

	UHorizontalBox* XpRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	LevelText = MakeText(18, Gold, true);
	XpRow->AddChildToHorizontalBox(LevelText)->SetVerticalAlignment(VAlign_Center);
	USizeBox* XpSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	XpSize->SetWidthOverride(220.f);
	XpSize->SetHeightOverride(8.f);
	XpBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	XpBar->SetFillColorAndOpacity(FLinearColor(0.55f, 0.75f, 1.f));
	XpSize->SetContent(XpBar);
	UHorizontalBoxSlot* XpSlot = XpRow->AddChildToHorizontalBox(XpSize);
	XpSlot->SetVerticalAlignment(VAlign_Center);
	XpSlot->SetPadding(FMargin(10.f, 0.f));
	PointsText = MakeText(16, Gold, true);
	XpRow->AddChildToHorizontalBox(PointsText)->SetVerticalAlignment(VAlign_Center);
	Box->AddChildToVerticalBox(XpRow)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

	UHorizontalBox* Tiles = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	for (int32 i = 0; i < (int32)EJamSkill::Count; ++i)
	{
		UOverlay* Stack = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		UBorder* Tile = MakePanel(FLinearColor(0.f, 0.f, 0.f, 0.55f), 6.f, FMargin(6.f, 4.f));
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(86.f);
		Size->SetHeightOverride(80.f);
		UVerticalBox* V = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Key = MakeText(15, Gold, true);
		UTextBlock* Name = MakeText(12, FLinearColor::White, true);
		Name->SetWrapTextAt(80.f);
		Name->SetJustification(ETextJustify::Center);
		UTextBlock* Rank = MakeText(13, Gold, false);
		V->AddChildToVerticalBox(Key)->SetHorizontalAlignment(HAlign_Center);
		UVerticalBoxSlot* NameSlot = V->AddChildToVerticalBox(Name);
		NameSlot->SetHorizontalAlignment(HAlign_Center);
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		V->AddChildToVerticalBox(Rank)->SetHorizontalAlignment(HAlign_Center);
		Size->SetContent(V);
		Tile->SetContent(Size);
		Stack->AddChildToOverlay(Tile);
		// Cooldown sweep: a dark fill that drains from the top as the skill comes back.
		UProgressBar* Cool = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		Cool->SetBarFillType(EProgressBarFillType::TopToBottom);
		Cool->SetFillColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.65f));
		FProgressBarStyle CoolStyle = Cool->GetWidgetStyle();
		CoolStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor::Transparent);
		Cool->SetWidgetStyle(CoolStyle);
		UOverlaySlot* CoolSlot = Stack->AddChildToOverlay(Cool);
		CoolSlot->SetHorizontalAlignment(HAlign_Fill);
		CoolSlot->SetVerticalAlignment(VAlign_Fill);
		Tiles->AddChildToHorizontalBox(Stack)->SetPadding(FMargin(i == 0 ? 0.f : 6.f, 0.f, 0.f, 0.f));
		SkillTiles.Add(Tile);
		SkillKeys.Add(Key);
		SkillNames.Add(Name);
		SkillRanks.Add(Rank);
		SkillCooldowns.Add(Cool);
	}
	Box->AddChildToVerticalBox(Tiles);

	UCanvasPanelSlot* S = Root->AddChildToCanvas(Panel);
	S->SetAnchors(FAnchors(0.f, 1.f));
	S->SetAlignment(FVector2D(0.f, 1.f));
	S->SetPosition(FVector2D(36.f, -28.f));
	S->SetAutoSize(true);
	S->SetZOrder(5);
}

void UJamHudWidget::ApplySkills(const FJamHudView& View, float RealSeconds)
{
	const bool bBar = View.bShootVisible && View.Skills.Num() > 0;
	Show(SkillBar, bBar);
	if (!bBar) { return; }
	SetTextIfChanged(LevelText, FString::Printf(TEXT("LV %d"), View.Level));
	XpBar->SetPercent(FMath::Clamp(View.Xp, 0.f, 1.f));
	Show(PointsText, View.Points > 0);
	SetTextIfChanged(PointsText, FString::Printf(TEXT("+%d point%s"), View.Points, View.Points == 1 ? TEXT("") : TEXT("s")));
	const bool bPulse = FMath::Fmod(RealSeconds, 0.8f) < 0.4f;
	for (int32 i = 0; i < SkillTiles.Num(); ++i)
	{
		const bool bSlot = View.Skills.IsValidIndex(i);
		Show(SkillTiles[i]->GetParent(), bSlot);
		if (!bSlot) { continue; }
		const FJamHudSkill& K = View.Skills[i];
		SetTextIfChanged(SkillKeys[i], K.Key);
		SetTextIfChanged(SkillNames[i], K.Name);
		FString Pips;
		for (int32 r = 0; r < K.MaxRank; ++r) { Pips += r < K.Rank ? TEXT("\u25CF") : TEXT("\u25CB"); }
		SetTextIfChanged(SkillRanks[i], K.bUnlocked ? Pips : FString(TEXT("locked")));
		const FLinearColor Edge = K.bSpendable ? (bPulse ? Gold : FLinearColor(1.f, 0.95f, 0.7f)) : FLinearColor(1.f, 1.f, 1.f, K.bUnlocked ? 0.25f : 0.1f);
		SkillTiles[i]->SetBrush(Rounded(FLinearColor::White, 6.f, Edge, K.bSpendable ? 3.f : 1.f));
		SkillTiles[i]->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.55f));
		SkillTiles[i]->GetParent()->SetRenderOpacity(K.bUnlocked || K.bSpendable ? 1.f : 0.45f);
		SkillCooldowns[i]->SetPercent(FMath::Clamp(K.Cooldown, 0.f, 1.f));
	}
}
