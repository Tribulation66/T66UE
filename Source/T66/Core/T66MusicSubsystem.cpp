// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MusicSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/Shutdown/T66ShutdownSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Music, Log, All);

// NOTE: This file must remain in the runtime module so the UHT-generated glue links correctly.

static bool IsT66MusicSoundAssetData(const FAssetData& AssetData)
{
	const FName AssetClassName = AssetData.AssetClassPath.GetAssetName();
	return AssetClassName == TEXT("SoundWave")
		|| AssetClassName == TEXT("SoundCue")
		|| AssetClassName == TEXT("MetaSoundSource");
}

// ResolveObject can return an object that is still ASYNC-LOADING (RF_NeedLoad/RF_NeedPostLoad):
// handing it to SpawnSound2D crashes FAudioDevice::CreateComponent on uninitialized audio data.
// Only treat a sound as resident once it is fully loaded and not mid-async-load.
static bool T66IsSoundReadyToPlay(const USoundBase* Sound)
{
	return IsValid(Sound)
		&& !Sound->HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects);
}

static USoundBase* ResolveFirstResidentSoundAsset(const TArray<FSoftObjectPath>& Candidates, TSoftObjectPtr<USoundBase>& InOutSoftPtr)
{
	for (const FSoftObjectPath& Path : Candidates)
	{
		if (!Path.IsValid())
		{
			continue;
		}

		USoundBase* Sound = Cast<USoundBase>(Path.ResolveObject());
		if (T66IsSoundReadyToPlay(Sound))
		{
			InOutSoftPtr = TSoftObjectPtr<USoundBase>(Path);
			return Sound;
		}
	}
	return nullptr;
}

static TArray<FSoftObjectPath> CollectSoundAssetPathsInFolder(const FString& FolderPath)
{
	TArray<FSoftObjectPath> OutPaths;
	if (FolderPath.IsEmpty())
	{
		return OutPaths;
	}

	const FName PathName(*FolderPath);
	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> Assets;
	ARM.Get().GetAssetsByPath(PathName, Assets, /*bRecursive=*/true);
	if (Assets.Num() <= 0)
	{
		return OutPaths;
	}

	// Deterministic selection: alphabetical by asset name.
	Assets.Sort([](const FAssetData& A, const FAssetData& B)
	{
		return A.AssetName.LexicalLess(B.AssetName);
	});

	for (const FAssetData& AD : Assets)
	{
		if (IsT66MusicSoundAssetData(AD))
		{
			OutPaths.AddUnique(AD.GetSoftObjectPath());
		}
	}
	return OutPaths;
}

void UT66MusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Ensure dependent subsystems exist so delegates/volume reads work reliably.
	Collection.InitializeDependency(UT66ShutdownSubsystem::StaticClass());
	Collection.InitializeDependency<UT66PlayerSettingsSubsystem>();
	Collection.InitializeDependency<UT66RunStateSubsystem>();

	Super::Initialize(Collection);

	// Convention-based soft references (user imports to Content/Audio).
	// Do NOT eagerly load here; resolve on demand when the game actually begins play.
	MainThemeCandidates = {
		FSoftObjectPath(TEXT("/Game/Audio/OSTS/MainTheme.MainTheme")),
		FSoftObjectPath(TEXT("/Game/Audio/Music/MainTheme.MainTheme")),
		FSoftObjectPath(TEXT("/Game/Audio/MainTheme.MainTheme")),
	};
	ThemeCandidates = {
		FSoftObjectPath(TEXT("/Game/Audio/OSTS/Theme.Theme")),
		FSoftObjectPath(TEXT("/Game/Audio/Music/Theme.Theme")),
		FSoftObjectPath(TEXT("/Game/Audio/Theme.Theme")),
	};
	QueueBaseMusicPreloads();
	QueueStingerPreloads();

	// Listen for world creation so we can start Theme even on FrontendLevel.
	PostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UT66MusicSubsystem::HandlePostWorldInit);
	// Reliable for PIE: map-load hook (fires after the PIE map is loaded).
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UT66MusicSubsystem::HandlePostLoadMap);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			ShutdownParticipantHandle = Shutdown->RegisterParticipant(
				this,
				FName(TEXT("Music.MediaAudio")),
				ET66ShutdownPhase::MediaAudio,
				20,
				1.0,
				false,
				FT66ShutdownParticipantDelegate::CreateUObject(this, &UT66MusicSubsystem::HandleShutdown));
		}
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->BossChanged.AddDynamic(this, &UT66MusicSubsystem::HandleBossChanged);
			RunState->StageChanged.AddDynamic(this, &UT66MusicSubsystem::HandleStageChanged);
		}
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &UT66MusicSubsystem::HandleSettingsChanged);
		}
	}

	// Best-effort immediate start (covers cases where delegates fire before we bind),
	// but only if we can reliably identify the current map.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			if (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)
			{
				const FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);
				if (!LevelName.IsEmpty())
				{
					HandlePostLoadMap(World);
				}
			}
		}
	}
}

void UT66MusicSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			Shutdown->UnregisterParticipant(ShutdownParticipantHandle);
		}
	}
	ShutdownParticipantHandle.Reset();
	ShutdownRuntimeResources(TEXT("Deinitialize"));
	Super::Deinitialize();
}

bool UT66MusicSubsystem::HandleShutdown(const FT66ShutdownContext& /*Context*/)
{
	ShutdownRuntimeResources(TEXT("ShutdownSystem"));
	return true;
}

void UT66MusicSubsystem::ShutdownRuntimeResources(const TCHAR* /*Reason*/)
{
	if (PostWorldInitHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitHandle);
		PostWorldInitHandle.Reset();
	}
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->BossChanged.RemoveDynamic(this, &UT66MusicSubsystem::HandleBossChanged);
			RunState->StageChanged.RemoveDynamic(this, &UT66MusicSubsystem::HandleStageChanged);
		}
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &UT66MusicSubsystem::HandleSettingsChanged);
		}
	}

	StopMainTheme(0.0f);
	StopTheme(0.0f);
	StopBoss(0.0f);
	StopArea(0.0f);
	AreaMusicStack.Reset();
	ActiveAreaID = NAME_None;
	bStingerActive = false;

	if (MainThemeComp)
	{
		MainThemeComp->OnAudioFinished.RemoveDynamic(this, &UT66MusicSubsystem::HandleMainThemeFinished);
		MainThemeComp->Stop();
		MainThemeComp = nullptr;
	}
	if (ThemeComp)
	{
		ThemeComp->OnAudioFinished.RemoveDynamic(this, &UT66MusicSubsystem::HandleThemeFinished);
		ThemeComp->Stop();
		ThemeComp = nullptr;
	}
	if (BossComp)
	{
		BossComp->OnAudioFinished.RemoveDynamic(this, &UT66MusicSubsystem::HandleBossFinished);
		BossComp->Stop();
		BossComp = nullptr;
	}
	if (AreaComp)
	{
		AreaComp->OnAudioFinished.RemoveDynamic(this, &UT66MusicSubsystem::HandleAreaFinished);
		AreaComp->Stop();
		AreaComp = nullptr;
	}
	if (StingerComp)
	{
		StingerComp->OnAudioFinished.RemoveDynamic(this, &UT66MusicSubsystem::HandleStingerFinished);
		StingerComp->Stop();
		StingerComp = nullptr;
	}

	if (MainThemeLoadHandle.IsValid())
	{
		MainThemeLoadHandle->CancelHandle();
		MainThemeLoadHandle.Reset();
	}
	if (ThemeLoadHandle.IsValid())
	{
		ThemeLoadHandle->CancelHandle();
		ThemeLoadHandle.Reset();
	}
	for (TPair<FString, TSharedPtr<FStreamableHandle>>& Pair : PendingFolderSoundLoads)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->CancelHandle();
		}
	}
	PendingFolderSoundLoads.Reset();
	FolderSoundCandidatePaths.Reset();
	CachedFolderSounds.Reset();
	WarnedMissingFolderSounds.Reset();
}

void UT66MusicSubsystem::HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues)
{
	if (!World) return;

	// Only respond to the world owned by this game instance (prevents editor-world noise).
	if (World->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	// Avoid editor preview worlds.
	if (!(World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
	{
		return;
	}

	// Do nothing here; map-load hook is more reliable for PIE start timing.
}

void UT66MusicSubsystem::HandlePostLoadMap(UWorld* World)
{
	if (!World) return;
	if (World->GetGameInstance() != GetGameInstance()) return;
	if (!(World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)) return;

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);
	if (LevelName.IsEmpty())
	{
		return;
	}

	// Area requests belong to actors of the previous world; they re-push on overlap in the new one.
	AreaMusicStack.Reset();
	ActiveAreaID = NAME_None;

	// Components spawned in the previous world are dead with it — drop them BEFORE any
	// Play/Stop/IsPlaying call so the Ensure* paths respawn fresh ones in this world.
	ResetAudioComponentsForWorldChange(World);

	// Switch base music depending on where we are (mutually exclusive).
	DesiredBaseTrack = IsFrontendWorld(World) ? ET66BaseTrack::MainTheme : ET66BaseTrack::Theme;

	// DEFER to the new world's first tick: starting audio inside the LoadMap broadcast spawned
	// sound components against a mid-transition audio device (GI->GetWorld() can still name the
	// dying world here) — both Enter-the-Tribulation fatals. One frame later, the world and its
	// audio device are fully live.
	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &UT66MusicSubsystem::UpdateMusicState));
}

void UT66MusicSubsystem::ResetAudioComponentsForWorldChange(UWorld* NewWorld)
{
	auto DropIfForeign = [NewWorld](TObjectPtr<UAudioComponent>& Comp)
	{
		if (Comp && (!IsValid(Comp) || Comp->GetWorld() != NewWorld))
		{
			// Do NOT Stop()/FadeOut() — the old world's audio device is already gone; the engine
			// tore the playback down with the world. Just forget the component.
			Comp = nullptr;
		}
	};
	DropIfForeign(MainThemeComp);
	DropIfForeign(ThemeComp);
	DropIfForeign(BossComp);
	DropIfForeign(AreaComp);
	DropIfForeign(StingerComp);
	if (!ThemeComp) { bThemeStarted = false; }
	if (!MainThemeComp) { bMainThemeStarted = false; }
	bBossMusicActive = BossComp != nullptr && bBossMusicActive;
	bAreaMusicActive = AreaComp != nullptr && bAreaMusicActive;
	bStingerActive = StingerComp != nullptr && bStingerActive;
}

void UT66MusicSubsystem::HandleBossChanged()
{
	UpdateMusicState();
}

void UT66MusicSubsystem::HandleStageChanged()
{
	UpdateMusicState();
}

void UT66MusicSubsystem::PushAreaMusic(FName AreaID)
{
	if (AreaID.IsNone()) return;
	AreaMusicStack.Remove(AreaID);
	AreaMusicStack.Add(AreaID);
	UpdateMusicState();
}

void UT66MusicSubsystem::PopAreaMusic(FName AreaID)
{
	if (AreaID.IsNone()) return;
	if (AreaMusicStack.Remove(AreaID) > 0)
	{
		UpdateMusicState();
	}
}

void UT66MusicSubsystem::PlayStinger(FName StingerID)
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World || StingerID.IsNone()) return;

	const FString Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Stingers/%s"), *StingerID.ToString());
	USoundBase* Sound = ResolveFirstResidentSoundInFolder(Folder);
	if (!Sound)
	{
		// Not imported/resident yet (resolve queues an async preload for next time). Fire-and-forget cue.
		return;
	}

	bAllowMainThemeLoop = false;
	bAllowThemeLoop = false;
	bAllowBossLoop = false;
	bAllowAreaLoop = false;
	StopMainTheme(0.2f);
	StopTheme(0.2f);
	StopBoss(0.2f);
	StopArea(0.2f);

	if (!StingerComp)
	{
		StingerComp = UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, true, false);
		if (StingerComp)
		{
			StingerComp->bIsUISound = true;
			StingerComp->OnAudioFinished.AddDynamic(this, &UT66MusicSubsystem::HandleStingerFinished);
			bStingerActive = true;
			ApplyMusicVolumes();
		}
		return;
	}

	StingerComp->SetSound(Sound);
	bStingerActive = true;
	ApplyMusicVolumes();
	StingerComp->Play(0.0f);
}

void UT66MusicSubsystem::UpdateMusicState()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	UWorld* World = GI->GetWorld();
	if (!RunState || !World) return;
	// Never start/stop audio on a world that is going away (map transition window).
	if (World->bIsTearingDown)
	{
		return;
	}
	// Components from a previous world are dead with it; drop them so Ensure* respawns here.
	ResetAudioComponentsForWorldChange(World);

	// A one-shot stinger owns the music channel until it finishes.
	if (bStingerActive)
	{
		return;
	}

	// Boss music (for Espadas / Difficulty bosses / Special bosses).
	const bool bBossActive = RunState->GetBossActive();
	const FName ActiveBossID = RunState->GetActiveBossID();

	const bool bShouldBossMusic = bBossActive && !ActiveBossID.IsNone();
	if (bShouldBossMusic)
	{
		USoundBase* BossTrack = ResolveAndLoadBossThemeSound(World);
		if (BossTrack)
		{
			bBossMusicActive = true;
			bAllowBossLoop = true;
			bAllowMainThemeLoop = false;
			bAllowThemeLoop = false;
			bAllowAreaLoop = false;
			StopMainTheme(0.25f);
			StopTheme(0.25f);
			StopArea(0.25f);
			bAreaMusicActive = false;
			EnsureBossPlaying(World);
			ApplyMusicVolumes();
			return;
		}
	}

	if (bBossMusicActive)
	{
		bBossMusicActive = false;
		bAllowBossLoop = false;
		StopBoss(0.25f);
	}

	// Area override (NPC safe-zone bubbles etc.) outranks the base tracks.
	if (AreaMusicStack.Num() > 0)
	{
		USoundBase* AreaTrack = ResolveAndLoadAreaThemeSound();
		if (AreaTrack)
		{
			bAreaMusicActive = true;
			bAllowAreaLoop = true;
			bAllowMainThemeLoop = false;
			bAllowThemeLoop = false;
			StopMainTheme(0.25f);
			StopTheme(0.25f);
			EnsureAreaPlaying(World);
			ApplyMusicVolumes();
			return;
		}
	}

	if (bAreaMusicActive)
	{
		bAreaMusicActive = false;
		bAllowAreaLoop = false;
		StopArea(0.25f);
	}

	switch (DesiredBaseTrack)
	{
	case ET66BaseTrack::MainTheme:
		bAllowMainThemeLoop = true;
		bAllowThemeLoop = false;
		EnsureMainThemePlaying(World);
		break;
	case ET66BaseTrack::Theme:
		bAllowMainThemeLoop = false;
		bAllowThemeLoop = true;
		EnsureThemePlaying(World);
		break;
	default:
		break;
	}

	ApplyMusicVolumes();
}

void UT66MusicSubsystem::HandleSettingsChanged()
{
	ApplyMusicVolumes();
}

void UT66MusicSubsystem::ApplyMusicVolumes()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>();
	const float MasterVol = PS ? FMath::Clamp(PS->GetMasterVolume(), 0.f, 1.f) : 1.f;
	const float MusicVol = PS ? FMath::Clamp(PS->GetMusicVolume(), 0.f, 1.f) : 1.f;
	const float EffectiveMusic = MasterVol * MusicVol;

	if (MainThemeComp) MainThemeComp->SetVolumeMultiplier(EffectiveMusic);
	if (ThemeComp) ThemeComp->SetVolumeMultiplier(EffectiveMusic);
	if (BossComp) BossComp->SetVolumeMultiplier(EffectiveMusic);
	if (AreaComp) AreaComp->SetVolumeMultiplier(EffectiveMusic);
	if (StingerComp) StingerComp->SetVolumeMultiplier(EffectiveMusic);
}

bool UT66MusicSubsystem::IsFrontendWorld(UWorld* World) const
{
	if (!World) return false;
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);
	return LevelName.Equals(TEXT("FrontendLevel"), ESearchCase::IgnoreCase);
}

void UT66MusicSubsystem::EnsureMainThemePlaying(UWorld* World)
{
	if (!World) return;

	USoundBase* Sound = ResolveAndLoadMainThemeSound();
	if (!Sound)
	{
		if (!bMainThemeStarted && !MainThemeLoadHandle.IsValid())
		{
			UE_LOG(LogT66Music, Warning, TEXT("MainTheme not found. Import the MainTheme asset so one of these exists: /Game/Audio/OSTS/MainTheme, /Game/Audio/Music/MainTheme, /Game/Audio/MainTheme."));
			bMainThemeStarted = true; // avoid spam
		}
		return;
	}

	// Only one "base" track at a time.
	bAllowThemeLoop = false;
	StopTheme(0.15f);

	if (!MainThemeComp)
	{
		MainThemeComp = UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, true, false);
		if (MainThemeComp)
		{
			MainThemeComp->bIsUISound = true;
			MainThemeComp->OnAudioFinished.AddDynamic(this, &UT66MusicSubsystem::HandleMainThemeFinished);
			bMainThemeStarted = true;
		}
	}

	if (MainThemeComp && MainThemeComp->Sound != Sound)
	{
		MainThemeComp->SetSound(Sound);
	}

	if (MainThemeComp && !MainThemeComp->IsPlaying())
	{
		MainThemeComp->Play(0.0f);
	}

	ApplyMusicVolumes();
}

void UT66MusicSubsystem::EnsureThemePlaying(UWorld* World)
{
	if (!World) return;

	USoundBase* Sound = ResolveAndLoadGameplayThemeSound(World);
	if (!Sound)
	{
		if (!bThemeStarted && !ThemeLoadHandle.IsValid())
		{
			UE_LOG(LogT66Music, Warning, TEXT("Theme music not found. Import the Theme asset into UE (SoundWave/SoundCue) so one of these exists: /Game/Audio/OSTS/Theme, /Game/Audio/Music/Theme, /Game/Audio/Theme."));
			bThemeStarted = true; // avoid spam
		}
		return;
	}

	// Only one "base" track at a time.
	bAllowMainThemeLoop = false;
	StopMainTheme(0.15f);

	if (!ThemeComp)
	{
		ThemeComp = UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, true, false);
		if (ThemeComp)
		{
			ThemeComp->bIsUISound = true;
			ThemeComp->OnAudioFinished.AddDynamic(this, &UT66MusicSubsystem::HandleThemeFinished);
			bThemeStarted = true;
		}
	}

	if (ThemeComp && ThemeComp->Sound != Sound)
	{
		ThemeComp->SetSound(Sound);
	}

	if (ThemeComp && !ThemeComp->IsPlaying())
	{
		ThemeComp->Play(0.0f);
	}

	ApplyMusicVolumes();
}

void UT66MusicSubsystem::EnsureBossPlaying(UWorld* World)
{
	if (!World) return;
	USoundBase* Sound = ResolveAndLoadBossThemeSound(World);
	if (!Sound)
	{
		return;
	}

	if (!BossComp)
	{
		BossComp = UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, true, false);
		if (BossComp)
		{
			BossComp->bIsUISound = true;
			BossComp->OnAudioFinished.AddDynamic(this, &UT66MusicSubsystem::HandleBossFinished);
		}
	}

	if (BossComp)
	{
		// If the sound changed (new boss), restart with the new track.
		if (BossComp->Sound != Sound)
		{
			BossComp->SetSound(Sound);
		}
		if (!BossComp->IsPlaying())
		{
			BossComp->FadeIn(0.15f, 1.0f, 0.0f);
		}
	}
}

void UT66MusicSubsystem::EnsureAreaPlaying(UWorld* World)
{
	if (!World) return;
	USoundBase* Sound = ResolveAndLoadAreaThemeSound();
	if (!Sound)
	{
		return;
	}

	if (!AreaComp)
	{
		AreaComp = UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, true, false);
		if (AreaComp)
		{
			AreaComp->bIsUISound = true;
			AreaComp->OnAudioFinished.AddDynamic(this, &UT66MusicSubsystem::HandleAreaFinished);
		}
	}

	if (AreaComp)
	{
		// If the sound changed (different area), restart with the new track.
		if (AreaComp->Sound != Sound)
		{
			AreaComp->SetSound(Sound);
		}
		if (!AreaComp->IsPlaying())
		{
			AreaComp->FadeIn(0.25f, 1.0f, 0.0f);
		}
	}
}

void UT66MusicSubsystem::StopTheme(float FadeSeconds)
{
	if (!ThemeComp) return;
	bAllowThemeLoop = false;
	if (ThemeComp->IsPlaying())
	{
		ThemeComp->FadeOut(FMath::Max(0.f, FadeSeconds), 0.0f);
	}
}

void UT66MusicSubsystem::StopMainTheme(float FadeSeconds)
{
	if (!MainThemeComp) return;
	bAllowMainThemeLoop = false;
	if (MainThemeComp->IsPlaying())
	{
		MainThemeComp->FadeOut(FMath::Max(0.f, FadeSeconds), 0.0f);
	}
}

void UT66MusicSubsystem::StopBoss(float FadeSeconds)
{
	if (!BossComp) return;
	bAllowBossLoop = false;
	if (BossComp->IsPlaying())
	{
		BossComp->FadeOut(FMath::Max(0.f, FadeSeconds), 0.0f);
	}
}

void UT66MusicSubsystem::StopArea(float FadeSeconds)
{
	if (!AreaComp) return;
	bAllowAreaLoop = false;
	if (AreaComp->IsPlaying())
	{
		AreaComp->FadeOut(FMath::Max(0.f, FadeSeconds), 0.0f);
	}
}

void UT66MusicSubsystem::HandleAreaFinished()
{
	if (AreaComp && bAreaMusicActive && bAllowAreaLoop && AreaMusicStack.Num() > 0)
	{
		AreaComp->Play(0.0f);
	}
}

void UT66MusicSubsystem::HandleStingerFinished()
{
	if (!bStingerActive)
	{
		return;
	}
	bStingerActive = false;
	UpdateMusicState();
}

void UT66MusicSubsystem::HandleThemeFinished()
{
	// Poor-man looping without requiring a SoundCue asset.
	if (ThemeComp && bAllowThemeLoop && DesiredBaseTrack == ET66BaseTrack::Theme)
	{
		ThemeComp->Play(0.0f);
	}
}

void UT66MusicSubsystem::HandleMainThemeFinished()
{
	if (MainThemeComp && bAllowMainThemeLoop && DesiredBaseTrack == ET66BaseTrack::MainTheme)
	{
		MainThemeComp->Play(0.0f);
	}
}

void UT66MusicSubsystem::HandleBossFinished()
{
	if (BossComp && bBossMusicActive && bAllowBossLoop)
	{
		BossComp->Play(0.0f);
	}
}

USoundBase* UT66MusicSubsystem::ResolveAndLoadMainThemeSound()
{
	if (USoundBase* Existing = MainThemeSound.Get())
	{
		return Existing;
	}
	QueueMainThemePreload();
	return ResolveFirstResidentSoundAsset(MainThemeCandidates, MainThemeSound);
}

USoundBase* UT66MusicSubsystem::ResolveAndLoadThemeSound()
{
	if (USoundBase* Existing = ThemeSound.Get())
	{
		return Existing;
	}
	QueueThemePreload();
	return ResolveFirstResidentSoundAsset(ThemeCandidates, ThemeSound);
}

USoundBase* UT66MusicSubsystem::ResolveAndLoadGameplayThemeSound(UWorld* World)
{
	// Hero-specific theme folder (optional) overrides the default Theme.
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;

	FName HeroKey = NAME_None;
	if (T66GI && !T66GI->SelectedHeroID.IsNone())
	{
		FHeroData HeroData;
		if (T66GI->GetHeroData(T66GI->SelectedHeroID, HeroData))
		{
			HeroKey = !HeroData.MapTheme.IsNone() ? HeroData.MapTheme : HeroData.HeroID;
		}
		else
		{
			HeroKey = T66GI->SelectedHeroID;
		}
	}

	if (!HeroKey.IsNone())
	{
		const FString Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Heroes/%s"), *HeroKey.ToString());
		if (USoundBase* HeroTheme = ResolveFirstResidentSoundInFolder(Folder))
		{
			return HeroTheme;
		}
	}

	// Stage-specific theme folder (optional) overrides the default Theme.
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		const int32 Stage = RunState->GetCurrentStage();
		if (Stage > 0)
		{
			const FString Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Stages/Stage_%02d"), Stage);
			if (USoundBase* StageTheme = ResolveFirstResidentSoundInFolder(Folder))
			{
				return StageTheme;
			}
		}
	}

	// Fallback: project-wide Theme.
	return ResolveAndLoadThemeSound();
}

USoundBase* UT66MusicSubsystem::ResolveAndLoadBossThemeSound(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return nullptr;

	const FName BossID = RunState->GetActiveBossID();
	if (BossID.IsNone()) return nullptr;

	const FString BossIdStr = BossID.ToString();

	// Every boss resolves its own folder; drop a track in to give a boss unique music.
	const FString Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Bosses/%s"), *BossIdStr);
	if (USoundBase* BossTheme = ResolveFirstResidentSoundInFolder(Folder))
	{
		return BossTheme;
	}

	// Legacy layout: special bosses (Vendor/Ouroboros) under Bosses/Special/.
	const FString SpecialFolder = FString::Printf(TEXT("/Game/Audio/OSTS/Bosses/Special/%s"), *BossIdStr);
	if (USoundBase* BossTheme = ResolveFirstResidentSoundInFolder(SpecialFolder))
	{
		return BossTheme;
	}

	// If nothing in the folder yet, fall back to base theme (no override).
	return nullptr;
}

USoundBase* UT66MusicSubsystem::ResolveAndLoadAreaThemeSound()
{
	for (int32 Index = AreaMusicStack.Num() - 1; Index >= 0; --Index)
	{
		const FName AreaID = AreaMusicStack[Index];
		const FString Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Areas/%s"), *AreaID.ToString());
		if (USoundBase* AreaTheme = ResolveFirstResidentSoundInFolder(Folder))
		{
			ActiveAreaID = AreaID;
			return AreaTheme;
		}
	}
	ActiveAreaID = NAME_None;
	return nullptr;
}

void UT66MusicSubsystem::QueueBaseMusicPreloads()
{
	QueueMainThemePreload();
	QueueThemePreload();
}

void UT66MusicSubsystem::QueueStingerPreloads()
{
	// Stingers fire at moments that cannot wait for an async load (death/victory),
	// so warm them as soon as the subsystem exists.
	ResolveFirstResidentSoundInFolder(TEXT("/Game/Audio/OSTS/Stingers/Victory"));
	ResolveFirstResidentSoundInFolder(TEXT("/Game/Audio/OSTS/Stingers/Defeat"));
}

void UT66MusicSubsystem::QueueMainThemePreload()
{
	if (MainThemeLoadHandle.IsValid() || MainThemeSound.Get())
	{
		return;
	}

	MainThemeLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		MainThemeCandidates,
		FStreamableDelegate::CreateUObject(this, &UT66MusicSubsystem::HandleMainThemePreloaded));
}

void UT66MusicSubsystem::QueueThemePreload()
{
	if (ThemeLoadHandle.IsValid() || ThemeSound.Get())
	{
		return;
	}

	ThemeLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		ThemeCandidates,
		FStreamableDelegate::CreateUObject(this, &UT66MusicSubsystem::HandleThemePreloaded));
}

void UT66MusicSubsystem::HandleMainThemePreloaded()
{
	MainThemeLoadHandle.Reset();
	if (!ResolveFirstResidentSoundAsset(MainThemeCandidates, MainThemeSound))
	{
		UE_LOG(LogT66Music, Warning, TEXT("MainTheme music async preload completed but no candidate resolved as a USoundBase."));
	}
	UpdateMusicState();
}

void UT66MusicSubsystem::HandleThemePreloaded()
{
	ThemeLoadHandle.Reset();
	if (!ResolveFirstResidentSoundAsset(ThemeCandidates, ThemeSound))
	{
		UE_LOG(LogT66Music, Warning, TEXT("Theme music async preload completed but no candidate resolved as a USoundBase."));
	}
	UpdateMusicState();
}

USoundBase* UT66MusicSubsystem::ResolveFirstResidentSoundInFolder(const FString& FolderPath)
{
	if (FolderPath.IsEmpty())
	{
		return nullptr;
	}

	if (TObjectPtr<USoundBase>* CachedSound = CachedFolderSounds.Find(FolderPath))
	{
		if (T66IsSoundReadyToPlay(CachedSound->Get()))
		{
			return CachedSound->Get();
		}
		// GC nulled it (map transition) or it never finished loading — drop and re-resolve.
		CachedFolderSounds.Remove(FolderPath);
	}

	TArray<FSoftObjectPath> CandidatePaths = CollectSoundAssetPathsInFolder(FolderPath);
	for (const FSoftObjectPath& CandidatePath : CandidatePaths)
	{
		USoundBase* Sound = Cast<USoundBase>(CandidatePath.ResolveObject());
		if (T66IsSoundReadyToPlay(Sound))
		{
			CachedFolderSounds.Add(FolderPath, Sound);
			return Sound;
		}
	}

	QueueFolderSoundPreload(FolderPath, CandidatePaths);
	return nullptr;
}

void UT66MusicSubsystem::QueueFolderSoundPreload(const FString& FolderPath, const TArray<FSoftObjectPath>& CandidatePaths)
{
	if (FolderPath.IsEmpty() || CandidatePaths.Num() <= 0 || PendingFolderSoundLoads.Contains(FolderPath))
	{
		return;
	}

	FolderSoundCandidatePaths.Add(FolderPath, CandidatePaths);
	TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		CandidatePaths,
		FStreamableDelegate::CreateUObject(this, &UT66MusicSubsystem::HandleFolderSoundPreloaded, FolderPath));
	if (Handle.IsValid())
	{
		PendingFolderSoundLoads.Add(FolderPath, Handle);
	}
}

void UT66MusicSubsystem::HandleFolderSoundPreloaded(FString FolderPath)
{
	PendingFolderSoundLoads.Remove(FolderPath);

	TArray<FSoftObjectPath> CandidatePaths;
	FolderSoundCandidatePaths.RemoveAndCopyValue(FolderPath, CandidatePaths);
	for (const FSoftObjectPath& CandidatePath : CandidatePaths)
	{
		if (USoundBase* Sound = Cast<USoundBase>(CandidatePath.ResolveObject()))
		{
			CachedFolderSounds.Add(FolderPath, Sound);
			UpdateMusicState();
			return;
		}
	}

	if (!WarnedMissingFolderSounds.Contains(FolderPath))
	{
		WarnedMissingFolderSounds.Add(FolderPath);
		UE_LOG(LogT66Music, Warning, TEXT("Music folder async preload completed but no USoundBase resolved under %s."), *FolderPath);
	}
	UpdateMusicState();
}
