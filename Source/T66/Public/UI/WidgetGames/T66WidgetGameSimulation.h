// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI/WidgetGames/T66WidgetGameHostContext.h"
#include "T66WidgetGameSimulation.generated.h"

UINTERFACE()
class T66_API UT66WidgetGameSimulation : public UInterface
{
	GENERATED_BODY()
};

class T66_API IT66WidgetGameSimulation
{
	GENERATED_BODY()

public:
	virtual void ActivateWidgetGameSimulation(const FT66WidgetGameHostContext& HostContext) {}
	virtual void DeactivateWidgetGameSimulation() {}
	virtual void TickWidgetGameSimulation(const float DeltaSeconds) {}
};
