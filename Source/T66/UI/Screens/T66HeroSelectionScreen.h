// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "Core/T66LocalizationSubsystem.h"
#include "T66HeroSelectionScreen.generated.h"

class UT66LocalizationSubsystem;
class AT66HeroPreviewStage;
class UMediaPlayer;
class UMediaTexture;
class UFileMediaSource;
struct FSlateBrush;
class SVerticalBox;
class SImage;
class STextBlock;
class SBorder;

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
	TSharedPtr<STextBlock> HeroDescWidget;        // Summary stats in right panel
	TSharedPtr<STextBlock> HeroLoreWidget;        // Lore text when Lore panel is open
	TSharedPtr<STextBlock> HeroFullStatsWidget;   // Full stats text in left panel
	TSharedPtr<SImage> HeroRecordMedalImageWidget;
	TSharedPtr<STextBlock> HeroRecordMedalWidget;
	TSharedPtr<STextBlock> HeroRecordUnityWidget;
	TSharedPtr<STextBlock> HeroRecordGamesWidget;
	TSharedPtr<STextBlock> HeroRecordThirdLabelWidget;
	TSharedPtr<STextBlock> HeroRecordThirdValueWidget;
	TSharedPtr<SBorder> HeroPreviewColorBox;      // Fallback colored box when no 3D preview
	TSharedPtr<STextBlock> DifficultyDropdownText; // Current difficulty display

	/** Brushes for the 5-slot hero carousel portraits (prev2..next2). */
	TArray<TSharedPtr<struct FSlateBrush>> HeroCarouselPortraitBrushes;
	TArray<FLinearColor> HeroCarouselSlotColors;
	TArray<EVisibility> HeroCarouselSlotVisibility;

	/** Brushes for the 5-slot companion carousel portraits (prev2..next2). */
	TArray<TSharedPtr<struct FSlateBrush>> CompanionCarouselPortraitBrushes;
	TArray<FLinearColor> CompanionCarouselSlotColors;
	TArray<EVisibility> CompanionCarouselSlotVisibility;
	TArray<FText> CompanionCarouselSlotLabels;

	/** Skins list container; refreshed in place when Equip/Buy changes so buttons toggle without full rebuild. */
	TSharedPtr<SVerticalBox> SkinsListBoxWidget;

	/** AC balance text in skins panel; updated dynamically when purchasing skins. */
	TSharedPtr<STextBlock> ACBalanceTextBlock;
	TSharedPtr<FSlateBrush> ACBalanceIconBrush;
	TSharedPtr<FSlateBrush> HeroRecordMedalBrush;

	/** Hero preview video (e.g. KnightClip): media player and texture for [VIDEO PREVIEW] area. */
	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> HeroPreviewMediaPlayer;
	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> HeroPreviewMediaTexture;
	UPROPERTY(Transient)
	TObjectPtr<UFileMediaSource> KnightPreviewMediaSource;
	/** Brush bound to HeroPreviewMediaTexture for Slate SImage; kept alive so Slate does not hold raw UObject. */
	TSharedPtr<FSlateBrush> HeroPreviewVideoBrush;
	/** Brush for the bottom party leader portrait; must outlive BuildSlateUI because Slate stores a raw brush pointer. */
	TSharedPtr<FSlateBrush> PartyHeroPortraitBrush;
	TArray<TSharedPtr<FSlateBrush>> PartyAvatarBrushes;
	TArray<TSharedPtr<FSlateBrush>> SelectedTemporaryBuffBrushes;

	/** Video area widgets: image shows video when Knight selected; placeholder shows "[VIDEO PREVIEW]" otherwise. */
	TSharedPtr<SImage> HeroPreviewVideoImage;
	TSharedPtr<STextBlock> HeroPreviewPlaceholderText;

	/** Dropdown for switching the left skins panel between hero and companion skins. */
	TArray<TSharedPtr<FString>> SkinTargetOptions;
	TSharedPtr<FString> CurrentSkinTargetOption;
	TArray<TSharedPtr<FString>> InfoTargetOptions;
	TSharedPtr<FString> CurrentInfoTargetOption;

	/** Start or stop hero preview video based on PreviewedHeroID (Knight = Hero_1 uses KnightClip). */
	void UpdateHeroPreviewVideo();
	void RequestKnightPreviewMediaSource();
	void HandleKnightPreviewMediaSourceLoaded();

	/** Create media player, texture, and brush if not yet created (called from BuildSlateUI). */
	void EnsureHeroPreviewVideoResources();

	/** Find the hero preview stage in the world (FrontendLevel) */
	AT66HeroPreviewStage* GetHeroPreviewStage() const;

	/** Build center preview widget: transparent overlay for in-world preview, or colored box fallback */
	TSharedRef<SWidget> CreateHeroPreviewWidget(const FLinearColor& FallbackColor);

	// Placeholder skins list
	TArray<FSkinData> PlaceholderSkins;

	/** When set, preview shows this companion skin without equipping. NAME_None = use equipped companion skin. */
	FName PreviewedCompanionSkinIDOverride = NAME_None;
	
	// Difficulty dropdown options
	TArray<TSharedPtr<FString>> DifficultyOptions;
	TSharedPtr<FString> CurrentDifficultyOption;

	void RefreshHeroList();
	void RefreshCompanionList();
	void RefreshHeroCarouselPortraits();
	void RefreshCompanionCarouselPortraits();
	void UpdateHeroDisplay();
	FText BuildHeroSummaryStatsText(const FHeroData& HeroData, const FT66HeroStatBlock& BaseStats, const FT66HeroStatBonuses& PermanentBuffBonuses) const;
	FText BuildHeroFullStatsText(const FHeroData& HeroData, const FT66HeroStatBlock& BaseStats, const FT66HeroPerLevelStatGains& PerLevelGains, const FT66HeroStatBonuses& PermanentBuffBonuses) const;
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

	UT66LocalizationSubsystem* GetLocSubsystem() const;
	void CommitLocalSelectionsToLobby(bool bResetReady);
	void HandlePartyStateChanged();
	void HandleSessionStateChanged();
	void SyncToSharedPartyScreen();

	/** True when the Lore panel is visible (right-side panel swaps to lore). */
	bool bShowingLore = false;
	bool bShowingStatsPanel = false;

	/** When set, preview shows this skin (e.g. Beachgoer) without equipping. NAME_None = use SelectedHeroSkinID. */
	FName PreviewSkinIDOverride;

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
	FReply HandleTemporaryBuffPresetCreateClicked();
	FReply HandleSelectBuffsClicked();
	FReply HandleLoreClicked();
	FReply HandleStatsClicked();
	FReply HandleBodyTypeAClicked();
	FReply HandleBodyTypeBClicked();
	FReply HandleEnterClicked();
	FReply HandleBackClicked();
	FReply HandleBackToPartyClicked();

	// Difficulty dropdown
	void OnDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnSkinTargetChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnInfoTargetChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);

	FDelegateHandle PartyStateChangedHandle;
	FDelegateHandle SessionStateChangedHandle;
};
