// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniProjectile.generated.h"

class UBillboardComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UTexture2D;

UENUM()
enum class ET66MiniProjectileBehavior : uint8
{
	BasicAttack,
	Pierce,
	Bounce,
	AOE,
	DOT
};

UCLASS()
class T66MINI_API AT66MiniProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeProjectile(
		AActor* InOwnerActor,
		const FVector& InSpawnLocation,
		const FVector& InDirection,
		ET66MiniProjectileBehavior InBehavior,
		float InPrimaryDamage,
		float InFollowUpDamage,
		float InSpeed,
		float InRadius,
		int32 InRemainingHits,
		int32 InRemainingBounces,
		float InDotTickDamage,
		float InDotTickInterval,
		float InDotDuration,
		UTexture2D* InPrimaryTexture,
		UTexture2D* InFollowUpTexture,
		AActor* InInitialTarget,
		FName InIdolID,
		float InStunDuration);

protected:
	virtual void BeginPlay() override;
	virtual void LifeSpanExpired() override;

private:
	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	class AT66MiniEnemyBase* FindNextBounceTarget(AActor* IgnoreActor, float MaxRange) const;
	void ExplodeAt(const FVector& Location);
	void SpawnFollowUpPulse(const FVector& Location) const;

	UFUNCTION()
	void OnRep_ProjectileVisualState();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	TWeakObjectPtr<AActor> OwnerActor;
	TWeakObjectPtr<AActor> HomingTarget;
	TSet<TWeakObjectPtr<AActor>> HitActors;
	UPROPERTY(Replicated)
	ET66MiniProjectileBehavior Behavior = ET66MiniProjectileBehavior::Pierce;

	UPROPERTY(Replicated)
	FName IdolID = NAME_None;
	float PrimaryDamage = 0.f;
	float FollowUpDamage = 0.f;
	float Speed = 2200.f;
	float Radius = 220.f;
	int32 RemainingHits = 1;
	int32 RemainingBounces = 0;
	float DotTickDamage = 0.f;
	float DotTickInterval = 0.5f;
	float DotDuration = 3.f;
	float StunDuration = 0.f;
	bool bPrimaryHitResolved = false;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisualState)
	TObjectPtr<UTexture2D> PrimaryTexture;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileVisualState)
	TObjectPtr<UTexture2D> FollowUpTexture;
};
