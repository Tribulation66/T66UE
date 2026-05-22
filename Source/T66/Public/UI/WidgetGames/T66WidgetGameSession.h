// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI/WidgetGames/T66WidgetGameHostContext.h"
#include "T66WidgetGameSession.generated.h"

UINTERFACE()
class T66_API UT66WidgetGameSession : public UInterface
{
	GENERATED_BODY()
};

class T66_API IT66WidgetGameSession
{
	GENERATED_BODY()

public:
	virtual void ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext) {}
	virtual void DeactivateWidgetGame() {}
	virtual void PauseWidgetGame() {}
	virtual void ResumeWidgetGame() {}
	virtual void RequestWidgetGameExit() {}
};
