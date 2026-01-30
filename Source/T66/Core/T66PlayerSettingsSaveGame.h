// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66PlayerSettingsSaveGame.generated.h"

/**
 * Persistent, per-player local settings saved to disk.
 * This is intentionally separate from run save slots (UT66SaveSubsystem).
 */
UCLASS()
class T66_API UT66PlayerSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Bump when adding/changing fields in a breaking way.
	UPROPERTY(SaveGame)
	int32 SchemaVersion = 1;

	// ===== Settings UI =====
	// Saved as an int so SettingsScreen doesn't need to include UI enums here.
	UPROPERTY(SaveGame)
	int32 LastSettingsTabIndex = 0; // 0=Gameplay

	// ===== Gameplay =====
	UPROPERTY(SaveGame)
	bool bIntenseVisuals = true;

	UPROPERTY(SaveGame)
	bool bAutoSprint = false;

	UPROPERTY(SaveGame)
	bool bSubmitScoresToLeaderboard = true;

	UPROPERTY(SaveGame)
	bool bScreenShake = true;

	UPROPERTY(SaveGame)
	bool bCameraSmoothing = true;

	// ===== Audio =====
	UPROPERTY(SaveGame)
	float MasterVolume = 0.8f;

	UPROPERTY(SaveGame)
	float MusicVolume = 0.6f;

	UPROPERTY(SaveGame)
	float SfxVolume = 0.8f;

	UPROPERTY(SaveGame)
	bool bMuteWhenUnfocused = false;

	// Stored for future use (when audio device routing is implemented).
	UPROPERTY(SaveGame)
	FString OutputDeviceId;
};

