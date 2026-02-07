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

