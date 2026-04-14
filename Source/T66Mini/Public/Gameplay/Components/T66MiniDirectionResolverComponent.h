// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66MiniDirectionResolverComponent.generated.h"

class UT66MiniSpritePresentationComponent;

UCLASS(ClassGroup=(Mini), meta=(BlueprintSpawnableComponent))
class T66MINI_API UT66MiniDirectionResolverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66MiniDirectionResolverComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BindPresentation(UT66MiniSpritePresentationComponent* InPresentation);
	void SetFacingWorldTarget(const FVector& InTargetLocation);
	void ClearFacingWorldTarget();

private:
	UPROPERTY()
	TObjectPtr<UT66MiniSpritePresentationComponent> Presentation;

	FVector LastOwnerLocation = FVector::ZeroVector;
	FVector FacingWorldTarget = FVector::ZeroVector;
	bool bHasLastOwnerLocation = false;
	bool bHasFacingWorldTarget = false;
};
