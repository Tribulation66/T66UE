// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "Core/T66LocalizationSubsystem.h"
#include "T66HeroSelectionScreen.generated.h"

class UT66LocalizationSubsystem;

/**
 * Hero Selection Screen - Bible 1.10 Layout
 * Top: Hero belt carousel with HERO GRID button
 * Center: 3D preview area with CHOOSE COMPANION button
 * Left: Skins panel
 * Right: Hero info panel (name, video, description, medals, lore button)
 * Bottom: Difficulty selector, THE LAB button, ENTER THE TRIBULATION button
 */
UCLASS(Blueprintable)
class T66_API UT66HeroSelectionScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66HeroSelectionScreen(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Hero Selection")
	FName PreviewedHeroID;

	UPROPERTY(BlueprintReadWrite, Category = "Hero Selection")
	ET66Difficulty SelectedDifficulty = ET66Difficulty::Easy;

	UPROPERTY(BlueprintReadWrite, Category = "Hero Selection")
	ET66BodyType SelectedBodyType = ET66BodyType::TypeA;

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	TArray<FHeroData> GetAllHeroes();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	bool GetPreviewedHeroData(FHeroData& OutHeroData);

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	bool GetSelectedCompanionData(FCompanionData& OutCompanionData);

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewHero(FName HeroID);

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewNextHero();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewPreviousHero();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void SelectDifficulty(ET66Difficulty Difficulty);

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void ToggleBodyType();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnHeroGridClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnChooseCompanionClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnHeroLoreClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnTheLabClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnEnterTribulationClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnBackClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Selection")
	void OnPreviewedHeroChanged(const FHeroData& NewHeroData);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TArray<FName> AllHeroIDs;
	int32 CurrentHeroIndex = 0;
	
	// Widgets that need updating
	TSharedPtr<STextBlock> HeroNameWidget;        // Hero name in left panel
	TSharedPtr<STextBlock> HeroDescWidget;        // Description in right panel
	TSharedPtr<STextBlock> BodyTypeWidget;        // Body type toggle text
	TSharedPtr<SBorder> HeroPreviewColorBox;      // Colored box for hero preview
	TSharedPtr<STextBlock> DifficultyDropdownText; // Current difficulty display

	// Placeholder skins list
	TArray<FSkinData> PlaceholderSkins;
	
	// Difficulty dropdown options
	TArray<TSharedPtr<FString>> DifficultyOptions;
	TSharedPtr<FString> CurrentDifficultyOption;

	void RefreshHeroList();
	void UpdateHeroDisplay();
	void GeneratePlaceholderSkins();

	UT66LocalizationSubsystem* GetLocSubsystem() const;

	// Handle language change to rebuild UI
	UFUNCTION()
	void OnLanguageChanged(ET66Language NewLanguage);

	// Click handlers
	FReply HandlePrevClicked();
	FReply HandleNextClicked();
	FReply HandleHeroGridClicked();
	FReply HandleCompanionClicked();
	FReply HandleLoreClicked();
	FReply HandleBodyTypeClicked();
	FReply HandleTheLabClicked();
	FReply HandleEnterClicked();
	FReply HandleBackClicked();
	
	// Difficulty dropdown
	void OnDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	TSharedRef<SWidget> GenerateDifficultyItem(TSharedPtr<FString> InItem);
};
