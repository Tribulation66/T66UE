// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"
#include "T66AudioTypes.generated.h"

/**
 * Data-authored gameplay/UI audio event.
 *
 * Row names are the stable event IDs used by C++ (for example, UI.Click or Hero.Attack.Hero_1).
 * SoundAssetPaths are soft object paths to imported SoundWave/SoundCue assets and can be swapped without code changes.
 */
USTRUCT(BlueprintType)
struct T66_API FT66AudioEventRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TArray<FSoftObjectPath> SoundAssetPaths;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FName VolumeBus = FName(TEXT("SFX"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	bool bPlay2D = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	bool bIsUISound = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	bool bPrimeOnLoad = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.01"))
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0"))
	float PitchRandomRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0"))
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0"))
	float MinReplayIntervalSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FSoftObjectPath AttenuationAssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FSoftObjectPath ConcurrencyAssetPath;
};
