// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66StartGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * START GATE: Two poles next to each other. Walk through (get really close / between them)
 * to start the stage timer. No interaction (F) required.
 */
UCLASS(Blueprintable)
class T66_API AT66StartGate : public AActor
{
	GENERATED_BODY()

public:
	AT66StartGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> PoleLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> PoleRight;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool bTriggered = false;

	/** Extra safety: if overlaps fail, trigger when hero gets very close. */
	UPROPERTY(EditAnywhere, Category = "StartGate")
	float TriggerDistance2D = 90.f;

private:
	void TryTriggerForActor(AActor* OtherActor);
};
