#include "UI/JamNoteEditor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Film/JamNoteClamp.h"
#include "Styling/CoreStyle.h"

namespace
{
	const FLinearColor Violet(0.78f, 0.58f, 1.f);
	const FLinearColor Dim(0.75f, 0.75f, 0.72f);
	const FLinearColor Gold(1.f, 0.82f, 0.3f);

	FSlateBrush RoundedBrush(float Radius, const FLinearColor& Outline, float OutlineWidth)
	{
		FSlateBrush B;
		B.DrawAs = ESlateBrushDrawType::RoundedBox;
		B.TintColor = FSlateColor(FLinearColor::White);
		B.OutlineSettings = FSlateBrushOutlineSettings(FVector4(Radius, Radius, Radius, Radius), FSlateColor(Outline), OutlineWidth);
		B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		return B;
	}
}

void UJamNoteEditor::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
}

TSharedRef<SWidget> UJamNoteEditor::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget) { BuildTree(); }
	return Super::RebuildWidget();
}

void UJamNoteEditor::BuildTree()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = Root;

	auto MakeText = [this](int32 Size, const FLinearColor& Colour, bool bBold)
	{
		UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		T->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size));
		T->SetColorAndOpacity(FSlateColor(Colour));
		T->SetShadowOffset(FVector2D(2.f, 2.f));
		T->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));
		return T;
	};

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Panel->SetBrush(RoundedBrush(12.f, Violet, 3.f));
	Panel->SetBrushColor(FLinearColor(0.05f, 0.02f, 0.09f, 0.96f));
	Panel->SetPadding(FMargin(48.f, 30.f));
	USizeBox* Width = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Width->SetWidthOverride(1080.f);
	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Width->SetContent(Box);
	Panel->SetContent(Width);

	UTextBlock* Title = MakeText(44, Violet, true);
	Title->SetText(FText::FromString(TEXT("YOUR NOTE, DIRECTOR")));
	Box->AddChildToVerticalBox(Title)->SetHorizontalAlignment(HAlign_Center);
	PitchText = MakeText(20, Dim, false);
	PitchText->SetJustification(ETextJustify::Center);
	PitchText->SetWrapTextAt(1000.f);
	UVerticalBoxSlot* PitchSlot = Box->AddChildToVerticalBox(PitchText);
	PitchSlot->SetHorizontalAlignment(HAlign_Center);
	PitchSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 18.f));

	// The text box, in a frame that lights up when it is the selected row.
	TextFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	TextFrame->SetBrush(RoundedBrush(8.f, Gold, 3.f));
	TextFrame->SetPadding(FMargin(6.f));
	TextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
	FEditableTextBoxStyle Style = TextBox->GetWidgetStyle();
	Style.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 30));
	Style.SetForegroundColor(FSlateColor(FLinearColor(0.08f, 0.04f, 0.12f)));
	TextBox->SetWidgetStyle(Style);
	TextBox->SetHintText(FText::FromString(TEXT("Make the rats enormous and ashamed.")));
	TextBox->OnTextChanged.AddDynamic(this, &UJamNoteEditor::OnTextChanged);
	TextFrame->SetContent(TextBox);
	Box->AddChildToVerticalBox(TextFrame)->SetHorizontalAlignment(HAlign_Fill);
	CountText = MakeText(16, Dim, false);
	UVerticalBoxSlot* CountSlot = Box->AddChildToVerticalBox(CountText);
	CountSlot->SetHorizontalAlignment(HAlign_Right);
	CountSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 14.f));

	UTextBlock* Or = MakeText(20, Violet, true);
	Or->SetText(FText::FromString(TEXT("OR PICK ONE")));
	Box->AddChildToVerticalBox(Or)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));

	// Three suggestions: a borderless button around a row that lights up when selected.
	FButtonStyle Plain;
	FSlateBrush None;
	None.DrawAs = ESlateBrushDrawType::NoDrawType;
	Plain.SetNormal(None).SetHovered(None).SetPressed(None).SetDisabled(None);
	Plain.SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
	for (int32 i = 0; i < 3; ++i)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		Button->SetStyle(Plain);
		UBorder* Row = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Row->SetBrush(RoundedBrush(8.f, Gold, 0.f));
		Row->SetPadding(FMargin(20.f, 10.f));
		Row->SetHorizontalAlignment(HAlign_Left);
		UTextBlock* RowText = MakeText(26, FLinearColor::White, false);
		Row->SetContent(RowText);
		if (UButtonSlot* ContentSlot = Cast<UButtonSlot>(Button->SetContent(Row))) { ContentSlot->SetHorizontalAlignment(HAlign_Fill); ContentSlot->SetPadding(FMargin(0.f)); }
		if (i == 0) { Button->OnClicked.AddDynamic(this, &UJamNoteEditor::OnClick0); Button->OnHovered.AddDynamic(this, &UJamNoteEditor::OnHover0); }
		if (i == 1) { Button->OnClicked.AddDynamic(this, &UJamNoteEditor::OnClick1); Button->OnHovered.AddDynamic(this, &UJamNoteEditor::OnHover1); }
		if (i == 2) { Button->OnClicked.AddDynamic(this, &UJamNoteEditor::OnClick2); Button->OnHovered.AddDynamic(this, &UJamNoteEditor::OnHover2); }
		UVerticalBoxSlot* RowSlot = Box->AddChildToVerticalBox(Button);
		RowSlot->SetPadding(FMargin(0.f, 3.f));
		RowSlot->SetHorizontalAlignment(HAlign_Fill);
		Rows.Add(Row);
		RowTexts.Add(RowText);
	}

	HintText = MakeText(19, Dim, false);
	HintText->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* HintSlot = Box->AddChildToVerticalBox(HintText);
	HintSlot->SetHorizontalAlignment(HAlign_Center);
	HintSlot->SetPadding(FMargin(0.f, 18.f, 0.f, 0.f));

	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.45f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	PanelSlot->SetAutoSize(true);
}

void UJamNoteEditor::Open(const FString& Pitch, const TArray<FString>& Suggestions, bool bInGamepad)
{
	bOpen = true;
	bGamepad = bInGamepad;
	Suggested = Suggestions;
	if (PitchText) { PitchText->SetText(FText::FromString(Pitch.IsEmpty() ? FString() : FString::Printf(TEXT("Her idea: “%s”  Yours is better. Probably."), *Pitch))); }
	if (TextBox) { TextBox->SetText(FText::GetEmpty()); }
	for (int32 i = 0; i < RowTexts.Num(); ++i)
	{
		RowTexts[i]->SetText(FText::FromString(Suggested.IsValidIndex(i) ? Suggested[i] : FString()));
	}
	Select(bGamepad && Suggested.Num() > 0 ? 1 : 0);
}

UWidget* UJamNoteEditor::FocusTarget() const
{
	return Selection == 0 ? static_cast<UWidget*>(TextBox.Get()) : const_cast<UJamNoteEditor*>(this);
}

void UJamNoteEditor::Select(int32 Row)
{
	Selection = FMath::Clamp(Row, 0, FMath::Min(3, Suggested.Num()));
	if (UWidget* Target = FocusTarget()) { Target->SetFocus(); }
	Refresh();
}

void UJamNoteEditor::Refresh()
{
	if (TextFrame) { TextFrame->SetBrushColor(Selection == 0 ? FLinearColor(1.f, 1.f, 1.f, 0.95f) : FLinearColor(0.6f, 0.6f, 0.65f, 0.6f)); }
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		const bool bOn = Selection == i + 1;
		Rows[i]->SetBrushColor(bOn ? FLinearColor(0.45f, 0.25f, 0.75f, 0.95f) : FLinearColor(1.f, 1.f, 1.f, 0.06f));
		RowTexts[i]->SetColorAndOpacity(FSlateColor(bOn ? FLinearColor::White : Dim));
		Rows[i]->SetVisibility(Suggested.IsValidIndex(i) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	const int32 Len = TextBox ? TextBox->GetText().ToString().Len() : 0;
	if (CountText) { CountText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d   (typed notes go to the game's note server)"), Len, JamNoteClamp::NoteMaxChars))); }
	if (HintText)
	{
		HintText->SetText(FText::FromString(bGamepad
			? TEXT("D-pad  choose        (A)  give the note        (B)  back")
			: TEXT("[Enter]  give the note        [Up] [Down]  choose        [Esc]  back")));
	}
}

void UJamNoteEditor::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// A click into the box selects it.
	if (Selection != 0 && TextBox && TextBox->HasKeyboardFocus()) { Selection = 0; Refresh(); }
}

FReply UJamNoteEditor::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey K = InKeyEvent.GetKey();
	if (K.IsGamepadKey() != bGamepad) { bGamepad = K.IsGamepadKey(); Refresh(); }
	if (K == EKeys::Up || K == EKeys::Gamepad_DPad_Up || K == EKeys::Gamepad_LeftStick_Up) { Select(Selection - 1); return FReply::Handled(); }
	if (K == EKeys::Down || K == EKeys::Gamepad_DPad_Down || K == EKeys::Gamepad_LeftStick_Down) { Select(Selection + 1); return FReply::Handled(); }
	if (K == EKeys::Enter || K == EKeys::Gamepad_FaceButton_Bottom) { Submit(); return FReply::Handled(); }
	if (K == EKeys::Escape || K == EKeys::Gamepad_FaceButton_Right || K == EKeys::Gamepad_Special_Right)
	{
		bOpen = false;
		OnCancel.ExecuteIfBound();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UJamNoteEditor::Submit()
{
	FString Note;
	if (Selection == 0) { Note = TextBox ? TextBox->GetText().ToString() : FString(); }
	else if (Suggested.IsValidIndex(Selection - 1)) { Note = Suggested[Selection - 1]; }
	Note = JamNoteClamp::SanitizeNote(Note);
	if (Note.IsEmpty()) { return; }   // nothing typed yet: Enter does nothing
	bOpen = false;
	OnSubmit.ExecuteIfBound(Note);
}

void UJamNoteEditor::OnTextChanged(const FText& Text)
{
	const FString S = Text.ToString();
	if (S.Len() > JamNoteClamp::NoteMaxChars && TextBox) { TextBox->SetText(FText::FromString(S.Left(JamNoteClamp::NoteMaxChars))); }
	Refresh();
}

void UJamNoteEditor::OnClick0() { Select(1); Submit(); }
void UJamNoteEditor::OnClick1() { Select(2); Submit(); }
void UJamNoteEditor::OnClick2() { Select(3); Submit(); }
void UJamNoteEditor::OnHover0() { Select(1); }
void UJamNoteEditor::OnHover1() { Select(2); }
void UJamNoteEditor::OnHover2() { Select(3); }
