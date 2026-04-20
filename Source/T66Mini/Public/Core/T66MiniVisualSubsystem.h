// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniVisualSubsystem.generated.h"

class UTexture2D;

UCLASS()
class T66MINI_API UT66MiniVisualSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UTexture2D* GetWhiteTexture();
	UTexture2D* LoadLooseTexture(const FString& RelativePath);
	UTexture2D* LoadTextureByAssetPath(const FString& AssetPath);
	UTexture2D* LoadBackgroundTexture();
	UTexture2D* LoadHeroTexture(const FString& HeroDisplayName);
	UTexture2D* LoadHeroAnimationTexture(const FString& HeroDisplayName, const FString& FrameKey);
	UTexture2D* LoadHeroProjectileTexture(const FString& HeroDisplayName);
	UTexture2D* LoadCompanionTexture(const FString& CompanionVisualID);
	UTexture2D* LoadCompanionAnimationTexture(const FString& CompanionVisualID, const FString& FrameKey);
	UTexture2D* LoadIdolEffectTexture(FName IdolID);
	UTexture2D* LoadEnemyTexture(const FString& EnemyVisualID);
	UTexture2D* LoadBossTexture(const FString& EnemyVisualID);
	UTexture2D* LoadInteractableTexture(const FString& InteractableVisualID);
	UTexture2D* LoadEffectTexture(const FString& EffectName);
	UTexture2D* LoadHudTexture(const FString& TextureName);
	UTexture2D* LoadItemTexture(FName ItemID, const FString& IconPath = FString());

private:
	static FString BuildAbsoluteProjectPath(const FString& RelativePath);
	static FString GetPackagePathFromObjectPath(const FString& AssetPath);
	UTexture2D* LoadImportedTexture(const FString& AssetPath);
	UTexture2D* LoadImportedOrLooseTexture(const FString& CacheKey, const FString& AssetPath, const FString& RelativePath);
	UTexture2D* LoadCachedTexture(const FString& CacheKey, const FString& RelativePath);
	UTexture2D* LoadCachedTextureFromCandidates(const FString& CacheKey, const TArray<FString>& RelativePaths);

	UPROPERTY()
	TMap<FString, TObjectPtr<UTexture2D>> CachedTextures;

	UPROPERTY()
	TObjectPtr<UTexture2D> CachedWhiteTexture;

	TSet<FString> MissingImportedPackages;
};
