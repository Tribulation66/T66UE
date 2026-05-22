// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "T66CompanionSelectionScreen.generated.h"

class UT66LocalizationSubsystem;
class UT66FrontendVideoPlayer;
class SImage;
class UTexture2D;
struct FSlateBrush;

/**
 * Companion Selection Screen - Mirrors Hero Selection layout
 * Top: Companion grid button + carousel (colored tiles). Center: video preview or colored box.
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

	UPROPERTY(BlueprintReadWrite, Category = "Companion Selection")
	ET66Difficulty SelectedDifficulty = ET66Difficulty::Easy;

	/** When set, video preview shows this skin instead of equipped (e.g. Beachgoer preview). */
	FName PreviewedCompanionSkinIDOverride = NAME_None;

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
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TArray<FName> AllCompanionIDs;
	int32 CurrentCompanionIndex = -1;
	TSharedPtr<STextBlock> CompanionNameWidget;
	TSharedPtr<STextBlock> CompanionLoreWidget;
	TSharedPtr<STextBlock> CompanionUnionText;
	TSharedPtr<STextBlock> CompanionUnionHealingText;
	TSharedPtr<STextBlock> CompanionRecordRankWidget;
	TSharedPtr<STextBlock> DifficultyDropdownText;
	TSharedPtr<SBorder> CompanionPreviewColorBox;
	TSharedPtr<SBox> CompanionUnionBox;
	TSharedPtr<class SProgressBar> CompanionUnionProgressBar;
	// Cached union UI state (updated on PreviewCompanion / UpdateCompanionDisplay)
	float CompanionUnionProgress01 = 0.f;
	int32 CompanionUnionStagesCleared = 0;

	// Placeholder skins list
	TArray<FSkinData> PlaceholderSkins;

	/** Skins list container; refreshed in place when Equip/Buy (same pattern as hero selection). */
	TSharedPtr<class SVerticalBox> SkinsListBoxWidget;
	/** AC balance text in skins panel; updated when purchasing. */
	TSharedPtr<class STextBlock> ACBalanceTextBlock;
	TSharedPtr<struct FSlateBrush> ACBalanceIconBrush;
	TStrongObjectPtr<UTexture2D> ACBalanceIconTexture;

	TSharedRef<SWidget> CreateCompanionPreviewWidget(const FLinearColor& FallbackColor);

	void RefreshCompanionList();
	void UpdateCompanionDisplay();
	void RefreshCompanionCarouselPortraits();
	void RefreshDifficultyDropdownText();
	void RefreshCompanionRecordRank();
	void HandleBackendMyRankDataReady(const FString& Key, bool bSuccess, int32 Rank, int32 TotalEntries);
	void HandlePartyStateChanged();
	void HandleSessionStateChanged();
	void GeneratePlaceholderSkins();
	/** Repopulate skins list and AC display without full rebuild. */
	void RefreshSkinsList();
	void AddSkinRowsToBox(const TSharedPtr<class SVerticalBox>& Box);

	/** Brushes for the 5-slot companion carousel portraits (prev2..next2). */
	TArray<TSharedPtr<struct FSlateBrush>> CompanionCarouselPortraitBrushes;
	TArray<TSharedPtr<struct FSlateBrush>> PartyAvatarBrushes;
	TArray<TSharedPtr<struct FSlateBrush>> PartyHeroPortraitBrushes;
	TArray<TSharedPtr<SImage>> PartyAvatarImageWidgets;
	TArray<TSharedPtr<SImage>> PartyHeroPortraitImageWidgets;
	UPROPERTY(Transient)
	TObjectPtr<UT66FrontendVideoPlayer> CompanionPreviewVideoPlayer;
	TArray<TSharedPtr<FString>> DifficultyOptions;
	TSharedPtr<FString> CurrentDifficultyOption;
	FString CompanionRecordRankRequestKey;
	FDelegateHandle BackendMyRankReadyHandle;
	FDelegateHandle PartyStateChangedHandle;
	FDelegateHandle SessionStateChangedHandle;

	UT66LocalizationSubsystem* GetLocSubsystem() const;
	bool IsCompanionUnlocked(FName CompanionID) const;
	bool IsCompanionPlayable(FName CompanionID) const;

	/** True when the Lore panel is visible (right-side panel swaps to lore, same as hero selection). */
	bool bShowingLore = false;

	/** Lore detail text shown in the lore panel (scrollable). */
	TSharedPtr<STextBlock> CompanionLoreDetailWidget;

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
	FReply HandleEnterClicked();
	FReply HandleChallengesClicked();
	FReply HandleModsClicked();
	FReply HandleBackClicked();
	void OnDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnEnterTribulationClicked();
	void OnChallengesClicked();
	void OnModsClicked();
	void OpenCommunityContent(bool bOpenMods);
	const FSlateBrush* GetCompanionPreviewVideoBrush() const;
};
