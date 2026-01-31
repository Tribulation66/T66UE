// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;
class UT66RunStateSubsystem;
class UT66PlayerSettingsSubsystem;

/**
 * Simple music state manager.
 * - Plays Theme music immediately (including frontend).
 * - Switches to Survival music during Last Stand (0 hearts but still alive).
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
	// Survival:
	//   /Game/Audio/OSTS/Survival
	//   /Game/Audio/Music/Survival
	//   /Game/Audio/Survival
	UPROPERTY()
	TSoftObjectPtr<USoundBase> MainThemeSound;

	UPROPERTY()
	TSoftObjectPtr<USoundBase> ThemeSound;

	UPROPERTY()
	TSoftObjectPtr<USoundBase> SurvivalSound;

	TArray<FSoftObjectPath> MainThemeCandidates;
	TArray<FSoftObjectPath> ThemeCandidates;
	TArray<FSoftObjectPath> SurvivalCandidates;

	UPROPERTY()
	TObjectPtr<UAudioComponent> MainThemeComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ThemeComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> SurvivalComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BossComp;

	UPROPERTY()
	TSoftObjectPtr<USoundBase> BossSound;

	bool bThemeStarted = false;
	bool bMainThemeStarted = false;
	bool bSurvivalActive = false;
	bool bBossMusicActive = false;

	// Prevent "FadeOut -> OnAudioFinished -> loop again" while switching tracks.
	bool bAllowThemeLoop = true;
	bool bAllowMainThemeLoop = true;
	bool bAllowSurvivalLoop = true;
	bool bAllowBossLoop = true;

	ET66BaseTrack DesiredBaseTrack = ET66BaseTrack::None;

	FDelegateHandle PostWorldInitHandle;
	FDelegateHandle PostLoadMapHandle;

	void HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void HandlePostLoadMap(UWorld* World);

	UFUNCTION()
	void HandleSurvivalChanged();

	UFUNCTION()
	void HandleBossChanged();

	void UpdateMusicState();

	UFUNCTION()
	void HandleSettingsChanged();

	void ApplyMusicVolumes();

	bool IsFrontendWorld(UWorld* World) const;

	USoundBase* ResolveAndLoadMainThemeSound();
	USoundBase* ResolveAndLoadThemeSound();
	USoundBase* ResolveAndLoadSurvivalSound();
	USoundBase* ResolveAndLoadGameplayThemeSound(UWorld* World);
	USoundBase* ResolveAndLoadBossThemeSound(UWorld* World);

	void EnsureMainThemePlaying(UWorld* World);
	void EnsureThemePlaying(UWorld* World);
	void EnsureSurvivalPlaying(UWorld* World);
	void EnsureBossPlaying(UWorld* World);

	void StopMainTheme(float FadeSeconds);
	void StopTheme(float FadeSeconds);
	void StopSurvival(float FadeSeconds);
	void StopBoss(float FadeSeconds);

	UFUNCTION()
	void HandleThemeFinished();

	UFUNCTION()
	void HandleMainThemeFinished();

	UFUNCTION()
	void HandleSurvivalFinished();

	UFUNCTION()
	void HandleBossFinished();
};

