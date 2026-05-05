// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DeckDataTypes.h"
#include "UI/Components/T66MinigameMenuLayout.h"
#include "UI/T66ScreenBase.h"
#include "T66DeckMainMenuScreen.generated.h"

UCLASS(Blueprintable)
class T66DECK_API UT66DeckMainMenuScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66DeckMainMenuScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EDeckViewMode : uint8
	{
		MainMenu,
		HeroSelect,
		Map,
		Gameplay,
		Reward
	};

	struct FRuntimeCard
	{
		FName CardID = NAME_None;
		FText Name;
		FText Rules;
		int32 Cost = 1;
		int32 Damage = 0;
		int32 Block = 0;
		FLinearColor Accent = FLinearColor(0.45f, 0.25f, 0.86f, 1.f);
	};

	TSharedRef<SWidget> BuildMainMenuUI();
	TSharedRef<SWidget> BuildSharedMainMenuUI();
	TSharedRef<SWidget> BuildHeroSelectUI();
	TSharedRef<SWidget> BuildMapUI();
	TSharedRef<SWidget> BuildGameplayUI();
	TSharedRef<SWidget> BuildRewardUI();
	TSharedRef<SWidget> BuildMockupBackdrop(const FString& SourceRelativePath, const FLinearColor& FallbackColor) const;
	TSharedRef<SWidget> MakeDeckButton(const FText& Text, const FOnClicked& OnClicked, float Width = 340.f, float Height = 54.f) const;
	TSharedRef<SWidget> MakeChoiceButton(const FText& Title, const FText& Body, const FLinearColor& Accent, const FOnClicked& OnClicked) const;
	TSharedRef<SWidget> MakeCardWidget(int32 CardIndex);
	TSharedRef<SWidget> MakeMeterPanel(const FText& Label, const TAttribute<FText>& Value, const FLinearColor& Accent) const;
	TArray<FT66MinigameDifficultyOption> BuildDifficultyOptions() const;
	TArray<FT66MinigameLeaderboardEntry> BuildDailyLeaderboardEntries(FName DifficultyID) const;
	TArray<FT66MinigameLeaderboardEntry> BuildAllTimeLeaderboardEntries(FName DifficultyID) const;
	FText GetDailyLeaderboardStatus(FName DifficultyID) const;
	FText GetAllTimeLeaderboardStatus(FName DifficultyID) const;

	void StartPlayableRun();
	void BuildHandFromDeck();
	void EnterMap();
	void EnterEncounter(FName EncounterID);
	void CompleteEnemy();
	void RefreshCurrentStageFromFloor();
	void SaveCurrentRunState();
	bool RestoreRunFromSave(const class UT66DeckRunSaveGame* RunSave);
	void SubmitLeaderboardProgressIfNeeded();
	void RefreshEnemyIntent();
	bool IsRunActive() const;
	const class UT66DeckDataSubsystem* GetDeckDataSubsystem() const;
	const FT66DeckTuningDefinition& GetDeckTuning() const;
	FName ResolveStartingHeroID(const class UT66DeckDataSubsystem* DataSubsystem) const;
	FName ResolveStartingCompanionID(const class UT66DeckDataSubsystem* DataSubsystem) const;
	FName ResolveStartingDeckID(const class UT66DeckDataSubsystem* DataSubsystem, FName HeroID) const;
	void AddCardToDeck(FName CardID);
	void ApplyRewardItem(FName ItemID);

	FText GetEnemyText() const;
	FText GetPlayerText() const;
	FText GetEnergyText() const;
	FText GetStatusText() const;
	TOptional<float> GetEnemyHealthPercent() const;
	TOptional<float> GetPlayerHealthPercent() const;

	FReply HandlePlayClicked();
	FReply HandleLoadClicked();
	FReply HandleDailyClicked();
	FReply HandleSelectHeroClicked(FName HeroID);
	FReply HandleSelectCompanionClicked(FName CompanionID);
	FReply HandleStartDescentClicked();
	FReply HandleMapNodeClicked(FName EncounterID);
	FReply HandleRewardCardClicked(FName CardID);
	FReply HandleRewardItemClicked(FName ItemID);
	FReply HandleSkipRewardClicked();
	FReply HandleCollectionClicked();
	FReply HandleOptionsClicked();
	FReply HandleGameplayBackClicked();
	FReply HandleCardClicked(int32 CardIndex);
	FReply HandleEndTurnClicked();
	FReply HandleBackClicked();
	FReply HandleNewRunClicked();

	EDeckViewMode ViewMode = EDeckViewMode::MainMenu;
	TSharedPtr<ST66MinigameMenuLayout> SharedMenuLayout;
	int32 ActiveSaveSlotIndex = 0;
	bool bAppliedAutomationStart = false;
	bool bRunStarted = false;
	bool bRunDefeated = false;
	FName SelectedHeroID = NAME_None;
	FName SelectedCompanionID = NAME_None;
	FName SelectedStartingDeckID = NAME_None;
	FName CurrentEncounterID = NAME_None;
	FName CurrentEnemyID = NAME_None;
	FName CurrentStageID = NAME_None;
	int32 FloorIndex = 1;
	int32 StageIndex = 1;
	int32 HighestStageIndexCleared = 0;
	int32 PlayerHealth = 80;
	int32 PlayerMaxHealth = 80;
	int32 PlayerBlock = 0;
	int32 Energy = 3;
	int32 EnemyHealth = 42;
	int32 EnemyMaxHealth = 42;
	int32 EnemyIntent = 7;
	int32 Gold = 0;
	int32 LastSubmittedLeaderboardScore = INDEX_NONE;
	FText StatusText;
	TArray<FRuntimeCard> Hand;
	TArray<FName> DeckCardIDs;
	TArray<FName> RewardCardIDs;
	TArray<FName> RewardItemIDs;
};
