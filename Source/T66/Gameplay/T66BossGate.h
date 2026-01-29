// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66BossGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/** Walk-through pillars that awaken the stage boss. */
UCLASS(Blueprintable)
class T66_API AT66BossGate : public AActor
{
	GENERATED_BODY()

public:
	AT66BossGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> PoleLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> PoleRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossGate")
	float TriggerDistance2D = 90.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bTriggered = false;
	void TryTriggerForActor(AActor* OtherActor);
};

