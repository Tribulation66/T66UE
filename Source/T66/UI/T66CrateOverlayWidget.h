// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/T66Rarity.h"
#include "T66CrateOverlayWidget.generated.h"

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

private:
	struct FCrateItemEntry
	{
		FName ItemID;
		ET66Rarity Rarity;
		FLinearColor Color;
	};

	TArray<FCrateItemEntry> StripItems;
	int32 WinnerIndex = 0;
	FName WinnerItemID;

	TSharedPtr<SBorder> StripContainer;
	TSharedPtr<STextBlock> StatusText;

	FTimerHandle ScrollTickHandle;
	FTimerHandle StartHandle;
	FTimerHandle CloseHandle;

	bool bScrolling = false;
	float ScrollElapsed = 0.f;
	float ScrollDuration = 3.0f;
	float LastTickTimeSeconds = 0.f;
	float TotalScrollDistance = 0.f;
	float CurrentScrollOffset = 0.f;

	static constexpr float ItemTileWidth = 90.f;
	static constexpr int32 StripItemCount = 40;
	static constexpr int32 WinnerPosition = 35;

	void GenerateStrip();
	void StartScrolling();
	void TickScroll();
	void ResolveOpen();
	void CloseAfterResolve();
};
