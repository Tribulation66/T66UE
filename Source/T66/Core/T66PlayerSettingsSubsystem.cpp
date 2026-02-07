// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"
#include "UI/Style/T66Style.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/App.h"
#include "Sound/SoundClass.h"
#include "Core/T66MediaViewerSubsystem.h"

const FString UT66PlayerSettingsSubsystem::SlotName(TEXT("T66_PlayerSettings"));

void UT66PlayerSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreate();

	// Apply saved UI theme (FT66Style::Initialize already ran with Dark defaults at module startup;
	// override to Light here if the player saved that preference).
	if (SettingsObj && SettingsObj->bLightTheme)
	{
		FT66Style::SetTheme(ET66UITheme::Light);
	}

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

bool UT66PlayerSettingsSubsystem::GetPracticeMode() const
{
	return SettingsObj ? SettingsObj->bPracticeMode : false;
}

void UT66PlayerSettingsSubsystem::SetPracticeMode(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bPracticeMode = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetSubmitLeaderboardAnonymous() const
{
	return SettingsObj ? SettingsObj->bSubmitLeaderboardAnonymous : false;
}

void UT66PlayerSettingsSubsystem::SetSubmitLeaderboardAnonymous(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bSubmitLeaderboardAnonymous = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetSpeedRunMode() const
{
	return SettingsObj ? SettingsObj->bSpeedRunMode : false;
}

void UT66PlayerSettingsSubsystem::SetSpeedRunMode(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bSpeedRunMode = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetGoonerMode() const
{
	return SettingsObj ? SettingsObj->bGoonerMode : false;
}

void UT66PlayerSettingsSubsystem::SetGoonerMode(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bGoonerMode = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetLightTheme() const
{
	return SettingsObj ? SettingsObj->bLightTheme : false;
}

void UT66PlayerSettingsSubsystem::SetLightTheme(bool bLight)
{
	if (!SettingsObj) return;
	SettingsObj->bLightTheme = bLight;
	FT66Style::SetTheme(bLight ? ET66UITheme::Light : ET66UITheme::Dark);
	Save();  // broadcasts OnSettingsChanged
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
	ApplyClassVolumesIfPresent();
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
	ApplyClassVolumesIfPresent();
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

void UT66PlayerSettingsSubsystem::SetOutputDeviceId(const FString& NewId)
{
	if (!SettingsObj) return;
	SettingsObj->OutputDeviceId = NewId;
	// TODO: Apply to audio device routing when implemented.
	Save();
}

FString UT66PlayerSettingsSubsystem::GetOutputDeviceId() const
{
	return SettingsObj ? SettingsObj->OutputDeviceId : FString();
}

void UT66PlayerSettingsSubsystem::ApplyAudioToEngine()
{
	if (!SettingsObj) return;
	// Bible: when Media Viewer is open, all other game audio is muted.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				return;
			}
		}
	}
	// Global master volume multiplier.
	FApp::SetVolumeMultiplier(FMath::Clamp(SettingsObj->MasterVolume, 0.0f, 1.0f));
	ApplyClassVolumesIfPresent();
}

void UT66PlayerSettingsSubsystem::ApplyUnfocusedAudioToEngine()
{
	if (!SettingsObj) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				return;
			}
		}
	}
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

void UT66PlayerSettingsSubsystem::ApplyClassVolumesIfPresent()
{
	if (!SettingsObj) return;

	// Foundation: if/when we create SoundClass assets, we can apply per-class volume multipliers.
	// This is safe to call even if assets don't exist yet.
	static const TCHAR* MusicClassPath = TEXT("/Game/Audio/SC_Music.SC_Music");
	static const TCHAR* SfxClassPath = TEXT("/Game/Audio/SC_SFX.SC_SFX");

	if (USoundClass* Music = LoadObject<USoundClass>(nullptr, MusicClassPath))
	{
		Music->Properties.Volume = FMath::Clamp(SettingsObj->MusicVolume, 0.0f, 1.0f);
	}
	if (USoundClass* Sfx = LoadObject<USoundClass>(nullptr, SfxClassPath))
	{
		Sfx->Properties.Volume = FMath::Clamp(SettingsObj->SfxVolume, 0.0f, 1.0f);
	}
}