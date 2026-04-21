// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66TDVisualSubsystem.generated.h"

class UTexture2D;

UCLASS()
class T66TD_API UT66TDVisualSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UTexture2D* LoadHeroTexture(FName HeroID, const FString& HeroDisplayName);
	UTexture2D* LoadEnemyTexture(const FString& EnemyVisualID);
	UTexture2D* LoadBossTexture(const FString& EnemyVisualID);

private:
	static FString SanitizeVisualID(const FString& RawValue);
	UTexture2D* LoadCachedLooseTexture(const FString& CacheKey, const FString& RelativePath);

	UPROPERTY()
	TMap<FString, TObjectPtr<UTexture2D>> CachedTextures;
};
