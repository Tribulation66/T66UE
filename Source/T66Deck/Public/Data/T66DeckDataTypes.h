// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "T66DeckDataTypes.generated.h"

UENUM(BlueprintType)
enum class ET66DeckCardType : uint8
{
	Attack UMETA(DisplayName = "Attack"),
	Skill UMETA(DisplayName = "Skill"),
	Power UMETA(DisplayName = "Power"),
	Curse UMETA(DisplayName = "Curse")
};

UENUM(BlueprintType)
enum class ET66DeckNodeType : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Elite UMETA(DisplayName = "Elite"),
	Boss UMETA(DisplayName = "Boss"),
	Rest UMETA(DisplayName = "Rest"),
	Shop UMETA(DisplayName = "Shop"),
	Event UMETA(DisplayName = "Event"),
	Treasure UMETA(DisplayName = "Treasure")
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckHeroDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FLinearColor AccentColor = FLinearColor(0.72f, 0.50f, 0.24f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 StartingHealth = 80;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 StartingGold = 0;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckCompanionDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName CompanionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FLinearColor AccentColor = FLinearColor(0.34f, 0.62f, 0.82f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 StartingBlock = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 PassiveDamageBonus = 0;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckCardDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName CardID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString RulesText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	ET66DeckCardType CardType = ET66DeckCardType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 EnergyCost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName ArchetypeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 Block = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName RarityID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FLinearColor AccentColor = FLinearColor(0.45f, 0.25f, 0.86f, 1.0f);
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckRelicDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName RelicID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString RulesText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName RarityID = NAME_None;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckEnemyDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName EnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString IntentLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FLinearColor AccentColor = FLinearColor(0.72f, 0.20f, 0.18f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 BaseHealth = 40;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 HealthPerFloor = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 BaseIntentDamage = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 IntentDamagePerFloor = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 GoldReward = 10;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString RulesText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName RarityID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FLinearColor AccentColor = FLinearColor(0.66f, 0.48f, 0.22f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 BonusMaxHealth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 BonusStartingBlock = 0;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckEncounterDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName EncounterID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	ET66DeckNodeType NodeType = ET66DeckNodeType::Combat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 ActIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	TArray<FName> EnemyIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	TArray<FName> RewardCardIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	TArray<FName> RewardItemIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 MinFloor = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 MaxFloor = 999;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 GoldRewardBonus = 0;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckStartingDeckDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName StartingDeckID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	TArray<FName> CardIDs;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckTuningDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName TuningID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName StartingHeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName StartingCompanionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName StartingDeckID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 StartingEnergy = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 HandSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 MapChoicesPerFloor = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 StartingFloor = 1;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckStageDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName StageID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 ActIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 StageIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 FirstFloor = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 BossFloor = 9;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName BossEncounterID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 ClearGoldReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName NextStageID = NAME_None;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckRunNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FName NodeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	ET66DeckNodeType NodeType = ET66DeckNodeType::Combat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 ActIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 DepthIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	TArray<FName> LinkedNodeIDs;
};

USTRUCT(BlueprintType)
struct T66DECK_API FT66DeckSaveSlotSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 ActIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	int32 FloorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
	FString LastUpdatedUtc;
};
