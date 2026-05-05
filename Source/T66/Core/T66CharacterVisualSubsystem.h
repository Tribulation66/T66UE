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
struct FStreamableHandle;

USTRUCT()
struct FT66ResolvedCharacterVisual
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> LoopingAnim = nullptr;

	/** Optional alert/stand animation (preview). */
	UPROPERTY()
	TObjectPtr<UAnimationAsset> AlertAnim = nullptr;

	/** Optional run animation (gameplay; when moving fast). */
	UPROPERTY()
	TObjectPtr<UAnimationAsset> RunAnim = nullptr;

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
		bool bEnableSingleNodeAnimation = true,
		bool bUseAlertAnimation = false,
		bool bIsPreviewContext = false);

	/** Compute the legacy hero visual row ID from HeroID + body style + SkinID (for example Chad -> Hero_1_Chad). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Visuals")
	static FName GetHeroVisualID(FName HeroID, ET66BodyType BodyType, FName SkinID);

	/** Compute companion VisualID from CompanionID + SkinID (e.g. Companion_01 + Beachgoer -> Companion_01_Beachgoer). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Visuals")
	static FName GetCompanionVisualID(FName CompanionID, FName SkinID);

	/** Resolve the canonical fallback visual row ID (for example Hero_1_Chad_Skin -> Hero_1_Chad). */
	static FName GetFallbackVisualID(FName VisualID);

	/** Append all known preload candidates for a visual row, including animation fallback variants. */
	static void AppendCharacterVisualPreloadPaths(const FT66CharacterVisualRow& Row, TArray<FSoftObjectPath>& OutPaths);

	/** Preload a visual mapping asynchronously and cache it once ready. */
	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
	void PreloadCharacterVisual(FName VisualID);

	/** Returns true when a visual no longer has pending preload work for the given ID. */
	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
	bool IsCharacterVisualReady(FName VisualID) const;

	/** Get alert, walk, and run animations for a visual (for runtime animation state). OutRun/OutAlert may be null. */
	UFUNCTION(BlueprintCallable, Category = "T66|Visuals")
	void GetMovementAnimsForVisual(FName VisualID, UAnimationAsset*& OutWalk, UAnimationAsset*& OutRun, UAnimationAsset*& OutAlert);

private:
	FT66ResolvedCharacterVisual ResolveVisual(FName VisualID);
	UDataTable* GetVisualsDataTable() const;
	UAnimationAsset* FindFallbackLoopingAnim(USkeleton* Skeleton) const;
	const FT66CharacterVisualRow* FindVisualRow(FName VisualID, FName* OutResolvedVisualID = nullptr) const;
	void HandleCharacterVisualPreloadCompleted(FName VisualID);

	UPROPERTY(Transient)
	mutable TObjectPtr<UDataTable> CachedVisualsDataTable;

	UPROPERTY(Transient)
	mutable TMap<FName, FT66ResolvedCharacterVisual> ResolvedCache;

	TMap<FName, TSharedPtr<FStreamableHandle>> PendingPreloadHandles;

	/** Cache: Skeleton asset path -> chosen looping animation */
	UPROPERTY(Transient)
	mutable TMap<FName, TObjectPtr<UAnimationAsset>> SkeletonAnimCache;
};

