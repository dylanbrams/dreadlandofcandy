#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/JamHudView.h"
#include "JamHudWidget.generated.h"

enum class EJamHudAction : uint8 { Skill1, Skill2, Ultimate, TakeNote, RefuseNote, WriteNote, Interact, Continue, Character, Pause };
DECLARE_DELEGATE_OneParam(FOnJamHudAction, EJamHudAction);

class UBorder;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;
class UOverlay;
class UMaterialInstanceDynamic;

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
	FOnJamHudAction OnAction;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	friend class FJamHudInputTest;
	void BuildTree();
	UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> ChoiceTiles;
	UPROPERTY(Transient) TObjectPtr<UBorder> PauseControl;
	FJamHudView InputView;
	UBorder* MakePanel(const FLinearColor& Colour, float Radius, const FMargin& InPadding);
	UTextBlock* MakeText(int32 Size, const FLinearColor& Colour, bool bBold);
	UBorder* GetBubble(int32 Index);
	void BuildWrapPanel();
	void BuildSkillBar();
	void ApplySkills(const FJamHudView& View, float RealSeconds);
	void ApplyWrap(const FJamWrapView& Wrap);
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
	UPROPERTY(Transient) TObjectPtr<UWidget> DignityRow;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> DignityLabel;
	UPROPERTY(Transient) TObjectPtr<UProgressBar> DignityBar;
	UPROPERTY(Transient) TObjectPtr<UProgressBar> PromptFill;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CaptionSpeaker;

	UPROPERTY(Transient) TObjectPtr<UBorder> NotePanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NotePitchText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeKeyText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeSubText;
	UPROPERTY(Transient) TObjectPtr<UProgressBar> TakeFill;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> RefuseKeyText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WriteKeyText;
	UPROPERTY(Transient) TObjectPtr<UWidget> NoteTiles;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NoteReadingText;

	// Skill bar and XP bar (T14a), bottom left.
	UPROPERTY(Transient) TObjectPtr<UWidget> SkillBar;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> LevelText;
	UPROPERTY(Transient) TObjectPtr<UProgressBar> XpBar;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> XpText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PointsText;
	UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> SkillTiles;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> SkillKeys;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> SkillNames;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> SkillRanks;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> SkillIconMaterials;
	UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> SkillDamageAccents;
	TArray<FString> ShownIconPaths;

	// The wrap panel (T14): ledger on the left, loot cards and the character sheet on the right.
	UPROPERTY(Transient) TObjectPtr<UBorder> WrapPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WrapScene;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WrapGross;
	UPROPERTY(Transient) TArray<TObjectPtr<UWidget>> LedgerRows;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LedgerItems;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LedgerNotes;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LedgerAmounts;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WrapCut;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WrapFooter;
	UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> LootCards;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LootRarity;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LootName;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LootFlavour;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LootStat;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LootTag;
	UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> LootStamp;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> StatNames;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> StatValues;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WrapLevel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WrapContinue;
	UPROPERTY(Transient) TObjectPtr<UBorder> WrapContinueButton;

	UPROPERTY(Transient) TObjectPtr<UBorder> CaptionPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CaptionText;

	UPROPERTY(Transient) TObjectPtr<UBorder> CardPanel;
	UPROPERTY(Transient) TObjectPtr<UBorder> CardAccent;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardTitle;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardSubtitle;

	UPROPERTY(Transient) TObjectPtr<UBorder> DirectionPanel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> DirectionText;
	FString ShownObjectiveId;
	float DirectionShownAt = -100.f;

	FString ShownCardTitle;
	bool bWrapShown = false;
	float WrapShownAt = -1.f;
	float CardShownAt = -1.f;
};
