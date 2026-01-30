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
	void SetIntenseVisuals(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetIntenseVisuals() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetAutoSprint(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetAutoSprint() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetSubmitScoresToLeaderboard(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetSubmitScoresToLeaderboard() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetScreenShake(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetScreenShake() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Gameplay")
	void SetCameraSmoothing(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Gameplay")
	bool GetCameraSmoothing() const;

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
};

