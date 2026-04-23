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
	PopupMinigame,
	WorldMode,
};

UENUM(BlueprintType)
enum class ET66ArcadePopupGameType : uint8
{
	None,
	WhackAMole,
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
	ET66ArcadeInteractableClass ArcadeClass = ET66ArcadeInteractableClass::PopupMinigame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	ET66ArcadePopupGameType PopupGameType = ET66ArcadePopupGameType::None;

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
	inline const FName PopupRoundSeconds(TEXT("PopupRoundSeconds"));
	inline const FName PopupSpawnInterval(TEXT("PopupSpawnInterval"));
	inline const FName PopupTargetHits(TEXT("PopupTargetHits"));
	inline const FName PopupSuccessGold(TEXT("PopupSuccessGold"));
	inline const FName PopupFailureGoldPenalty(TEXT("PopupFailureGoldPenalty"));
	inline const FName VehicleDuration(TEXT("VehicleDuration"));
	inline const FName VehicleDriveSpeed(TEXT("VehicleDriveSpeed"));
	inline const FName VehicleTurnSpeed(TEXT("VehicleTurnSpeed"));
	inline const FName VehicleMowRadius(TEXT("VehicleMowRadius"));
	inline const FName VehicleMowMinSpeed(TEXT("VehicleMowMinSpeed"));
}
