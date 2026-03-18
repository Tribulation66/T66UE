// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Data/T66DataTypes.h"
#include "T66CompanionBase.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UAnimationAsset;
class UT66HeroSpeedSubsystem;
class UT66AchievementsSubsystem;
class UT66RunStateSubsystem;
class APawn;

/**
 * Base class for companions. Uses a sphere mesh (placeholder).
 * In gameplay, companions follow the hero; in UI, used for 3D preview.
 */
UCLASS(Blueprintable)
class T66_API AT66CompanionBase : public APawn
{
	GENERATED_BODY()

public:
	AT66CompanionBase();

	UPROPERTY(BlueprintReadWrite, Category = "Companion")
	FName CompanionID;

	UPROPERTY(BlueprintReadOnly, Category = "Companion")
	FCompanionData CompanionData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	/** Imported skeletal mesh visual (optional; driven by DT_CharacterVisuals). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	/** Initialize with data and optional skin (Default if None). VisualID = GetCompanionVisualID(CompanionID, SkinID). */
	UFUNCTION(BlueprintCallable, Category = "Companion")
	void InitializeCompanion(const FCompanionData& InData, FName SkinID = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void SetPlaceholderColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Companion")
	bool IsPreviewMode() const { return bIsPreviewMode; }

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void SetPreviewMode(bool bPreview);

	/** UI locking: show black silhouette (hide skeletal mesh, show placeholder). */
	UFUNCTION(BlueprintCallable, Category = "Companion")
	void SetLockedVisual(bool bLocked);

	/** Offset from hero when following (e.g. behind and to the side) */
	UPROPERTY(EditDefaultsOnly, Category = "Follow")
	FVector FollowOffset = FVector(-120.f, 80.f, 0.f);

	/** How fast the companion moves toward the follow target */
	UPROPERTY(EditDefaultsOnly, Category = "Follow")
	float FollowSpeed = 8.f;

	/** Smooth companion height changes against traced ground instead of snapping. */
	UPROPERTY(EditDefaultsOnly, Category = "Follow")
	float GroundFollowSpeed = 12.f;

	/** Uniform actor scale for imported companion visuals. */
	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	float CompanionActorScale = 1.5f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PlaceholderMaterial;

	bool bIsPreviewMode = false;

	UPROPERTY(Transient)
	bool bUsingCharacterVisual = false;

	UPROPERTY(Transient)
	bool bLockedVisual = false;

	/** Cached alert/walk/run anims; companion uses hero speed subsystem (same rules: idle=alert, walk, run). */
	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedAlertAnim = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedWalkAnim = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedRunAnim = nullptr;

	/** Last animation state so we only call PlayAnimation on change. */
	uint8 LastMovementAnimState = 0; // 0=Idle, 1=Walk, 2=Run

	/** Run ground LineTrace only every Nth tick to reduce per-frame cost. */
	int32 GroundTraceTickCounter = 0;
	static constexpr int32 GroundTraceEveryNTicks = 3;

	UPROPERTY(Transient)
	TObjectPtr<UT66HeroSpeedSubsystem> CachedHeroSpeedSubsystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66AchievementsSubsystem> CachedAchievementsSubsystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66RunStateSubsystem> CachedRunStateSubsystem = nullptr;

	TWeakObjectPtr<APawn> CachedHeroPawn;
	int32 CachedUnionStagesCleared = 0;
	bool bHasCachedGroundZ = false;
	float CachedGroundZ = 0.f;

	// Healing (gameplay only)
	float HealAccumSeconds = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Healing")
	float HealIntervalSeconds = 10.0f; // Basic Union tier

	UPROPERTY(EditDefaultsOnly, Category = "Healing")
	int32 HealAmountHearts = 1;

	UFUNCTION()
	void HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs);

	void ApplyCompanionScale();
};
