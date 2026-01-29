// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "T66EnemyBase.generated.h"

class UWidgetComponent;
class UStaticMeshComponent;
class AT66EnemyDirector;
class AT66ItemPickup;

UCLASS(Blueprintable)
class T66_API AT66EnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AT66EnemyBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 MaxHP = 50;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 CurrentHP = 50;

	/** Visible mesh (cylinder) so enemy is seen */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	/** Health bar widget above head */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	/** Director that spawned this enemy (for death notification) */
	UPROPERTY(BlueprintReadWrite, Category = "AI")
	TObjectPtr<AT66EnemyDirector> OwningDirector;

	/** Apply damage from hero. Returns true if enemy died. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TakeDamageFromHero(int32 Damage);

	/** Refresh health bar display (call when HP changes) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Called when HP reaches 0: notify director, spawn pickup, destroy */
	void OnDeath();

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Touch damage: last time we dealt damage to player (cooldown) */
	float LastTouchDamageTime = -9999.f;
	static constexpr float TouchDamageCooldown = 0.5f;
};
