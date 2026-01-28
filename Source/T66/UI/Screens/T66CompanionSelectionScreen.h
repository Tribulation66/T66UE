// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66CompanionSelectionScreen.generated.h"

UCLASS(Blueprintable)
class T66_API UT66CompanionSelectionScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66CompanionSelectionScreen(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Companion Selection")
	FName PreviewedCompanionID;

	UPROPERTY(BlueprintReadWrite, Category = "Companion Selection")
	ET66BodyType SelectedBodyType = ET66BodyType::TypeA;

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	TArray<FCompanionData> GetAllCompanions();

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	bool GetPreviewedCompanionData(FCompanionData& OutCompanionData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Companion Selection")
	bool IsNoCompanionSelected() const { return PreviewedCompanionID.IsNone(); }

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void PreviewCompanion(FName CompanionID);

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void SelectNoCompanion();

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void PreviewNextCompanion();

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void PreviewPreviousCompanion();

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void ToggleBodyType();

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void OnCompanionGridClicked();

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void OnCompanionLoreClicked();

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void OnConfirmCompanionClicked();

	UFUNCTION(BlueprintCallable, Category = "Companion Selection")
	void OnBackClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Companion Selection")
	void OnPreviewedCompanionChanged(const FCompanionData& NewCompanionData, bool bIsNoCompanion);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TArray<FName> AllCompanionIDs;
	int32 CurrentCompanionIndex = -1;
	TSharedPtr<STextBlock> CompanionNameWidget;

	void RefreshCompanionList();
	void UpdateCompanionDisplay();

	FReply HandlePrevClicked();
	FReply HandleNextClicked();
	FReply HandleNoCompanionClicked();
	FReply HandleConfirmClicked();
	FReply HandleBackClicked();
};
