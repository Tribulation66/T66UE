// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;
class UT66RunStateSubsystem;
class UT66PlayerSettingsSubsystem;
struct FStreamableHandle;

/**
 * Simple music state manager.
 * - Plays Theme music immediately (including frontend).
 *
 * Note: Unreal must import audio into SoundWave/SoundCue assets.
 * Dropping .ogg files into Content/ is not enough until the editor imports them.
 */
UCLASS()
class T66_API UT66MusicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	enum class ET66BaseTrack : uint8
	{
		None,
		MainTheme,
		Theme,
	};

	// Expected assets after import (SoundWave/SoundCue). We support a few common folder layouts.
	// Main Theme (FrontendLevel):
	//   /Game/Audio/OSTS/MainTheme
	//   /Game/Audio/Music/MainTheme
	//   /Game/Audio/MainTheme
	// Theme (Gameplay):
	//   /Game/Audio/OSTS/Theme
	//   /Game/Audio/Music/Theme
	//   /Game/Audio/Theme
	UPROPERTY()
	TSoftObjectPtr<USoundBase> MainThemeSound;

	UPROPERTY()
	TSoftObjectPtr<USoundBase> ThemeSound;

	TArray<FSoftObjectPath> MainThemeCandidates;
	TArray<FSoftObjectPath> ThemeCandidates;

	UPROPERTY()
	TObjectPtr<UAudioComponent> MainThemeComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ThemeComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BossComp;

	UPROPERTY()
	TSoftObjectPtr<USoundBase> BossSound;

	bool bThemeStarted = false;
	bool bMainThemeStarted = false;
	bool bBossMusicActive = false;

	// Prevent "FadeOut -> OnAudioFinished -> loop again" while switching tracks.
	bool bAllowThemeLoop = true;
	bool bAllowMainThemeLoop = true;
	bool bAllowBossLoop = true;

	ET66BaseTrack DesiredBaseTrack = ET66BaseTrack::None;

	FDelegateHandle PostWorldInitHandle;
	FDelegateHandle PostLoadMapHandle;

	void HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void HandlePostLoadMap(UWorld* World);

	UFUNCTION()
	void HandleBossChanged();

	void UpdateMusicState();

	UFUNCTION()
	void HandleSettingsChanged();

	void ApplyMusicVolumes();

	bool IsFrontendWorld(UWorld* World) const;

	USoundBase* ResolveAndLoadMainThemeSound();
	USoundBase* ResolveAndLoadThemeSound();
	USoundBase* ResolveAndLoadGameplayThemeSound(UWorld* World);
	USoundBase* ResolveAndLoadBossThemeSound(UWorld* World);
	void QueueBaseMusicPreloads();
	void QueueMainThemePreload();
	void QueueThemePreload();
	void HandleMainThemePreloaded();
	void HandleThemePreloaded();
	USoundBase* ResolveFirstResidentSoundInFolder(const FString& FolderPath);
	void QueueFolderSoundPreload(const FString& FolderPath, const TArray<FSoftObjectPath>& CandidatePaths);
	void HandleFolderSoundPreloaded(FString FolderPath);

	void EnsureMainThemePlaying(UWorld* World);
	void EnsureThemePlaying(UWorld* World);
	void EnsureBossPlaying(UWorld* World);

	void StopMainTheme(float FadeSeconds);
	void StopTheme(float FadeSeconds);
	void StopBoss(float FadeSeconds);

	UFUNCTION()
	void HandleThemeFinished();

	UFUNCTION()
	void HandleMainThemeFinished();

	UFUNCTION()
	void HandleBossFinished();

	TSharedPtr<FStreamableHandle> MainThemeLoadHandle;
	TSharedPtr<FStreamableHandle> ThemeLoadHandle;
	TMap<FString, TObjectPtr<USoundBase>> CachedFolderSounds;
	TMap<FString, TSharedPtr<FStreamableHandle>> PendingFolderSoundLoads;
	TMap<FString, TArray<FSoftObjectPath>> FolderSoundCandidatePaths;
	TSet<FString> WarnedMissingFolderSounds;
};
