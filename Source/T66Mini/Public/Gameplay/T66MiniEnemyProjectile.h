// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniEnemyProjectile.generated.h"

class UBillboardComponent;
class USceneComponent;
class USphereComponent;
class UTexture2D;

UCLASS()
class T66MINI_API AT66MiniEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniEnemyProjectile();

	virtual void Tick(float DeltaSeconds) override;
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
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> SpriteComponent;

	FVector MoveDirection = FVector::ForwardVector;
	float Speed = 900.f;
	float Damage = 8.f;
	float LifetimeRemaining = 4.f;

	UFUNCTION()
	void OnRep_ProjectileTexture();

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileTexture)
	TObjectPtr<UTexture2D> ProjectileTexture;
};
