// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "T66VendorBoss.generated.h"

class UStaticMeshComponent;

/** Vendor Boss spawned when Vendor anger reaches 100%. */
UCLASS(Blueprintable)
class T66_API AT66VendorBoss : public ACharacter
{
	GENERATED_BODY()

public:
	AT66VendorBoss();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	int32 MaxHP = 1000;

	UPROPERTY(BlueprintReadOnly, Category = "Boss")
	int32 CurrentHP = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float MoveSpeed = 520.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float FireIntervalSeconds = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float ProjectileSpeed = 950.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	int32 ProjectileDamageHearts = 1;

	/** Called by hero projectile overlap; returns true if boss died. */
	bool TakeDamageFromHeroHit(int32 DamageAmount);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void FireAtPlayer();
	void Die();

	FTimerHandle FireTimerHandle;
};

