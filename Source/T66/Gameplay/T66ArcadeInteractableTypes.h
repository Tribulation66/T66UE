// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "T66ArcadeInteractableTypes.generated.h"

class UStaticMesh;
class UT66ArcadePopupWidget;

UENUM(BlueprintType)
enum class ET66ArcadeInteractableClass : uint8
{
	PopupArcade,
	WorldMode,
};

UENUM(BlueprintType)
enum class ET66ArcadeGameType : uint8
{
	None,
	Random,
	WhackAMole,
	Topwar,
};

USTRUCT(BlueprintType)
struct T66_API FT66ArcadeRewardTuning
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	int32 ScoreForFullRewardChance = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float LootBagBaseChance = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float LootBagMaxChance = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	int32 MaxLootBags = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float ChestBaseChance = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float ChestMaxChance = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float WeaponCrateBaseChance = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float WeaponCrateMaxChance = 0.40f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	int32 MaxWeaponCrates = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float AmplifierBaseChance = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float AmplifierMaxChance = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	int32 MaxAmplifiers = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	int32 AmplifierStatBonus = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	float AmplifierDurationSeconds = 10.f;
};

USTRUCT(BlueprintType)
struct T66_API FT66ArcadeModifierSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	TMap<FName, float> FloatValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	TMap<FName, int32> IntValues;

	float GetFloat(const FName Key, const float DefaultValue) const
	{
		if (const float* Value = FloatValues.Find(Key))
		{
			return *Value;
		}

		return DefaultValue;
	}

	int32 GetInt(const FName Key, const int32 DefaultValue) const
	{
		if (const int32* Value = IntValues.Find(Key))
		{
			return *Value;
		}

		return DefaultValue;
	}
};

USTRUCT(BlueprintType)
struct T66_API FT66ArcadeInteractableData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	FName ArcadeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	FText DisplayName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	FText InteractionVerb = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	FText ExitInteractionVerb = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	ET66ArcadeInteractableClass ArcadeClass = ET66ArcadeInteractableClass::PopupArcade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	ET66ArcadeGameType ArcadeGameType = ET66ArcadeGameType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	TArray<ET66ArcadeGameType> RandomGameTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Popup")
	TSubclassOf<UT66ArcadePopupWidget> PopupWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	bool bConsumeOnSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	bool bConsumeOnFailure = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Visual")
	TSoftObjectPtr<UStaticMesh> DisplayMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Visual")
	FVector DisplayMeshScale = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Visual")
	FVector DisplayMeshOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Visual")
	FLinearColor Tint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Modifiers")
	FT66ArcadeModifierSet Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Rewards")
	FT66ArcadeRewardTuning RewardTuning;
};

USTRUCT(BlueprintType)
struct T66_API FT66ArcadeInteractableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	FT66ArcadeInteractableData ArcadeData;
};

namespace T66ArcadeModifierKeys
{
	inline const FName ArcadeRoundSeconds(TEXT("ArcadeRoundSeconds"));
	inline const FName ArcadeStartInterval(TEXT("ArcadeStartInterval"));
	inline const FName ArcadeEndInterval(TEXT("ArcadeEndInterval"));
	inline const FName ArcadeTargetScore(TEXT("ArcadeTargetScore"));
	inline const FName ArcadeScorePerHit(TEXT("ArcadeScorePerHit"));
	inline const FName ArcadeMissPenalty(TEXT("ArcadeMissPenalty"));
	inline const FName TopwarStartingSquad(TEXT("TopwarStartingSquad"));
	inline const FName TopwarChoiceScore(TEXT("TopwarChoiceScore"));
	inline const FName TopwarSquadGain(TEXT("TopwarSquadGain"));
	inline const FName VehicleDuration(TEXT("VehicleDuration"));
	inline const FName VehicleDriveSpeed(TEXT("VehicleDriveSpeed"));
	inline const FName VehicleTurnSpeed(TEXT("VehicleTurnSpeed"));
	inline const FName VehicleMowRadius(TEXT("VehicleMowRadius"));
	inline const FName VehicleMowMinSpeed(TEXT("VehicleMowMinSpeed"));
}
