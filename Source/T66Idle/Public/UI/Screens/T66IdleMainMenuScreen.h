// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/T66MinigameMenuLayout.h"
#include "UI/T66ScreenBase.h"
#include "UI/WidgetGames/T66WidgetGamePersistence.h"
#include "UI/WidgetGames/T66WidgetGameSession.h"
#include "T66IdleMainMenuScreen.generated.h"

class UT66IdleDataSubsystem;
class UT66IdleProfileSaveGame;
class ST66MinigameMenuLayout;

UCLASS(Blueprintable)
class T66IDLE_API UT66IdleMainMenuScreen : public UT66ScreenBase, public IT66WidgetGameSession, public IT66WidgetGamePersistence
{
	GENERATED_BODY()

public:
	UT66IdleMainMenuScreen(const FObjectInitializer& ObjectInitializer);
	virtual void ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext) override;
	virtual void DeactivateWidgetGame() override;
	virtual void PauseWidgetGame() override;
	virtual void ResumeWidgetGame() override;
	virtual void RequestWidgetGameExit() override;
	virtual void SaveWidgetGameState() override;
	virtual void LoadWidgetGameState() override;
	virtual void FlushWidgetGamePersistence() override;
	virtual void RefreshWidgetGamePersistence() override;

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EIdleViewMode : uint8
	{
		MainMenu,
		Gameplay,
		Summary
	};

	TSharedRef<SWidget> BuildMainMenuUI();
	TSharedRef<SWidget> BuildGameplayUI();
	TSharedRef<SWidget> BuildSummaryUI();
	TSharedRef<SWidget> BuildSharedMainMenuUI();
	TArray<FT66MinigameDifficultyOption> BuildDifficultyOptions() const;
	TArray<FT66MinigameLeaderboardEntry> BuildDailyLeaderboardEntries(FName DifficultyID) const;
	TArray<FT66MinigameLeaderboardEntry> BuildAllTimeLeaderboardEntries(FName DifficultyID) const;
	FText GetDailyLeaderboardStatus(FName DifficultyID) const;
	FText GetAllTimeLeaderboardStatus(FName DifficultyID) const;
	TSharedRef<SWidget> BuildMockupBackdrop(const FString& SourceRelativePath, const FLinearColor& FallbackColor) const;
	TSharedRef<SWidget> MakeIdleButton(const FText& Text, const FOnClicked& OnClicked, float Width = 340.f, float Height = 54.f) const;
	TSharedRef<SWidget> MakeStatPanel(const FText& Label, const TAttribute<FText>& Value, const FLinearColor& Accent) const;
	TSharedRef<SWidget> MakePurchasePanel();
	TSharedRef<SWidget> MakePurchaseButton(const FText& Label, const FText& Body, const FLinearColor& Accent, const FOnClicked& OnClicked) const;

	void EnsureProfileLoaded();
	void SaveProfileState(bool bSubmitLeaderboard = true);
	void ReportWidgetGameResult(bool bSuccessful, int32 FinalScore);
	void ResetClosedLoopRunState();
	void StartPlayableRun();
	void FinishIdleRun(bool bWasVictory);
	void SubmitLeaderboardProgressIfNeeded();
	void SpawnEnemyForCurrentStage();
	bool AwardEnemyClear();
	void TickIdleRun(float DeltaSeconds);
	void RecalculatePowerFromOwned(const UT66IdleProfileSaveGame* ProfileSave);
	UT66IdleDataSubsystem* GetIdleDataSubsystem() const;
	const struct FT66IdleTuningDefinition& GetIdleTuning() const;
	double GetGoldRewardMultiplier(const UT66IdleProfileSaveGame* ProfileSave) const;
	FName ResolveStartingHeroID(const UT66IdleDataSubsystem* DataSubsystem) const;
	void EnsureStartingUnlocks(UT66IdleProfileSaveGame* ProfileSave, const UT66IdleDataSubsystem* DataSubsystem) const;
	bool TrySpendGold(double Cost);
	int32 GetFinalStageIndex() const;
	const struct FT66IdleHeroDefinition* FindNextPurchasableHero(const UT66IdleProfileSaveGame* ProfileSave) const;
	const struct FT66IdleCompanionDefinition* FindNextPurchasableCompanion(const UT66IdleProfileSaveGame* ProfileSave) const;
	const struct FT66IdleItemDefinition* FindNextPurchasableItem(const UT66IdleProfileSaveGame* ProfileSave) const;
	const struct FT66IdleIdolDefinition* FindNextPurchasableIdol(const UT66IdleProfileSaveGame* ProfileSave) const;

	FText GetGoldText() const;
	FText GetStageText() const;
	FText GetPowerText() const;
	FText GetEnemyText() const;
	FText GetProgressText() const;
	FText GetOwnedText() const;
	TOptional<float> GetEnemyHealthPercent() const;
	TOptional<float> GetEngineProgressPercent() const;

	FReply HandlePlayClicked();
	FReply HandleLoadClicked();
	FReply HandleDailyClicked();
	FReply HandleUpgradeClicked();
	FReply HandleEngineClicked();
	FReply HandleTapClicked();
	FReply HandleCollectClicked();
	FReply HandleBuyHeroClicked();
	FReply HandleBuyCompanionClicked();
	FReply HandleBuyItemClicked();
	FReply HandleBuyIdolClicked();
	FReply HandleOptionsClicked();
	FReply HandleGameplayBackClicked();
	FReply HandleStartClicked();
	FReply HandleBackClicked();

	EIdleViewMode ViewMode = EIdleViewMode::MainMenu;
	TSharedPtr<ST66MinigameMenuLayout> SharedMenuLayout;
	bool bAppliedAutomationStart = false;
	bool bProfileLoaded = false;
	bool bRunStarted = false;
	bool bRunComplete = false;
	bool bLastRunVictory = false;
	double Gold = 0.0;
	double LifetimeGold = 0.0;
	double TapDamage = 1.0;
	double PassiveDamagePerSecond = 0.5;
	double EnemyHealth = 10.0;
	double EnemyMaxHealth = 10.0;
	double EngineProgress = 0.0;
	double UncollectedProgress = 0.0;
	float AutosaveAccumulator = 0.f;
	float IdleRunTickAccumulator = 0.f;
	static constexpr float IdleRunTickIntervalSeconds = 1.f / 15.f;
	int32 CurrentStage = 1;
	int32 SummaryStageReached = 0;
	int32 SummaryBossesCleared = 0;
	double SummaryGoldBanked = 0.0;
	double SummaryLifetimeGold = 0.0;
	int32 BossStagesCleared = 0;
	int32 LastSubmittedLeaderboardScore = INDEX_NONE;
	FName CurrentStageID = NAME_None;
	FName CurrentEnemyID = NAME_None;
	bool bCurrentEnemyIsStageBoss = false;
	FString CurrentEnemyDisplayName;
	FString CurrentHeroDisplayName;
	FString CurrentZoneDisplayName;
	FT66WidgetGameHostContext WidgetGameHostContext;
};
