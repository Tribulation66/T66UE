// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "UI/T66ScreenBase.h"
#include "T66SaveSlotsScreen.generated.h"

struct FSlateBrush;
class UT66GameInstance;
class UT66RunSaveGame;

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

	UFUNCTION(BlueprintCallable, Category = "Save Slots")
	void OnLoadClicked(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Save Slots")
	void OnPreviewClicked(int32 SlotIndex);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void RefreshScreen_Implementation() override;

private:
	static constexpr int32 SlotsPerPage = 4;
	static constexpr int32 MaxPartyPreviewSlots = 4;

	ET66PartySize ActivePartySizeFilter = ET66PartySize::Solo;
	TArray<TArray<TSharedPtr<FSlateBrush>>> SlotPartyAvatarBrushes;
	TArray<TArray<TSharedPtr<FSlateBrush>>> SlotHeroPortraitBrushes;
	TArray<int32> VisibleSlotIndices;

	void RebuildVisibleSlotIndices();
	int32 GetVisibleSlotIndexForPageEntry(int32 LocalIndex) const;
	void PrepareGameInstanceForLoadedSave(UT66GameInstance* GI, const UT66RunSaveGame* Loaded, int32 SlotIndex, bool bPreviewMode);
	bool CanLoadSlotWithCurrentParty(const UT66RunSaveGame* Loaded) const;

	FReply HandleBackClicked();
	FReply HandleLoadClicked(int32 SlotIndex);
	FReply HandlePreviewClicked(int32 SlotIndex);
	FReply HandlePrevPageClicked();
	FReply HandleNextPageClicked();
};
