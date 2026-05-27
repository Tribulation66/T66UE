// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66EnemyProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class AT66HeroBase;
class UT66RunStateSubsystem;

UCLASS(Blueprintable)
class T66_API AT66EnemyProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AT66EnemyProjectileBase();

	static int32 GetActiveEnemyProjectileCount();

	void FireInDirection(const FVector& Direction);
	void SetVisualOnly(bool bInVisualOnly);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> Sphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> AccentMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 HitDamageHearts = 1;

	void ConfigureTemporaryProjectileVisual(FName ProfileID, const FLinearColor& CoreColor, FName AccentProfileID = NAME_None, const FLinearColor& AccentColor = FLinearColor::Transparent);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void HandleHeroHit(AT66HeroBase* Hero, UT66RunStateSubsystem* RunState);

	bool bVisualOnly = false;
	bool bCountedAsActiveProjectile = false;
};
