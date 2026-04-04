// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/T66DataTypes.h"
#include "T66RunSaveGame.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66SavedLuckAccumulator
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName Category = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float Sum01 = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct T66_API FT66SavedRunSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CurrentStage = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float CurrentHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CurrentGold = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CurrentDebt = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 DifficultyTier = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 DifficultySkulls = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 TotemsActivatedCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float GamblerAnger01 = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FName> OwedBossIDs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CowardiceGatesTakenCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66InventorySlot> InventorySlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 ActiveGamblersTokenLevel = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FString> EventLog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FRunEvent> StructuredEventLog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bStageTimerActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float StageTimerSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float SpeedRunElapsedSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bSpeedRunStarted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float CompletedStageActiveSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float FinalRunElapsedSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bRunEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bRunEndedAsVictory = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CurrentScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 HeroLevel = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 HeroXP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 XPToNextLevel = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66HeroStatBlock HeroStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PowerCrystalsEarnedThisRun = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bBossActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName ActiveBossID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 BossMaxHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 BossCurrentHP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FName> EquippedIdols;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<uint8> EquippedIdolTiers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 RemainingCatchUpIdolPicks = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bPendingDifficultyClearSummary = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bSaintBlessingActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float FinalSurvivalEnemyScalar = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66SavedLuckAccumulator> LuckQuantityAccumulators;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66SavedLuckAccumulator> LuckQualityAccumulators;
};

UCLASS(BlueprintType)
class T66_API UT66RunSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = 4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName HeroID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66BodyType HeroBodyType = ET66BodyType::TypeA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName CompanionID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66PartySize PartySize = ET66PartySize::Solo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString MapName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FTransform PlayerTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString LastPlayedUtc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString OwnerPlayerId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString OwnerDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FString> PartyMemberIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FString> PartyMemberDisplayNames;

	/** Stage reached when save was written (for slot display). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 StageReached = 1;

	/** Equipped idol IDs when save was written (for slot display). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FName> EquippedIdols;

	/** Equipped idol tier values aligned with EquippedIdols (0 empty, 1 black .. 4 white). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<uint8> EquippedIdolTiers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 RunSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bRunIneligibleForLeaderboard = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66SavedRunSnapshot RunSnapshot;
};
