// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/T66BossAttackTypes.h"
#include "T66BossGroundAOE.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraSystem;

/**
 * Ground AOE attack spawned by a boss.
 * Phase 1 (warning): visible telegraph disc.
 * Phase 2 (activation): damage check + themed impact burst for PillarLingerSeconds.
 * Then self-destructs.
 */
UCLASS(Blueprintable)
class T66_API AT66BossGroundAOE : public AActor
{
	GENERATED_BODY()

public:
	AT66BossGroundAOE();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float Radius = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float WarningDurationSeconds = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOE")
	float PillarLingerSeconds = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	int32 DamageHP = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	bool bDamageEnemies = false;

	void ConfigureVisualStyle(ET66BossAttackProfile InAttackProfile, const FLinearColor& InTelegraphColor, const FLinearColor& InImpactColor);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageZone;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WarningMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ImpactMesh;

	void ActivateDamage();
	void DestroySelf();
	void RefreshVisualState(float WarningAlpha);

	FTimerHandle ActivateTimerHandle;
	FTimerHandle DestroyTimerHandle;
	bool bDamageActivated = false;

	float WarningElapsed = 0.f;
	ET66BossAttackProfile AttackProfile = ET66BossAttackProfile::Balanced;
	FLinearColor TelegraphColor = FLinearColor(1.f, 0.15f, 0.05f, 1.f);
	FLinearColor ImpactColor = FLinearColor(1.f, 0.72f, 0.18f, 1.f);
	UNiagaraSystem* CachedImpactVFX = nullptr;
};
