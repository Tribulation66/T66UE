// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"
#include "Gameplay/T66PlayerController.h"
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
	ApplyUITheme();

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
		return;
	}

	bool bNeedsSave = false;

	if (SettingsObj->SchemaVersion < 5)
	{
		SettingsObj->SchemaVersion = 5;
		SettingsObj->bFogEnabled = true;
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 6)
	{
		SettingsObj->SchemaVersion = 6;
		SettingsObj->bFogEnabled = true;
		SettingsObj->FogIntensityPercent = 55.0f;
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 7)
	{
		SettingsObj->SchemaVersion = 7;
		SettingsObj->UIThemeIndex = static_cast<int32>(ET66UITheme::Dota);
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 8)
	{
		SettingsObj->SchemaVersion = 8;
		SettingsObj->UIThemeIndex = static_cast<int32>(ET66UITheme::Dota);
		bNeedsSave = true;
	}

	if (SettingsObj->SchemaVersion < 9)
	{
		SettingsObj->SchemaVersion = 9;
		SettingsObj->UIThemeIndex = static_cast<int32>(ET66UITheme::Dota);
		bNeedsSave = true;
	}

	if (SettingsObj->bLightTheme)
	{
		SettingsObj->bLightTheme = false;
		bNeedsSave = true;
	}

	const int32 ForcedThemeIndex = static_cast<int32>(ET66UITheme::Dota);
	if (SettingsObj->UIThemeIndex != ForcedThemeIndex)
	{
		SettingsObj->UIThemeIndex = ForcedThemeIndex;
		bNeedsSave = true;
	}

	if (bNeedsSave)
	{
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
	SettingsObj->LastSettingsTabIndex = FMath::Clamp(TabIndex, 0, 7);
	Save();
}

void UT66PlayerSettingsSubsystem::SetUITheme(ET66UITheme NewTheme)
{
	if (!SettingsObj) return;
	(void)NewTheme;

	const int32 NewThemeIndex = static_cast<int32>(ET66UITheme::Dota);
	if (SettingsObj->UIThemeIndex == NewThemeIndex)
	{
		return;
	}

	SettingsObj->UIThemeIndex = NewThemeIndex;
	ApplyUITheme();
	Save();
}

ET66UITheme UT66PlayerSettingsSubsystem::GetUITheme() const
{
	return ET66UITheme::Dota;
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

void UT66PlayerSettingsSubsystem::SetSubtitlesAlwaysOn(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bSubtitlesAlwaysOn = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetSubtitlesAlwaysOn() const
{
	return SettingsObj ? SettingsObj->bSubtitlesAlwaysOn : false;
}

void UT66PlayerSettingsSubsystem::SetDisplayModeIndex(int32 Index)
{
	if (!SettingsObj) return;
	SettingsObj->DisplayModeIndex = FMath::Clamp(Index, 0, 1);
	Save();
}

int32 UT66PlayerSettingsSubsystem::GetDisplayModeIndex() const
{
	return SettingsObj ? SettingsObj->DisplayModeIndex : 0;
}

void UT66PlayerSettingsSubsystem::SetMonitorIndex(int32 Index)
{
	if (!SettingsObj) return;
	SettingsObj->MonitorIndex = FMath::Max(0, Index);
	Save();
}

int32 UT66PlayerSettingsSubsystem::GetMonitorIndex() const
{
	return SettingsObj ? SettingsObj->MonitorIndex : 0;
}

void UT66PlayerSettingsSubsystem::SetHudToggleAffectsInventory(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bHudToggleAffectsInventory = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetHudToggleAffectsInventory() const
{
	return SettingsObj ? SettingsObj->bHudToggleAffectsInventory : true;
}

void UT66PlayerSettingsSubsystem::SetHudToggleAffectsMinimap(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bHudToggleAffectsMinimap = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetHudToggleAffectsMinimap() const
{
	return SettingsObj ? SettingsObj->bHudToggleAffectsMinimap : true;
}

void UT66PlayerSettingsSubsystem::SetHudToggleAffectsIdolSlots(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bHudToggleAffectsIdolSlots = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetHudToggleAffectsIdolSlots() const
{
	return SettingsObj ? SettingsObj->bHudToggleAffectsIdolSlots : true;
}

void UT66PlayerSettingsSubsystem::SetHudToggleAffectsPortraitStats(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bHudToggleAffectsPortraitStats = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetHudToggleAffectsPortraitStats() const
{
	return SettingsObj ? SettingsObj->bHudToggleAffectsPortraitStats : true;
}

void UT66PlayerSettingsSubsystem::SetMediaViewerEnabled(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bMediaViewerEnabled = bEnabled;
	Save();
}

bool UT66PlayerSettingsSubsystem::GetMediaViewerEnabled() const
{
	return SettingsObj ? SettingsObj->bMediaViewerEnabled : true;
}

void UT66PlayerSettingsSubsystem::SetFogEnabled(bool bEnabled)
{
	if (!SettingsObj) return;
	SettingsObj->bFogEnabled = bEnabled;
	if (bEnabled && SettingsObj->FogIntensityPercent <= KINDA_SMALL_NUMBER)
	{
		SettingsObj->FogIntensityPercent = 55.0f;
	}
	Save();
}

bool UT66PlayerSettingsSubsystem::GetFogEnabled() const
{
	return SettingsObj ? (SettingsObj->bFogEnabled && SettingsObj->FogIntensityPercent > KINDA_SMALL_NUMBER) : true;
}

void UT66PlayerSettingsSubsystem::SetFogIntensityPercent(float NewValue)
{
	if (!SettingsObj) return;

	SettingsObj->FogIntensityPercent = FMath::Clamp(NewValue, 0.0f, 100.0f);
	SettingsObj->bFogEnabled = SettingsObj->FogIntensityPercent > KINDA_SMALL_NUMBER;
	Save();
}

float UT66PlayerSettingsSubsystem::GetFogIntensityPercent() const
{
	return SettingsObj ? FMath::Clamp(SettingsObj->FogIntensityPercent, 0.0f, 100.0f) : 55.0f;
}

void UT66PlayerSettingsSubsystem::SetRetroFXSettings(const FT66RetroFXSettings& NewSettings)
{
	if (!SettingsObj) return;
	SettingsObj->RetroFXSettings = NewSettings;
	Save();
}

FT66RetroFXSettings UT66PlayerSettingsSubsystem::GetRetroFXSettings() const
{
	static const FT66RetroFXSettings DefaultSettings;
	return SettingsObj ? SettingsObj->RetroFXSettings : DefaultSettings;
}

void UT66PlayerSettingsSubsystem::ResetRetroFXSettingsToDefaults()
{
	if (!SettingsObj) return;
	SettingsObj->RetroFXSettings = FT66RetroFXSettings();
	Save();
}

void UT66PlayerSettingsSubsystem::ApplyUITheme()
{
	FT66Style::SetActiveTheme(GetUITheme());

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			if (AT66PlayerController* PC = Cast<AT66PlayerController>(World->GetFirstPlayerController()))
			{
				PC->RebuildThemeAwareUI();
			}
		}
	}
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
	SettingsObj->RetroFXSettings = FT66RetroFXSettings();

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



