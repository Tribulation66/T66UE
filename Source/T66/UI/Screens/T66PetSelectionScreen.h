// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66PetSelectionScreen.generated.h"

class SBorder;
class SProgressBar;
class STextBlock;
class SVerticalBox;

/**
 * Pet Selection Screen
 * Selects one captured boss pet for the next run. Pet bond is movement-only QoL.
 */
UCLASS(Blueprintable)
class T66_API UT66PetSelectionScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66PetSelectionScreen(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Pet Selection")
	FName PreviewedPetID;

	UFUNCTION(BlueprintCallable, Category = "Pet Selection")
	TArray<FPetData> GetCapturedPets() const;

	UFUNCTION(BlueprintCallable, Category = "Pet Selection")
	bool GetPreviewedPetData(FPetData& OutPetData) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pet Selection")
	bool IsNoPetSelected() const { return PreviewedPetID.IsNone(); }

	UFUNCTION(BlueprintCallable, Category = "Pet Selection")
	void PreviewPet(FName PetID);

	UFUNCTION(BlueprintCallable, Category = "Pet Selection")
	void SelectNoPet();

	UFUNCTION(BlueprintCallable, Category = "Pet Selection")
	void PreviewNextPet();

	UFUNCTION(BlueprintCallable, Category = "Pet Selection")
	void PreviewPreviousPet();

	UFUNCTION(BlueprintCallable, Category = "Pet Selection")
	void OnConfirmPetClicked();

	UFUNCTION(BlueprintCallable, Category = "Pet Selection")
	void OnBackClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual bool HandleBackAction() override;

private:
	TArray<FName> CapturedPetIDs;
	int32 CurrentPetIndex = -1;
	TSharedPtr<STextBlock> PetNameWidget;
	TSharedPtr<STextBlock> PetSourceWidget;
	TSharedPtr<STextBlock> PetBondTextWidget;
	TSharedPtr<STextBlock> PetMovementTextWidget;
	TSharedPtr<SBorder> PetPreviewColorBox;
	TSharedPtr<SProgressBar> PetBondProgressBar;
	TSharedPtr<SVerticalBox> PetListBoxWidget;
	TSharedPtr<SVerticalBox> PetSkinListBoxWidget;

	void RefreshPetList();
	void RebuildPetList();
	void RebuildPetSkinList();
	void UpdatePetDisplay();
	bool IsPetCaptured(FName PetID) const;
	FName GetEquippedSkinID(FName PetID) const;

	FReply HandlePrevClicked();
	FReply HandleNextClicked();
	FReply HandleNoPetClicked();
	FReply HandlePetRowClicked(FName PetID);
	FReply HandleSkinClicked(FName SkinID);
	FReply HandleConfirmClicked();
	FReply HandleBackClicked();
};
