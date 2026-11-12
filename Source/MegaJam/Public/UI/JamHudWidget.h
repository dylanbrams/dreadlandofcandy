#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/JamHudView.h"
#include "JamHudWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;

/**
 * The HUD (BUILD_SPEC 5C), in layers: slate label with REC and timecode (top left), objective, meter and hint
 * (top centre), bubble layer, interact prompt, card layer (centre). Draws an FJamHudView and nothing else.
 *
 * The widget tree is built in C++ (no editor MCP to author a WBP; BUILD_SPEC allows this). Everything it makes
 * is Transient, so a WBP reparented to this class later keeps only its own layout.
 */
UCLASS()
class MEGAJAM_API UJamHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Draw this view. Time is real seconds, for the REC blink and the card pop. */
	void ApplyView(const FJamHudView& View, float RealSeconds);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildTree();
	UBorder* MakePanel(const FLinearColor& Colour, float Radius, const FMargin& InPadding);
	UTextBlock* MakeText(int32 Size, const FLinearColor& Colour, bool bBold);
	UBorder* GetBubble(int32 Index);
	UBorder* MakeChoiceTile(const FLinearColor& Accent, const TCHAR* Title, UTextBlock*& OutKey, UTextBlock*& OutSub, UProgressBar** OutFill);

	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UBorder> SlatePanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SceneText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> RecText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TimecodeText;

	UPROPERTY(Transient) TObjectPtr<UBorder> ObjectivePanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ObjectiveText;
	UPROPERTY(Transient) TObjectPtr<UProgressBar> MeterBar;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MeterText;
	UPROPERTY(Transient) TObjectPtr<UBorder> HintPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HintText;

	UPROPERTY(Transient) TObjectPtr<UBorder> FlashLayer;
	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> BubbleLayer;
	UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> Bubbles;
	UPROPERTY(Transient) TObjectPtr<UBorder> PromptPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(Transient) TObjectPtr<UTextBlock> NotesText;
	UPROPERTY(Transient) TObjectPtr<UProgressBar> PromptFill;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CaptionSpeaker;

	UPROPERTY(Transient) TObjectPtr<UBorder> NotePanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NotePitchText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeKeyText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeSubText;
	UPROPERTY(Transient) TObjectPtr<UProgressBar> TakeFill;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> RefuseKeyText;

	UPROPERTY(Transient) TObjectPtr<UBorder> CaptionPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CaptionText;

	UPROPERTY(Transient) TObjectPtr<UBorder> CardPanel;
	UPROPERTY(Transient) TObjectPtr<UBorder> CardAccent;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardTitle;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardSubtitle;

	FString ShownCardTitle;
	float CardShownAt = -1.f;
};
