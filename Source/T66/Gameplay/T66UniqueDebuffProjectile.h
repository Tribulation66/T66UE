// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66UniqueDebuffProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

/** Fast straight projectile fired by a Unique enemy. Disappears on hit and applies a hero status effect. */
UCLASS(Blueprintable)
class T66_API AT66UniqueDebuffProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66UniqueDebuffProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debuff")
	ET66HeroStatusEffectType EffectType = ET66HeroStatusEffectType::Burn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debuff")
	float EffectDurationSeconds = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debuff")
	int32 HitDamageHearts = 1;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

