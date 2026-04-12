// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66CircusInteractable.generated.h"

class UT66RunStateSubsystem;
class USphereComponent;

UCLASS(Blueprintable)
class T66_API AT66CircusInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66CircusInteractable();

	virtual bool Interact(APlayerController* PC) override;
	float GetSafeZoneRadius() const { return SafeZoneRadius; }
	void ConfigureCompactTowerVariant(float InScaleMultiplier = 0.72f, float InSafeZoneRadius = 720.f);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ApplyRarityVisuals() override;
	virtual bool ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const override;
	virtual float GetInteractionPromptWorldSize() const override { return 92.f; }
	virtual float GetInteractionPromptVerticalPadding() const override { return 220.f; }
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(360.f, 310.f, 220.f); }
	virtual FVector GetInteractionBoundsPadding() const override { return FVector(60.f, 60.f, 50.f); }
	virtual FVector GetImportedVisualScale() const override { return FVector(0.5f, 0.5f, 0.5f); }

private:
	UFUNCTION()
	void HandleBossStateChanged();

	UFUNCTION()
	void HandleSafeZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleSafeZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UT66RunStateSubsystem* GetRunState() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SafeZone", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SafeZoneSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SafeZone", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float SafeZoneRadius = 1100.f;
};

