// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66ReleaseVariantSubsystem.generated.h"

UENUM(BlueprintType)
enum class ET66ReleaseVariant : uint8
{
	FullGame UMETA(DisplayName = "Full Game"),
	SteamDemo UMETA(DisplayName = "Steam Demo")
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "T66 Release Variant"))
class T66_API UT66ReleaseVariantSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Release")
	ET66ReleaseVariant ReleaseVariant = ET66ReleaseVariant::FullGame;

	UPROPERTY(Config, EditAnywhere, Category = "Steam")
	bool bDetectSteamDemoAppId = true;

	UPROPERTY(Config, EditAnywhere, Category = "Steam")
	int32 FullGameSteamAppId = 4464300;

	UPROPERTY(Config, EditAnywhere, Category = "Steam")
	int32 DemoSteamAppId = 4718770;

	UPROPERTY(Config, EditAnywhere, Category = "Demo")
	TArray<FName> DemoAllowedHeroIDs;

	UPROPERTY(Config, EditAnywhere, Category = "Demo")
	TArray<FName> DemoAllowedDifficultyIDs;
};

UCLASS()
class T66_API UT66ReleaseVariantSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Release")
	ET66ReleaseVariant GetEffectiveReleaseVariant() const;

	UFUNCTION(BlueprintCallable, Category = "Release")
	bool IsSteamDemoBuild() const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsHeroAllowed(FName HeroID) const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsDifficultyAllowed(ET66Difficulty Difficulty) const;

	TArray<FName> FilterHeroIDs(const TArray<FName>& HeroIDs) const;
	TArray<ET66Difficulty> GetPlayableDifficulties() const;
	ET66Difficulty ResolvePlayableDifficulty(ET66Difficulty Difficulty) const;

private:
	static bool ParseReleaseVariantName(const FString& Value, ET66ReleaseVariant& OutVariant);
	static bool ParseDifficultyName(FName DifficultyID, ET66Difficulty& OutDifficulty);
};
