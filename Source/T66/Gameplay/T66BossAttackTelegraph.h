// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66BossAttackTelegraph.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

/** Short-lived primitive telegraph used by boss attacks before the damaging object spawns. */
UCLASS()
class T66_API AT66BossAttackTelegraph : public AActor
{
	GENERATED_BODY()

public:
	AT66BossAttackTelegraph();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	void ConfigureTelegraph(
		UStaticMesh* Mesh,
		const FLinearColor& Color,
		const FVector& StartScale,
		const FVector& EndScale,
		float DurationSeconds,
		float SpinDegreesPerSecond = 180.f);

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	FLinearColor TelegraphColor = FLinearColor(0.32f, 1.0f, 0.12f, 1.f);
	FVector InitialScale = FVector(0.2f);
	FVector FinalScale = FVector(1.f);
	float Duration = 0.65f;
	float Age = 0.f;
	float SpinRate = 180.f;
};
