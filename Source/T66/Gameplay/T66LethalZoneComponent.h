// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "T66LethalZoneComponent.generated.h"

class AT66HeroBase;

/** Reusable lethal-radius trigger for hidden-boss or hazard NPCs. */
UCLASS(ClassGroup = (T66), meta = (BlueprintSpawnableComponent))
class T66_API UT66LethalZoneComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UT66LethalZoneComponent();

	UFUNCTION(BlueprintCallable, Category = "LethalZone")
	void ConfigureLethalZone(float InRadius, FName InDeliveryMethod);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleLethalZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "LethalZone")
	FName DeliveryMethod = FName(TEXT("LethalZone"));
};
