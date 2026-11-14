#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/JamScreens.h"
#include "JamScreenWidget.generated.h"

class UBorder;
class UCanvasPanel;
class USizeBox;
class UTextBlock;
class UVerticalBox;

enum class EJamMenuInput : uint8 { Up, Down, Left, Right, Confirm, Back, Hover, Click };

DECLARE_DELEGATE_TwoParams(FOnJamMenuInput, EJamMenuInput /*Input*/, int32 /*OptionIndex, Hover/Click only*/);
DECLARE_DELEGATE_OneParam(FOnJamMenuDevice, bool /*bGamepad*/);

/**
 * Full-screen menus and cards (Title, Notes, Pause, Quit, Premiere, Credits). Draws an FJamScreenView and turns
 * keys, gamepad buttons and mouse clicks into menu inputs for its owner (AJamHUD). Holds no game state.
 * Tree built in C++ like the HUD; everything it makes is Transient.
 */
UCLASS()
class MEGAJAM_API UJamScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ApplyView(const FJamScreenView& View, float RealSeconds);

	FOnJamMenuInput OnMenuInput;
	FOnJamMenuDevice OnDevice;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

private:
	friend class FJamQuestScreenInputTest;
	void BuildTree();
	UTextBlock* MakeText(int32 Size, const FLinearColor& Colour, bool bBold);
	UBorder* MakePanel(const FLinearColor& Colour, float Radius, const FMargin& InPadding);
	int32 OptionUnder(const FVector2D& ScreenPosition) const;

	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UBorder> Backdrop;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> Column;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Kicker;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Title;
	UPROPERTY(Transient) TObjectPtr<UBorder> Stamp;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> StampText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Subtitle;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> RowsBox;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> RowLabels;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> RowValues;
	UPROPERTY(Transient) TArray<TObjectPtr<UWidget>> RowWidgets;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> QuotesBox;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> QuoteTexts;
	UPROPERTY(Transient) TObjectPtr<USizeBox> CreditsClip;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> CreditsBox;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> CreditTexts;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> OptionsBox;
	UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> OptionRows;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> OptionLabels;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> OptionValues;
	UPROPERTY(Transient) TObjectPtr<UBorder> CaptionPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CaptionText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> FooterText;

	int32 ShownOptions = 0;
	EJamScreen ShownScreen = EJamScreen::None;
	float ShownAt = 0.f;
};
