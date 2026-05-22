// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "UObject/Interface.h"
#include "T66WidgetGameInputSink.generated.h"

UINTERFACE()
class T66_API UT66WidgetGameInputSink : public UInterface
{
	GENERATED_BODY()
};

class T66_API IT66WidgetGameInputSink
{
	GENERATED_BODY()

public:
	virtual bool HandleWidgetGamePointerInput(const FVector2D& LocalPosition, const bool bPressed) { return false; }
	virtual bool HandleWidgetGameKeyInput(const FKey& Key, const bool bPressed) { return false; }
};
