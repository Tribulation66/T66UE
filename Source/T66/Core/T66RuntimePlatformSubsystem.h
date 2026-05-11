// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66RuntimePlatformSubsystem.generated.h"

UENUM(BlueprintType)
enum class ET66RuntimePlatformProfile : uint8
{
	Desktop UMETA(DisplayName = "Desktop"),
	SteamDeck UMETA(DisplayName = "Steam Deck")
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "T66 Runtime Platform"))
class T66_API UT66RuntimePlatformSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Steam Deck")
	bool bDetectSteamDeck = true;

	UPROPERTY(Config, EditAnywhere, Category = "Steam Deck")
	bool bAllowMediaViewerOnSteamDeck = false;

	UPROPERTY(Config, EditAnywhere, Category = "Steam Deck", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float SteamDeckDefaultUIScale = 1.1f;

	UPROPERTY(Config, EditAnywhere, Category = "Steam Deck", meta = (ClampMin = "0", ClampMax = "3"))
	int32 SteamDeckDefaultScalabilityLevel = 1;

	UPROPERTY(Config, EditAnywhere, Category = "Steam Deck", meta = (ClampMin = "0.0", ClampMax = "240.0"))
	float SteamDeckDefaultFrameRateLimit = 60.0f;
};

UCLASS()
class T66_API UT66RuntimePlatformSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Runtime Platform")
	ET66RuntimePlatformProfile GetEffectiveRuntimePlatformProfile() const;

	UFUNCTION(BlueprintCallable, Category = "Runtime Platform")
	bool IsSteamDeckLikeDevice() const;

	UFUNCTION(BlueprintCallable, Category = "Runtime Platform")
	bool ShouldShowMediaViewer() const;

	UFUNCTION(BlueprintCallable, Category = "Runtime Platform|Steam Deck")
	float GetDefaultUIScale() const;

	UFUNCTION(BlueprintCallable, Category = "Runtime Platform|Steam Deck")
	int32 GetDefaultScalabilityLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Runtime Platform|Steam Deck")
	float GetDefaultFrameRateLimit() const;

private:
	static bool ParseRuntimePlatformName(const FString& Value, ET66RuntimePlatformProfile& OutProfile);
	static bool IsRunningOnSteamDeckViaSteamworks();
};
