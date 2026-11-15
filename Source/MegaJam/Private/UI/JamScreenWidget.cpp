#include "UI/JamScreenWidget.h"
#include "UI/JamCandyBorder.h"
#include "UI/JamUiStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ScaleBox.h"
#include "Engine/Texture2D.h"
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
	const FLinearColor ScreenGold = JamUi::Mint();
	const FLinearColor ScreenDim = JamUi::Dim();
	const FLinearColor StampRed = JamUi::Ruby();
	constexpr int32 MaxRows = 4;
	constexpr int32 MaxQuotes = 6;
	constexpr int32 MaxOptions = 9;
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
		return JamUi::Frame(Outline, OutlineWidth);
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
	T->SetFont(JamUi::Font(Size, bBold));
	T->SetColorAndOpacity(FSlateColor(Colour == FLinearColor::White ? JamUi::Ink() : Colour));
	T->SetShadowOffset(FVector2D(1.f, 1.f));
	T->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));
	T->SetJustification(ETextJustify::Center);
	return T;
}

UBorder* UJamScreenWidget::MakePanel(const FLinearColor& Colour, float Radius, const FMargin& InPadding)
{
	UBorder* B = WidgetTree->ConstructWidget<UJamCandyBorder>(UJamCandyBorder::StaticClass());
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
	Backdrop->SetBrushColor(JamUi::Surface());
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(Backdrop);
		S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		S->SetOffsets(FMargin(0.f));
	}
	MenuShade = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	MenuShade->SetBrushColor(JamUi::Surface(0.72f));
	MenuShade->SetVisibility(ESlateVisibility::HitTestInvisible);
	{
		UCanvasPanelSlot* S = Root->AddChildToCanvas(MenuShade);
		S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 1.f));
		S->SetOffsets(FMargin(0.f, 0.f, 980.f, 0.f));
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
	USizeBox* CrestSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	CrestSize->SetWidthOverride(420.f);
	CrestSize->SetHeightOverride(64.f);
	UJamCandyBorder* Crest = WidgetTree->ConstructWidget<UJamCandyBorder>(UJamCandyBorder::StaticClass());
	Crest->bCrest = true;
	Crest->SetVisibility(ESlateVisibility::HitTestInvisible);
	CrestSize->SetContent(Crest);
	if (UTexture2D* CrestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Art/UI/BlackSugar/T_UI_BlackSugarCrest"), nullptr, LOAD_NoWarn | LOAD_Quiet))
	{
		UImage* Art = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Art->SetBrushFromTexture(CrestTexture, true);
		Art->SetVisibility(ESlateVisibility::HitTestInvisible);
		UScaleBox* Fit = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass());
		Fit->SetStretch(EStretch::ScaleToFit);
		Fit->AddChild(Art);
		CrestSize->SetContent(Fit);
	}
	Add(CrestSize, FMargin(0.f, 0.f, 0.f, 8.f));

	// The title, with a rubber stamp slapped across it.
	UOverlay* TitleStack = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Title = MakeText(64, JamUi::Ink(), true);
	Title->SetFont(JamUi::Font(64, true, true));
	Title->SetShadowOffset(FVector2D(2.f, 2.f));
	Title->SetJustification(ETextJustify::Center);
	Title->SetWrapTextAt(1760.f);   // a generated title (T16a) can run to 48 characters
	TitleStack->AddChildToOverlay(Title)->SetHorizontalAlignment(HAlign_Center);
	Stamp = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Stamp->SetBrush(ScreenRounded(StampRed, 2.f, 2.f));
	Stamp->SetBrushColor(FLinearColor::Transparent);   // outline and ink only; the title reads through it
	Stamp->SetPadding(FMargin(10.f, 3.f));
	StampText = MakeText(22, StampRed, true);
	StampText->SetShadowOffset(FVector2D::ZeroVector);
	Stamp->SetContent(StampText);
	// Dylan: at 45 degrees, a little below and to the right of the title's last word (placed in ApplyView).
	Stamp->SetRenderTransformAngle(-45.f);
	UOverlaySlot* StampSlot = TitleStack->AddChildToOverlay(Stamp);
	StampSlot->SetHorizontalAlignment(HAlign_Right);
	StampSlot->SetVerticalAlignment(VAlign_Bottom);
	Add(TitleStack);

	Subtitle = MakeText(28, FLinearColor::White, false);
	Subtitle->SetWrapTextAt(1300.f);
	Add(Subtitle, FMargin(0.f, 18.f, 0.f, 10.f));

	// Screen reactions belong in the flow, so they cannot cover menu options.
	CaptionPanel = MakePanel(FLinearColor(0.f, 0.f, 0.f, 0.72f), 6.f, FMargin(22.f, 10.f));
	CaptionText = MakeText(26, FLinearColor::White, false);
	CaptionText->SetWrapTextAt(1400.f);
	CaptionPanel->SetContent(CaptionText);
	Add(CaptionPanel, FMargin(0.f, 4.f, 0.f, 4.f));

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
		UTextBlock* Q = MakeText(24, JamUi::Dim(), false);
		Q->SetWrapTextAt(1400.f);
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
		UBorder* Row = WidgetTree->ConstructWidget<UJamCandyBorder>(UJamCandyBorder::StaticClass());
		Row->SetPadding(FMargin(30.f, 16.f));
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(700.f);
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

	const bool bSideMenu = View.Screen == EJamScreen::Title || View.Screen == EJamScreen::Pause || View.Screen == EJamScreen::Options || View.Screen == EJamScreen::QuitConfirm;
	Backdrop->SetBrushColor(JamUi::Surface(View.bOpaque ? 0.94f : bSideMenu ? 0.48f : 0.92f));
	ShowScreenPart(MenuShade, bSideMenu);
	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(Column->Slot))
	{
		S->SetAnchors(bSideMenu ? FAnchors(0.075f, 0.48f) : FAnchors(0.5f, 0.48f));
		S->SetAlignment(bSideMenu ? FVector2D(0.f, 0.5f) : FVector2D(0.5f, 0.5f));
	}
	Title->SetWrapTextAt(bSideMenu ? 760.f : 1760.f);
	Subtitle->SetWrapTextAt(bSideMenu ? 740.f : 1300.f);
	CaptionText->SetWrapTextAt(bSideMenu ? 710.f : 1400.f);

	SetScreenText(Kicker, View.Kicker);
	ShowScreenPart(Kicker, !View.Kicker.IsEmpty());
	SetScreenText(Title, View.Title);
	// The rating card's single letter is huge; everything else is a title.
	const int32 HeadingSize = View.Screen == EJamScreen::Rating ? 180 : (View.Screen == EJamScreen::Title || View.Options.Num() >= 8) ? 50 : (View.Title.Len() > 36 ? 44 : View.Title.Len() > 26 ? 50 : 64);
	Title->SetFont(JamUi::Font(HeadingSize, true, true));
	SetScreenText(StampText, View.Stamp);
	ShowScreenPart(Stamp, !View.Stamp.IsEmpty());
	// The stamp lands a beat after the title.
	const float StampT = FMath::Clamp((RealSeconds - ShownAt - 0.5f) / 0.15f, 0.f, 1.f);
	Stamp->SetRenderScale(FVector2D(1.f));
	{
		// Centre it just past the title's right end and below its baseline. Its size is known from the second frame
		// (it fades in later than that).
		const FVector2D Size = Stamp->GetDesiredSize();
		// Unmoved, its centre is half its width left of the title's right edge and half its height above the bottom.
		Stamp->SetRenderTranslation(FVector2D(Size.X * 0.62f, Size.Y * 0.5f + Size.X * 0.1f));
	}
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
		if (View.Quotes.IsValidIndex(i))
		{
			SetScreenText(QuoteTexts[i], View.Quotes[i]);
			const bool bQuestCopy = View.Screen == EJamScreen::QuestBranches;
			QuoteTexts[i]->SetFont(JamUi::Font(24, bQuestCopy));
			QuoteTexts[i]->SetColorAndOpacity(FSlateColor(bQuestCopy ? JamUi::Ink() : JamUi::Dim()));
		}
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
			CreditTexts[i]->SetColorAndOpacity(FSlateColor(bHead ? ScreenGold : JamUi::Ink()));
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
		// Tall menus need room for their caption and footer as well as every option.
		OptionRows[i]->SetPadding(FMargin(30.f, ShownOptions >= 8 ? 6.5f : 14.f));
		if (USizeBox* Size = Cast<USizeBox>(OptionRows[i]->GetContent())) { Size->SetWidthOverride(View.Screen == EJamScreen::QuestBranches ? 1000.f : View.Screen == EJamScreen::Character ? 900.f : 700.f); }
		OptionLabels[i]->SetWrapTextAt(View.Screen == EJamScreen::QuestBranches ? 950.f : 0.f);
		const FJamMenuOption& O = View.Options[i];
		const bool bSel = i == View.Selected && O.bEnabled;
		OptionRows[i]->SetBrush(JamUi::Frame(bSel ? JamUi::Mint() : JamUi::Silver().CopyWithNewOpacity(O.bEnabled ? 0.55f : 0.25f), bSel ? 2.f : 1.f));
		OptionRows[i]->SetBrushColor(bSel ? JamUi::Selection() : JamUi::Surface(0.88f));
		SetScreenText(OptionLabels[i], bSel ? FString::Printf(TEXT("▶  %s"), *O.Label) : O.Label);
		OptionLabels[i]->SetColorAndOpacity(FSlateColor(!O.bEnabled ? JamUi::Silver() : (bSel ? ScreenGold : JamUi::Ink())));
		SetScreenText(OptionValues[i], O.Value);
		OptionValues[i]->SetColorAndOpacity(FSlateColor(O.bEnabled ? ScreenDim : JamUi::Silver()));
	}

	// Credits keep the post-credits pitch for the end of the scroll.
	const bool bCaption = !View.Caption.IsEmpty() && (!bCredits || View.Scroll >= 1.f);
	ShowScreenPart(CaptionPanel, bCaption);
	SetScreenText(CaptionText, View.Caption);
	SetScreenText(FooterText, View.Footer);
	ShowScreenPart(FooterText, !View.Footer.IsEmpty());

	// Long quest copy and tall menus must fit above the independent input footer.
	// Render scaling preserves the real cached mouse geometry and the existing row/action model.
	const FVector2D RootSize = Root->GetCachedGeometry().GetLocalSize();
	const FVector2D ContentSize = Column->GetDesiredSize();
	const float AvailableHeight = FMath::Max(1.f, (RootSize.Y > 0.f ? RootSize.Y : 1080.f) - 160.f);
	const float AvailableWidth = bSideMenu ? (RootSize.X > 0.f ? RootSize.X : 1920.f) * 0.47f : (RootSize.X > 0.f ? RootSize.X : 1920.f) - 96.f;
	const float Fit = FMath::Min(1.f, FMath::Min(AvailableHeight / FMath::Max(1.f, ContentSize.Y), AvailableWidth / FMath::Max(1.f, ContentSize.X)));
	Column->SetRenderTransformPivot(bSideMenu ? FVector2D(0.f, 0.5f) : FVector2D(0.5f, 0.5f));
	Column->SetRenderScale(FVector2D(Fit, Fit));
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
	else if (IsAny(K, { EKeys::F, EKeys::Enter, EKeys::SpaceBar, EKeys::Gamepad_FaceButton_Bottom }))
	{
		// Holding F to talk must not auto-accept a newly opened quest choice.
		if (InKeyEvent.IsRepeat()) { return FReply::Handled(); }
		Input = EJamMenuInput::Confirm;
	}
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
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton) { return FReply::Handled(); }
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
