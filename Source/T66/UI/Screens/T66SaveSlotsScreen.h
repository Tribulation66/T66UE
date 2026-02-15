// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66SaveSlotsScreen.generated.h"

struct FSlateBrush;

UCLASS(Blueprintable)
class T66_API UT66SaveSlotsScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66SaveSlotsScreen(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Save Slots")
	int32 CurrentPage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slots")
	int32 TotalPages = 1;

	UFUNCTION(BlueprintCallable, Category = "Save Slots")
	void OnSlotClicked(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save Slots")
	bool IsSlotOccupied(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Save Slots")
	void OnPreviousPageClicked();

	UFUNCTION(BlueprintCallable, Category = "Save Slots")
	void OnNextPageClicked();

	UFUNCTION(BlueprintCallable, Category = "Save Slots")
	void OnBackClicked();

	/** Load and play from this slot (normal load). */
	UFUNCTION(BlueprintCallable, Category = "Save Slots")
	void OnLoadClicked(int32 SlotIndex);

	/** Load into level frozen; LOAD button on overlay unfreezes. */
	UFUNCTION(BlueprintCallable, Category = "Save Slots")
	void OnPreviewClicked(int32 SlotIndex);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void RefreshScreen_Implementation() override;

private:
	static constexpr int32 SlotsPerPage = 3;

	TSharedPtr<FSlateBrush> SaveSlotsBackgroundBrush;
	TArray<TSharedPtr<FSlateBrush>> SlotHeroPortraitBrushes;
	TArray<TArray<TSharedPtr<FSlateBrush>>> SlotIdolBrushes;

	FReply HandleBackClicked();
	FReply HandleLoadClicked(int32 SlotIndex);
	FReply HandlePreviewClicked(int32 SlotIndex);
	FReply HandlePrevPageClicked();
	FReply HandleNextPageClicked();
};
