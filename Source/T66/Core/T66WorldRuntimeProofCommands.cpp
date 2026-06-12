// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66WorldRuntimeProofTypes.h"

#if !UE_BUILD_SHIPPING

#include "Core/T66GameInstance.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Gameplay/T66BossHazardSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66MobLootSubsystem.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66OutgoingTravelerPoolSubsystem.h"
#include "Gameplay/T66ProjectileManagerSubsystem.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "NiagaraComponent.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66WorldRuntimeProof, Log, All);

namespace
{
struct FT66WorldRuntimeTravelProofState
{
	FString ManifestPath;
	FString Status = TEXT("running");
	int32 RequestedTravelCount = 2;
	int32 CompletedTravelCount = 0;
	int32 ExitCode = 0;
	bool bExitOnComplete = false;
	float CaptureDelaySeconds = 0.75f;
	TArray<TSharedPtr<FJsonValue>> Snapshots;
	FDelegateHandle PostLoadMapHandle;
	FTimerHandle CaptureTimerHandle;
	FTimerHandle StartTravelTimerHandle;
	bool bStress = false;
	bool bWaitingForInitialStressGameplay = false;
	int32 StressCount = 6;
	float StressSettleSeconds = 0.35f;
	TSharedPtr<FJsonObject> StressPopulation;
};

TUniquePtr<FT66WorldRuntimeTravelProofState> GTravelProofState;

FString T66WorldRuntimeProofDefaultPath(const TCHAR* Prefix)
{
	const FString Timestamp = FDateTime::UtcNow().ToString(TEXT("%Y%m%dT%H%M%SZ"));
	return FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("WorldRuntimeProof"),
		FString::Printf(TEXT("%s_%s.json"), Prefix ? Prefix : TEXT("manifest"), *Timestamp));
}

FString T66FindArgValue(const TArray<FString>& Args, const TCHAR* Key, const FString& DefaultValue = FString())
{
	for (const FString& Arg : Args)
	{
		FString ParsedKey;
		FString ParsedValue;
		if (Arg.Split(TEXT("="), &ParsedKey, &ParsedValue) && ParsedKey.Equals(Key, ESearchCase::IgnoreCase))
		{
			return ParsedValue.TrimQuotes();
		}
	}
	return DefaultValue;
}

int32 T66FindArgInt(const TArray<FString>& Args, const TCHAR* Key, const int32 DefaultValue)
{
	const FString Value = T66FindArgValue(Args, Key);
	if (Value.IsEmpty())
	{
		return DefaultValue;
	}

	int32 Parsed = DefaultValue;
	LexTryParseString(Parsed, *Value);
	return Parsed;
}

float T66FindArgFloat(const TArray<FString>& Args, const TCHAR* Key, const float DefaultValue)
{
	const FString Value = T66FindArgValue(Args, Key);
	if (Value.IsEmpty())
	{
		return DefaultValue;
	}

	float Parsed = DefaultValue;
	LexTryParseString(Parsed, *Value);
	return Parsed;
}

bool T66FindArgBool(const TArray<FString>& Args, const TCHAR* Key, const bool bDefaultValue)
{
	const FString Value = T66FindArgValue(Args, Key);
	if (Value.IsEmpty())
	{
		return bDefaultValue;
	}

	return Value.Equals(TEXT("1"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("true"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("yes"), ESearchCase::IgnoreCase);
}

FString T66WorldTypeString(const EWorldType::Type WorldType)
{
	switch (WorldType)
	{
	case EWorldType::None:
		return TEXT("None");
	case EWorldType::Game:
		return TEXT("Game");
	case EWorldType::Editor:
		return TEXT("Editor");
	case EWorldType::PIE:
		return TEXT("PIE");
	case EWorldType::EditorPreview:
		return TEXT("EditorPreview");
	case EWorldType::GamePreview:
		return TEXT("GamePreview");
	case EWorldType::GameRPC:
		return TEXT("GameRPC");
	case EWorldType::Inactive:
		return TEXT("Inactive");
	default:
		return TEXT("Unknown");
	}
}

FString T66WorldPointerString(const UWorld* World)
{
	return FString::Printf(TEXT("%p"), World);
}

TArray<TSharedPtr<FJsonValue>> T66StringArrayJson(const TArray<FString>& Values)
{
	TArray<TSharedPtr<FJsonValue>> Result;
	for (const FString& Value : Values)
	{
		Result.Add(MakeShared<FJsonValueString>(Value));
	}
	return Result;
}

TSharedPtr<FJsonObject> T66RuntimeSnapshotToJson(const FT66WorldRuntimeDebugSnapshot& Snapshot)
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("system"), Snapshot.SystemName);
	Json->SetBoolField(TEXT("present"), true);

	TSharedPtr<FJsonObject> Counters = MakeShared<FJsonObject>();
	for (const TPair<FString, int64>& Pair : Snapshot.Counters)
	{
		Counters->SetNumberField(Pair.Key, static_cast<double>(Pair.Value));
	}
	Json->SetObjectField(TEXT("counters"), Counters);

	TSharedPtr<FJsonObject> Flags = MakeShared<FJsonObject>();
	for (const TPair<FString, bool>& Pair : Snapshot.Flags)
	{
		Flags->SetBoolField(Pair.Key, Pair.Value);
	}
	Json->SetObjectField(TEXT("flags"), Flags);

	TSharedPtr<FJsonObject> Evidence = MakeShared<FJsonObject>();
	for (const TPair<FString, FString>& Pair : Snapshot.Evidence)
	{
		Evidence->SetStringField(Pair.Key, Pair.Value);
	}
	Json->SetObjectField(TEXT("evidence"), Evidence);
	Json->SetArrayField(TEXT("measurement_gaps"), T66StringArrayJson(Snapshot.MeasurementGaps));
	Json->SetArrayField(TEXT("notes"), T66StringArrayJson(Snapshot.Notes));
	return Json;
}

TSharedPtr<FJsonObject> T66MissingSubsystemJson(const TCHAR* SystemName)
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("system"), SystemName ? SystemName : TEXT("Unknown"));
	Json->SetBoolField(TEXT("present"), false);
	Json->SetStringField(TEXT("evidence"), TEXT("Subsystem is not present on this world."));
	return Json;
}

template <typename TSubsystem>
void T66AddSubsystemSnapshot(UWorld* World, const TCHAR* SystemName, TArray<TSharedPtr<FJsonValue>>& OutSubsystems)
{
	if (!World)
	{
		OutSubsystems.Add(MakeShared<FJsonValueObject>(T66MissingSubsystemJson(SystemName)));
		return;
	}

	if (const TSubsystem* Subsystem = World->GetSubsystem<TSubsystem>())
	{
		OutSubsystems.Add(MakeShared<FJsonValueObject>(T66RuntimeSnapshotToJson(Subsystem->GetWorldRuntimeDebugSnapshot())));
		return;
	}

	OutSubsystems.Add(MakeShared<FJsonValueObject>(T66MissingSubsystemJson(SystemName)));
}

struct FT66GlobalWorldRuntimeCounts
{
	const UWorld* World = nullptr;
	FString WorldName;
	EWorldType::Type WorldType = EWorldType::None;
	int32 ActorCount = 0;
	int32 T66MobActorCount = 0;
	int32 RuntimeRenderHostActorCount = 0;
	int32 HismComponentCount = 0;
	int32 HismInstanceCount = 0;
	int32 NiagaraComponentCount = 0;
	int32 ActiveNiagaraComponentCount = 0;
};

bool T66IsRuntimeRenderHostActorName(const FString& ActorName)
{
	return ActorName.StartsWith(TEXT("T66MobLootPoolHost"))
		|| ActorName.StartsWith(TEXT("T66OutgoingTravelerPoolHost"))
		|| ActorName.StartsWith(TEXT("T66ProjectileManagerHost"))
		|| ActorName.StartsWith(TEXT("T66BossHazardRenderHost"));
}

FT66GlobalWorldRuntimeCounts& T66FindOrAddWorldCounts(TArray<FT66GlobalWorldRuntimeCounts>& Counts, const UWorld* World)
{
	for (FT66GlobalWorldRuntimeCounts& Entry : Counts)
	{
		if (Entry.World == World)
		{
			return Entry;
		}
	}

	FT66GlobalWorldRuntimeCounts& Entry = Counts.AddDefaulted_GetRef();
	Entry.World = World;
	Entry.WorldName = GetNameSafe(World);
	Entry.WorldType = World ? static_cast<EWorldType::Type>(World->WorldType) : EWorldType::None;
	return Entry;
}

TArray<TSharedPtr<FJsonValue>> T66BuildGlobalWorldCountsJson(const UWorld* CurrentWorld, int32& OutNonCurrentResourceCount)
{
	TArray<FT66GlobalWorldRuntimeCounts> Counts;
	OutNonCurrentResourceCount = 0;

	for (TObjectIterator<UWorld> It; It; ++It)
	{
		const UWorld* World = *It;
		if (!World || World->IsTemplate())
		{
			continue;
		}
		T66FindOrAddWorldCounts(Counts, World);
	}

	for (TObjectIterator<AActor> It; It; ++It)
	{
		const AActor* Actor = *It;
		if (!IsValid(Actor) || Actor->IsTemplate())
		{
			continue;
		}

		if (const UWorld* World = Actor->GetWorld())
		{
			FT66GlobalWorldRuntimeCounts& Entry = T66FindOrAddWorldCounts(Counts, World);
			++Entry.ActorCount;
			if (Cast<AT66MobBase>(Actor))
			{
				++Entry.T66MobActorCount;
			}
			if (T66IsRuntimeRenderHostActorName(Actor->GetName()))
			{
				++Entry.RuntimeRenderHostActorCount;
			}
		}
	}

	for (TObjectIterator<UHierarchicalInstancedStaticMeshComponent> It; It; ++It)
	{
		const UHierarchicalInstancedStaticMeshComponent* Component = *It;
		if (!IsValid(Component) || Component->IsTemplate())
		{
			continue;
		}

		if (const UWorld* World = Component->GetWorld())
		{
			FT66GlobalWorldRuntimeCounts& Entry = T66FindOrAddWorldCounts(Counts, World);
			++Entry.HismComponentCount;
			Entry.HismInstanceCount += Component->GetInstanceCount();
		}
	}

	for (TObjectIterator<UNiagaraComponent> It; It; ++It)
	{
		const UNiagaraComponent* Component = *It;
		if (!IsValid(Component) || Component->IsTemplate())
		{
			continue;
		}

		if (const UWorld* World = Component->GetWorld())
		{
			FT66GlobalWorldRuntimeCounts& Entry = T66FindOrAddWorldCounts(Counts, World);
			++Entry.NiagaraComponentCount;
			if (Component->IsActive())
			{
				++Entry.ActiveNiagaraComponentCount;
			}
		}
	}

	TArray<TSharedPtr<FJsonValue>> Rows;
	for (const FT66GlobalWorldRuntimeCounts& Entry : Counts)
	{
		const bool bCurrentWorld = Entry.World == CurrentWorld;
		const int32 ResourceCount =
			Entry.T66MobActorCount
			+ Entry.RuntimeRenderHostActorCount
			+ Entry.HismComponentCount
			+ Entry.NiagaraComponentCount;
		if (!bCurrentWorld)
		{
			OutNonCurrentResourceCount += ResourceCount;
		}

		TSharedPtr<FJsonObject> Row = MakeShared<FJsonObject>();
		Row->SetStringField(TEXT("world_name"), Entry.WorldName);
		Row->SetStringField(TEXT("world_ptr"), T66WorldPointerString(Entry.World));
		Row->SetStringField(TEXT("world_type"), T66WorldTypeString(Entry.WorldType));
		Row->SetBoolField(TEXT("is_current_world"), bCurrentWorld);
		Row->SetNumberField(TEXT("actor_count"), Entry.ActorCount);
		Row->SetNumberField(TEXT("t66_mob_actor_count"), Entry.T66MobActorCount);
		Row->SetNumberField(TEXT("runtime_render_host_actor_count"), Entry.RuntimeRenderHostActorCount);
		Row->SetNumberField(TEXT("hism_component_count"), Entry.HismComponentCount);
		Row->SetNumberField(TEXT("hism_instance_count"), Entry.HismInstanceCount);
		Row->SetNumberField(TEXT("niagara_component_count"), Entry.NiagaraComponentCount);
		Row->SetNumberField(TEXT("active_niagara_component_count"), Entry.ActiveNiagaraComponentCount);
		Row->SetNumberField(TEXT("proof_candidate_resource_count"), ResourceCount);
		Rows.Add(MakeShared<FJsonValueObject>(Row));
	}

	return Rows;
}

TSharedPtr<FJsonObject> T66BuildWorldRuntimeSnapshot(UWorld* World, const FString& Label, const int32 Sequence)
{
	TSharedPtr<FJsonObject> Snapshot = MakeShared<FJsonObject>();
	Snapshot->SetStringField(TEXT("label"), Label);
	Snapshot->SetNumberField(TEXT("sequence"), Sequence);
	Snapshot->SetStringField(TEXT("timestamp_utc"), FDateTime::UtcNow().ToIso8601());
	Snapshot->SetBoolField(TEXT("world_valid"), World != nullptr);
	Snapshot->SetStringField(TEXT("world_name"), GetNameSafe(World));
	Snapshot->SetStringField(TEXT("world_ptr"), T66WorldPointerString(World));
	Snapshot->SetStringField(TEXT("world_type"), World ? T66WorldTypeString(static_cast<EWorldType::Type>(World->WorldType)) : TEXT("Missing"));
	Snapshot->SetStringField(TEXT("map_name"), World ? World->GetMapName() : FString());
	Snapshot->SetNumberField(TEXT("time_seconds"), World ? World->GetTimeSeconds() : 0.0);
	Snapshot->SetNumberField(TEXT("real_time_seconds"), World ? World->GetRealTimeSeconds() : 0.0);

	TArray<TSharedPtr<FJsonValue>> Subsystems;
	T66AddSubsystemSnapshot<UT66MobManagerSubsystem>(World, TEXT("UT66MobManagerSubsystem"), Subsystems);
	T66AddSubsystemSnapshot<UT66MobLootSubsystem>(World, TEXT("UT66MobLootSubsystem"), Subsystems);
	T66AddSubsystemSnapshot<UT66ProjectileManagerSubsystem>(World, TEXT("UT66ProjectileManagerSubsystem"), Subsystems);
	T66AddSubsystemSnapshot<UT66BossHazardSubsystem>(World, TEXT("UT66BossHazardSubsystem"), Subsystems);
	T66AddSubsystemSnapshot<UT66OutgoingTravelerPoolSubsystem>(World, TEXT("UT66OutgoingTravelerPoolSubsystem"), Subsystems);
	T66AddSubsystemSnapshot<UT66PixelVFXSubsystem>(World, TEXT("UT66PixelVFXSubsystem"), Subsystems);
	Snapshot->SetArrayField(TEXT("candidate_subsystems"), Subsystems);

	int32 NonCurrentResourceCount = 0;
	Snapshot->SetArrayField(TEXT("global_world_counts"), T66BuildGlobalWorldCountsJson(World, NonCurrentResourceCount));
	Snapshot->SetNumberField(TEXT("non_current_world_proof_candidate_resource_count"), NonCurrentResourceCount);
	Snapshot->SetStringField(TEXT("status"), NonCurrentResourceCount > 0 ? TEXT("evidence_has_non_current_world_resources") : TEXT("no_non_current_world_resources_observed"));
	return Snapshot;
}

bool T66WriteWorldRuntimeManifest(const TSharedRef<FJsonObject>& Root, const FString& Path)
{
	FString JsonText;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		UE_LOG(LogT66WorldRuntimeProof, Warning, TEXT("[WorldRuntimeProof] Manifest serialization failed path=%s"), *Path);
		return false;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	const bool bWrote = FFileHelper::SaveStringToFile(JsonText, *Path);
	UE_LOG(LogT66WorldRuntimeProof, Display, TEXT("[WorldRuntimeProof] Manifest write status=%s path=%s"),
		bWrote ? TEXT("ok") : TEXT("write-failed"),
		*Path);
	return bWrote;
}

TSharedRef<FJsonObject> T66BuildManifestRoot(const FString& Mode, const FString& Status)
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("tool"), TEXT("T66WorldRuntimeProof"));
	Root->SetStringField(TEXT("mode"), Mode);
	Root->SetStringField(TEXT("status"), Status);
	Root->SetStringField(TEXT("timestamp_utc"), FDateTime::UtcNow().ToIso8601());
	Root->SetStringField(TEXT("contract"), TEXT("Evidence-only repeated world travel proof for owner-local teardown candidates; does not drain, destroy, or coordinate runtime systems."));
	return Root;
}

AT66HeroBase* T66ResolveLocalHero(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		return Cast<AT66HeroBase>(PlayerController->GetPawn());
	}

	return nullptr;
}

FVector T66ResolveStressOrigin(UWorld* World)
{
	if (const AT66HeroBase* Hero = T66ResolveLocalHero(World))
	{
		return Hero->GetActorLocation();
	}

	return FVector::ZeroVector;
}

void T66ResolveMobStressDefaults(UWorld* World, int32& OutStageNum, float& OutDifficultyScalar, float& OutEnemyProgressionScalar, float& OutFinaleScalar)
{
	OutStageNum = 1;
	OutDifficultyScalar = 1.f;
	OutEnemyProgressionScalar = 1.f;
	OutFinaleScalar = 1.f;

	if (UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
		{
			OutStageNum = FMath::Max(1, RunState->GetCurrentStage());
			OutDifficultyScalar = RunState->GetDifficultyScalar();
			OutFinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
		}
	}
}

AT66MobBase* T66SpawnStressMob(UWorld* World, const FName MobID, const ET66EnemyFamily Family, const FVector& SpawnLocation)
{
	if (!World)
	{
		return nullptr;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	AT66MobBase* Mob = World->SpawnActorDeferred<AT66MobBase>(AT66MobBase::StaticClass(), SpawnTransform);
	if (!Mob)
	{
		return nullptr;
	}

	Mob->Tags.AddUnique(FName(TEXT("T66WorldRuntimeStressMob")));
	Mob->MobID = MobID;
	Mob->CharacterVisualID = MobID;
	Mob->LifecycleState = ET66MobLifecycleState::Active;
	Mob->FinishSpawning(SpawnTransform);

	int32 StageNum = 1;
	float DifficultyScalar = 1.f;
	float EnemyProgressionScalar = 1.f;
	float FinaleScalar = 1.f;
	T66ResolveMobStressDefaults(World, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar);
	Mob->ConfigureAsMob(MobID, Family, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
	Mob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 30.f);
	return Mob;
}

TSharedPtr<FJsonObject> T66PopulateWorldRuntimeStress(UWorld* World, const int32 RequestedCount)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("phase"), TEXT("active_runtime_stress_populate"));
	Result->SetBoolField(TEXT("world_valid"), World != nullptr);
	Result->SetStringField(TEXT("world_name"), GetNameSafe(World));
	Result->SetNumberField(TEXT("requested_count"), RequestedCount);

	if (!World || !World->IsGameWorld())
	{
		Result->SetStringField(TEXT("status"), TEXT("failed_missing_game_world"));
		return Result;
	}

	const int32 Count = FMath::Clamp(RequestedCount, 1, 24);
	const FVector Origin = T66ResolveStressOrigin(World);
	const FVector Forward = T66ResolveLocalHero(World)
		? T66ResolveLocalHero(World)->GetActorForwardVector().GetSafeNormal2D()
		: FVector::XAxisVector;
	const FVector Right = T66ResolveLocalHero(World)
		? T66ResolveLocalHero(World)->GetActorRightVector().GetSafeNormal2D()
		: FVector::YAxisVector;

	Result->SetStringField(TEXT("origin"), Origin.ToCompactString());

	int32 MobsSpawned = 0;
	if (World->GetSubsystem<UT66MobManagerSubsystem>())
	{
		static const FName MobIDs[] =
		{
			FName(TEXT("Slime")),
			FName(TEXT("RatPack")),
			FName(TEXT("CaveBat")),
			FName(TEXT("HexSlinger"))
		};
		static const ET66EnemyFamily Families[] =
		{
			ET66EnemyFamily::Melee,
			ET66EnemyFamily::Rush,
			ET66EnemyFamily::Flying,
			ET66EnemyFamily::Ranged
		};

		for (int32 Index = 0; Index < Count; ++Index)
		{
			const FVector SpawnLocation = Origin + Forward * 950.f + Right * static_cast<float>((Index - Count / 2) * 180) + FVector(0.f, 0.f, 80.f);
			if (T66SpawnStressMob(World, MobIDs[Index % UE_ARRAY_COUNT(MobIDs)], Families[Index % UE_ARRAY_COUNT(Families)], SpawnLocation))
			{
				++MobsSpawned;
			}
		}
	}
	Result->SetNumberField(TEXT("mobs_spawned"), MobsSpawned);

	int32 LootSpawned = 0;
	if (UT66MobLootSubsystem* MobLoot = World->GetSubsystem<UT66MobLootSubsystem>())
	{
		for (int32 Index = 0; Index < Count; ++Index)
		{
			FT66MobLootSpawnParams Params;
			Params.Position = Origin + Forward * 480.f + Right * static_cast<float>((Index - Count / 2) * 70) + FVector(0.f, 0.f, 25.f);
			Params.Quantity = 1 + (Index % 4);
			Params.GoldValue = Params.Quantity;
			Params.SourceID = FName(TEXT("WorldRuntimeStress"));
			Params.Color = FLinearColor(1.f, 0.78f, 0.18f, 1.f);
			Params.Scale = 1.0f;
			Params.LifetimeSeconds = 60.f;
			FT66MobLootHandle Handle;
			if (MobLoot->SpawnMobLoot(Params, Handle))
			{
				++LootSpawned;
			}
		}
	}
	Result->SetBoolField(TEXT("mob_loot_enabled"), UT66MobLootSubsystem::IsEnabled());
	Result->SetNumberField(TEXT("mob_loot_spawned"), LootSpawned);

	AActor* StressSourceActor = nullptr;
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("T66WorldRuntimeStressSource");
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		StressSourceActor = World->SpawnActor<AActor>(AActor::StaticClass(), Origin + FVector(0.f, 0.f, 5000.f), FRotator::ZeroRotator, SpawnParams);
		if (StressSourceActor)
		{
			StressSourceActor->SetActorHiddenInGame(true);
			StressSourceActor->SetActorEnableCollision(false);
			StressSourceActor->SetActorTickEnabled(false);
		}
	}
	Result->SetBoolField(TEXT("stress_source_actor_spawned"), StressSourceActor != nullptr);

	int32 ProjectilesFired = 0;
	if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
	{
		static const FName Profiles[] =
		{
			UT66ProjectileManagerSubsystem::DefaultEnemySpitVisualProfileID(),
			UT66ProjectileManagerSubsystem::EnemyWebVisualProfileID(),
			UT66ProjectileManagerSubsystem::EnemyWebNiagaraVisualProfileID()
		};

		for (int32 Index = 0; Index < Count; ++Index)
		{
			FT66ManagedProjectileFireParams Params;
			Params.SourceActor = StressSourceActor;
			Params.SourceID = FName(TEXT("WorldRuntimeStressProjectile"));
			Params.VisualProfileID = Profiles[Index % UE_ARRAY_COUNT(Profiles)];
			Params.Origin = Origin + FVector(-1800.f, static_cast<float>((Index - Count / 2) * 95), 2800.f);
			Params.Direction = FVector::XAxisVector;
			Params.Speed = 120.f;
			Params.Damage = 0.f;
			Params.Radius = 12.f;
			Params.Lifetime = 30.f;
			Params.Delivery = ET66ManagedProjectileDelivery::EnemyProjectile;
			if (ProjectileManager->FireManagedProjectile(Params))
			{
				++ProjectilesFired;
			}
		}
	}
	Result->SetNumberField(TEXT("projectiles_fired"), ProjectilesFired);

	int32 HazardsSpawned = 0;
	if (UT66BossHazardSubsystem* BossHazards = World->GetSubsystem<UT66BossHazardSubsystem>())
	{
		for (int32 Index = 0; Index < FMath::Min(Count, 8); ++Index)
		{
			FT66BossHazardSpawnParams Params;
			Params.SourceActor = StressSourceActor;
			Params.SourceID = FName(TEXT("WorldRuntimeStressHazard"));
			Params.HazardID = FName(TEXT("BossHazard.SlimePatch"));
			Params.Location = Origin + Forward * 1250.f + Right * static_cast<float>((Index - Count / 2) * 110);
			Params.RadiusScale = 0.75f;
			Params.TelegraphScale = 0.75f;
			Params.VisualScaleMultiplier = 0.75f;
			Params.DamageOverrideHP = 0;
			if (BossHazards->SpawnBossHazard(Params))
			{
				++HazardsSpawned;
			}
		}
	}
	Result->SetNumberField(TEXT("hazards_spawned"), HazardsSpawned);

	int32 TravelersFired = 0;
	if (UT66OutgoingTravelerPoolSubsystem* Travelers = World->GetSubsystem<UT66OutgoingTravelerPoolSubsystem>())
	{
		static const FName VisualProfiles[] =
		{
			FName(TEXT("TravelerVisual.Fire.AOE")),
			FName(TEXT("TravelerVisual.Ice.Summon")),
			FName(TEXT("TravelerVisual.Electricity.Bounce")),
			FName(TEXT("TravelerVisual.Nature.DOT"))
		};

		for (int32 Index = 0; Index < Count; ++Index)
		{
			FT66OutgoingTravelerFireParams Params;
			Params.ProfileID = FName(TEXT("EnemySpit"));
			Params.TravelerVisualProfileID = VisualProfiles[Index % UE_ARRAY_COUNT(VisualProfiles)];
			Params.StartPosition = Origin + FVector(-900.f, static_cast<float>((Index - Count / 2) * 80), 260.f);
			Params.TargetPosition = Params.StartPosition + FVector(4800.f, 0.f, 0.f);
			Params.Color = FLinearColor(0.45f, 0.85f, 1.f, 1.f);
			Params.ScaleMultiplier = 1.0f;
			Params.Speed = 90.f;
			Params.LifetimeSeconds = 45.f;
			Params.ArrivalRadius = 24.f;
			Params.DamageAmount = 0;
			Params.DamageSourceID = FName(TEXT("WorldRuntimeStressTraveler"));
			Params.EventType = FName(TEXT("WorldRuntimeStress"));
			Params.bTrackTarget = false;
			Params.bEnableArrivalCollision = false;
			Params.bApplyDamageOnArrival = false;
			FT66OutgoingTravelerHandle Handle;
			if (Travelers->FireOutgoingTraveler(Params, Handle))
			{
				++TravelersFired;
			}
		}
	}
	Result->SetNumberField(TEXT("travelers_fired"), TravelersFired);

	int32 PixelVFXSpawned = 0;
	if (UT66PixelVFXSubsystem* PixelVFX = World->GetSubsystem<UT66PixelVFXSubsystem>())
	{
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const FVector Location = Origin + FVector(0.f, static_cast<float>((Index - Count / 2) * 85), 180.f);
			if (PixelVFX->SpawnPixelAtLocation(
				Location,
				FLinearColor(0.4f, 1.f, 0.75f, 1.f),
				FVector2D(64.f, 64.f),
				ET66PixelVFXPriority::High,
				FRotator::ZeroRotator,
				FVector(1.f),
				nullptr,
				false,
				true))
			{
				++PixelVFXSpawned;
			}
		}
	}
	Result->SetNumberField(TEXT("pixel_vfx_spawned"), PixelVFXSpawned);

	Result->SetStringField(TEXT("status"), TEXT("complete"));
	return Result;
}

void T66RunWorldRuntimeSnapshotCommand(const TArray<FString>& Args, UWorld* World)
{
	FString Path = T66FindArgValue(Args, TEXT("Path"));
	if (Path.IsEmpty())
	{
		Path = T66WorldRuntimeProofDefaultPath(TEXT("world_runtime_snapshot"));
	}
	const FString Label = T66FindArgValue(Args, TEXT("Label"), TEXT("manual_snapshot"));
	const int32 ExitCode = T66FindArgInt(Args, TEXT("ExitCode"), 0);
	const bool bExit = T66FindArgBool(Args, TEXT("ExitOnComplete"), false);

	TSharedRef<FJsonObject> Root = T66BuildManifestRoot(TEXT("snapshot"), TEXT("complete"));
	TArray<TSharedPtr<FJsonValue>> Snapshots;
	Snapshots.Add(MakeShared<FJsonValueObject>(T66BuildWorldRuntimeSnapshot(World, Label, 0)));
	Root->SetArrayField(TEXT("snapshots"), Snapshots);
	Root->SetBoolField(TEXT("observer_only"), true);
	const bool bWrote = T66WriteWorldRuntimeManifest(Root, Path);
	if (bExit)
	{
		FPlatformMisc::RequestExitWithStatus(false, bWrote ? ExitCode : 74, TEXT("T66WorldRuntimeSnapshotComplete"));
	}
}

FString T66CurrentTravelTargetName(UWorld* World)
{
	const FString MapName = World ? World->GetMapName() : FString();
	const bool bCurrentGameplay = MapName.Contains(TEXT("GameplayLevel"), ESearchCase::IgnoreCase);
	return bCurrentGameplay
		? UT66GameInstance::GetFrontendLevelName().ToString()
		: UT66GameInstance::GetGameplayLevelName().ToString();
}

void T66WriteTravelProofManifest(const FString& Status)
{
	if (!GTravelProofState)
	{
		return;
	}

	TSharedRef<FJsonObject> Root = T66BuildManifestRoot(TEXT("proof_travel"), Status);
	Root->SetNumberField(TEXT("requested_travel_count"), GTravelProofState->RequestedTravelCount);
	Root->SetNumberField(TEXT("completed_travel_count"), GTravelProofState->CompletedTravelCount);
	Root->SetNumberField(TEXT("capture_delay_seconds"), GTravelProofState->CaptureDelaySeconds);
	Root->SetBoolField(TEXT("stress_enabled"), GTravelProofState->bStress);
	Root->SetNumberField(TEXT("stress_count"), GTravelProofState->StressCount);
	Root->SetNumberField(TEXT("stress_settle_seconds"), GTravelProofState->StressSettleSeconds);
	if (GTravelProofState->StressPopulation.IsValid())
	{
		Root->SetObjectField(TEXT("stress_population"), GTravelProofState->StressPopulation);
	}
	Root->SetBoolField(TEXT("observer_only"), true);
	Root->SetStringField(TEXT("assessment_policy"), TEXT("Evidence-only. Non-current-world resources are leak candidates, not automatic coordinator approval."));
	Root->SetArrayField(TEXT("snapshots"), GTravelProofState->Snapshots);
	T66WriteWorldRuntimeManifest(Root, GTravelProofState->ManifestPath);
}

void T66FinishTravelProof(const FString& Status)
{
	if (!GTravelProofState)
	{
		return;
	}

	if (GTravelProofState->PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(GTravelProofState->PostLoadMapHandle);
		GTravelProofState->PostLoadMapHandle.Reset();
	}

	GTravelProofState->Status = Status;
	T66WriteTravelProofManifest(Status);
	const bool bExit = GTravelProofState->bExitOnComplete;
	const int32 ExitCode = GTravelProofState->ExitCode;
	GTravelProofState.Reset();

	if (bExit)
	{
		FPlatformMisc::RequestExitWithStatus(false, ExitCode, TEXT("T66WorldRuntimeTravelProofComplete"));
	}
}

bool T66IsGameplayWorld(UWorld* World)
{
	return World && World->GetMapName().Contains(TEXT("GameplayLevel"), ESearchCase::IgnoreCase);
}

void T66RequestGameplayTravel(UWorld* World)
{
	if (!World)
	{
		T66FinishTravelProof(TEXT("failed_missing_world_for_stress_gameplay"));
		return;
	}

	UE_LOG(LogT66WorldRuntimeProof, Display, TEXT("[WorldRuntimeProof] requesting initial gameplay world for stress current=%s"),
		*World->GetMapName());

	if (UT66GameInstance* GameInstance = Cast<UT66GameInstance>(World->GetGameInstance()))
	{
		GameInstance->TransitionToGameplayLevel();
		return;
	}

	UGameplayStatics::OpenLevel(World, UT66GameInstance::GetGameplayLevelName());
}

void T66RequestNextTravel(UWorld* World)
{
	if (!GTravelProofState || !World)
	{
		T66FinishTravelProof(TEXT("failed_missing_world"));
		return;
	}

	const FString TargetName = T66CurrentTravelTargetName(World);
	UE_LOG(LogT66WorldRuntimeProof, Display, TEXT("[WorldRuntimeProof] requesting travel %d/%d target=%s current=%s"),
		GTravelProofState->CompletedTravelCount + 1,
		GTravelProofState->RequestedTravelCount,
		*TargetName,
		*World->GetMapName());

	const bool bTargetFrontend = TargetName.Contains(TEXT("FrontendLevel"), ESearchCase::IgnoreCase);
	if (bTargetFrontend)
	{
		UT66GameInstance::TransitionToFrontendLevel(World);
		return;
	}

	if (UT66GameInstance* GameInstance = Cast<UT66GameInstance>(World->GetGameInstance()))
	{
		GameInstance->TransitionToGameplayLevel();
		return;
	}

	UGameplayStatics::OpenLevel(World, UT66GameInstance::GetGameplayLevelName());
}

void T66CaptureTravelProofStep(TWeakObjectPtr<UWorld> WeakWorld)
{
	if (!GTravelProofState)
	{
		return;
	}

	UWorld* World = WeakWorld.Get();
	if (!World)
	{
		T66FinishTravelProof(TEXT("failed_missing_world_after_travel"));
		return;
	}

	++GTravelProofState->CompletedTravelCount;
	const FString Label = FString::Printf(TEXT("after_travel_%d"), GTravelProofState->CompletedTravelCount);
	GTravelProofState->Snapshots.Add(MakeShared<FJsonValueObject>(
		T66BuildWorldRuntimeSnapshot(World, Label, GTravelProofState->Snapshots.Num())));
	T66WriteTravelProofManifest(TEXT("running"));

	if (GTravelProofState->CompletedTravelCount >= GTravelProofState->RequestedTravelCount)
	{
		T66FinishTravelProof(TEXT("complete"));
		return;
	}

	T66RequestNextTravel(World);
}

void T66StartTravelProofAfterStress(TWeakObjectPtr<UWorld> WeakWorld)
{
	if (!GTravelProofState)
	{
		return;
	}

	UWorld* World = WeakWorld.Get();
	if (!World)
	{
		T66FinishTravelProof(TEXT("failed_missing_world_after_stress"));
		return;
	}

	GTravelProofState->Snapshots.Add(MakeShared<FJsonValueObject>(
		T66BuildWorldRuntimeSnapshot(World, TEXT("before_travel_stress"), GTravelProofState->Snapshots.Num())));
	T66WriteTravelProofManifest(TEXT("running"));
	T66RequestNextTravel(World);
}

void T66PopulateAndScheduleStressTravel(UWorld* World)
{
	if (!GTravelProofState)
	{
		return;
	}

	if (!World || !T66IsGameplayWorld(World))
	{
		T66FinishTravelProof(TEXT("failed_missing_gameplay_world_for_stress"));
		return;
	}

	GTravelProofState->StressPopulation = T66PopulateWorldRuntimeStress(World, GTravelProofState->StressCount);
	T66WriteTravelProofManifest(TEXT("stress_populated"));

	World->GetTimerManager().SetTimer(
		GTravelProofState->StartTravelTimerHandle,
		FTimerDelegate::CreateStatic(&T66StartTravelProofAfterStress, TWeakObjectPtr<UWorld>(World)),
		FMath::Max(0.05f, GTravelProofState->StressSettleSeconds),
		false);
}

void T66HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
	if (!GTravelProofState || !LoadedWorld || !LoadedWorld->IsGameWorld())
	{
		return;
	}

	if (GTravelProofState->bWaitingForInitialStressGameplay)
	{
		if (!T66IsGameplayWorld(LoadedWorld))
		{
			UE_LOG(LogT66WorldRuntimeProof, Display, TEXT("[WorldRuntimeProof] stress proof waiting for gameplay; ignoring loaded world=%s"),
				*LoadedWorld->GetMapName());
			return;
		}
		GTravelProofState->bWaitingForInitialStressGameplay = false;
		T66PopulateAndScheduleStressTravel(LoadedWorld);
		return;
	}

	LoadedWorld->GetTimerManager().SetTimer(
		GTravelProofState->CaptureTimerHandle,
		FTimerDelegate::CreateStatic(&T66CaptureTravelProofStep, TWeakObjectPtr<UWorld>(LoadedWorld)),
		FMath::Max(0.05f, GTravelProofState->CaptureDelaySeconds),
		false);
}

void T66RunWorldRuntimeProofTravelCommand(const TArray<FString>& Args, UWorld* World)
{
	if (GTravelProofState)
	{
		UE_LOG(LogT66WorldRuntimeProof, Warning, TEXT("[WorldRuntimeProof] Travel proof is already running."));
		return;
	}

	if (!World)
	{
		UE_LOG(LogT66WorldRuntimeProof, Warning, TEXT("[WorldRuntimeProof] Cannot start travel proof without a world."));
		return;
	}

	GTravelProofState = MakeUnique<FT66WorldRuntimeTravelProofState>();
	GTravelProofState->ManifestPath = T66FindArgValue(Args, TEXT("Path"));
	if (GTravelProofState->ManifestPath.IsEmpty())
	{
		GTravelProofState->ManifestPath = T66WorldRuntimeProofDefaultPath(TEXT("world_runtime_travel_proof"));
	}
	GTravelProofState->RequestedTravelCount = FMath::Clamp(T66FindArgInt(Args, TEXT("Travels"), 2), 1, 24);
	GTravelProofState->CaptureDelaySeconds = FMath::Clamp(T66FindArgFloat(Args, TEXT("Delay"), 0.75f), 0.05f, 10.0f);
	GTravelProofState->ExitCode = T66FindArgInt(Args, TEXT("ExitCode"), 0);
	GTravelProofState->bExitOnComplete = T66FindArgBool(Args, TEXT("ExitOnComplete"), false);
	GTravelProofState->bStress = T66FindArgBool(Args, TEXT("Stress"), false);
	GTravelProofState->StressCount = FMath::Clamp(T66FindArgInt(Args, TEXT("StressCount"), 6), 1, 24);
	GTravelProofState->StressSettleSeconds = FMath::Clamp(T66FindArgFloat(Args, TEXT("StressSettle"), 0.35f), 0.05f, 5.0f);
	GTravelProofState->PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(&T66HandlePostLoadMapWithWorld);

	if (GTravelProofState->bStress)
	{
		T66WriteTravelProofManifest(TEXT("starting_stress"));
		if (T66IsGameplayWorld(World))
		{
			T66PopulateAndScheduleStressTravel(World);
			return;
		}

		GTravelProofState->bWaitingForInitialStressGameplay = true;
		T66RequestGameplayTravel(World);
		return;
	}

	GTravelProofState->Snapshots.Add(MakeShared<FJsonValueObject>(T66BuildWorldRuntimeSnapshot(World, TEXT("before_travel"), 0)));
	T66WriteTravelProofManifest(TEXT("running"));
	T66RequestNextTravel(World);
}

static FAutoConsoleCommandWithWorldAndArgs T66WorldRuntimeSnapshotCommand(
	TEXT("T66.WorldRuntime.Snapshot"),
	TEXT("Development proof: write a read-only world-runtime teardown snapshot. Args: Path=<manifest.json> Label=<label> ExitOnComplete=0|1 ExitCode=0."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66RunWorldRuntimeSnapshotCommand));

static FAutoConsoleCommandWithWorldAndArgs T66WorldRuntimeProofTravelCommand(
	TEXT("T66.WorldRuntime.ProofTravel"),
	TEXT("Development proof: alternate gameplay/frontend world travel and write owner-local runtime snapshots. Args: Path=<manifest.json> Travels=2 Delay=0.75 Stress=0|1 StressCount=6 StressSettle=0.35 ExitOnComplete=0|1 ExitCode=0."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66RunWorldRuntimeProofTravelCommand));
}

#endif
