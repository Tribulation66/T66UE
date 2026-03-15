// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66SlashEffect.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;

/**
 * Legacy compatibility effect for slash visuals.
 * If anything still instantiates this actor, it now emits a short pixel ring
 * instead of loading the removed /Game/VFX/M_SlashDisc material.
 */
UCLASS(NotBlueprintable)
class T66_API AT66SlashEffect : public AActor
{
	GENERATED_BODY()

public:
	AT66SlashEffect();

	/** Call right after spawning to configure the visual. */
	void InitEffect(float InRadius, const FLinearColor& InColor);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> DiscMesh;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> PixelVFXSystem;

	float EffectRadius = 300.f;
	FLinearColor EffectColor = FLinearColor(1.f, 0.3f, 0.1f, 0.8f);

	/** Total lifetime of the visual effect (seconds). */
	float Lifetime = 0.25f;
};
