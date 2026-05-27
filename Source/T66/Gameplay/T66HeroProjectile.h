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
	void SetProjectileSpeed(float InSpeed);
	void SetTrailVFX(UNiagaraSystem* InTrailSystem, const FLinearColor& InTrailColor);
	void SetVisualOnly(bool bInVisualOnly);
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

	FLinearColor TintColor = FLinearColor(0.08f, 0.52f, 1.f, 1.f);
	FVector TargetLocation = FVector::ZeroVector;
	bool bHasTargetLocation = false;
	bool bVisualOnly = false;

	bool IsTargetAlive() const;
	void ApplyDamageToTarget(AActor* Target);
};
