// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/Actor.h"
#include "T66SummonActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UT66CombatComponent;

UCLASS()
class T66_API AT66SummonActor : public AActor
{
	GENERATED_BODY()

public:
	AT66SummonActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeSummon(
		UT66CombatComponent* InOwnerCombat,
		FName InIdolID,
		ET66IdolElement InElement,
		ET66ItemRarity InRarity,
		int32 InDamage,
		float InStatusChance,
		float InStatusDuration,
		float InMoveSpeed,
		float InContactRadius,
		float InLifetimeSeconds,
		int32 InMaxHits,
		float InBounceSpeed,
		float InSearchRange,
		AActor* InInitialTarget);

	FName GetIdolID() const { return IdolID; }
	void ApplyVisualMeshOverride(UStaticMesh* InMesh, float InMeshScale);

private:
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<USphereComponent> ContactSphere;

	UPROPERTY(VisibleAnywhere, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(Transient)
	TObjectPtr<UT66CombatComponent> OwnerCombat;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentTarget;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> LastHitTarget;

	FName IdolID = NAME_None;
	ET66IdolElement Element = ET66IdolElement::Fire;
	ET66ItemRarity Rarity = ET66ItemRarity::Black;
	int32 Damage = 1;
	float StatusChance = 0.f;
	float StatusDuration = 0.f;
	float MoveSpeed = 950.f;
	float ContactRadius = 70.f;
	float LifetimeSeconds = 6.f;
	int32 MaxHits = 3;
	float BounceSpeed = 620.f;
	float SearchRange = 1000.f;
	float AgeSeconds = 0.f;
	float BounceSecondsRemaining = 0.f;
	float LastHitCooldownSeconds = 0.f;
	int32 HitCount = 0;
	FVector BounceVelocity = FVector::ZeroVector;

	UFUNCTION()
	void OnContactBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void RefreshTarget();
	void BounceAwayFrom(AActor* HitActor);
};
