// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66PerActorLightDirection.generated.h"

UCLASS(ClassGroup=(Rendering), meta=(BlueprintSpawnableComponent))
class T66_API UT66PerActorLightDirection : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66PerActorLightDirection();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToonStyle")
	bool bUseLightDirectionOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToonStyle", meta = (EditCondition = "bUseLightDirectionOverride"))
	FVector LightDirectionOverride = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, Category = "ToonStyle")
	void SetLightDirectionOverride(FVector InDirection);

	UFUNCTION(BlueprintCallable, Category = "ToonStyle")
	void ClearLightDirectionOverride();

	bool GetEffectiveLightDirection(FVector& OutDirection) const;
};
