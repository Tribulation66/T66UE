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
class UPointLightComponent;
class UT66CombatComponent;
class UT66RunStateSubsystem;
class UWidgetComponent;
class UT66HeroSpeedSubsystem;
class UAnimationAsset;

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

	/** Character fill light: soft omnidirectional light attached to the character
	 *  so it's always visible regardless of scene lighting (no Lumen dependency). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	TObjectPtr<UPointLightComponent> CharacterFillLight;

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
	 * @param bPreviewMode If true, use alert animation (hero selection preview); otherwise use walk
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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleHeroDerivedStatsChanged();

	UFUNCTION()
	void HandleHUDPanelVisibilityChanged();

	/** Touch damage: when enemy overlaps capsule, apply 1 heart damage (RunState i-frames apply) */
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

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

	/** Cached alert/walk/run anims (gameplay: alert when stopped, walk when moving slow, run when above threshold). */
	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedAlertAnim = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedWalkAnim = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CachedRunAnim = nullptr;

	/** Last animation state so we only call PlayAnimation on change. */
	enum class EMovementAnimState : uint8 { Idle, Walk, Run };
	EMovementAnimState LastMovementAnimState = EMovementAnimState::Idle;

	float BaseMaxWalkSpeed = 1200.f;

	// Dash tuning
	float LastDashTime = -9999.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dash")
	float DashCooldownSeconds = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dash")
	float DashStrength = 1600.f;

	// Stage slide tuning/state
	float BaseGroundFriction = 8.f;
	float BaseBrakingFrictionFactor = 2.f;
	float BaseBrakingDecelerationWalking = 2048.f;
	float StageSlideSecondsRemaining = 0.f;

	/** When cooldown bar shows full (1), keep it at full for this long (seconds). */
	float CooldownDisplayHoldUntil = -1.f;
	static constexpr float CooldownBarHoldFullDuration = 0.01f;

	// Enemy touch damage + bounce (hero-side proximity check when collision is Block)
	float LastEnemyTouchDamageTime = -9999.f;
	float LastEnemyBounceTime = -9999.f;
	static constexpr float EnemyTouchDamageCooldown = 0.5f;
	static constexpr float EnemyBounceCooldown = 0.25f;
	static constexpr float EnemyTouchRadius = 90.f;
	static constexpr float EnemyBounceStrength = 420.f;
	static constexpr float EnemyBounceZ = 120.f;

private:
	/** Load and cache the static mesh assets */
	void CacheMeshAssets();

	void UpdateAttackRangeRing();
};
