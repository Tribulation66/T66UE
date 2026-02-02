// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66RunSummaryScreen.generated.h"

class ASceneCapture2D;
class UTextureRenderTarget2D;
class AT66HeroPreviewStage;
class AT66CompanionPreviewStage;
class UT66LeaderboardRunSummarySaveGame;
class SEditableTextBox;
class SMultiLineEditableTextBox;
class UTexture2D;

/**
 * Run Summary screen shown on death: 3D preview placeholder, event log, Restart / Main Menu.
 */
UCLASS(Blueprintable)
class T66_API UT66RunSummaryScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66RunSummaryScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Run Summary")
	void OnRestartClicked();

	UFUNCTION(BlueprintCallable, Category = "Run Summary")
	void OnMainMenuClicked();

	UFUNCTION(BlueprintCallable, Category = "Run Summary")
	void OnViewLogClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FReply HandleRestartClicked();
	FReply HandleMainMenuClicked();
	FReply HandleViewLogClicked();
	FReply HandleProofConfirmClicked();
	FReply HandleProofEditClicked();
	FReply HandleReportCheatingClicked();
	FReply HandleReportSubmitClicked();
	FReply HandleReportCloseClicked();

	void HandleProofLinkNavigate() const;

	void RebuildLogItems();
	TSharedRef<ITableRow> GenerateLogRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void EnsurePreviewCaptures();
	void DestroyPreviewCaptures();
	bool LoadSavedRunSummaryIfRequested();

	// Event log is hidden by default; toggled by the "EVENT LOG" button.
	bool bLogVisible = false;

	// Run Summary banners (set on activation for the most recent run).
	bool bNewPersonalHighScore = false;
	bool bNewPersonalBestTime = false;

	/** True when opened from a leaderboard entry (saved snapshot instead of current run state). */
	bool bViewingSavedLeaderboardRunSummary = false;

	/** Loaded snapshot when bViewingSavedLeaderboardRunSummary is true. */
	UPROPERTY(Transient)
	TObjectPtr<UT66LeaderboardRunSummarySaveGame> LoadedSavedSummary;

	/** Save slot name associated with LoadedSavedSummary (needed to persist proof edits). */
	FString LoadedSavedSummarySlotName;

	// Virtualized log list (prevents building a widget per entry).
	TArray<TSharedPtr<FString>> LogItems;
	TSharedPtr<SListView<TSharedPtr<FString>>> LogListView;

	// ===== Proof of Run (leaderboard snapshot owner only) =====
	FString ProofOfRunUrl;
	bool bProofOfRunLocked = false;

	TSharedPtr<SEditableTextBox> ProofUrlTextBox;

	// ===== Cheat report UI (available to all viewers) =====
	bool bReportPromptVisible = false;
	TSharedPtr<SMultiLineEditableTextBox> ReportReasonTextBox;

	// ===== 3D preview captures (runtime, no editor assets required) =====
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> HeroPreviewRT;

	UPROPERTY(Transient)
	TObjectPtr<ASceneCapture2D> HeroCaptureActor;

	/** Brushes for Slate image widgets (resource = render target). */
	TSharedPtr<struct FSlateBrush> HeroPreviewBrush;
	TSharedPtr<struct FSlateBrush> CompanionPreviewBrush;

	/** Brushes for item/idol icon images (resource = UTexture2D). */
	TArray<TSharedPtr<struct FSlateBrush>> InventoryItemIconBrushes;
	TArray<TSharedPtr<struct FSlateBrush>> IdolIconBrushes;

	/** Strong refs to keep icon textures alive while this screen is visible (Slate brushes don't keep UObjects alive). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> InventoryItemIconTextures;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> IdolIconTextures;

	// ===== Preview stages (reuse same system as hero/companion selection) =====
	UPROPERTY(Transient)
	TObjectPtr<AT66HeroPreviewStage> HeroPreviewStage;

	UPROPERTY(Transient)
	TObjectPtr<AT66CompanionPreviewStage> CompanionPreviewStage;
};
