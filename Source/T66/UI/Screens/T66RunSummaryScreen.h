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

private:
	FReply HandleRestartClicked();
	FReply HandleMainMenuClicked();
	FReply HandleViewLogClicked();

	void RebuildLogItems();
	TSharedRef<ITableRow> GenerateLogRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void EnsurePreviewCaptures();
	void DestroyPreviewCaptures();
	void LoadSavedRunSummaryIfRequested();

	bool bLogVisible = true;

	/** True when opened from a leaderboard entry (saved snapshot instead of current run state). */
	bool bViewingSavedLeaderboardRunSummary = false;

	/** Loaded snapshot when bViewingSavedLeaderboardRunSummary is true. */
	UPROPERTY(Transient)
	TObjectPtr<UT66LeaderboardRunSummarySaveGame> LoadedSavedSummary;

	// Virtualized log list (prevents building a widget per entry).
	TArray<TSharedPtr<FString>> LogItems;

	// ===== 3D preview captures (runtime, no editor assets required) =====
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> HeroPreviewRT;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CompanionPreviewRT;

	UPROPERTY(Transient)
	TObjectPtr<ASceneCapture2D> HeroCaptureActor;

	UPROPERTY(Transient)
	TObjectPtr<ASceneCapture2D> CompanionCaptureActor;

	/** Brushes for Slate image widgets (resource = render target). */
	TSharedPtr<struct FSlateBrush> HeroPreviewBrush;
	TSharedPtr<struct FSlateBrush> CompanionPreviewBrush;

	// ===== Preview stages (reuse same system as hero/companion selection) =====
	UPROPERTY(Transient)
	TObjectPtr<AT66HeroPreviewStage> HeroPreviewStage;

	UPROPERTY(Transient)
	TObjectPtr<AT66CompanionPreviewStage> CompanionPreviewStage;
};
