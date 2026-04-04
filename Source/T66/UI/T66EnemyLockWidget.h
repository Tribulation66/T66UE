// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66EnemyLockWidget.generated.h"

/** Dedicated bullseye indicator shown above a manually locked enemy. */
UCLASS(Blueprintable)
class T66_API UT66EnemyLockWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
