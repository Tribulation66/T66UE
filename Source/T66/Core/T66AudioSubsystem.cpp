// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66AudioSubsystem.h"

#include "Core/T66PlayerSettingsSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundConcurrency.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY(LogT66Audio);

namespace
{
	const TCHAR* T66AudioEventTablePath = TEXT("/Game/Data/DT_AudioEvents.DT_AudioEvents");
	const FName T66VolumeBusMusic(TEXT("Music"));
	const FName T66VolumeBusMaster(TEXT("Master"));
}

void UT66AudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadAudioEvents();
	PrimeConfiguredAssets();
}

void UT66AudioSubsystem::Deinitialize()
{
	AudioEventTable = nullptr;
	AudioEvents.Reset();
	CachedSounds.Reset();
	CachedAttenuations.Reset();
	CachedConcurrencies.Reset();
	LastPlayTimeByEvent.Reset();
	WarnedMissingSoundPaths.Reset();
	Super::Deinitialize();
}

void UT66AudioSubsystem::LoadAudioEvents()
{
	AudioEvents.Reset();

	AudioEventTable = LoadObject<UDataTable>(nullptr, T66AudioEventTablePath);
	if (!AudioEventTable)
	{
		UE_LOG(LogT66Audio, Warning, TEXT("Audio event table not found at %s."), T66AudioEventTablePath);
		return;
	}

	if (AudioEventTable->GetRowStruct() != FT66AudioEventRow::StaticStruct())
	{
		UE_LOG(LogT66Audio, Warning, TEXT("Audio event table %s uses row struct %s, expected %s."),
			T66AudioEventTablePath,
			AudioEventTable->GetRowStruct() ? *AudioEventTable->GetRowStruct()->GetName() : TEXT("<null>"),
			*FT66AudioEventRow::StaticStruct()->GetName());
		return;
	}

	for (const FName& RowName : AudioEventTable->GetRowNames())
	{
		const FT66AudioEventRow* Row = AudioEventTable->FindRow<FT66AudioEventRow>(RowName, TEXT("T66AudioSubsystem"));
		if (Row)
		{
			AudioEvents.Add(RowName, *Row);
		}
	}

	UE_LOG(LogT66Audio, Log, TEXT("Loaded %d audio event rows from %s."), AudioEvents.Num(), T66AudioEventTablePath);
}

void UT66AudioSubsystem::PrimeConfiguredAssets()
{
	for (const TPair<FName, FT66AudioEventRow>& Pair : AudioEvents)
	{
		const FT66AudioEventRow& Row = Pair.Value;
		if (!Row.bEnabled || !Row.bPrimeOnLoad)
		{
			continue;
		}

		for (const FSoftObjectPath& SoundPath : Row.SoundAssetPaths)
		{
			ResolveSound(SoundPath, false);
		}

		ResolveAttenuation(Row.AttenuationAssetPath);
		ResolveConcurrency(Row.ConcurrencyAssetPath);
	}
}

bool UT66AudioSubsystem::IsEventConfigured(const FName EventID) const
{
	const FT66AudioEventRow* Row = AudioEvents.Find(EventID);
	return Row && Row->bEnabled && Row->SoundAssetPaths.Num() > 0;
}

bool UT66AudioSubsystem::PlayEvent(FName EventID, UObject* WorldContextObject, const FVector Location, AActor* OwningActor)
{
	if (EventID.IsNone())
	{
		return false;
	}

	const FT66AudioEventRow* Row = AudioEvents.Find(EventID);
	if (!Row || !Row->bEnabled)
	{
		return false;
	}

	UWorld* World = ResolveWorld(WorldContextObject, OwningActor);
	if (!World || IsThrottled(EventID, *Row))
	{
		return false;
	}

	USoundBase* Sound = SelectSoundForRow(EventID, *Row);
	if (!Sound)
	{
		return false;
	}

	const float Volume = ResolveVolumeMultiplier(*Row);
	if (Volume <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float Pitch = ResolvePitchMultiplier(*Row);
	USoundConcurrency* Concurrency = ResolveConcurrency(Row->ConcurrencyAssetPath);

	if (Row->bPlay2D)
	{
		UGameplayStatics::PlaySound2D(World, Sound, Volume, Pitch, Row->StartTime, Concurrency, OwningActor, Row->bIsUISound);
		return true;
	}

	const FVector PlayLocation = OwningActor ? OwningActor->GetActorLocation() : Location;
	USoundAttenuation* Attenuation = ResolveAttenuation(Row->AttenuationAssetPath);
	UGameplayStatics::PlaySoundAtLocation(
		World,
		Sound,
		PlayLocation,
		FRotator::ZeroRotator,
		Volume,
		Pitch,
		Row->StartTime,
		Attenuation,
		Concurrency,
		OwningActor);
	return true;
}

bool UT66AudioSubsystem::PlayEventAtActor(const FName EventID, AActor* Actor)
{
	return Actor ? PlayEvent(EventID, Actor, Actor->GetActorLocation(), Actor) : false;
}

bool UT66AudioSubsystem::PlayEventFromWorldContext(UObject* WorldContextObject, const FName EventID, const FVector& Location, AActor* OwningActor)
{
	if (!WorldContextObject && !OwningActor)
	{
		return false;
	}

	UWorld* World = OwningActor ? OwningActor->GetWorld() : nullptr;
	if (!World && GEngine && WorldContextObject)
	{
		World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	}

	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66AudioSubsystem* Audio = GI ? GI->GetSubsystem<UT66AudioSubsystem>() : nullptr;
	return Audio ? Audio->PlayEvent(EventID, WorldContextObject ? WorldContextObject : OwningActor, Location, OwningActor) : false;
}

bool UT66AudioSubsystem::PlayEventAtActorFromWorldContext(UObject* WorldContextObject, const FName EventID, AActor* Actor)
{
	return Actor ? PlayEventFromWorldContext(WorldContextObject ? WorldContextObject : Actor, EventID, Actor->GetActorLocation(), Actor) : false;
}

bool UT66AudioSubsystem::PlayUIEventFromAnyWorld(const FName EventID)
{
	if (!GEngine)
	{
		return false;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (!World || (World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE))
		{
			continue;
		}

		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66AudioSubsystem* Audio = GI->GetSubsystem<UT66AudioSubsystem>())
			{
				if (Audio->PlayEvent(EventID, World, FVector::ZeroVector, nullptr))
				{
					return true;
				}
			}
		}
	}

	return false;
}

USoundBase* UT66AudioSubsystem::ResolveSound(const FSoftObjectPath& AssetPath, const bool bWarnOnFailure)
{
	if (AssetPath.IsNull())
	{
		return nullptr;
	}

	const FString AssetPathString = AssetPath.ToString();
	if (TObjectPtr<USoundBase>* Cached = CachedSounds.Find(AssetPathString))
	{
		return Cached->Get();
	}

	USoundBase* Sound = Cast<USoundBase>(AssetPath.TryLoad());
	if (!Sound)
	{
		if (bWarnOnFailure && !WarnedMissingSoundPaths.Contains(AssetPathString))
		{
			WarnedMissingSoundPaths.Add(AssetPathString);
			UE_LOG(LogT66Audio, Warning, TEXT("Audio sound asset missing or not a USoundBase: %s"), *AssetPathString);
		}
		return nullptr;
	}

	CachedSounds.Add(AssetPathString, Sound);
	return Sound;
}

USoundAttenuation* UT66AudioSubsystem::ResolveAttenuation(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsNull())
	{
		return nullptr;
	}

	const FString AssetPathString = AssetPath.ToString();
	if (TObjectPtr<USoundAttenuation>* Cached = CachedAttenuations.Find(AssetPathString))
	{
		return Cached->Get();
	}

	USoundAttenuation* Attenuation = Cast<USoundAttenuation>(AssetPath.TryLoad());
	if (Attenuation)
	{
		CachedAttenuations.Add(AssetPathString, Attenuation);
	}
	return Attenuation;
}

USoundConcurrency* UT66AudioSubsystem::ResolveConcurrency(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsNull())
	{
		return nullptr;
	}

	const FString AssetPathString = AssetPath.ToString();
	if (TObjectPtr<USoundConcurrency>* Cached = CachedConcurrencies.Find(AssetPathString))
	{
		return Cached->Get();
	}

	USoundConcurrency* Concurrency = Cast<USoundConcurrency>(AssetPath.TryLoad());
	if (Concurrency)
	{
		CachedConcurrencies.Add(AssetPathString, Concurrency);
	}
	return Concurrency;
}

USoundBase* UT66AudioSubsystem::SelectSoundForRow(const FName EventID, const FT66AudioEventRow& Row)
{
	const int32 NumSounds = Row.SoundAssetPaths.Num();
	if (NumSounds <= 0)
	{
		return nullptr;
	}

	const int32 StartIndex = NumSounds > 1 ? FMath::RandRange(0, NumSounds - 1) : 0;
	for (int32 Offset = 0; Offset < NumSounds; ++Offset)
	{
		const int32 Index = (StartIndex + Offset) % NumSounds;
		if (USoundBase* Sound = ResolveSound(Row.SoundAssetPaths[Index], true))
		{
			return Sound;
		}
	}

	UE_LOG(LogT66Audio, Warning, TEXT("Audio event %s has no loadable sound assets."), *EventID.ToString());
	return nullptr;
}

float UT66AudioSubsystem::ResolveVolumeMultiplier(const FT66AudioEventRow& Row) const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66PlayerSettingsSubsystem* Settings = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const float MasterVolume = Settings ? FMath::Clamp(Settings->GetMasterVolume(), 0.f, 1.f) : 1.f;
	float BusVolume = 1.f;

	if (Settings && Row.VolumeBus == T66VolumeBusMusic)
	{
		BusVolume = FMath::Clamp(Settings->GetMusicVolume(), 0.f, 1.f);
	}
	else if (Settings && Row.VolumeBus != T66VolumeBusMaster)
	{
		BusVolume = FMath::Clamp(Settings->GetSfxVolume(), 0.f, 1.f);
	}

	return FMath::Max(0.f, Row.VolumeMultiplier) * MasterVolume * BusVolume;
}

float UT66AudioSubsystem::ResolvePitchMultiplier(const FT66AudioEventRow& Row) const
{
	const float RandomOffset = Row.PitchRandomRange > 0.f
		? FMath::FRandRange(-Row.PitchRandomRange, Row.PitchRandomRange)
		: 0.f;
	return FMath::Max(0.01f, Row.PitchMultiplier + RandomOffset);
}

bool UT66AudioSubsystem::IsThrottled(const FName EventID, const FT66AudioEventRow& Row)
{
	if (Row.MinReplayIntervalSeconds <= 0.f)
	{
		return false;
	}

	const double Now = FPlatformTime::Seconds();
	if (const double* LastTime = LastPlayTimeByEvent.Find(EventID))
	{
		if ((Now - *LastTime) < static_cast<double>(Row.MinReplayIntervalSeconds))
		{
			return true;
		}
	}

	LastPlayTimeByEvent.Add(EventID, Now);
	return false;
}

UWorld* UT66AudioSubsystem::ResolveWorld(UObject* WorldContextObject, AActor* OwningActor) const
{
	if (OwningActor)
	{
		return OwningActor->GetWorld();
	}

	if (GEngine && WorldContextObject)
	{
		return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	}

	return nullptr;
}
