#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JamNoteEditor.generated.h"

class UBorder;
class UButton;
class UEditableTextBox;
class UTextBlock;

/**
 * T12b: the player writes Fenella's note at CUT! (BUILD_SPEC 3a). A text box (80 characters) and the three
 * suggested notes as buttons; gamepad players pick a suggestion. Built in C++ (all Transient, like the HUD).
 *   Keyboard: type, Enter gives the note, Up/Down picks a suggestion, Esc goes back to the CUT! choices.
 *   Gamepad: D-pad picks, (A) gives, (B) back.   Mouse: click the box to type, click a suggestion to give it.
 */
UCLASS()
class MEGAJAM_API UJamNoteEditor : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_OneParam(FOnSubmit, const FString&);
	FOnSubmit OnSubmit;
	FSimpleDelegate OnCancel;

	void Open(const FString& Pitch, const TArray<FString>& Suggestions, bool bGamepad);
	/** The widget that should hold keyboard focus right now (the text box, or the editor itself on a suggestion). */
	UWidget* FocusTarget() const;
	bool IsOpen() const { return bOpen; }
	void Close() { bOpen = false; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void BuildTree();
	void Select(int32 Row);
	void Submit();
	void Refresh();

	UFUNCTION() void OnTextChanged(const FText& Text);
	UFUNCTION() void OnClick0();
	UFUNCTION() void OnClick1();
	UFUNCTION() void OnClick2();
	UFUNCTION() void OnHover0();
	UFUNCTION() void OnHover1();
	UFUNCTION() void OnHover2();

	UPROPERTY(Transient) TObjectPtr<UTextBlock> PitchText;
	UPROPERTY(Transient) TObjectPtr<UEditableTextBox> TextBox;
	UPROPERTY(Transient) TObjectPtr<UBorder> TextFrame;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CountText;
	UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> Rows;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> RowTexts;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HintText;

	TArray<FString> Suggested;
	/** 0 = the text box, 1 to 3 = a suggestion. */
	int32 Selection = 0;
	bool bOpen = false;
	bool bGamepad = false;
};
