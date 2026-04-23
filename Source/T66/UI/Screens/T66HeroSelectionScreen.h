// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "Core/T66LocalizationSubsystem.h"
#include "T66HeroSelectionScreen.generated.h"

class UT66LocalizationSubsystem;
class UT66LeaderboardRunSummarySaveGame;
class UT66HeroSelectionPreviewController;
struct FSlateBrush;
class SVerticalBox;
class SBox;
class SImage;
class STextBlock;
class SBorder;
class SProgressBar;

/**
 * Hero Selection Screen - Bible 1.10 Layout
 * Top: Back button plus hero belt carousel with HERO GRID button
 * Middle: Skins panel, 3D preview area, and hero info panel aligned to preview height
 * Bottom: Full-width control bar with party box, companion controls, and run controls
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
	FName PreviewedCompanionID;

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
	bool GetPreviewedCompanionData(FCompanionData& OutCompanionData);

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewHero(FName HeroID);

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewCompanion(FName CompanionID);

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void SelectNoCompanion();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewNextHero();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewNextCompanion();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewPreviousHero();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void PreviewPreviousCompanion();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void SelectDifficulty(ET66Difficulty Difficulty);

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void ToggleBodyType();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnHeroGridClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnCompanionGridClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnChooseCompanionClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnHeroLoreClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnEnterTribulationClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnChallengesClicked();

	UFUNCTION(BlueprintCallable, Category = "Hero Selection")
	void OnBackClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Selection")
	void OnPreviewedHeroChanged(const FHeroData& NewHeroData);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TArray<FName> AllHeroIDs;
	int32 CurrentHeroIndex = 0;
	TArray<FName> AllCompanionIDs;
	int32 CurrentCompanionIndex = -1;
	
	// Widgets that need updating
	TSharedPtr<STextBlock> HeroNameWidget;        // Selected hero title in right panel
	TSharedPtr<STextBlock> HeroQuoteWidget;       // Deprecated summary quote widget
	TSharedPtr<STextBlock> HeroLoreWidget;        // Lore text when Lore panel is open
	TSharedPtr<SBox> HeroSummaryStatsHost;        // Shared summary stats panel in right panel
	TSharedPtr<SBox> HeroFullStatsHost;           // Shared full stats panel in left panel
	TSharedPtr<SImage> HeroRecordMedalImageWidget;
	TSharedPtr<STextBlock> HeroRecordMedalWidget;
	TSharedPtr<STextBlock> HeroRecordUnityWidget;
	TSharedPtr<STextBlock> HeroRecordGamesWidget;
	TSharedPtr<STextBlock> HeroRecordThirdLabelWidget;
	TSharedPtr<STextBlock> HeroRecordThirdValueWidget;
	TSharedPtr<STextBlock> CompanionHealsPerSecondWidget;
	TSharedPtr<STextBlock> CompanionUnityTextWidget;
	TSharedPtr<STextBlock> DifficultyDropdownText; // Current difficulty display
	TSharedPtr<STextBlock> SkinTargetDropdownText;
	TSharedPtr<STextBlock> InfoTargetDropdownText;
	TSharedPtr<class SWidgetSwitcher> LeftPanelWidgetSwitcher;
	TSharedPtr<class SWidgetSwitcher> RightInfoWidgetSwitcher;
	TSharedPtr<class SWidgetSwitcher> RightFooterWidgetSwitcher;
	TSharedPtr<SProgressBar> CompanionUnityProgressBar;

	/** Brushes for the 5-slot hero carousel portraits (prev2..next2). */
	TArray<TSharedPtr<struct FSlateBrush>> HeroCarouselPortraitBrushes;
	TArray<FLinearColor> HeroCarouselSlotColors;
	TArray<EVisibility> HeroCarouselSlotVisibility;
	TArray<TSharedPtr<class SImage>> HeroCarouselImageWidgets;

	/** Brushes for the 5-slot companion carousel portraits (prev2..next2). */
	TArray<TSharedPtr<struct FSlateBrush>> CompanionCarouselPortraitBrushes;
	TArray<FLinearColor> CompanionCarouselSlotColors;
	TArray<EVisibility> CompanionCarouselSlotVisibility;
	TArray<FText> CompanionCarouselSlotLabels;
	TArray<TSharedPtr<class SImage>> CompanionCarouselImageWidgets;
	TArray<TSharedPtr<class STextBlock>> CompanionCarouselLabelWidgets;

	/** Skins list container; refreshed in place when Equip/Buy changes so buttons toggle without full rebuild. */
	TSharedPtr<SVerticalBox> SkinsListBoxWidget;

	/** AC balance text in skins panel; updated dynamically when purchasing skins. */
	TSharedPtr<STextBlock> ACBalanceTextBlock;
	TSharedPtr<FSlateBrush> ACBalanceIconBrush;
	TSharedPtr<FSlateBrush> ChallengesButtonIconBrush;
	TSharedPtr<FSlateBrush> HeroRecordMedalBrush;
	TArray<TSharedPtr<FSlateBrush>> PartyAvatarBrushes;
	TArray<TSharedPtr<FSlateBrush>> PartyHeroPortraitBrushes;
	TArray<TSharedPtr<FSlateBrush>> SelectedTemporaryBuffBrushes;
	TArray<TSharedPtr<class SImage>> PartyAvatarImageWidgets;
	TArray<TSharedPtr<class SImage>> PartyHeroPortraitImageWidgets;

	TSharedPtr<FSlateBrush> HeroUltimateIconBrush;
	TSharedPtr<FSlateBrush> HeroPassiveIconBrush;

	/** Dropdown for switching the left skins panel between hero and companion skins. */
	TArray<TSharedPtr<FString>> SkinTargetOptions;
	TSharedPtr<FString> CurrentSkinTargetOption;
	TArray<TSharedPtr<FString>> InfoTargetOptions;
	TSharedPtr<FString> CurrentInfoTargetOption;

	// Placeholder skins list
	TArray<FSkinData> PlaceholderSkins;
	
	// Difficulty dropdown options
	TArray<TSharedPtr<FString>> DifficultyOptions;
	TSharedPtr<FString> CurrentDifficultyOption;

	void RefreshHeroList();
	void RefreshCompanionList();
	void RefreshHeroCarouselPortraits();
	void RefreshCompanionCarouselPortraits();
	void RefreshPanelSwitchers();
	void RefreshTargetDropdownTexts();
	void RefreshDifficultyDropdownText();
	void UpdateHeroDisplay();
	void EnsureHeroStatsSnapshot();
	void PopulateHeroStatsSnapshot(const FHeroData& HeroData, const FT66HeroStatBlock& BaseStats, const FT66HeroStatBonuses& PermanentBuffBonuses);
	void RefreshHeroStatsPanels();
	void GeneratePlaceholderSkins();
	/** Repopulate skins list from current PlaceholderSkins (call after Equip/Buy so Equipped/Equip toggle in place). */
	void RefreshSkinsList();
	/** Add current PlaceholderSkins rows to the given vertical box (used by BuildSlateUI and RefreshSkinsList). */
	void AddSkinRowsToBox(const TSharedPtr<SVerticalBox>& Box);
	bool IsShowingCompanionSkins() const;
	void SetShowingCompanionSkins(bool bShowCompanionSkins);
	bool IsShowingCompanionInfo() const;
	void SetShowingCompanionInfo(bool bShowCompanionInfo);
	FName GetCurrentSkinEntityID() const;
	FName GetEffectivePreviewedCompanionSkinID() const;
	UT66HeroSelectionPreviewController* GetOrCreatePreviewController();
	const UT66HeroSelectionPreviewController* GetPreviewController() const;

	UT66LocalizationSubsystem* GetLocSubsystem() const;
	void CommitLocalSelectionsToLobby(bool bResetReady);
	void HandlePartyStateChanged();
	void HandleSessionStateChanged();
	void SyncToSharedPartyScreen();

	/** True when the Lore panel is visible (right-side panel swaps to lore). */
	bool bShowingLore = false;
	bool bShowingStatsPanel = false;
	float CompanionUnityProgress01 = 0.f;
	int32 CompanionUnityStagesCleared = 0;

	/** True when the left panel should show companion skins instead of hero skins. */
	bool bShowingCompanionSkins = false;
	bool bShowingCompanionInfo = false;

	// Handle language change to rebuild UI
	UFUNCTION()
	void OnLanguageChanged(ET66Language NewLanguage);

	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	// Click handlers
	FReply HandlePrevClicked();
	FReply HandleNextClicked();
	FReply HandleCompanionPrevClicked();
	FReply HandleCompanionNextClicked();
	FReply HandleHeroGridClicked();
	FReply HandleCompanionGridClicked();
	FReply HandleCompanionClicked();
	FReply HandleTemporaryBuffSlotClicked(int32 SlotIndex);
	FReply HandleSelectBuffsClicked();
	FReply HandleLoreClicked();
	FReply HandleStatsClicked();
	FReply HandleOpenStatsPanelClicked();
	FReply HandleUltimatePreviewClicked();
	FReply HandlePassivePreviewClicked();
	FReply HandleBodyTypeAClicked();
	FReply HandleBodyTypeBClicked();
	FReply HandleEnterClicked();
	FReply HandleChallengesClicked();
	FReply HandleBackClicked();
	FReply HandleBackToPartyClicked();

	// Difficulty dropdown
	void OnDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnSkinTargetChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnInfoTargetChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);

	FDelegateHandle PartyStateChangedHandle;
	FDelegateHandle SessionStateChangedHandle;
	ET66Language LastBuiltLanguage = ET66Language::English;

	UPROPERTY(Transient)
	TObjectPtr<UT66HeroSelectionPreviewController> PreviewController;

	UPROPERTY(Transient)
	TObjectPtr<UT66LeaderboardRunSummarySaveGame> HeroStatsSnapshot;
};
