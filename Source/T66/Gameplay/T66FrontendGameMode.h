// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T66FrontendGameMode.generated.h"

class ACameraActor;

/**
 * Game Mode for the frontend/menu level
 * Doesn't spawn pawns, just handles menu UI.
 * Manages a world camera that views the preview characters directly (full Lumen GI).
 */
UCLASS(Blueprintable)
class T66_API AT66FrontendGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AT66FrontendGameMode();

	/** Position the world camera to view the hero preview stage. Call when hero selection screen activates. */
	void PositionCameraForHeroPreview();

	/** Position the world camera to view the companion preview stage. Call when companion selection screen activates. */
	void PositionCameraForCompanionPreview();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleSettingsChanged();

	/** Camera that views the preview characters (set as the PlayerController's view target). */
	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> PreviewCamera;
};
