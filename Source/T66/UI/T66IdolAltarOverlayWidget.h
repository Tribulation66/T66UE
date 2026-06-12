// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/T66Rarity.h"
#include "Core/Animation/T66AnimationGroup.h"
#include "Core/Animation/T66AnimationSequence.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66IdolAltar.h"
#include "Input/Reply.h"
#include "UI/Animation/T66AnimationMarkerDispatch.h"
#include "T66IdolAltarOverlayWidget.generated.h"

class FActiveTimerHandle;
class SBorder;
class SBox;
class SImage;
class SWidget;
class SRichTextBlock;
class STextBlock;
class SWidget;
struct FSlateBrush;
class AT66IdolAltar;

UCLASS(Blueprintable)
class T66_API UT66IdolAltarOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	void SetSourceAltar(AT66IdolAltar* InSourceAltar) { SourceAltar = InSourceAltar; }

private:
	static constexpr int32 OfferSlotCount = 20;
	static constexpr int32 OfferSlotsPerCategory = 4;
	static constexpr int32 OfferCategoryCount = 6;

	TArray<TSharedPtr<SBox>> OfferCardBoxes;
	TArray<TSharedPtr<STextBlock>> OfferNameTexts;
	TArray<TSharedPtr<SRichTextBlock>> OfferDescriptionTexts;
	TArray<TSharedPtr<SImage>> OfferIconImages;
	TArray<TSharedPtr<FSlateBrush>> OfferIconBrushes;
	TArray<TSharedPtr<SBorder>> OfferTileBorders;
	TArray<TSharedPtr<SBorder>> OfferIconBorders;
	TArray<TSharedPtr<SWidget>> OfferButtons;
	TArray<TSharedPtr<SBorder>> OfferButtonBorders;
	TArray<TSharedPtr<STextBlock>> OfferButtonTexts;
	TArray<float> OfferRevealAlphas;
	TArray<float> OfferLiftOffsets;
	TArray<float> OfferGlowAlphas;
	TArray<float> OfferSelectionAlphas;
	TArray<FLinearColor> OfferBaseBorderColors;
	TArray<FLinearColor> OfferGlowColors;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SWidget> RerollButton;
	TSharedPtr<SBorder> RerollButtonBorder;
	TSharedPtr<STextBlock> RerollButtonText;
	TWeakObjectPtr<AT66IdolAltar> SourceAltar;
	int32 ActiveOfferCategoryIndex = 0;
	FT66AnimationGroup RevealAnimationGroup;
	FT66AnimationSequence SelectionAnimationSequence;
	FT66AnimationMarkerDispatcher MarkerDispatcher;
	TWeakPtr<SWidget> AnimationActiveTimerWidget;
	TSharedPtr<FActiveTimerHandle> AnimationActiveTimerHandle;
	bool bRevealAnimationActive = false;
	bool bSelectionAnimationActive = false;
	bool bCloseAfterSelectionCommit = false;

	struct FPendingSelection
	{
		bool bPending = false;
		bool bCommitAttempted = false;
		bool bTutorialSingleOffer = false;
		bool bWasUpgrade = false;
		bool bNoIdolSelection = false;
		int32 VisibleSlotIndex = INDEX_NONE;
		int32 StockIndex = INDEX_NONE;
		FName IdolID = NAME_None;
		ET66ItemRarity OfferRarity = ET66ItemRarity::Black;
	};
	FPendingSelection PendingSelection;

	FReply OnReroll();
	FReply OnToggleSlot(int32 SlotIndex);
	FReply OnBack();

	void RefreshStock();
	bool HasSelectionsRemaining() const;
	void ConsumeSelectionBudget(int32 SlotIndex);
	void RefundSelectionBudget(int32 SlotIndex);
	int32 GetOfferStockIndexForVisibleSlot(int32 VisibleSlotIndex) const;
	bool IsTutorialSingleOfferMode() const;
	FName GetTutorialOfferedIdolID() const;
	void StartRevealAnimation(const TSharedRef<SWidget>& OwningWidget);
	void StartSelectionAnimation(int32 VisibleSlotIndex, int32 StockIndex, FName IdolID, bool bTutorialSingleOffer, bool bWasUpgrade, ET66ItemRarity OfferRarity, bool bNoIdolSelection);
	void StartAnimationActiveTimer(const TSharedRef<SWidget>& OwningWidget);
	void StopAnimationActiveTimer();
	EActiveTimerReturnType HandleAnimationActiveTimer(double CurrentTime, float DeltaTime);
	void TickAnimations(float DeltaSeconds);
	void ApplyOfferAnimationVisuals();
	void RegisterMarkerHandlers();
	void CommitPendingSelectionIfNeeded();
	void ClearPendingSelection();
	void CloseAfterCommittedSelection();

	UFUNCTION()
	void HandleIdolsChanged();
};
