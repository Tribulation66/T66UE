// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66BossLaneBlockerHazard.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

/** Telegraphs, then activates a large primitive hazard for Slime King leg attacks. */
UCLASS()
class T66_API AT66BossLaneBlockerHazard : public AActor
{
	GENERATED_BODY()

public:
	AT66BossLaneBlockerHazard();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> DamageBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> TelegraphMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> ActiveMesh;

	void ConfigureHazard(
		const FLinearColor& InTelegraphColor,
		const FLinearColor& InActiveColor,
		const FVector& InVisualScale,
		const FVector& InDamageExtent,
		float InWarningSeconds,
		float InActiveSeconds,
		int32 InDamageHP);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION()
	void OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ActivateHazard();
	void ApplyDamageToActor(AActor* OtherActor);

	FLinearColor TelegraphColor = FLinearColor(0.26f, 1.0f, 0.08f, 1.f);
	FLinearColor ActiveColor = FLinearColor(0.04f, 0.95f, 0.16f, 1.f);
	FVector VisualScale = FVector(2.4f, 2.4f, 0.55f);
	FVector DamageExtent = FVector(190.f, 190.f, 120.f);
	float WarningSeconds = 0.85f;
	float ActiveSeconds = 1.5f;
	float Age = 0.f;
	int32 DamageHP = 28;
	bool bActivated = false;

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> DamagedActors;
};
