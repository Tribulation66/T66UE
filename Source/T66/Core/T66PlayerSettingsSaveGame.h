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
	int32 SchemaVersion = 2;

	// ===== Settings UI =====
	// Saved as an int so SettingsScreen doesn't need to include UI enums here.
	UPROPERTY(SaveGame)
	int32 LastSettingsTabIndex = 0; // 0=Gameplay

	// ===== Gameplay =====
	UPROPERTY(SaveGame)
	bool bPracticeMode = false;

	UPROPERTY(SaveGame)
	bool bSubmitLeaderboardAnonymous = false;

	UPROPERTY(SaveGame)
	bool bSpeedRunMode = false;

	UPROPERTY(SaveGame)
	bool bGoonerMode = false;

	// ===== Theme =====
	/** false = Dark (default), true = Light */
	UPROPERTY(SaveGame)
	bool bLightTheme = false;

	// ===== Legacy (not surfaced in Settings UI) =====
	// Kept for backward compatibility with existing saves / future VFX tuning.
	UPROPERTY(SaveGame)
	bool bIntenseVisuals = false;

	UPROPERTY(SaveGame)
	bool bAutoSprint = false;

	UPROPERTY(SaveGame)
	bool bSubmitScoresToLeaderboard = true;

	UPROPERTY(SaveGame)
	bool bScreenShake = false;

	UPROPERTY(SaveGame)
	bool bCameraSmoothing = false;

	// ===== Audio =====
	UPROPERTY(SaveGame)
	float MasterVolume = 0.8f;

	UPROPERTY(SaveGame)
	float MusicVolume = 0.6f;

	UPROPERTY(SaveGame)
	float SfxVolume = 0.8f;

	UPROPERTY(SaveGame)
	bool bMuteWhenUnfocused = false;

	/** When true, subtitles/captions are shown whenever available (e.g. dialogue). Read by gameplay when a subtitle system exists. */
	UPROPERTY(SaveGame)
	bool bSubtitlesAlwaysOn = false;

	// Stored for future use (when audio device routing is implemented).
	UPROPERTY(SaveGame)
	FString OutputDeviceId;

	/** Display mode: 0 = Standard, 1 = Widescreen. Persisted for Settings UI; viewport/camera can read from subsystem when applied. */
	UPROPERTY(SaveGame)
	int32 DisplayModeIndex = 0;

	/** Monitor index for game window: 0 = primary, 1+ = secondary. Applied when changing graphics (window moved to that monitor). */
	UPROPERTY(SaveGame)
	int32 MonitorIndex = 0;

	// ===== HUD (which elements the HUD toggle key affects) =====
	UPROPERTY(SaveGame)
	bool bHudToggleAffectsInventory = true;

	UPROPERTY(SaveGame)
	bool bHudToggleAffectsMinimap = true;

	UPROPERTY(SaveGame)
	bool bHudToggleAffectsIdolSlots = true;

	UPROPERTY(SaveGame)
	bool bHudToggleAffectsPortraitStats = true;

	// ===== Media Viewer (TikTok / YouTube Shorts) =====
	/** When false, Toggle TikTok / Toggle Media Viewer do nothing. */
	UPROPERTY(SaveGame)
	bool bMediaViewerEnabled = true;

	/** When false, exponential height fog is disabled in gameplay (default off). */
	UPROPERTY(SaveGame)
	bool bFogEnabled = false;
};

