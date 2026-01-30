// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/App.h"

const FString UT66PlayerSettingsSubsystem::SlotName(TEXT("T66_PlayerSettings"));

void UT66PlayerSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreate();
	ApplyAudioToEngine();
	ApplyUnfocusedAudioToEngine();
}

void UT66PlayerSettingsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UT66PlayerSettingsSubsystem::LoadOrCreate()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	SettingsObj = Cast<UT66PlayerSettingsSaveGame>(Loaded);
	if (!SettingsObj)
	{
		SettingsObj = Cast<UT66PlayerSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66PlayerSettingsSaveGame::StaticClass()));
		Save();
	}
}

void UT66PlayerSettingsSubsystem::Save()
{
	if (!SettingsObj) return;
	UGameplayStatics::SaveGameToSlot(SettingsObj, SlotName, 0);
	OnSettingsChanged.Broadcast();
}

int32 UT66PlayerSettingsSubsystem::GetLastSettingsTabIndex() const
{
	return SettingsObj ? SettingsObj->LastSettingsTabIndex : 0;
}

void UT66PlayerSettingsSubsystem::SetLastSettingsTabIndex(int32 TabIndex)
{
	if (!SettingsObj) return;
	SettingsObj->LastSettingsTabIndex = FMath::Clamp(TabIndex, 0, 4);
	Save();
}

bool UT66PlayerSettingsSubsystem::GetIntenseVisuals() const
{
	return SettingsObj ? SettingsObj->bIntenseVisuals : true;
}

void UT66PlayerSettingsSubsystem::SetIntenseVisuals(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bIntenseVisuals = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetAutoSprint() const
{
	return SettingsObj ? SettingsObj->bAutoSprint : false;
}

void UT66PlayerSettingsSubsystem::SetAutoSprint(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bAutoSprint = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetSubmitScoresToLeaderboard() const
{
	return SettingsObj ? SettingsObj->bSubmitScoresToLeaderboard : true;
}

void UT66PlayerSettingsSubsystem::SetSubmitScoresToLeaderboard(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bSubmitScoresToLeaderboard = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetScreenShake() const
{
	return SettingsObj ? SettingsObj->bScreenShake : true;
}

void UT66PlayerSettingsSubsystem::SetScreenShake(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bScreenShake = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetCameraSmoothing() const
{
	return SettingsObj ? SettingsObj->bCameraSmoothing : true;
}

void UT66PlayerSettingsSubsystem::SetCameraSmoothing(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bCameraSmoothing = bEnabled;
	Save();
}

float UT66PlayerSettingsSubsystem::GetMasterVolume() const
{
	return SettingsObj ? SettingsObj->MasterVolume : 0.8f;
}

void UT66PlayerSettingsSubsystem::SetMasterVolume(float NewValue01)
{
	if (!SettingsObj) return;
	SettingsObj->MasterVolume = FMath::Clamp(NewValue01, 0.0f, 1.0f);
	ApplyAudioToEngine();
	Save();
}

float UT66PlayerSettingsSubsystem::GetMusicVolume() const
{
	return SettingsObj ? SettingsObj->MusicVolume : 0.6f;
}

void UT66PlayerSettingsSubsystem::SetMusicVolume(float NewValue01)
{
	if (!SettingsObj) return;
	SettingsObj->MusicVolume = FMath::Clamp(NewValue01, 0.0f, 1.0f);
	// Stored now; applied later when music/sfx routing is implemented via SoundClasses/Submixes.
	Save();
}

float UT66PlayerSettingsSubsystem::GetSfxVolume() const
{
	return SettingsObj ? SettingsObj->SfxVolume : 0.8f;
}

void UT66PlayerSettingsSubsystem::SetSfxVolume(float NewValue01)
{
	if (!SettingsObj) return;
	SettingsObj->SfxVolume = FMath::Clamp(NewValue01, 0.0f, 1.0f);
	// Stored now; applied later when music/sfx routing is implemented via SoundClasses/Submixes.
	Save();
}

bool UT66PlayerSettingsSubsystem::GetMuteWhenUnfocused() const
{
	return SettingsObj ? SettingsObj->bMuteWhenUnfocused : false;
}

void UT66PlayerSettingsSubsystem::SetMuteWhenUnfocused(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bMuteWhenUnfocused = bEnabled;
	ApplyUnfocusedAudioToEngine();
	Save();
}

void UT66PlayerSettingsSubsystem::ApplyAudioToEngine()
{
	if (!SettingsObj) return;
	// Global master volume multiplier.
	FApp::SetVolumeMultiplier(FMath::Clamp(SettingsObj->MasterVolume, 0.0f, 1.0f));
}

void UT66PlayerSettingsSubsystem::ApplyUnfocusedAudioToEngine()
{
	if (!SettingsObj) return;
	// If muted when unfocused, drop unfocused volume to 0; otherwise keep at 1.
	FApp::SetUnfocusedVolumeMultiplier(SettingsObj->bMuteWhenUnfocused ? 0.0f : 1.0f);
}

void UT66PlayerSettingsSubsystem::ApplySafeModeSettings()
{
	if (!SettingsObj) return;

	// Gameplay-side stability toggles.
	SettingsObj->bIntenseVisuals = false;

	// Audio: keep user master, but enforce mute-unfocused off for stability/debug.
	SettingsObj->bMuteWhenUnfocused = false;
	ApplyUnfocusedAudioToEngine();

	// Graphics: conservative defaults.
	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		GUS->SetOverallScalabilityLevel(0);     // Best performance
		GUS->SetFrameRateLimit(60.0f);          // Conservative cap
		GUS->SetFullscreenMode(EWindowMode::Windowed);
		GUS->ApplySettings(false);
		GUS->SaveSettings();
	}

	Save();
}

