// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/SaveGame.h"
#include "Core/T66RetroFXSettings.h"
#include "T66PlayerSettingsSaveGame.generated.h"

UENUM(BlueprintType)
enum class ET66BeatTargetSelectionMode : uint8
{
	PersonalBest UMETA(DisplayName = "Personal Best"),
	FriendsTop UMETA(DisplayName = "Friends"),
	StreamersTop UMETA(DisplayName = "Streamers"),
	GlobalTop UMETA(DisplayName = "Global"),
	FavoriteRun UMETA(DisplayName = "Favorite Run"),
};

USTRUCT(BlueprintType)
struct T66_API FT66BeatTargetSelection
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ET66BeatTargetSelectionMode Mode = ET66BeatTargetSelectionMode::GlobalTop;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString FavoriteEntryId;
};

USTRUCT(BlueprintType)
struct T66_API FT66FavoriteLeaderboardRun
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString EntryId;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ET66LeaderboardType LeaderboardType = ET66LeaderboardType::Score;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ET66LeaderboardFilter Filter = ET66LeaderboardFilter::Global;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ET66LeaderboardTime TimeScope = ET66LeaderboardTime::AllTime;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	ET66PartySize PartySize = ET66PartySize::Solo;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 Rank = 0;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString DisplayName;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int64 Score = 0;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float TimeSeconds = 0.f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bHasRunSummary = false;
};

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
	int32 SchemaVersion = 17;

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
	bool bSpeedRunMode = true;

	UPROPERTY(SaveGame)
	bool bGoonerMode = false;

	/** Controls whether the HUD shows the "Time to Beat" line. */
	UPROPERTY(SaveGame)
	bool bShowTimeToBeat = true;

	/** Controls whether the HUD shows the "Score to Beat" line. */
	UPROPERTY(SaveGame)
	bool bShowScoreToBeat = true;

	/** Adds the global stage-pace time line beneath the live timer when available. */
	UPROPERTY(SaveGame)
	bool bShowTimePacing = false;

	/** Adds the global stage-pace score line beneath the live score when available. */
	UPROPERTY(SaveGame)
	bool bShowScorePacing = false;

	/** Selected source for the Time to Beat target. */
	UPROPERTY(SaveGame)
	FT66BeatTargetSelection TimeToBeatSelection;

	/** Selected source for the Score to Beat target. */
	UPROPERTY(SaveGame)
	FT66BeatTargetSelection ScoreToBeatSelection;

	/** Player-facing UI scale multiplier applied on top of engine DPI scaling. */
	UPROPERTY(SaveGame)
	float UIScale = 1.0f;

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
	float MasterVolume = 0.0f;

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

	/** Preferred feed opened by the media viewer: 0 = TikTok, 1 = Shorts, 2 = Reels. */
	UPROPERTY(SaveGame)
	int32 MediaViewerSourceIndex = 0;

	/** When false, exponential height fog is disabled in gameplay. */
	UPROPERTY(SaveGame)
	bool bFogEnabled = true;

	/** Native gameplay fog intensity from 0..100. 0 disables fog entirely, 100 is intentionally very heavy. */
	UPROPERTY(SaveGame)
	float FogIntensityPercent = 55.0f;

	/** Steam friends pinned to the top of the frontend friend list. */
	UPROPERTY(SaveGame)
	TArray<FString> FavoriteFriendSteamIds;

	/** Achievement IDs pinned to the top of the pause-screen progress tracker. */
	UPROPERTY(SaveGame)
	TArray<FName> FavoriteAchievementIds;

	/** Favorited leaderboard rows that can be selected as score/time targets in gameplay HUD. */
	UPROPERTY(SaveGame)
	TArray<FT66FavoriteLeaderboardRun> FavoriteLeaderboardRuns;

	// ===== Retro FX =====
	UPROPERTY(SaveGame)
	FT66RetroFXSettings RetroFXSettings;
};

