// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngTuningConfig.h"
#include "Styling/SlateBrush.h"
#include "T66CrateOverlayWidget.generated.h"

class UT66GameplayHUDWidget;
class STextBlock;
class SBorder;

/** CS:GO-style crate opening overlay: scrolling item strip that decelerates to a stop. */
UCLASS(Blueprintable)
class T66_API UT66CrateOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	void SetPresentationHost(UT66GameplayHUDWidget* InPresentationHost);
	void RequestSkip();

private:
	struct FCrateItemEntry
	{
		FName ItemID;
		ET66Rarity Rarity;
		FLinearColor Color;
		TSharedPtr<FSlateBrush> IconBrush;
	};

	TArray<FCrateItemEntry> StripItems;
	int32 WinnerIndex = 0;
	FName WinnerItemID;
	ET66Rarity WinnerRarity = ET66Rarity::Black;
	int32 WinnerRarityDrawIndex = INDEX_NONE;
	int32 WinnerRarityPreDrawSeed = 0;
	bool bWinnerRarityHasReplayWeights = false;
	FT66RarityWeights WinnerRarityReplayWeights;
	TWeakObjectPtr<UT66GameplayHUDWidget> PresentationHost;

	TSharedPtr<SBorder> StripContainer;
	TSharedPtr<STextBlock> SkipText;

	FTimerHandle ScrollTickHandle;
	FTimerHandle StartHandle;

	bool bScrolling = false;
	bool bResolved = false;
	float ScrollElapsed = 0.f;
	float ScrollDuration = 2.8f;
	float LastTickTimeSeconds = 0.f;
	float TotalScrollDistance = 0.f;
	float CurrentScrollOffset = 0.f;

	static constexpr float ItemTileWidth = 88.f;
	static constexpr float ItemTileHeight = 88.f;
	static constexpr float ItemTileGap = 4.f;
	static constexpr float ItemTileStride = ItemTileWidth + ItemTileGap;
	static constexpr float ItemPreviewSize = 68.f;
	static constexpr int32 VisibleTileCount = 5;
	static constexpr int32 StripItemCount = 34;
	static constexpr int32 WinnerPosition = 29;

	void GenerateStrip();
	void StartScrolling();
	void TickScroll();
	void UpdateSkipText();
	void ResolveOpen();
};
