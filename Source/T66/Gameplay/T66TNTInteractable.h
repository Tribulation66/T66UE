#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66TNTInteractable.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable)
class T66_API AT66TNTInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66TNTInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TNT", meta = (ClampMin = "0.10"))
	float FuseSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TNT", meta = (ClampMin = "50.0"))
	float ExplosionRadius = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TNT", meta = (ClampMin = "0"))
	int32 DamageHP = 35;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TNT")
	bool bDamageBosses = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TNT")
	TObjectPtr<UStaticMeshComponent> FuseMesh;

	virtual bool Interact(APlayerController* PlayerController) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ApplyRarityVisuals() override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual float GetInteractionPromptVerticalPadding() const override;
	virtual FVector GetMinimumInteractionExtent() const override;
	virtual FVector GetInteractionBoundsPadding() const override;

private:
	FTimerHandle FuseTimerHandle;
	bool bFuseLit = false;

	void Explode();
	void ApplyExplosionDamage(int32& OutHeroesDamaged, int32& OutEnemiesDamaged, int32& OutMobsDamaged, int32& OutBossesDamaged);
	bool IsActorInExplosionRadius(const AActor* Actor, const FVector& Origin, float RadiusSq) const;
	void UpdateFuseVisuals();
};
