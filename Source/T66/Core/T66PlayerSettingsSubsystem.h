// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/T66RetroFXSettings.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"
#include "UI/T66UITypes.h"
#include "T66PlayerSettingsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnT66PlayerSettingsChanged);

/**
 * Per-player local settings (not tied to run save slots).
 * Applies runtime-facing settings immediately (audio, etc).
 */
UCLASS()
class T66_API UT66PlayerSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString SlotName;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ===== Access =====
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings")
	UT66PlayerSettingsSaveGame* GetSettings() const { return SettingsObj; }

	// ===== Settings UI =====
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetLastSettingsTabIndex(int32 TabIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings")
	int32 GetLastSettingsTabIndex() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|UI")
	void SetUITheme(ET66UITheme NewTheme);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|UI")
	ET66UITheme GetUITheme() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|UI")
	void SetUIScale(float NewScale);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|UI")
	float GetUIScale() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|UI")
	void SetUIFontPreset(ET66UIFontPreset NewPreset);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|UI")
	ET66UIFontPreset GetUIFontPreset() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Social")
	bool IsFavoriteFriend(const FString& FriendSteamId) const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Social")
	void SetFavoriteFriend(const FString& FriendSteamId, bool bFavorite);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Social")
	bool IsFavoriteAchievement(FName AchievementID) const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Social")
	void SetFavoriteAchievement(FName AchievementID, bool bFavorite);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Social")
	TArray<FName> GetFavoriteAchievementIds() const;

	// ===== Gameplay =====
	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetPracticeMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetPracticeMode() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetSubmitLeaderboardAnonymous(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetSubmitLeaderboardAnonymous() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetSpeedRunMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetSpeedRunMode() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetGoonerMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetGoonerMode() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetShowTimeToBeat(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetShowTimeToBeat() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetShowScoreToBeat(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetShowScoreToBeat() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetShowTimePacing(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetShowTimePacing() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetShowScorePacing(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetShowScorePacing() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetTimeToBeatSelection(const FT66BeatTargetSelection& Selection);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	FT66BeatTargetSelection GetTimeToBeatSelection() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetScoreToBeatSelection(const FT66BeatTargetSelection& Selection);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	FT66BeatTargetSelection GetScoreToBeatSelection() const;

	bool IsFavoriteLeaderboardRun(const FString& EntryId) const;
	bool FindFavoriteLeaderboardRun(const FString& EntryId, FT66FavoriteLeaderboardRun& OutFavorite) const;
	TArray<FT66FavoriteLeaderboardRun> GetFavoriteLeaderboardRuns() const;
	void SetFavoriteLeaderboardRun(const FT66FavoriteLeaderboardRun& Favorite, bool bFavorite);

	// ===== Audio =====
	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMasterVolume(float NewValue01);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	float GetMasterVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMusicVolume(float NewValue01);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	float GetMusicVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetSfxVolume(float NewValue01);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	float GetSfxVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMuteWhenUnfocused(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	bool GetMuteWhenUnfocused() const;

	// Stored for future use (when audio device routing is implemented).
	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetOutputDeviceId(const FString& NewId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	FString GetOutputDeviceId() const;

	/** When true, subtitles should be shown when available. Persisted; gameplay reads when subtitle system is used. */
	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetSubtitlesAlwaysOn(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	bool GetSubtitlesAlwaysOn() const;

	/** Display mode: 0 = Standard, 1 = Widescreen. Persisted; viewport/camera can read when widescreen letterboxing is implemented. */
	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetDisplayModeIndex(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Graphics")
	int32 GetDisplayModeIndex() const;

	/** Monitor index: 0 = primary, 1+ = secondary. Applied when applying graphics (window moved to that monitor). */
	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetMonitorIndex(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Graphics")
	int32 GetMonitorIndex() const;

	// ===== HUD (which elements the HUD toggle key affects) =====
	UFUNCTION(BlueprintCallable, Category = "Settings|HUD")
	void SetHudToggleAffectsInventory(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|HUD")
	bool GetHudToggleAffectsInventory() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|HUD")
	void SetHudToggleAffectsMinimap(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|HUD")
	bool GetHudToggleAffectsMinimap() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|HUD")
	void SetHudToggleAffectsIdolSlots(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|HUD")
	bool GetHudToggleAffectsIdolSlots() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|HUD")
	void SetHudToggleAffectsPortraitStats(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|HUD")
	bool GetHudToggleAffectsPortraitStats() const;

	// ===== Media Viewer =====
	UFUNCTION(BlueprintCallable, Category = "Settings|MediaViewer")
	void SetMediaViewerEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|MediaViewer")
	bool GetMediaViewerEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|MediaViewer")
	void SetMediaViewerSource(ET66MediaViewerSource NewSource);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|MediaViewer")
	ET66MediaViewerSource GetMediaViewerSource() const;

	/** Fog in gameplay level (exponential height fog). */
	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetFogEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Graphics")
	bool GetFogEnabled() const;

	/** Native gameplay fog intensity from 0..100. 0 disables fog entirely. */
	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetFogIntensityPercent(float NewValue);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	float GetFogIntensityPercent() const;

	// ===== Retro FX =====
	UFUNCTION(BlueprintCallable, Category = "Settings|RetroFX")
	void SetRetroFXSettings(const FT66RetroFXSettings& NewSettings);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|RetroFX")
	FT66RetroFXSettings GetRetroFXSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|RetroFX")
	void ResetRetroFXSettingsToDefaults();

	// ===== Utilities =====
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplySafeModeSettings();

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnT66PlayerSettingsChanged OnSettingsChanged;

private:
	UPROPERTY()
	TObjectPtr<UT66PlayerSettingsSaveGame> SettingsObj;

	void LoadOrCreate();
	void Save();
	void ApplyUIFontPreset();
	void ApplyUITheme();
	void ApplyUIScale();

	void ApplyAudioToEngine();
	void ApplyUnfocusedAudioToEngine();

	void ApplyClassVolumesIfPresent();
};


