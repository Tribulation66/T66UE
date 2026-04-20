// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/T66BossAttackTypes.h"
#include "T66BossProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/** Boss projectile: visible projectile body with themed Niagara trail/impact. */
UCLASS(Blueprintable)
class T66_API AT66BossProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66BossProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 DamageHearts = 1;

	void ConfigureVisualStyle(ET66BossAttackProfile InAttackProfile, const FLinearColor& InPrimaryColor, const FLinearColor& InSecondaryColor, bool bInUseSecondaryTint = false);
	void SetTargetLocation(const FVector& TargetLoc, float Speed);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void UpdateVisualStyle();
	void SpawnImpactEffect();

	ET66BossAttackProfile AttackProfile = ET66BossAttackProfile::Balanced;
	FLinearColor PrimaryColor = FLinearColor(0.95f, 0.16f, 0.12f, 1.f);
	FLinearColor SecondaryColor = FLinearColor(1.f, 0.72f, 0.18f, 1.f);
	bool bUseSecondaryTint = false;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> TrailComponent = nullptr;

	UNiagaraSystem* CachedTrailSystem = nullptr;
	UNiagaraSystem* CachedImpactSystem = nullptr;
};
