// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Shutdown/T66ShutdownSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;
class UT66RunStateSubsystem;
class UT66PlayerSettingsSubsystem;
struct FStreamableHandle;

/**
 * Music state manager. Priority: Stinger (one-shot) > Boss > Area override > base track.
 * - Base track: MainTheme on FrontendLevel, Theme (hero/stage variants) in gameplay.
 * - All tracks resolve by folder convention so replacing audio is a file drop + reimport:
 *     Hero theme:  /Game/Audio/OSTS/Heroes/<HeroKey>/      (HeroKey = Heroes.csv MapTheme, else HeroID)
 *     Stage theme: /Game/Audio/OSTS/Stages/Stage_<NN>/     (NN = global stage number, e.g. Stage_01)
 *     Boss theme:  /Game/Audio/OSTS/Bosses/<BossID>/       (legacy fallback: Bosses/Special/<BossID>/)
 *     Area theme:  /Game/Audio/OSTS/Areas/<AreaID>/        (AreaID = NPCID for NPC safe-zone bubbles)
 *     Stinger:     /Game/Audio/OSTS/Stingers/<StingerID>/  (Victory, Defeat)
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

	/** Area music override (NPC safe-zone bubbles etc.). Last pushed wins; outranked only by boss music. */
	UFUNCTION(BlueprintCallable, Category = "T66|Music")
	void PushAreaMusic(FName AreaID);

	UFUNCTION(BlueprintCallable, Category = "T66|Music")
	void PopAreaMusic(FName AreaID);

	/** One-shot musical cue (e.g. Victory/Defeat). Fades out other music; base music resumes when it ends. */
	UFUNCTION(BlueprintCallable, Category = "T66|Music")
	void PlayStinger(FName StingerID);

private:
	bool HandleShutdown(const FT66ShutdownContext& Context);
	void ShutdownRuntimeResources(const TCHAR* Reason);

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

	UPROPERTY()
	TObjectPtr<UAudioComponent> AreaComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> StingerComp;

	/** Active area-music requests (e.g. overlapping NPC bubbles); last entry wins. */
	TArray<FName> AreaMusicStack;
	FName ActiveAreaID;

	bool bThemeStarted = false;
	bool bMainThemeStarted = false;
	bool bBossMusicActive = false;
	bool bAreaMusicActive = false;
	bool bStingerActive = false;

	// Prevent "FadeOut -> OnAudioFinished -> loop again" while switching tracks.
	bool bAllowThemeLoop = true;
	bool bAllowMainThemeLoop = true;
	bool bAllowBossLoop = true;
	bool bAllowAreaLoop = true;

	ET66BaseTrack DesiredBaseTrack = ET66BaseTrack::None;

	FDelegateHandle PostWorldInitHandle;
	FDelegateHandle PostLoadMapHandle;

	void HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void HandlePostLoadMap(UWorld* World);

	/** Drop audio components that belong to a torn-down world (map transition). Calling Play/Stop
	 * on them dereferences the dead world's audio device (crash: Enter the Tribulation). */
	void ResetAudioComponentsForWorldChange(UWorld* NewWorld);

	UFUNCTION()
	void HandleBossChanged();

	UFUNCTION()
	void HandleStageChanged();

	void UpdateMusicState();

	UFUNCTION()
	void HandleSettingsChanged();

	void ApplyMusicVolumes();

	bool IsFrontendWorld(UWorld* World) const;

	USoundBase* ResolveAndLoadMainThemeSound();
	USoundBase* ResolveAndLoadThemeSound();
	USoundBase* ResolveAndLoadGameplayThemeSound(UWorld* World);
	USoundBase* ResolveAndLoadBossThemeSound(UWorld* World);
	USoundBase* ResolveAndLoadAreaThemeSound();
	void QueueBaseMusicPreloads();
	void QueueStingerPreloads();
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
	void EnsureAreaPlaying(UWorld* World);

	void StopMainTheme(float FadeSeconds);
	void StopTheme(float FadeSeconds);
	void StopBoss(float FadeSeconds);
	void StopArea(float FadeSeconds);

	UFUNCTION()
	void HandleThemeFinished();

	UFUNCTION()
	void HandleMainThemeFinished();

	UFUNCTION()
	void HandleBossFinished();

	UFUNCTION()
	void HandleAreaFinished();

	UFUNCTION()
	void HandleStingerFinished();

	TSharedPtr<FStreamableHandle> MainThemeLoadHandle;
	TSharedPtr<FStreamableHandle> ThemeLoadHandle;

	/** UPROPERTY so GC nulls entries instead of leaving dangling pointers across map transitions
	 * (a bare TMap member is invisible to the GC — cached frontend sounds died with their world
	 * and SpawnSound2D crashed on the freed pointer: the Enter-the-Tribulation fatals). */
	UPROPERTY()
	TMap<FString, TObjectPtr<USoundBase>> CachedFolderSounds;
	TMap<FString, TSharedPtr<FStreamableHandle>> PendingFolderSoundLoads;
	TMap<FString, TArray<FSoftObjectPath>> FolderSoundCandidatePaths;
	TSet<FString> WarnedMissingFolderSounds;
	FT66ShutdownParticipantHandle ShutdownParticipantHandle;
};
