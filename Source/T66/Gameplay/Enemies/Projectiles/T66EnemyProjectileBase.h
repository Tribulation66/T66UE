// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66EnemyProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class AT66HeroBase;
class UT66RunStateSubsystem;

UCLASS(Blueprintable)
class T66_API AT66EnemyProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AT66EnemyProjectileBase();

	void FireInDirection(const FVector& Direction);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> Sphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 HitDamageHearts = 1;

protected:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void HandleHeroHit(AT66HeroBase* Hero, UT66RunStateSubsystem* RunState);
};
