// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "Core/T66LocalizationSubsystem.h"
#include "T66CompanionSelectionScreen.generated.h"

class UT66LocalizationSubsystem;
class AT66CompanionPreviewStage;

/**
 * Companion Selection Screen - Mirrors Hero Selection layout
 * Top: Companion grid button + carousel (colored tiles). Center: 3D preview or colored box.
 * Left: Skins. Right: Companion info (name + LORE, passive, medals). No body type.
 */
UCLASS(Blueprintable)
class T66_API UT66CompanionSelectionScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66CompanionSelectionScreen(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Companion Selection")
	FName PreviewedCompanionID;

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
	TSharedPtr<STextBlock> CompanionLoreWidget;
	TSharedPtr<SBorder> CompanionPreviewColorBox;
	TSharedPtr<struct FSlateBrush> CompanionPreviewBrush;

	// Placeholder skins list
	TArray<FSkinData> PlaceholderSkins;

	AT66CompanionPreviewStage* GetCompanionPreviewStage() const;
	TSharedRef<SWidget> CreateCompanionPreviewWidget(const FLinearColor& FallbackColor);

	void RefreshCompanionList();
	void UpdateCompanionDisplay();
	void GeneratePlaceholderSkins();

	UT66LocalizationSubsystem* GetLocSubsystem() const;

	// Handle language change to rebuild UI
	UFUNCTION()
	void OnLanguageChanged(ET66Language NewLanguage);

	// Click handlers
	FReply HandlePrevClicked();
	FReply HandleNextClicked();
	FReply HandleCompanionGridClicked();
	FReply HandleNoCompanionClicked();
	FReply HandleLoreClicked();
	FReply HandleConfirmClicked();
	FReply HandleBackClicked();
};
