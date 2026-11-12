#include "UI/JamScreenWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"

namespace
{
	const FLinearColor ScreenGold(1.f, 0.82f, 0.3f);
	const FLinearColor ScreenDim(0.72f, 0.72f, 0.7f);
	const FLinearColor StampRed(1.f, 0.2f, 0.12f);
	constexpr int32 MaxRows = 4;
	constexpr int32 MaxQuotes = 6;
	constexpr int32 MaxOptions = 8;
	constexpr int32 MaxCredits = 40;
	constexpr float CreditsClipHeight = 560.f;
	constexpr float CreditLineHeight = 46.f;

	void SetScreenText(UTextBlock* Block, const FString& Text)
	{
		if (Block && !Block->GetText().ToString().Equals(Text, ESearchCase::CaseSensitive)) { Block->SetText(FText::FromString(Text)); }
	}

	void ShowScreenPart(UWidget* W, bool bVisible)
	{
		if (!W) { return; }
		const ESlateVisibility V = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (W->GetVisibility() != V) { W->SetVisibility(V); }
	}

	FSlateBrush ScreenRounded(const FLinearColor& Outline, float Radius, float OutlineWidth)
	{
		FSlateBrush B;
		B.DrawAs = ESlateBrushDrawType::RoundedBox;
		B.TintColor = FSlateColor(FLinearColor::White);
		B.OutlineSettings = FSlateBrushOutlineSettings(FVector4(Radius, Radius, Radius, Radius), FSlateColor(Outline), OutlineWidth);
		B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		return B;
	}

	bool IsAny(const FKey& K, std::initializer_list<FKey> Keys)
	{
		for (const FKey& X : Keys) { if (K == X) { return true; } }
		return false;
	}
}

void UJamScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
}

TSharedRef<SWidget> UJamScreenWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget) { BuildTree(); }
	return Super::RebuildWidget();
}

UTextBlock* UJamScreenWidget::MakeText(int32 Size, const FLinearColor& Colour, bool bBold)
{
	UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	T->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size));
	T->SetColorAndOpacity(FSlateColor(Colour));
	T->SetShadowOffset(FVector2D(2.f, 2.f));
	T->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));
	T->SetJustification(ETextJustify::Center);
	return T;
}

UBorder* UJamScreenWidget::MakePanel(const FLinearColor& Colour, float Radius, const FMargin& InPadding)
{
	UBorder* B = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	B->SetBrush(ScreenRounded(FLinearColor::Transparent, Radius, 0.f));
	B->SetBrushColor(Colour);
	B->SetPadding(InPadding);
	return B;
}

void UJamScreenWidget::BuildTree()
{
	Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = Root;

	Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Backdrop->SetBrushColor(FLinearColor(0.02f, 0.015f, 0.03f, 0.94f));
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(Backdrop);
		S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		S->SetOffsets(FMargin(0.f));
	}

	Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	auto Add = [this](UWidget* W, const FMargin& Pad = FMargin(0.f))
	{
		UVerticalBoxSlot* S = Column->AddChildToVerticalBox(W);
		S->SetHorizontalAlignment(HAlign_Center);
		S->SetPadding(Pad);
		return S;
	};

	Kicker = MakeText(24, ScreenDim, true);
	Add(Kicker, FMargin(0.f, 0.f, 0.f, 6.f));

	// The title, with a rubber stamp slapped across it.
	UOverlay* TitleStack = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Title = MakeText(84, ScreenGold, true);
	Title->SetShadowOffset(FVector2D(5.f, 5.f));
	Title->SetJustification(ETextJustify::Center);
	Title->SetWrapTextAt(1760.f);   // a generated title (T16a) can run to 48 characters
	TitleStack->AddChildToOverlay(Title)->SetHorizontalAlignment(HAlign_Center);
	Stamp = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Stamp->SetBrush(ScreenRounded(StampRed, 6.f, 5.f));
	Stamp->SetBrushColor(FLinearColor::Transparent);   // outline and ink only; the title reads through it
	Stamp->SetPadding(FMargin(18.f, 4.f));
	StampText = MakeText(46, StampRed, true);
	StampText->SetShadowOffset(FVector2D::ZeroVector);
	Stamp->SetContent(StampText);
	Stamp->SetRenderTransformAngle(-9.f);
	UOverlaySlot* StampSlot = TitleStack->AddChildToOverlay(Stamp);
	// Across the end of the title, like a stamp on a poster.
	StampSlot->SetHorizontalAlignment(HAlign_Right);
	StampSlot->SetVerticalAlignment(VAlign_Center);
	StampSlot->SetPadding(FMargin(0.f, 0.f, -40.f, 0.f));
	Add(TitleStack);

	Subtitle = MakeText(28, FLinearColor::White, false);
	Subtitle->SetWrapTextAt(1300.f);
	Add(Subtitle, FMargin(0.f, 18.f, 0.f, 10.f));

	RowsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	for (int32 i = 0; i < MaxRows; ++i)
	{
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(720.f);
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTextBlock* L = MakeText(32, ScreenDim, true);
		UTextBlock* V = MakeText(32, FLinearColor::White, true);
		UHorizontalBoxSlot* LS = Row->AddChildToHorizontalBox(L);
		LS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LS->SetHorizontalAlignment(HAlign_Left);
		Row->AddChildToHorizontalBox(V);
		Size->SetContent(Row);
		RowsBox->AddChildToVerticalBox(Size)->SetPadding(FMargin(0.f, 3.f));
		RowLabels.Add(L);
		RowValues.Add(V);
		RowWidgets.Add(Size);
	}
	Add(RowsBox, FMargin(0.f, 10.f));

	QuotesBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	for (int32 i = 0; i < MaxQuotes; ++i)
	{
		UTextBlock* Q = MakeText(24, FLinearColor(0.9f, 0.85f, 1.f), false);
		QuotesBox->AddChildToVerticalBox(Q)->SetPadding(FMargin(0.f, 3.f));
		QuoteTexts.Add(Q);
	}
	Add(QuotesBox, FMargin(0.f, 12.f));

	// Credits: a clipped window the lines scroll up through.
	CreditsClip = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	CreditsClip->SetWidthOverride(1400.f);
	CreditsClip->SetHeightOverride(CreditsClipHeight);
	CreditsClip->SetClipping(EWidgetClipping::ClipToBounds);
	CreditsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	for (int32 i = 0; i < MaxCredits; ++i)
	{
		UTextBlock* C = MakeText(28, FLinearColor::White, false);
		USizeBox* LineSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		LineSize->SetHeightOverride(CreditLineHeight);
		LineSize->SetContent(C);
		CreditsBox->AddChildToVerticalBox(LineSize)->SetHorizontalAlignment(HAlign_Center);
		CreditTexts.Add(C);
	}
	CreditsClip->SetContent(CreditsBox);
	Add(CreditsClip, FMargin(0.f, 16.f));

	OptionsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	for (int32 i = 0; i < MaxOptions; ++i)
	{
		UBorder* Row = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Row->SetPadding(FMargin(26.f, 10.f));
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(620.f);
		UHorizontalBox* H = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTextBlock* L = MakeText(32, FLinearColor::White, true);
		UTextBlock* V = MakeText(28, ScreenDim, false);
		UHorizontalBoxSlot* LS = H->AddChildToHorizontalBox(L);
		LS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LS->SetHorizontalAlignment(HAlign_Left);
		H->AddChildToHorizontalBox(V)->SetVerticalAlignment(VAlign_Center);
		Size->SetContent(H);
		Row->SetContent(Size);
		OptionsBox->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 4.f));
		OptionRows.Add(Row);
		OptionLabels.Add(L);
		OptionValues.Add(V);
	}
	Add(OptionsBox, FMargin(0.f, 30.f, 0.f, 0.f));

	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(Column);
		S->SetAnchors(FAnchors(0.5f, 0.45f));
		S->SetAlignment(FVector2D(0.5f, 0.5f));
		S->SetAutoSize(true);
	}

	CaptionPanel = MakePanel(FLinearColor(0.f, 0.f, 0.f, 0.72f), 6.f, FMargin(22.f, 10.f));
	CaptionText = MakeText(26, FLinearColor::White, false);
	CaptionText->SetWrapTextAt(1400.f);
	CaptionPanel->SetContent(CaptionText);
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(CaptionPanel);
		S->SetAnchors(FAnchors(0.5f, 1.f));
		S->SetAlignment(FVector2D(0.5f, 1.f));
		S->SetPosition(FVector2D(0.f, -96.f));
		S->SetAutoSize(true);
	}

	FooterText = MakeText(20, ScreenDim, true);
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(FooterText);
		S->SetAnchors(FAnchors(0.5f, 1.f));
		S->SetAlignment(FVector2D(0.5f, 1.f));
		S->SetPosition(FVector2D(0.f, -36.f));
		S->SetAutoSize(true);
	}
}

void UJamScreenWidget::ApplyView(const FJamScreenView& View, float RealSeconds)
{
	if (!Root) { return; }
	if (View.Screen != ShownScreen) { ShownScreen = View.Screen; ShownAt = RealSeconds; }

	Backdrop->SetBrushColor(View.bOpaque ? FLinearColor(0.02f, 0.015f, 0.03f, 0.95f) : FLinearColor(0.f, 0.f, 0.f, 0.6f));

	SetScreenText(Kicker, View.Kicker);
	ShowScreenPart(Kicker, !View.Kicker.IsEmpty());
	SetScreenText(Title, View.Title);
	// The rating card's single letter is huge; everything else is a title.
	Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", View.Screen == EJamScreen::Rating ? 220 : (View.Title.Len() > 36 ? 52 : View.Title.Len() > 26 ? 60 : View.Title.Len() > 18 ? 72 : 84)));
	SetScreenText(StampText, View.Stamp);
	ShowScreenPart(Stamp, !View.Stamp.IsEmpty());
	// The stamp lands a beat after the title.
	const float StampT = FMath::Clamp((RealSeconds - ShownAt - 0.5f) / 0.15f, 0.f, 1.f);
	Stamp->SetRenderScale(FVector2D(FMath::Lerp(2.2f, 1.f, StampT)));
	Stamp->SetRenderOpacity(StampT);
	SetScreenText(Subtitle, View.Subtitle);
	ShowScreenPart(Subtitle, !View.Subtitle.IsEmpty());

	ShowScreenPart(RowsBox, View.Rows.Num() > 0);
	for (int32 i = 0; i < RowWidgets.Num(); ++i)
	{
		const bool bRow = View.Rows.IsValidIndex(i);
		ShowScreenPart(RowWidgets[i], bRow);
		if (bRow) { SetScreenText(RowLabels[i], View.Rows[i].Key); SetScreenText(RowValues[i], View.Rows[i].Value); }
	}
	ShowScreenPart(QuotesBox, View.Quotes.Num() > 0);
	for (int32 i = 0; i < QuoteTexts.Num(); ++i)
	{
		ShowScreenPart(QuoteTexts[i], View.Quotes.IsValidIndex(i));
		if (View.Quotes.IsValidIndex(i)) { SetScreenText(QuoteTexts[i], View.Quotes[i]); }
	}

	const bool bCredits = View.Body.Num() > 0;
	ShowScreenPart(CreditsClip, bCredits);
	if (bCredits)
	{
		for (int32 i = 0; i < CreditTexts.Num(); ++i)
		{
			const bool bLine = View.Body.IsValidIndex(i);
			ShowScreenPart(CreditTexts[i]->GetParent(), bLine);
			if (!bLine) { continue; }
			SetScreenText(CreditTexts[i], View.Body[i]);
			// Section heads and the real credits read differently from the jokes.
			const bool bHead = View.Body[i].StartsWith(TEXT("—"));
			CreditTexts[i]->SetColorAndOpacity(FSlateColor(bHead ? ScreenGold : FLinearColor::White));
		}
		const float Content = FMath::Min(View.Body.Num(), MaxCredits) * CreditLineHeight;
		CreditsBox->SetRenderTranslation(FVector2D(0.f, CreditsClipHeight - FMath::Clamp(View.Scroll, 0.f, 1.f) * (CreditsClipHeight + Content)));
	}

	ShownOptions = FMath::Min(View.Options.Num(), OptionRows.Num());
	ShowScreenPart(OptionsBox, ShownOptions > 0);
	for (int32 i = 0; i < OptionRows.Num(); ++i)
	{
		const bool bOpt = i < ShownOptions;
		if (OptionRows[i]->GetVisibility() != (bOpt ? ESlateVisibility::Visible : ESlateVisibility::Collapsed))
		{
			OptionRows[i]->SetVisibility(bOpt ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
		if (!bOpt) { continue; }
		const FJamMenuOption& O = View.Options[i];
		const bool bSel = i == View.Selected && O.bEnabled;
		OptionRows[i]->SetBrush(ScreenRounded(bSel ? ScreenGold : FLinearColor(1.f, 1.f, 1.f, 0.15f), 8.f, bSel ? 3.f : 1.f));
		OptionRows[i]->SetBrushColor(bSel ? FLinearColor(0.25f, 0.18f, 0.04f, 0.85f) : FLinearColor(0.f, 0.f, 0.f, 0.45f));
		SetScreenText(OptionLabels[i], bSel ? FString::Printf(TEXT("▶  %s"), *O.Label) : O.Label);
		OptionLabels[i]->SetColorAndOpacity(FSlateColor(!O.bEnabled ? FLinearColor(0.4f, 0.4f, 0.4f) : (bSel ? ScreenGold : FLinearColor::White)));
		SetScreenText(OptionValues[i], O.Value);
		OptionValues[i]->SetColorAndOpacity(FSlateColor(O.bEnabled ? ScreenDim : FLinearColor(0.35f, 0.35f, 0.35f)));
	}

	// Credits keep the post-credits pitch for the end of the scroll.
	const bool bCaption = !View.Caption.IsEmpty() && (!bCredits || View.Scroll >= 1.f);
	ShowScreenPart(CaptionPanel, bCaption);
	SetScreenText(CaptionText, View.Caption);
	SetScreenText(FooterText, View.Footer);
	ShowScreenPart(FooterText, !View.Footer.IsEmpty());
}

int32 UJamScreenWidget::OptionUnder(const FVector2D& ScreenPosition) const
{
	for (int32 i = 0; i < ShownOptions; ++i)
	{
		if (OptionRows[i] && OptionRows[i]->GetCachedGeometry().IsUnderLocation(ScreenPosition)) { return i; }
	}
	return INDEX_NONE;
}

FReply UJamScreenWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey K = InKeyEvent.GetKey();
	OnDevice.ExecuteIfBound(K.IsGamepadKey());
	EJamMenuInput Input;
	if (IsAny(K, { EKeys::W, EKeys::Up, EKeys::Gamepad_DPad_Up, EKeys::Gamepad_LeftStick_Up })) { Input = EJamMenuInput::Up; }
	else if (IsAny(K, { EKeys::S, EKeys::Down, EKeys::Gamepad_DPad_Down, EKeys::Gamepad_LeftStick_Down })) { Input = EJamMenuInput::Down; }
	else if (IsAny(K, { EKeys::A, EKeys::Left, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_LeftStick_Left })) { Input = EJamMenuInput::Left; }
	else if (IsAny(K, { EKeys::D, EKeys::Right, EKeys::Gamepad_DPad_Right, EKeys::Gamepad_LeftStick_Right })) { Input = EJamMenuInput::Right; }
	else if (IsAny(K, { EKeys::E, EKeys::Enter, EKeys::SpaceBar, EKeys::Gamepad_FaceButton_Bottom })) { Input = EJamMenuInput::Confirm; }
	else if (IsAny(K, { EKeys::Escape, EKeys::Gamepad_FaceButton_Right, EKeys::Gamepad_Special_Right })) { Input = EJamMenuInput::Back; }
	else { return Super::NativeOnKeyDown(InGeometry, InKeyEvent); }
	OnMenuInput.ExecuteIfBound(Input, INDEX_NONE);
	return FReply::Handled();
}

FReply UJamScreenWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const int32 Index = OptionUnder(InMouseEvent.GetScreenSpacePosition());
	if (Index != INDEX_NONE)
	{
		OnDevice.ExecuteIfBound(false);
		OnMenuInput.ExecuteIfBound(EJamMenuInput::Hover, Index);
	}
	return FReply::Handled();
}

FReply UJamScreenWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnDevice.ExecuteIfBound(false);
	const int32 Index = OptionUnder(InMouseEvent.GetScreenSpacePosition());
	// Clicking anywhere on a card without options (Premiere, Credits) moves on.
	OnMenuInput.ExecuteIfBound(Index != INDEX_NONE ? EJamMenuInput::Click : (ShownOptions == 0 ? EJamMenuInput::Confirm : EJamMenuInput::Hover), Index);
	return FReply::Handled();
}

FReply UJamScreenWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	return FReply::Handled();
}
