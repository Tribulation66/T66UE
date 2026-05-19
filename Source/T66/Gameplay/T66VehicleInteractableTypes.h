// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "T66VehicleInteractableTypes.generated.h"

class UStaticMesh;

USTRUCT(BlueprintType)
struct T66_API FT66VehicleInteractableData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FName VehicleID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FText DisplayName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FText InteractionVerb = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FText ExitInteractionVerb = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Visuals")
	TSoftObjectPtr<UStaticMesh> DisplayMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Visuals")
	FVector DisplayMeshScale = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Visuals")
	FVector DisplayMeshOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Visuals")
	FLinearColor Tint = FLinearColor(0.94f, 0.53f, 0.17f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Tuning", meta = (ClampMin = "0.0"))
	float PilotSeconds = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Tuning", meta = (ClampMin = "0.0"))
	float DriveSpeed = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Tuning", meta = (ClampMin = "0.0"))
	float TurnSpeedDegreesPerSecond = 620.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Tuning", meta = (ClampMin = "0.0"))
	float MowKillRadius = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Tuning", meta = (ClampMin = "0.0"))
	float MowMinSpeed = 125.f;
};

USTRUCT(BlueprintType)
struct T66_API FT66VehicleInteractableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FT66VehicleInteractableData VehicleData;
};