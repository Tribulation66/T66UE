// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66PlayerSettingsSubsystem.generated.h"

class UT66PlayerSettingsSaveGame;

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

	// ===== Theme =====
	/** Returns true when the Light UI theme is active (false = Dark, the default). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Theme")
	bool GetLightTheme() const;

	/** Switch between Dark (false) and Light (true) UI theme. Persists to disk and re-initializes styles. */
	UFUNCTION(BlueprintCallable, Category = "Settings|Theme")
	void SetLightTheme(bool bLight);

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

	// ===== Media Viewer (TikTok / YouTube Shorts) =====
	UFUNCTION(BlueprintCallable, Category = "Settings|MediaViewer")
	void SetMediaViewerEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|MediaViewer")
	bool GetMediaViewerEnabled() const;

	/** Fog in gameplay level (exponential height fog). Default off. */
	UFUNCTION(BlueprintCallable, Category = "Settings|Graphics")
	void SetFogEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Graphics")
	bool GetFogEnabled() const;

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

	void ApplyAudioToEngine();
	void ApplyUnfocusedAudioToEngine();

	void ApplyClassVolumesIfPresent();
};

