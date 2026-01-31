// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66CharacterVisualSubsystem.generated.h"

class UDataTable;
class USkeletalMeshComponent;
class USkeletalMesh;
class UAnimationAsset;
class USceneComponent;
class USkeleton;

USTRUCT()
struct FT66ResolvedCharacterVisual
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> LoopingAnim = nullptr;

	UPROPERTY()
	FT66CharacterVisualRow Row;

	UPROPERTY()
	bool bHasRow = false;
};

/**
 * Centralized character visuals resolver + applier.
 *
 * Goals:
 * - data-driven mapping: ID -> skeletal mesh + optional looping animation + per-character transform
 * - avoid repeated sync loads: load once per ID and cache
 */
UCLASS()
class T66_API UT66CharacterVisualSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Apply visual mapping to a SkeletalMeshComponent. Returns true if a mapping existed and was applied. */
	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
	bool ApplyCharacterVisual(
		FName VisualID,
		USkeletalMeshComponent* TargetMesh,
		USceneComponent* PlaceholderToHide = nullptr,
		bool bEnableSingleNodeAnimation = true);

	/** Preload a visual mapping (loads assets synchronously once and caches). */
	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
	void PreloadCharacterVisual(FName VisualID);

private:
	FT66ResolvedCharacterVisual ResolveVisual(FName VisualID);
	UDataTable* GetVisualsDataTable() const;
	UAnimationAsset* FindFallbackLoopingAnim(USkeleton* Skeleton) const;

	UPROPERTY(Transient)
	mutable TObjectPtr<UDataTable> CachedVisualsDataTable;

	UPROPERTY(Transient)
	mutable TMap<FName, FT66ResolvedCharacterVisual> ResolvedCache;

	/** Cache: Skeleton asset path -> chosen looping animation */
	UPROPERTY(Transient)
	mutable TMap<FName, TObjectPtr<UAnimationAsset>> SkeletonAnimCache;
};

