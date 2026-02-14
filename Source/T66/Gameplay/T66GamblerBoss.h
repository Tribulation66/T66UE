// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "T66GamblerBoss.generated.h"

class UStaticMeshComponent;

/** Gambler Boss spawned when Anger reaches 100%. */
UCLASS(Blueprintable)
class T66_API AT66GamblerBoss : public ACharacter
{
	GENERATED_BODY()

public:
	AT66GamblerBoss();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	int32 MaxHP = 1000;

	UPROPERTY(BlueprintReadOnly, Category = "Boss")
	int32 CurrentHP = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float MoveSpeed = 520.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float FireIntervalSeconds = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	float ProjectileSpeed = 1100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	int32 ProjectileDamageHearts = 1;

	/** Called by hero projectile overlap; returns true if boss died. DamageSourceID used for run damage log. */
	bool TakeDamageFromHeroHit(int32 DamageAmount, FName DamageSourceID = NAME_None, FName EventType = NAME_None);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void FireAtPlayer();
	void Die();

	FTimerHandle FireTimerHandle;
};

