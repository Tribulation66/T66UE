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
class ITableRow;
class SEditableTextBox;
class SMultiLineEditableTextBox;
class STableViewBase;
template<typename ItemType> class SListView;

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
	FReply HandlePowerCouponsThankYouClicked();
	FReply HandleContinueDifficultyClicked();
	FReply HandleSaveAndQuitClicked();
	FReply HandleQuitToMainMenuClicked();
	bool HasValidLiveRunSummaryContext() const;
	void PrepareChadCouponsPopupForLiveRun();
	void ResolveChadCouponsPopupForLiveRun(bool bAllowGrant);
	void HandleBackendSubmitRunDataReadyForSummary(
		const FString& RequestKey,
		bool bSuccess,
		int32 ScoreRankAlltime,
		int32 ScoreRankWeekly,
		int32 SpeedRunRankAlltime,
		int32 SpeedRunRankWeekly,
		bool bNewScorePersonalBest,
		bool bNewSpeedRunPersonalBest);
	void HandleBackendDailyClimbSubmitDataReadyForSummary(
		const FString& RequestKey,
		bool bSuccess,
		const FString& Status,
		int32 DailyRank,
		int32 CouponsAwarded);

	void HandleProofLinkNavigate() const;
	void ProcessRunSummaryLeaderboardSubmission(bool bTreatAsVictoryForTime);
	void ProcessLiveRunFinalAccounting();
	void ProcessLiveRunFinalSubmission();
	void ResetSavedRunSummaryViewerState();
	bool SaveCurrentRunToSlot(bool bFromDifficultyClearSummary);

	void RebuildLogItems();
	TSharedRef<ITableRow> GenerateLogRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void EnsurePreviewCaptures();
	void DestroyPreviewCaptures();
	bool LoadSavedRunSummaryIfRequested();

	// Event log is hidden by default; toggled by the "EVENT LOG" button.
	bool bLogVisible = false;

	// Run Summary banners (set on activation for the most recent run).
	bool bNewPersonalBestScore = false;
	bool bNewPersonalBestTime = false;
	bool bLiveRunSubmissionProcessed = false;
	bool bLiveRunFinalAccountingProcessed = false;
	bool bDifficultyClearSummaryMode = false;
	bool bDailyClimbSummaryMode = false;
	bool bBackendRankDataReceived = false;
	bool bAwaitingBackendRankData = false;
	int32 BackendScoreRankAllTime = 0;
	int32 BackendScoreRankWeekly = 0;
	int32 BackendSpeedRunRankAllTime = 0;
	int32 BackendSpeedRunRankWeekly = 0;
	int32 BackendDailyScoreRank = 0;
	bool bLiveRunCheatFlagged = false;
	bool bChadCouponsResolutionProcessed = false;

	/** When true, show "Power Coupons earned" popup (only when earned >= 1 this run, not when viewing saved). */
	bool bShowPowerCouponsPopup = false;

	/** Amount of Chad Coupons pending for this live summary. */
	int32 SummaryChadCouponsEarned = 0;

	/** Label describing what reward is represented in SummaryChadCouponsEarned. */
	FString SummaryChadCouponsSourceLabel;

	/** Failure note for a selected challenge that did not meet its payout requirements. */
	FString SummaryChadCouponsFailureReason;

	/** True when the popup checkbox is checked for the current summary. */
	bool bChadCouponsPopupDontShowAgainChecked = false;

	/** Brush for Power Coupons sprite (Content/UI/Sprites/PowerUp). */
	TSharedPtr<struct FSlateBrush> PowerCouponSpriteBrush;

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
	TArray<TSharedPtr<struct FSlateBrush>> TemporaryBuffIconBrushes;

	// ===== Preview stages (reuse same system as hero/companion selection) =====
	UPROPERTY(Transient)
	TObjectPtr<AT66HeroPreviewStage> HeroPreviewStage;

	UPROPERTY(Transient)
	TObjectPtr<AT66CompanionPreviewStage> CompanionPreviewStage;

	bool bStoredHeroPreviewStageVisibility = false;
	bool bHeroPreviewStageWasVisible = true;
};
