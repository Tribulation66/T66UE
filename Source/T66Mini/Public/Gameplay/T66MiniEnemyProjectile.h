// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniEnemyProjectile.generated.h"

class UBillboardComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UTexture2D;

UCLASS()
class T66MINI_API AT66MiniEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniEnemyProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeProjectile(const FVector& SpawnLocation, const FVector& Direction, float InSpeed, float InDamage, UTexture2D* InTexture = nullptr, float InLifetime = 4.0f);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> SpriteComponent;

	float Damage = 8.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UFUNCTION()
	void OnRep_ProjectileTexture();

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileTexture)
	TObjectPtr<UTexture2D> ProjectileTexture;
};
