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
class UT66CombatComponent;

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

	// ========== Placeholder Visuals (for prototyping) ==========
	
	/** The placeholder static mesh (Cylinder for TypeA, Cube for TypeB) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals|Placeholder")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

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
	 */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void InitializeHero(const FHeroData& InHeroData, ET66BodyType InBodyType = ET66BodyType::TypeA);

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

protected:
	virtual void BeginPlay() override;

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

private:
	/** Load and cache the static mesh assets */
	void CacheMeshAssets();
};
