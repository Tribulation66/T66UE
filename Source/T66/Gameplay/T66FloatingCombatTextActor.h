// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66FloatingCombatTextActor.generated.h"

class UWidgetComponent;

/** Short-lived actor that displays one floating combat text (damage number or status label) via a WidgetComponent. */
UCLASS()
class T66_API AT66FloatingCombatTextActor : public AActor
{
	GENERATED_BODY()

public:
	AT66FloatingCombatTextActor();

	/** Set content to a damage number. Call after spawn, before lifespan expires. */
	void SetDamageNumber(int32 Amount, FName EventType = NAME_None);

	/** Set content to a status event label (e.g. Crit, DoT). Call after spawn. */
	void SetStatusEvent(FName EventType);

private:
	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<UWidgetComponent> WidgetComponent;
};
