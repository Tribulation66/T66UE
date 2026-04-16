// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TrapArrowProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class UNiagaraSystem;
struct FHitResult;

UCLASS(Blueprintable)
class T66_API AT66TrapArrowProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66TrapArrowProjectile();

	void InitializeProjectile(
		const FVector& Direction,
		int32 InDamageHP,
		float InProjectileSpeed,
		const FLinearColor& InProjectileTint,
		const FLinearColor& InTrailColor);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void UpdateVisuals();

	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = "Trap")
	int32 DamageHP = 10;

	UPROPERTY(EditAnywhere, Category = "Trap")
	FLinearColor ProjectileTint = FLinearColor(0.95f, 0.78f, 0.20f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Trap")
	FLinearColor TrailColor = FLinearColor(1.f, 0.78f, 0.25f, 0.95f);

	float ProjectileSpeed = 2400.f;
	float VFXAccum = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedPixelVFX = nullptr;
};
