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
	int32 MaxHP = 100;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 CurrentHP = 100;

	/** Touch damage to player in hearts (scaled by difficulty). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 TouchDamageHearts = 1;

	/** Point value for wave budget and Bounty score (Bible 2.9) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 PointValue = 10;

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
	virtual bool TakeDamageFromHero(int32 Damage);

	/** If true, this enemy prefers to flee from the hero (used by Leprechaun). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	bool bRunAwayFromPlayer = false;

	/** Refresh health bar display (call when HP changes) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar();

	/** Show/hide the lock indicator on this enemy's health bar. */
	void SetLockedIndicator(bool bLocked);

	/** Apply difficulty tier (Tier 0 = base; Tier 1 doubles, etc). */
	void ApplyDifficultyTier(int32 Tier);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Called when HP reaches 0: notify director, spawn pickup, destroy */
	virtual void OnDeath();

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Touch damage: last time we dealt damage to player (cooldown) */
	float LastTouchDamageTime = -9999.f;
	static constexpr float TouchDamageCooldown = 0.5f;

private:
	// Safety/perf: avoid per-enemy per-frame scans for safe zones.
	float SafeZoneCheckAccumSeconds = 0.f;
	float SafeZoneCheckIntervalSeconds = 0.25f;
	bool bCachedInsideSafeZone = false;
	FVector CachedSafeZoneEscapeDir = FVector::ZeroVector;

	bool bBaseTuningInitialized = false;
	int32 BaseMaxHP = 0;
	int32 BaseTouchDamageHearts = 0;
};
