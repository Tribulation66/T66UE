// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "T66WidgetGamePersistence.generated.h"

UINTERFACE()
class T66_API UT66WidgetGamePersistence : public UInterface
{
	GENERATED_BODY()
};

class T66_API IT66WidgetGamePersistence
{
	GENERATED_BODY()

public:
	// Persist current widget game state through the implementation's existing save subsystem.
	virtual void SaveWidgetGameState() {}

	// Restore current widget game state through the implementation's existing load path.
	virtual void LoadWidgetGameState() {}

	// Flush any pending save using the implementation's existing persistence path.
	virtual void FlushWidgetGamePersistence() {}

	// Refresh in-memory state from the implementation's existing persistence path.
	virtual void RefreshWidgetGamePersistence() {}
};
