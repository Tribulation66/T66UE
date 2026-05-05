// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/T66MinigameMenuLayout.h"
#include "UI/T66ScreenBase.h"
#include "T66IdleMainMenuScreen.generated.h"

class UT66IdleDataSubsystem;
class UT66IdleProfileSaveGame;
class ST66MinigameMenuLayout;

UCLASS(Blueprintable)
class T66IDLE_API UT66IdleMainMenuScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66IdleMainMenuScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EIdleViewMode : uint8
	{
		MainMenu,
		Gameplay
	};

	TSharedRef<SWidget> BuildMainMenuUI();
	TSharedRef<SWidget> BuildGameplayUI();
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
	void SaveProfileState();
	void StartPlayableRun();
	void SubmitLeaderboardProgressIfNeeded();
	void SpawnEnemyForCurrentStage();
	void AwardEnemyClear();
	void TickIdleRun(float DeltaSeconds);
	void RecalculatePowerFromOwned(const UT66IdleProfileSaveGame* ProfileSave);
	UT66IdleDataSubsystem* GetIdleDataSubsystem() const;
	const struct FT66IdleTuningDefinition& GetIdleTuning() const;
	double GetGoldRewardMultiplier(const UT66IdleProfileSaveGame* ProfileSave) const;
	FName ResolveStartingHeroID(const UT66IdleDataSubsystem* DataSubsystem) const;
	void EnsureStartingUnlocks(UT66IdleProfileSaveGame* ProfileSave, const UT66IdleDataSubsystem* DataSubsystem) const;
	bool TrySpendGold(double Cost);
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
	double Gold = 0.0;
	double LifetimeGold = 0.0;
	double TapDamage = 1.0;
	double PassiveDamagePerSecond = 0.5;
	double EnemyHealth = 10.0;
	double EnemyMaxHealth = 10.0;
	double EngineProgress = 0.0;
	double UncollectedProgress = 0.0;
	float AutosaveAccumulator = 0.f;
	int32 CurrentStage = 1;
	int32 BossStagesCleared = 0;
	int32 LastSubmittedLeaderboardScore = INDEX_NONE;
	FName CurrentStageID = NAME_None;
	FName CurrentEnemyID = NAME_None;
	bool bCurrentEnemyIsStageBoss = false;
	FString CurrentEnemyDisplayName;
	FString CurrentHeroDisplayName;
	FString CurrentZoneDisplayName;
};
