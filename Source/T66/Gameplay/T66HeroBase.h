// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/T66DataTypes.h"
#include "T66HeroBase.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInstancedStaticMeshComponent;
class UT66CombatComponent;
class UT66RunStateSubsystem;
class UWidgetComponent;
class UT66HeroSpeedSubsystem;
class UAnimationAsset;
class AT66PilotableTractor;

/**
 * Base class for all playable heroes in Tribulation 66
 *
 * Visuals System (designed for easy FBX swap):
 * - PlaceholderMesh: Static mesh for prototyping (Cylinder=TypeA, Cube=TypeB)
 * - When ready for production: hide PlaceholderMesh, show SkeletalMesh from DataTable
 * - Color applied via dynamic material instance
 *
 * Camera System:
 * - Third-person camera with mouse look
 * - SpringArm for collision handling
 * - Pawn control rotation for smooth camera following
 */
UCLASS(Blueprintable)
class T66_API AT66HeroBase : public ACharacter
{
	GENERATED_BODY()

public:
	AT66HeroBase();

	/** The hero ID this actor represents (set at spawn time) */
	UPROPERTY(BlueprintReadWrite, Category = "Hero")
	FName HeroID;

	/** The selected body type for this hero */
	UPROPERTY(BlueprintReadWrite, Category = "Hero")
	ET66BodyType BodyType = ET66BodyType::TypeA;

	/** Cached hero data (populated on spawn) */
	UPROPERTY(BlueprintReadOnly, Category = "Hero")
	FHeroData HeroData;

	// ========== Camera Components ==========
	
	/** Spring arm for third-person camera (handles collision) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Third-person follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// ========== Combat ==========

	/** Auto-attack: finds closest enemy in range and applies damage on timer */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UT66CombatComponent> CombatComponent;

	/** Safe-zone overlap count (NPC safe bubbles). */
	void AddSafeZoneOverlap(int32 Delta);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SafeZone")
	bool IsInSafeZone() const { return SafeZoneOverlapCount > 0; }

	/** Force-refresh the attack range ring visibility/size. */
	void RefreshAttackRangeRing();

	void SetVehicleMounted(bool bMounted, AT66PilotableTractor* MountedTractor = nullptr, const FVector& VisualOffset = FVector::ZeroVector, const FRotator& VisualRotation = FRotator::ZeroRotator);
	void SetQuickReviveDowned(bool bDowned);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement|Vehicle")
	bool IsVehicleMounted() const { return bVehicleMounted; }

	AT66PilotableTractor* GetMountedTractor() const { return MountedTractor.Get(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hero|QuickRevive")
	bool IsQuickReviveDowned() const { return bQuickReviveDowned; }

	// ========== Placeholder Visuals (for prototyping) ==========
	
	/** The placeholder static mesh (Cylinder for TypeA, Cube for TypeB) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals|Placeholder")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	/** Visual ring showing the hero's current auto-attack range (toggle with HUD panels). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|UI")
	TObjectPtr<UInstancedStaticMeshComponent> AttackRangeRingISM;

	/** Inner ring at 10%% of attack range (close range damage zone). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|UI")
	TObjectPtr<UInstancedStaticMeshComponent> CloseRangeRingISM;

	/** Inner ring at 90%% of attack range (long range damage zone). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|UI")
	TObjectPtr<UInstancedStaticMeshComponent> LongRangeRingISM;

	/** Auto-attack cooldown bar below the hero's feet. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|UI")
	TObjectPtr<UWidgetComponent> CooldownBarWidgetComponent;

	// ========== Future FBX Support ==========
	// When ready for production models:
	// - Add SkeletalMeshComponent here
	// - DataTable will have soft refs to skeletal meshes per body type
	// - InitializeHero will load and assign the skeletal mesh, hiding placeholder

	/**
	 * Initialize this hero with data from the DataTable
	 * Called after spawning to set up visuals and stats
	 * @param InHeroData The hero's data from the DataTable
	 * @param InBodyType The selected body type (A or B)
	 * @param InSkinID Skin ID (e.g. Default, Beachgoer)
	 * @param bPreviewMode If true, use the idle animation for hero selection preview
	 */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void InitializeHero(const FHeroData& InHeroData, ET66BodyType InBodyType = ET66BodyType::TypeA, FName InSkinID = NAME_None, bool bPreviewMode = false);

	/**
	 * Set the placeholder color (for prototyping)
	 */
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void SetPlaceholderColor(FLinearColor Color);

	/**
	 * Set the body type and update placeholder mesh accordingly
	 * TypeA = Cylinder, TypeB = Cube
	 */
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void SetBodyType(ET66BodyType NewBodyType);

	/**
	 * Check if this hero is in preview mode (not possessed, just for display)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hero")
	bool IsPreviewMode() const { return bIsPreviewMode; }

	/**
	 * Set preview mode (disables camera, input, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void SetPreviewMode(bool bPreview);

	/** Dash forward in the direction the hero is facing. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DashForward();

	/** Stage effect helper: reduce friction so the hero slides for a short time. */
	UFUNCTION(BlueprintCallable, Category = "Movement|StageEffects")
	void ApplyStageSlide(float DurationSeconds);

	/** Begin the sky-drop entrance: hero falls from a high altitude and input is re-enabled on landing. */
	void BeginSkyDrop();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnJumped_Implementation() override;

	UFUNCTION()
	void HandleHeroDerivedStatsChanged();

	UFUNCTION()
	void HandleHUDPanelVisibilityChanged();

private:

	/** Dynamic material instance for color changes */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PlaceholderMaterial;

	/** The base material to use for colored placeholders */
	UPROPERTY()
	TObjectPtr<UMaterial> ColoredBaseMaterial;

	/** Cached current color for reapplication after mesh changes */
	FLinearColor CurrentPlaceholderColor = FLinearColor::White;

	/** Cached mesh references */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;
	
	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** Is this hero in preview mode (for UI display)? */
	bool bIsPreviewMode = false;

	int32 SafeZoneOverlapCount = 0;

	UPROPERTY()
	TObjectPtr<UT66RunStateSubsystem> CachedRunState;

	UPROPERTY(Transient)
	TObjectPtr<UT66HeroSpeedSubsystem> CachedHeroSpeedSubsystem = nullptr;

	/** Cached idle/walk/jump anims for the current hero visual. */
	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedIdleAnim = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedWalkAnim = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedJumpAnim = nullptr;

	/** Last animation state so we only call PlayAnimation on change. */
	enum class EMovementAnimState : uint8 { Idle, Walk, Jump };
	EMovementAnimState LastMovementAnimState = EMovementAnimState::Idle;

	bool bVehicleMounted = false;
	bool bQuickReviveDowned = false;
	bool bVehicleDefaultVisualTransformsCached = false;
	FVector DefaultPlaceholderRelativeLocation = FVector::ZeroVector;
	FRotator DefaultPlaceholderRelativeRotation = FRotator::ZeroRotator;
	FVector DefaultSkeletalMeshRelativeLocation = FVector::ZeroVector;
	FRotator DefaultSkeletalMeshRelativeRotation = FRotator::ZeroRotator;
	TWeakObjectPtr<AT66PilotableTractor> MountedTractor;

	float BaseMaxWalkSpeed = 1800.f;
	FVector QuickReviveDownedVisualOffset = FVector(0.f, 0.f, -58.f);
	FRotator QuickReviveDownedVisualRotation = FRotator(0.f, 0.f, 90.f);

	// Dash tuning
	float LastDashTime = -9999.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dash")
	float DashCooldownSeconds = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dash")
	float DashStrength = 1600.f;

	// Stage slide tuning/state
	float BaseGroundFriction = 8.f;
	float BaseBrakingFrictionFactor = 2.f;
	float BaseBrakingDecelerationWalking = 99999.f;
	float StageSlideSecondsRemaining = 0.f;

	/** Sky-drop entrance: hero falls from altitude; input disabled until landing. */
	bool bIsSkyDropping = false;
	static constexpr float SkyDropAltitude = 5000.f;

	FVector LastSafeGroundLocation = FVector::ZeroVector;
	FRotator LastSafeGroundRotation = FRotator::ZeroRotator;
	bool bHasLastSafeGroundTransform = false;
	float ContinuousFallSeconds = 0.f;
	float LastTerrainRecoveryTime = -9999.f;
	float GroundTraceAccumSeconds = 0.f;
	static constexpr float TerrainRecoveryCooldown = 1.0f;
	static constexpr float TerrainRecoveryFallSeconds = 2.0f;
	static constexpr float TerrainRecoveryMissingGroundDistance = 2500.f;
	static constexpr float GroundTraceIntervalGrounded = 0.075f;
	static constexpr float GroundTraceIntervalFalling = 0.066f;

	/** When cooldown bar shows full (1), keep it at full for this long (seconds). */
	float CooldownDisplayHoldUntil = -1.f;
	static constexpr float CooldownBarHoldFullDuration = 0.01f;

	// Enemy touch damage + bounce (hero-side proximity check when collision is Block)
	float LastEnemyTouchDamageTime = -9999.f;
	float LastEnemyBounceTime = -9999.f;
	float EnemyTouchCheckAccumSeconds = 0.f;
	static constexpr float EnemyTouchDamageCooldown = 0.5f;
	static constexpr float EnemyBounceCooldown = 0.25f;
	static constexpr float EnemyTouchRadius = 90.f;
	static constexpr float EnemyBounceStrength = 420.f;
	static constexpr float EnemyBounceZ = 120.f;
	static constexpr float EnemyTouchCheckInterval = 0.10f;

private:
	/** Load and cache the static mesh assets */
	void CacheMeshAssets();
	void CacheVehicleVisualDefaults();

	void UpdateAttackRangeRing();
};
