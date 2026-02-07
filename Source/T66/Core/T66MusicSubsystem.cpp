// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MusicSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

// NOTE: This file must remain in the runtime module so the UHT-generated glue links correctly.

static USoundBase* TryLoadFirstSoundAsset(const TArray<FSoftObjectPath>& Candidates, TSoftObjectPtr<USoundBase>& InOutSoftPtr)
{
	for (const FSoftObjectPath& P : Candidates)
	{
		if (!P.IsValid()) continue;
		if (UObject* Obj = P.TryLoad())
		{
			if (USoundBase* SB = Cast<USoundBase>(Obj))
			{
				InOutSoftPtr = TSoftObjectPtr<USoundBase>(P);
				return SB;
			}
		}
	}
	return nullptr;
}

static USoundBase* TryLoadFirstSoundInFolder(const FString& FolderPath)
{
	// FolderPath should be like "/Game/Audio/OSTS/Heroes/Hero_Example"
	if (FolderPath.IsEmpty())
	{
		return nullptr;
	}

	const FName PathName(*FolderPath);
	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> Assets;
	ARM.Get().GetAssetsByPath(PathName, Assets, /*bRecursive=*/true);
	if (Assets.Num() <= 0)
	{
		return nullptr;
	}

	// Deterministic selection: alphabetical by asset name.
	Assets.Sort([](const FAssetData& A, const FAssetData& B)
	{
		return A.AssetName.LexicalLess(B.AssetName);
	});

	for (const FAssetData& AD : Assets)
	{
		UObject* Obj = AD.GetAsset();
		if (USoundBase* SB = Cast<USoundBase>(Obj))
		{
			return SB;
		}
	}
	return nullptr;
}

void UT66MusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Ensure dependent subsystems exist so delegates/volume reads work reliably.
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
	SurvivalCandidates = {
		FSoftObjectPath(TEXT("/Game/Audio/OSTS/Survival.Survival")),
		FSoftObjectPath(TEXT("/Game/Audio/Music/Survival.Survival")),
		FSoftObjectPath(TEXT("/Game/Audio/Survival.Survival")),
	};

	// Listen for world creation so we can start Theme even on FrontendLevel.
	PostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UT66MusicSubsystem::HandlePostWorldInit);
	// Reliable for PIE: map-load hook (fires after the PIE map is loaded).
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UT66MusicSubsystem::HandlePostLoadMap);

	// Listen for last-stand changes.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->SurvivalChanged.AddDynamic(this, &UT66MusicSubsystem::HandleSurvivalChanged);
			RunState->BossChanged.AddDynamic(this, &UT66MusicSubsystem::HandleBossChanged);
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
			RunState->SurvivalChanged.RemoveDynamic(this, &UT66MusicSubsystem::HandleSurvivalChanged);
			RunState->BossChanged.RemoveDynamic(this, &UT66MusicSubsystem::HandleBossChanged);
		}
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &UT66MusicSubsystem::HandleSettingsChanged);
		}
	}

	Super::Deinitialize();
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

	// Switch base music depending on where we are (mutually exclusive).
	DesiredBaseTrack = IsFrontendWorld(World) ? ET66BaseTrack::MainTheme : ET66BaseTrack::Theme;
	UpdateMusicState();
}

void UT66MusicSubsystem::HandleSurvivalChanged()
{
	UpdateMusicState();
}

void UT66MusicSubsystem::HandleBossChanged()
{
	UpdateMusicState();
}

void UT66MusicSubsystem::UpdateMusicState()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	UWorld* World = GI->GetWorld();
	if (!RunState || !World) return;

	// Boss music (for Espadas / Difficulty bosses / Special bosses).
	const bool bBossActive = RunState->GetBossActive();
	const FName ActiveBossID = RunState->GetActiveBossID();

	const bool bShouldSurvival =
		RunState->IsInLastStand()
		&& (RunState->GetCurrentHearts() <= 0)
		&& (RunState->GetLastStandSecondsRemaining() > 0.f);

	if (bShouldSurvival)
	{
		if (!bSurvivalActive)
		{
			bSurvivalActive = true;
			bBossMusicActive = false;
			bAllowMainThemeLoop = false;
			bAllowThemeLoop = false;
			bAllowSurvivalLoop = true;
			bAllowBossLoop = false;
			StopMainTheme(0.25f);
			StopTheme(0.25f);
			StopBoss(0.15f);
			EnsureSurvivalPlaying(World);
		}
	}
	else
	{
		if (bSurvivalActive)
		{
			bSurvivalActive = false;
			bAllowSurvivalLoop = false;
			StopSurvival(0.25f);
		}

		// Boss music has priority over base tracks (but below survival).
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
				StopMainTheme(0.25f);
				StopTheme(0.25f);
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

		// Base music should play only when not in survival.
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
	if (SurvivalComp) SurvivalComp->SetVolumeMultiplier(EffectiveMusic);
	if (BossComp) BossComp->SetVolumeMultiplier(EffectiveMusic);
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
		if (!bMainThemeStarted)
		{
			UE_LOG(LogTemp, Warning, TEXT("MainTheme not found. Import the MainTheme asset so one of these exists: /Game/Audio/OSTS/MainTheme, /Game/Audio/Music/MainTheme, /Game/Audio/MainTheme."));
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

	if (MainThemeComp && !MainThemeComp->IsPlaying() && !bSurvivalActive)
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
		if (!bThemeStarted)
		{
			UE_LOG(LogTemp, Warning, TEXT("Theme music not found. Import the Theme asset into UE (SoundWave/SoundCue) so one of these exists: /Game/Audio/OSTS/Theme, /Game/Audio/Music/Theme, /Game/Audio/Theme."));
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

	if (ThemeComp && !ThemeComp->IsPlaying() && !bSurvivalActive)
	{
		ThemeComp->Play(0.0f);
	}

	ApplyMusicVolumes();
}

void UT66MusicSubsystem::EnsureSurvivalPlaying(UWorld* World)
{
	if (!World) return;

	USoundBase* Sound = ResolveAndLoadSurvivalSound();
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Survival music not found. Import the Survival asset into UE (SoundWave/SoundCue) so one of these exists: /Game/Audio/OSTS/Survival, /Game/Audio/Music/Survival, /Game/Audio/Survival."));
		return;
	}

	if (!SurvivalComp)
	{
		SurvivalComp = UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, true, false);
		if (SurvivalComp)
		{
			SurvivalComp->bIsUISound = true;
			SurvivalComp->OnAudioFinished.AddDynamic(this, &UT66MusicSubsystem::HandleSurvivalFinished);
		}
	}

	if (SurvivalComp && !SurvivalComp->IsPlaying())
	{
		SurvivalComp->FadeIn(0.25f, 1.0f, 0.0f);
	}
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

void UT66MusicSubsystem::StopSurvival(float FadeSeconds)
{
	if (!SurvivalComp) return;
	bAllowSurvivalLoop = false;
	if (SurvivalComp->IsPlaying())
	{
		SurvivalComp->FadeOut(FMath::Max(0.f, FadeSeconds), 0.0f);
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

void UT66MusicSubsystem::HandleThemeFinished()
{
	// Poor-man looping without requiring a SoundCue asset.
	if (ThemeComp && !bSurvivalActive && bAllowThemeLoop && DesiredBaseTrack == ET66BaseTrack::Theme)
	{
		ThemeComp->Play(0.0f);
	}
}

void UT66MusicSubsystem::HandleMainThemeFinished()
{
	if (MainThemeComp && !bSurvivalActive && bAllowMainThemeLoop && DesiredBaseTrack == ET66BaseTrack::MainTheme)
	{
		MainThemeComp->Play(0.0f);
	}
}

void UT66MusicSubsystem::HandleSurvivalFinished()
{
	if (SurvivalComp && bSurvivalActive && bAllowSurvivalLoop)
	{
		SurvivalComp->Play(0.0f);
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
	if (USoundBase* Loaded = MainThemeSound.LoadSynchronous())
	{
		return Loaded;
	}
	return TryLoadFirstSoundAsset(MainThemeCandidates, MainThemeSound);
}

USoundBase* UT66MusicSubsystem::ResolveAndLoadThemeSound()
{
	if (USoundBase* Existing = ThemeSound.Get())
	{
		return Existing;
	}
	if (USoundBase* Loaded = ThemeSound.LoadSynchronous())
	{
		return Loaded;
	}
	return TryLoadFirstSoundAsset(ThemeCandidates, ThemeSound);
}

USoundBase* UT66MusicSubsystem::ResolveAndLoadSurvivalSound()
{
	if (USoundBase* Existing = SurvivalSound.Get())
	{
		return Existing;
	}
	if (USoundBase* Loaded = SurvivalSound.LoadSynchronous())
	{
		return Loaded;
	}
	return TryLoadFirstSoundAsset(SurvivalCandidates, SurvivalSound);
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
		const FString Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Heros/%s"), *HeroKey.ToString());
		if (USoundBase* HeroTheme = TryLoadFirstSoundInFolder(Folder))
		{
			return HeroTheme;
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

	const int32 Stage = RunState->GetCurrentStage();

	// Categorize by stage number + explicit special boss IDs.
	const FString BossIdStr = BossID.ToString();
	const bool bSpecial =
		BossID == FName(TEXT("VendorBoss")) ||
		BossID == FName(TEXT("GamblerBoss")) ||
		BossID == FName(TEXT("OuroborosBoss")) ||
		BossIdStr.Contains(TEXT("Vendor"), ESearchCase::IgnoreCase) ||
		BossIdStr.Contains(TEXT("Gambler"), ESearchCase::IgnoreCase) ||
		BossIdStr.Contains(TEXT("Ouroboros"), ESearchCase::IgnoreCase);

	const bool bEspada = (!bSpecial) && (Stage > 0) && ((Stage % 10) == 5);
	const bool bDifficultyBoss = (!bSpecial) && (Stage > 0) && ((Stage % 10) == 0);

	// Stage bosses: no special music.
	if (!bSpecial && !bEspada && !bDifficultyBoss)
	{
		return nullptr;
	}

	FString Folder;
	if (bSpecial)
	{
		Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Bosses/Special/%s"), *BossID.ToString());
	}
	else if (bEspada)
	{
		Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Bosses/Espadas/%s"), *BossID.ToString());
	}
	else
	{
		Folder = FString::Printf(TEXT("/Game/Audio/OSTS/Bosses/Difficulty/%s"), *BossID.ToString());
	}

	if (USoundBase* BossTheme = TryLoadFirstSoundInFolder(Folder))
	{
		return BossTheme;
	}

	// If nothing in the folder yet, fall back to base theme (no override).
	return nullptr;
}

