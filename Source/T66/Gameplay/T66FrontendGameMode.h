// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T66FrontendGameMode.generated.h"

/**
 * Game Mode for the frontend/menu level
 * Doesn't spawn pawns, just handles menu UI
 */
UCLASS(Blueprintable)
class T66_API AT66FrontendGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AT66FrontendGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleSettingsChanged();
};
