// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Core/T66RunTypes.h"
#include "T66ReleaseVariantSubsystem.generated.h"

UENUM(BlueprintType)
enum class ET66ReleaseVariant : uint8
{
	FullGame UMETA(DisplayName = "Full Game"),
	SteamDemo UMETA(DisplayName = "Steam Demo")
};

UCLASS(Config = DemoMode, DefaultConfig, meta = (DisplayName = "T66 Demo Mode"))
class T66_API UT66DemoModeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Activation")
	bool bForceDemoMode = false;

	UPROPERTY(Config, EditAnywhere, Category = "Activation")
	bool bAllowCommandLineDemoMode = true;

	UPROPERTY(Config, EditAnywhere, Category = "Steam")
	bool bDetectSteamDemoAppId = true;

	UPROPERTY(Config, EditAnywhere, Category = "Steam")
	int32 FullGameSteamAppId = 4464300;

	UPROPERTY(Config, EditAnywhere, Category = "Steam")
	int32 DemoSteamAppId = 4718770;

	UPROPERTY(Config, EditAnywhere, Category = "Content")
	TArray<FName> AllowedHeroIDs;

	UPROPERTY(Config, EditAnywhere, Category = "Content")
	TArray<FName> AllowedCompanionIDs;

	UPROPERTY(Config, EditAnywhere, Category = "Content")
	TArray<FName> AllowedDifficultyIDs;

	UPROPERTY(Config, EditAnywhere, Category = "Content")
	TArray<FName> AllowedArcadeGameIDs;

	UPROPERTY(Config, EditAnywhere, Category = "Content")
	TArray<FName> AllowedCasinoGameIDs;

	UPROPERTY(Config, EditAnywhere, Category = "Content")
	bool bAllowLabRun = false;

	UPROPERTY(Config, EditAnywhere, Category = "Content")
	bool bAllowCollector = false;

	UPROPERTY(Config, EditAnywhere, Category = "Economy", meta = (ClampMin = "0"))
	int32 MaxDiplomaUpgradesPerStat = 1;

	UPROPERTY(Config, EditAnywhere, Category = "Economy")
	bool bAllowDrugPurchases = false;

	UPROPERTY(Config, EditAnywhere, Category = "Presentation")
	FString UnavailableContentText = TEXT("COMING SOON");
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
	bool IsDemoModeActive() const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsHeroAllowed(FName HeroID) const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsCompanionAllowed(FName CompanionID) const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsDifficultyAllowed(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsArcadeGameAllowed(FName ArcadeGameID) const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsCasinoGameAllowed(FName CasinoGameID) const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsRunCategoryAllowed(ET66RunCategory RunCategory) const;

	UFUNCTION(BlueprintCallable, Category = "Release|Demo")
	bool IsCollectorAllowed() const;

	TArray<FName> FilterHeroIDs(const TArray<FName>& HeroIDs) const;
	TArray<FName> FilterCompanionIDs(const TArray<FName>& CompanionIDs) const;
	TArray<ET66Difficulty> GetPlayableDifficulties() const;
	TArray<ET66Difficulty> GetVisibleDifficulties() const;
	ET66Difficulty ResolvePlayableDifficulty(ET66Difficulty Difficulty) const;
	ET66RunCategory ResolvePlayableRunCategory(ET66RunCategory RunCategory) const;
	bool IsDiplomaUpgradeAllowed(int32 CurrentUnlockedSteps) const;
	bool AreDrugPurchasesAllowed() const;
	FText GetUnavailableContentText() const;

private:
	static bool ParseReleaseVariantName(const FString& Value, ET66ReleaseVariant& OutVariant);
	static bool ParseDifficultyName(FName DifficultyID, ET66Difficulty& OutDifficulty);
};
