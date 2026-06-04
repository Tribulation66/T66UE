// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "T66SafeZoneComponent.generated.h"

class AT66HeroBase;
class UStaticMeshComponent;

/** Reusable safe-zone trigger for non-NPC interactables that should protect the local hero. */
UCLASS(ClassGroup = (T66), meta = (BlueprintSpawnableComponent))
class T66_API UT66SafeZoneComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UT66SafeZoneComponent();

	UFUNCTION(BlueprintCallable, Category = "SafeZone")
	void ConfigureSafeZone(float InRadius);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SafeZone")
	float GetSafeZoneRadius() const { return GetScaledSphereRadius(); }

#if !UE_BUILD_SHIPPING
	bool HasSafeZoneVisualForAutomation() const;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleSafeZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleSafeZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	AT66HeroBase* ResolveHero(AActor* OtherActor) const;
	void EnsureSafeZoneVisual();
	void UpdateSafeZoneVisual();

	UPROPERTY(Transient)
	TWeakObjectPtr<AT66HeroBase> OverlappingHero;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> SafeZoneVisualComponent = nullptr;
};
