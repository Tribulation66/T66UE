// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ArcadeInteractableBase.h"
#include "T66ArcadeMachineInteractable.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS(Blueprintable)
class T66_API AT66ArcadeMachineInteractable : public AT66ArcadeInteractableBase
{
	GENERATED_BODY()

public:
	AT66ArcadeMachineInteractable();
	float GetProtectionAuraRadius() const { return bProtectionAuraEnabled && !bConsumed ? ProtectionAuraRadius : 0.f; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void CreateProtectionAuraComponents();
	void ConfigureProtectionAuraCollision();
	void RefreshProtectionAuraVisual();

	UFUNCTION()
	void HandleProtectionAuraBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleProtectionAuraEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arcade|ProtectionAura", meta = (AllowPrivateAccess = "true"))
	bool bProtectionAuraEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arcade|ProtectionAura", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float ProtectionAuraRadius = 760.f;

	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> ProtectionAuraSphere;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> ProtectionAuraVisualMesh;
};
