// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66StageEffectTile.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * Colored ground tile that applies a helpful movement effect when stepped on.
 * Spawned procedurally per stage.
 */
UCLASS(Blueprintable)
class T66_API AT66StageEffectTile : public AActor
{
	GENERATED_BODY()

public:
	AT66StageEffectTile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StageEffect")
	ET66StageEffectType EffectType = ET66StageEffectType::Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StageEffect")
	FLinearColor EffectColor = FLinearColor(0.20f, 0.90f, 0.35f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StageEffect")
	float Strength = 1.f;

	void ApplyVisuals();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	float LastTriggerTime = -9999.f;
	TWeakObjectPtr<AActor> LastTriggerActor;
};

