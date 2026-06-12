// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HeroProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UStaticMesh;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(Blueprintable)
class T66_API AT66HeroProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> AccentMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 Damage = 20;

	/** Source ID for damage log (e.g. AutoAttack). Set by spawner before applying damage. */
	FName DamageSourceID = NAME_None;

	void SetTargetLocation(const FVector& TargetLoc);
	void SetTargetActor(AActor* InTargetActor);
	void SetScaleMultiplier(float InScaleMultiplier);
	void SetTintColor(const FLinearColor& InColor);
	void SetProjectileMesh(UStaticMesh* InMesh);
	/**
	 * BLACK-tier imported-mesh pass (t66.Combat.ProjectileMeshes): replace the temporary
	 * profile visual with an authored projectile mesh. Clears every profile-applied
	 * override (override materials, relative transform) so the mesh renders with its
	 * slot-default material instance and authored +X-forward orientation at authored
	 * size, and hides the AccentMesh overlay if one was active. Visual-only — collision
	 * and ProjectileMovement are untouched.
	 */
	void ApplyCustomVisualMeshOverride(UStaticMesh* InMesh);
	void SetProjectileSpeed(float InSpeed);
	void SetTrailVFX(UNiagaraSystem* InTrailSystem, const FLinearColor& InTrailColor);
	void SetVisualOnly(bool bInVisualOnly);
	void SetVisualArrivalCallback(TFunction<void()>&& InCallback);
	/**
	 * Drive an already-spawned authored Niagara carrier along this projectile's path:
	 * the component keeps its own spawn rotation/scale/playback and is moved to follow
	 * the projectile each tick, so the readable horizontal slash travels from the
	 * segment start to the impact point. The temporary cube/profile meshes are hidden.
	 * Visual-only: no damage/collision authority is moved into the carrier.
	 */
	void SetDrivenCarrierComponent(UNiagaraComponent* InComponent);
	/**
	 * Drive a visual-only Bounce link by deterministic game-time interpolation from
	 * StartLoc to EndLoc over DurationSeconds, instead of ProjectileMovement speed.
	 * Speed-based movement let a single large frame delta (a capture hitch) overshoot
	 * the whole segment in one tick, so the carrier never occupied the hero->target
	 * path and the authored age-revealed slash only became visible at the impact point.
	 * Time-based lerp guarantees intermediate positions across frames so the slash
	 * reads as travelling. Arrival fires the visual callback when alpha reaches 1.
	 */
	void SetTimedVisualTravel(const FVector& StartLoc, const FVector& EndLoc, float DurationSeconds);
	void ConfigureTemporaryProjectileVisual(
		FName ProfileID,
		const FLinearColor& CoreColor,
		float CoreScaleMultiplier,
		FName OverlayProfileID = NAME_None,
		const FLinearColor& OverlayColor = FLinearColor::Transparent,
		float OverlayScaleMultiplier = 1.f);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	float ScaleMultiplier = 1.f;
	float BaseVisualScale = 0.15f;

private:
	/** Intended target for this shot. If set, we only apply damage to this target. */
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> VisualMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> TrailVFXComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> DrivenCarrierComponent = nullptr;

	FLinearColor TintColor = FLinearColor(0.08f, 0.52f, 1.f, 1.f);
	FVector TargetLocation = FVector::ZeroVector;
	TFunction<void()> VisualArrivalCallback;
	bool bHasTargetLocation = false;
	bool bVisualOnly = false;

	// Deterministic visual-only travel (Bounce link carrier). Independent of frame delta.
	FVector VisualTravelStart = FVector::ZeroVector;
	float VisualTravelDuration = 0.f;
	float VisualTravelElapsed = 0.f;
	bool bTimedVisualTravel = false;

	bool IsTargetAlive() const;
	bool CanDamageTargetOnTowerFloor(AActor* Target) const;
	void ApplyDamageToTarget(AActor* Target);
};
