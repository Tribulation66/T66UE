// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66OutgoingTravelerPoolSubsystem.h"

#include "Core/T66ActorRegistrySubsystem.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Components/SceneComponent.h"
#include "Dom/JsonObject.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemInstanceController.h"
#include "RHI.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66OutgoingTravelerPool, Log, All);

namespace
{
	static TAutoConsoleVariable<int32> CVarT66OutgoingTravelerPoolEnabled(
		TEXT("T66.OutgoingTravelerPool.Enabled"),
		1,
		TEXT("Production path for supported outgoing hero travelers. Non-zero uses the actorless Niagara traveler pool; zero keeps the legacy actor-mesh baseline for A/B only."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66OutgoingTravelerPoolVerbose(
		TEXT("T66.OutgoingTravelerPool.Verbose"),
		0,
		TEXT("Logs outgoing traveler pool upload and allocation diagnostics when non-zero."),
		ECVF_Default);

	const TCHAR* T66OutgoingTravelerPoolAssetPath =
		TEXT("/Game/VFX/Foundation/OutgoingTravelers/NS_OutgoingTravelerPool.NS_OutgoingTravelerPool");
	const FName T66TravelerPositionsParam(TEXT("User.TravelerPositions"));
	const FName T66TravelerRotationsParam(TEXT("User.TravelerRotations"));
	const FName T66TravelerScalesParam(TEXT("User.TravelerScales"));
	const FName T66TravelerColorsParam(TEXT("User.TravelerColors"));
	const FName T66TravelerMeshIndicesParam(TEXT("User.TravelerMeshIndices"));
	const FName T66TravelerLiveCountParam(TEXT("User.TravelerLiveCount"));
	const FName T66TravelerPoolCapacityParam(TEXT("User.TravelerPoolCapacity"));

	struct FT66TravelerVisualProfileSlot
	{
		const TCHAR* ProfileID = TEXT("");
		int32 MeshIndex = 0;
	};

	const FT66TravelerVisualProfileSlot T66TravelerVisualProfileSlots[] =
	{
		{ TEXT("TravelerVisual.Fire.AOE"), 4 },
		{ TEXT("TravelerVisual.Fire.Pierce"), 5 },
		{ TEXT("TravelerVisual.Fire.Bounce"), 6 },
		{ TEXT("TravelerVisual.Fire.DOT"), 7 },
		{ TEXT("TravelerVisual.Ice.AOE"), 8 },
		{ TEXT("TravelerVisual.Ice.Pierce"), 9 },
		{ TEXT("TravelerVisual.Ice.Bounce"), 10 },
		{ TEXT("TravelerVisual.Ice.DOT"), 11 },
		{ TEXT("TravelerVisual.Electricity.AOE"), 12 },
		{ TEXT("TravelerVisual.Electricity.Pierce"), 13 },
		{ TEXT("TravelerVisual.Electricity.Bounce"), 14 },
		{ TEXT("TravelerVisual.Electricity.DOT"), 15 },
		{ TEXT("TravelerVisual.Nature.AOE"), 16 },
		{ TEXT("TravelerVisual.Nature.Pierce"), 17 },
		{ TEXT("TravelerVisual.Nature.Bounce"), 18 },
		{ TEXT("TravelerVisual.Nature.DOT"), 19 }
	};

	FString T66FormatMaybeNumber(const double Value)
	{
		return Value >= 0.0 ? FString::Printf(TEXT("%.4f"), Value) : TEXT("Unavailable");
	}
}

void UT66OutgoingTravelerPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeSlots();
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerPoolProofManifest="), ManifestPath);
}

void UT66OutgoingTravelerPoolSubsystem::Deinitialize()
{
	if (!ManifestPath.IsEmpty() && !bManifestWritten)
	{
		WriteProofManifest(TEXT("deinitialize"));
		bManifestWritten = true;
	}

	if (NiagaraComponent)
	{
		NiagaraComponent->DeactivateImmediate();
		NiagaraComponent->DestroyComponent();
		NiagaraComponent = nullptr;
	}
	if (RenderHost)
	{
		RenderHost->Destroy();
		RenderHost = nullptr;
	}
	RenderRoot = nullptr;
	Slots.Reset();
	FreeSlots.Reset();
	DenseSlots.Reset();
	PositionUpload.Reset();
	RotationUpload.Reset();
	ScaleUpload.Reset();
	ColorUpload.Reset();
	MeshIndexUpload.Reset();
	bInitializedSlots = false;
	Super::Deinitialize();
}

void UT66OutgoingTravelerPoolSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsEnabled())
	{
		SampleProofMetrics(DeltaTime);
		return;
	}

	BuildTargetSnapshot();
	TickSimulatedTravelers(DeltaTime);

	if (EnsureNiagaraComponent() && bDirty)
	{
		UploadLiveState();
	}

	SampleProofMetrics(DeltaTime);
}

TStatId UT66OutgoingTravelerPoolSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UT66OutgoingTravelerPoolSubsystem, STATGROUP_Tickables);
}

bool UT66OutgoingTravelerPoolSubsystem::IsTickable() const
{
	return !IsTemplate();
}

bool UT66OutgoingTravelerPoolSubsystem::FireOutgoingTraveler(
	const FT66OutgoingTravelerFireParams& FireParams,
	FT66OutgoingTravelerHandle& OutHandle)
{
	return FireOutgoingTraveler(FireParams, OutHandle, FT66OutgoingTravelerArrivalCallback());
}

bool UT66OutgoingTravelerPoolSubsystem::FireOutgoingTraveler(
	const FT66OutgoingTravelerFireParams& FireParams,
	FT66OutgoingTravelerHandle& OutHandle,
	FT66OutgoingTravelerArrivalCallback OnArrived)
{
	OutHandle.Reset();
	if (FireParams.ProfileID.IsNone())
	{
		return false;
	}

	const FT66TemporaryProjectileVisualSpec Spec =
		FT66TemporaryProjectileSystem::MakeSpec(FireParams.ProfileID, FireParams.Color, FireParams.ScaleMultiplier);

	const FVector ToTarget = FireParams.TargetPosition - FireParams.StartPosition;
	const FVector InitialDirection = ToTarget.GetSafeNormal();
	const FQuat ProfileRotation = Spec.RelativeRotation.Quaternion();
	const FQuat TravelRotation = InitialDirection.IsNearlyZero()
		? FQuat::Identity
		: InitialDirection.Rotation().Quaternion();

	FT66OutgoingTravelerVisualState InitialState;
	InitialState.Position = FireParams.StartPosition + TravelRotation.RotateVector(Spec.RelativeLocation);
	InitialState.Rotation = TravelRotation * ProfileRotation;
	InitialState.Scale = Spec.RelativeScale;
	InitialState.Color = FireParams.TravelerVisualProfileID.IsNone() ? Spec.Color : FireParams.Color;
	const int32 FallbackMeshIndex = GetMeshIndexForTemporaryProjectileShape(Spec.Shape);
	InitialState.MeshIndex = GetMeshIndexForTravelerVisualProfileID(
		FireParams.TravelerVisualProfileID,
		FallbackMeshIndex);

	if (!AllocateSlot(InitialState, OutHandle))
	{
		return false;
	}

	FT66OutgoingTravelerSlot& Slot = Slots[OutHandle.SlotIndex];
	Slot.TargetHandle = FireParams.TargetHandle;
	Slot.TargetPosition = FireParams.TargetPosition;
	Slot.TargetOffset = FireParams.TargetOffset;
	Slot.LastKnownTargetPosition = FireParams.TargetPosition;
	if (UWorld* World = GetWorld())
	{
		if (const AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode()))
		{
			Slot.SourceTowerFloorNumber = GameMode->GetTowerFloorIndexForLocation(FireParams.StartPosition);
		}
	}
	Slot.TargetActorKey = FireParams.TargetHandle.Actor.IsValid()
		? FObjectKey(FireParams.TargetHandle.Actor.Get())
		: FObjectKey();
	Slot.ProfileRotation = ProfileRotation;
	Slot.Speed = FMath::Max(1.0f, FireParams.Speed);
	Slot.RemainingLifetimeSeconds = FMath::Max(0.05f, FireParams.LifetimeSeconds);
	Slot.ArrivalRadius = FMath::Max(0.0f, FireParams.ArrivalRadius);
	Slot.DamageAmount = FMath::Max(0, FireParams.DamageAmount);
	Slot.DamageSourceID = FireParams.DamageSourceID;
	Slot.EventType = FireParams.EventType;
	Slot.OnArrived = OnArrived;
	Slot.bSimulated = true;
	Slot.bTrackTarget = FireParams.bTrackTarget;
	Slot.bHasTargetActorKey = FireParams.TargetHandle.Actor.IsValid();
	Slot.bEnableArrivalCollision = FireParams.bEnableArrivalCollision;
	Slot.bApplyDamageOnArrival = !Slot.OnArrived.IsBound()
		&& FireParams.bApplyDamageOnArrival
		&& Slot.DamageAmount > 0;
	++Diagnostics.SimulatedFiredTotal;

	if (CVarT66OutgoingTravelerPoolVerbose.GetValueOnGameThread() != 0)
	{
		UE_LOG(LogT66OutgoingTravelerPool, Display,
			TEXT("[OutgoingTravelerPool] phase=FireSimulated slot=%d generation=%d live=%d profile=%s visualProfile=%s meshIndex=%d start=%s target=%s speed=%.1f lifetime=%.2f"),
			OutHandle.SlotIndex,
			OutHandle.Generation,
			Diagnostics.LiveCount,
			*FireParams.ProfileID.ToString(),
			*FireParams.TravelerVisualProfileID.ToString(),
			InitialState.MeshIndex,
			*FireParams.StartPosition.ToCompactString(),
			*FireParams.TargetPosition.ToCompactString(),
			Slot.Speed,
			Slot.RemainingLifetimeSeconds);
	}
	return true;
}

void UT66OutgoingTravelerPoolSubsystem::CancelOutgoingTraveler(FT66OutgoingTravelerHandle& Handle)
{
	if (!IsHandleCurrent(Handle))
	{
		Handle.Reset();
		return;
	}

	const int32 SlotIndex = Handle.SlotIndex;
	ReleaseSlot(SlotIndex, EReleaseReason::Canceled);
	if (CVarT66OutgoingTravelerPoolVerbose.GetValueOnGameThread() != 0)
	{
		UE_LOG(LogT66OutgoingTravelerPool, Display,
			TEXT("[OutgoingTravelerPool] phase=Cancel slot=%d live=%d canceled=%d"),
			SlotIndex,
			Diagnostics.LiveCount,
			Diagnostics.CanceledTotal);
	}
	Handle.Reset();
}

void UT66OutgoingTravelerPoolSubsystem::NoteMainMeshSuppressed()
{
	++Diagnostics.MainMeshSuppressedTotal;
}

int32 UT66OutgoingTravelerPoolSubsystem::GetMeshIndexForTemporaryProjectileShape(
	const ET66TemporaryProjectileShape Shape)
{
	switch (Shape)
	{
	case ET66TemporaryProjectileShape::Cone:
		return 1;
	case ET66TemporaryProjectileShape::Cylinder:
		return 2;
	case ET66TemporaryProjectileShape::Cube:
		return 3;
	case ET66TemporaryProjectileShape::Sphere:
	default:
		return 0;
	}
}

int32 UT66OutgoingTravelerPoolSubsystem::GetMeshIndexForTravelerVisualProfileID(
	const FName TravelerVisualProfileID,
	const int32 FallbackMeshIndex)
{
	if (TravelerVisualProfileID.IsNone())
	{
		return FallbackMeshIndex;
	}

	for (const FT66TravelerVisualProfileSlot& Slot : T66TravelerVisualProfileSlots)
	{
		if (TravelerVisualProfileID == FName(Slot.ProfileID))
		{
			return Slot.MeshIndex;
		}
	}

	return FallbackMeshIndex;
}

void UT66OutgoingTravelerPoolSubsystem::AppendKnownTravelerVisualProfileIDs(TArray<FName>& OutProfileIDs)
{
	OutProfileIDs.Reserve(OutProfileIDs.Num() + UE_ARRAY_COUNT(T66TravelerVisualProfileSlots));
	for (const FT66TravelerVisualProfileSlot& Slot : T66TravelerVisualProfileSlots)
	{
		OutProfileIDs.Add(FName(Slot.ProfileID));
	}
}

bool UT66OutgoingTravelerPoolSubsystem::IsEnabled()
{
	return CVarT66OutgoingTravelerPoolEnabled.GetValueOnGameThread() != 0;
}

void UT66OutgoingTravelerPoolSubsystem::InitializeSlots()
{
	if (bInitializedSlots)
	{
		return;
	}

	Slots.SetNum(MaxOutgoingTravelers);
	FreeSlots.Reset(MaxOutgoingTravelers);
	for (int32 Index = MaxOutgoingTravelers - 1; Index >= 0; --Index)
	{
		FreeSlots.Add(Index);
	}
	DenseSlots.Reserve(MaxOutgoingTravelers);
	PositionUpload.Reserve(MaxOutgoingTravelers);
	RotationUpload.Reserve(MaxOutgoingTravelers);
	ScaleUpload.Reserve(MaxOutgoingTravelers);
	ColorUpload.Reserve(MaxOutgoingTravelers);
	MeshIndexUpload.Reserve(MaxOutgoingTravelers);
	bInitializedSlots = true;
}

bool UT66OutgoingTravelerPoolSubsystem::IsHandleCurrent(const FT66OutgoingTravelerHandle& Handle) const
{
	return Handle.IsValid()
		&& Slots.IsValidIndex(Handle.SlotIndex)
		&& Slots[Handle.SlotIndex].bActive
		&& Slots[Handle.SlotIndex].Generation == Handle.Generation;
}

bool UT66OutgoingTravelerPoolSubsystem::AllocateSlot(
	const FT66OutgoingTravelerVisualState& InitialState,
	FT66OutgoingTravelerHandle& OutHandle)
{
	OutHandle.Reset();
	if (!IsEnabled() || !EnsureNiagaraComponent())
	{
		return false;
	}

	InitializeSlots();
	if (FreeSlots.Num() <= 0)
	{
		++Diagnostics.DroppedTotal;
		UE_LOG(LogT66OutgoingTravelerPool, Warning, TEXT("[OutgoingTravelerPool] phase=Fire status=pool-full capacity=%d"),
			MaxOutgoingTravelers);
		return false;
	}

	const int32 SlotIndex = FreeSlots.Pop(EAllowShrinking::No);
	FT66OutgoingTravelerSlot& Slot = Slots[SlotIndex];
	Slot.State = InitialState;
	Slot.TargetHandle.Reset();
	Slot.TargetPosition = InitialState.Position;
	Slot.TargetOffset = FVector::ZeroVector;
	Slot.TargetActorKey = FObjectKey();
	Slot.LastKnownTargetPosition = InitialState.Position;
	Slot.ProfileRotation = FQuat::Identity;
	Slot.Speed = 0.0f;
	Slot.RemainingLifetimeSeconds = 0.0f;
	Slot.ArrivalRadius = 30.0f;
	Slot.DamageAmount = 0;
	Slot.SourceTowerFloorNumber = INDEX_NONE;
	Slot.DamageSourceID = NAME_None;
	Slot.EventType = NAME_None;
	Slot.OnArrived.Unbind();
	Slot.bSimulated = false;
	Slot.bTrackTarget = false;
	Slot.bHasTargetActorKey = false;
	Slot.bEnableArrivalCollision = false;
	Slot.bApplyDamageOnArrival = false;
	Slot.bActive = true;
	Slot.Generation = Slot.Generation == MAX_int32 ? 1 : Slot.Generation + 1;
	if (Slot.Generation == 0)
	{
		Slot.Generation = 1;
	}
	Slot.DenseIndex = DenseSlots.Add(SlotIndex);

	OutHandle.SlotIndex = SlotIndex;
	OutHandle.Generation = Slot.Generation;

	Diagnostics.LiveCount = DenseSlots.Num();
	Diagnostics.PeakLiveCount = FMath::Max(Diagnostics.PeakLiveCount, Diagnostics.LiveCount);
	++Diagnostics.FiredTotal;
	if (FirstActivityTimeSeconds <= 0.f)
	{
		if (const UWorld* World = GetWorld())
		{
			FirstActivityTimeSeconds = World->GetTimeSeconds();
		}
	}
	bDirty = true;
	return true;
}

void UT66OutgoingTravelerPoolSubsystem::ReleaseSlot(const int32 SlotIndex, const EReleaseReason Reason)
{
	if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex].bActive)
	{
		return;
	}

	FT66OutgoingTravelerSlot& Slot = Slots[SlotIndex];
	const int32 DenseIndex = Slot.DenseIndex;
	const int32 LastDenseIndex = DenseSlots.Num() - 1;
	if (DenseIndex != LastDenseIndex)
	{
		const int32 MovedSlotIndex = DenseSlots[LastDenseIndex];
		DenseSlots[DenseIndex] = MovedSlotIndex;
		Slots[MovedSlotIndex].DenseIndex = DenseIndex;
	}
	DenseSlots.Pop(EAllowShrinking::No);

	Slot.TargetHandle.Reset();
	Slot.TargetPosition = FVector::ZeroVector;
	Slot.TargetOffset = FVector::ZeroVector;
	Slot.TargetActorKey = FObjectKey();
	Slot.LastKnownTargetPosition = FVector::ZeroVector;
	Slot.ProfileRotation = FQuat::Identity;
	Slot.Speed = 0.0f;
	Slot.RemainingLifetimeSeconds = 0.0f;
	Slot.ArrivalRadius = 30.0f;
	Slot.DamageAmount = 0;
	Slot.SourceTowerFloorNumber = INDEX_NONE;
	Slot.DamageSourceID = NAME_None;
	Slot.EventType = NAME_None;
	Slot.OnArrived.Unbind();
	Slot.bSimulated = false;
	Slot.bTrackTarget = false;
	Slot.bHasTargetActorKey = false;
	Slot.bEnableArrivalCollision = false;
	Slot.bApplyDamageOnArrival = false;
	Slot.bActive = false;
	Slot.DenseIndex = INDEX_NONE;
	Slot.Generation = Slot.Generation == MAX_int32 ? 1 : Slot.Generation + 1;
	FreeSlots.Add(SlotIndex);

	Diagnostics.LiveCount = DenseSlots.Num();
	switch (Reason)
	{
	case EReleaseReason::Arrived:
		++Diagnostics.SimulatedArrivedTotal;
		break;
	case EReleaseReason::Expired:
		++Diagnostics.SimulatedExpiredTotal;
		break;
	case EReleaseReason::Canceled:
	default:
		++Diagnostics.CanceledTotal;
		break;
	}
	bDirty = true;
}

bool UT66OutgoingTravelerPoolSubsystem::EnsureNiagaraComponent()
{
	if (NiagaraComponent && NiagaraComponent->IsRegistered())
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (!NiagaraSystem)
	{
		NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, T66OutgoingTravelerPoolAssetPath);
		if (!NiagaraSystem)
		{
			UE_LOG(LogT66OutgoingTravelerPool, Warning, TEXT("[OutgoingTravelerPool] Missing Niagara system %s"),
				T66OutgoingTravelerPoolAssetPath);
			return false;
		}
	}

	if (!RenderHost)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("T66OutgoingTravelerPoolHost");
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;
		RenderHost = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
		if (RenderHost)
		{
			RenderHost->SetActorHiddenInGame(false);
			RenderHost->SetActorEnableCollision(false);
			RenderRoot = NewObject<USceneComponent>(RenderHost, TEXT("OutgoingTravelerPoolRoot"), RF_Transient);
			if (RenderRoot)
			{
				RenderRoot->Mobility = EComponentMobility::Movable;
				RenderRoot->RegisterComponent();
				RenderHost->SetRootComponent(RenderRoot);
			}
		}
	}
	if (!RenderHost || !RenderRoot)
	{
		return false;
	}

	NiagaraComponent = NewObject<UNiagaraComponent>(RenderHost, TEXT("OutgoingTravelerPoolNiagara"), RF_Transient);
	if (!NiagaraComponent)
	{
		return false;
	}
	NiagaraComponent->SetAsset(NiagaraSystem);
	NiagaraComponent->bAutoActivate = false;
	NiagaraComponent->SetVariableInt(T66TravelerPoolCapacityParam, MaxOutgoingTravelers);
	NiagaraComponent->SetUsingAbsoluteLocation(true);
	NiagaraComponent->SetUsingAbsoluteRotation(true);
	NiagaraComponent->SetUsingAbsoluteScale(true);
	NiagaraComponent->SetupAttachment(RenderRoot);
	NiagaraComponent->RegisterComponent();
	NiagaraComponent->SetVisibility(true, true);
	NiagaraComponent->SetHiddenInGame(false);
	NiagaraComponent->SetComponentTickEnabled(true);
	NiagaraComponent->SetVariableInt(T66TravelerPoolCapacityParam, MaxOutgoingTravelers);
	bDirty = true;

	UE_LOG(LogT66OutgoingTravelerPool, Display, TEXT("[OutgoingTravelerPool] phase=EnsureNiagara status=ready asset=%s capacity=%d"),
		T66OutgoingTravelerPoolAssetPath,
		MaxOutgoingTravelers);
	return true;
}

void UT66OutgoingTravelerPoolSubsystem::BuildTargetSnapshot()
{
	const uint64 StartCycles = FPlatformTime::Cycles64();
	TargetSnapshot.Reset();
	TargetSnapshotIndexByActor.Reset();

	const UWorld* World = GetWorld();
	const UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
	if (Registry)
	{
		for (const TWeakObjectPtr<AT66EnemyBase>& EnemyPtr : Registry->GetEnemies())
		{
			AddTargetSnapshotEntry(EnemyPtr.Get());
		}
		for (const TWeakObjectPtr<AT66MobBase>& MobPtr : Registry->GetActiveMobs())
		{
			AddTargetSnapshotEntry(MobPtr.Get());
		}
		for (const TWeakObjectPtr<AT66BossBase>& BossPtr : Registry->GetBosses())
		{
			AddTargetSnapshotEntry(BossPtr.Get());
		}
	}

	const uint64 EndCycles = FPlatformTime::Cycles64();
	Diagnostics.LastTargetSnapshotMs = FPlatformTime::ToMilliseconds64(EndCycles - StartCycles);
	Diagnostics.LastTargetSnapshotCount = TargetSnapshot.Num();
	++Diagnostics.TargetSnapshotBuildCount;
	const double SampleCount = static_cast<double>(Diagnostics.TargetSnapshotBuildCount);
	Diagnostics.AverageTargetSnapshotMs += (Diagnostics.LastTargetSnapshotMs - Diagnostics.AverageTargetSnapshotMs) / SampleCount;
	Diagnostics.MaxTargetSnapshotMs = FMath::Max(Diagnostics.MaxTargetSnapshotMs, Diagnostics.LastTargetSnapshotMs);
}

void UT66OutgoingTravelerPoolSubsystem::AddTargetSnapshotEntry(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	bool bAlive = false;
	FT66CombatTargetHandle ResolvedHandle;
	if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Actor))
	{
		bAlive = Enemy->CurrentHP > 0;
		if (bAlive)
		{
			ResolvedHandle = Enemy->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body);
		}
	}
	else if (AT66MobBase* Mob = Cast<AT66MobBase>(Actor))
	{
		bAlive = Mob->IsAliveAndActive();
		if (bAlive)
		{
			ResolvedHandle = Mob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body);
		}
	}
	else if (AT66BossBase* Boss = Cast<AT66BossBase>(Actor))
	{
		bAlive = Boss->IsAwakened() && Boss->IsAlive();
		if (bAlive)
		{
			ResolvedHandle = Boss->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body);
		}
	}

	if (!bAlive)
	{
		return;
	}

	FT66OutgoingTravelerTargetSnapshotEntry Entry;
	Entry.Actor = Actor;
	Entry.ActorKey = FObjectKey(Actor);
	Entry.Location = Actor->GetActorLocation();
	Entry.AimPoint = ResolvedHandle.AimPoint.IsNearlyZero() ? Entry.Location : ResolvedHandle.AimPoint;
	Entry.HitZoneType = ResolvedHandle.HitZoneType;
	Entry.HitZoneName = ResolvedHandle.HitZoneName;
	Entry.bAlive = true;

	const int32 EntryIndex = TargetSnapshot.Add(Entry);
	TargetSnapshotIndexByActor.Add(Entry.ActorKey, EntryIndex);
}

const UT66OutgoingTravelerPoolSubsystem::FT66OutgoingTravelerTargetSnapshotEntry*
UT66OutgoingTravelerPoolSubsystem::FindTargetSnapshotEntry(const FT66OutgoingTravelerSlot& Slot) const
{
	if (!Slot.bHasTargetActorKey)
	{
		return nullptr;
	}

	const int32* FoundIndex = TargetSnapshotIndexByActor.Find(Slot.TargetActorKey);
	if (!FoundIndex || !TargetSnapshot.IsValidIndex(*FoundIndex))
	{
		return nullptr;
	}
	return &TargetSnapshot[*FoundIndex];
}

FT66OutgoingTravelerArrivalEvent UT66OutgoingTravelerPoolSubsystem::MakeArrivalEvent(
	const FT66OutgoingTravelerHandle& Handle,
	const FT66OutgoingTravelerSlot& Slot,
	const FT66OutgoingTravelerTargetSnapshotEntry* SnapshotEntry,
	const FVector& ArrivalPosition) const
{
	FT66OutgoingTravelerArrivalEvent Event;
	Event.Handle = Handle;
	Event.ArrivalPosition = ArrivalPosition;
	Event.LastKnownTargetPosition = Slot.LastKnownTargetPosition;
	Event.ResolvedTargetHandle = Slot.TargetHandle;
	Event.bHitLiveTarget = SnapshotEntry && SnapshotEntry->bAlive && SnapshotEntry->Actor.IsValid();
	Event.bTargetLostOrDead = !Event.bHitLiveTarget;

	if (Event.bHitLiveTarget)
	{
		Event.ResolvedTargetHandle.Actor = SnapshotEntry->Actor.Get();
		if (Event.ResolvedTargetHandle.AimPoint.IsNearlyZero())
		{
			Event.ResolvedTargetHandle.AimPoint = SnapshotEntry->AimPoint;
		}
		if (Event.ResolvedTargetHandle.HitZoneType == ET66HitZoneType::None)
		{
			Event.ResolvedTargetHandle.HitZoneType = SnapshotEntry->HitZoneType;
		}
		if (Event.ResolvedTargetHandle.HitZoneName.IsNone())
		{
			Event.ResolvedTargetHandle.HitZoneName = SnapshotEntry->HitZoneName;
		}
	}

	return Event;
}

void UT66OutgoingTravelerPoolSubsystem::ResolveTravelerArrival(
	const int32 SlotIndex,
	const FT66OutgoingTravelerTargetSnapshotEntry* SnapshotEntry,
	const FVector& ArrivalPosition,
	uint64& InOutArrivalCollisionCycles)
{
	if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex].bActive)
	{
		return;
	}

	FT66OutgoingTravelerSlot& Slot = Slots[SlotIndex];
	FT66OutgoingTravelerHandle ArrivalHandle;
	ArrivalHandle.SlotIndex = SlotIndex;
	ArrivalHandle.Generation = Slot.Generation;

	if (Slot.OnArrived.IsBound())
	{
		const FT66OutgoingTravelerArrivalCallback Callback = Slot.OnArrived;
		const FT66OutgoingTravelerArrivalEvent Event =
			MakeArrivalEvent(ArrivalHandle, Slot, SnapshotEntry, ArrivalPosition);
		++Diagnostics.ArrivalCallbackTotal;
		Callback.Execute(Event);
	}
	else if (Slot.bEnableArrivalCollision)
	{
		++Diagnostics.ArrivalCheckTotal;
		const uint64 ArrivalStartCycles = FPlatformTime::Cycles64();
		if (!ApplyArrivalDamage(Slot, SnapshotEntry))
		{
			++Diagnostics.ArrivalFizzleNoTargetTotal;
		}
		InOutArrivalCollisionCycles += FPlatformTime::Cycles64() - ArrivalStartCycles;
	}

	if (IsHandleCurrent(ArrivalHandle))
	{
		ReleaseSlot(SlotIndex, EReleaseReason::Arrived);
	}
}

bool UT66OutgoingTravelerPoolSubsystem::ApplyArrivalDamage(
	const FT66OutgoingTravelerSlot& Slot,
	const FT66OutgoingTravelerTargetSnapshotEntry* SnapshotEntry)
{
	if (!Slot.bApplyDamageOnArrival || Slot.DamageAmount <= 0 || !SnapshotEntry || !SnapshotEntry->bAlive)
	{
		return false;
	}

	AActor* Actor = SnapshotEntry->Actor.Get();
	if (!IsValid(Actor))
	{
		return false;
	}
	if (UWorld* World = GetWorld())
	{
		if (const AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode()))
		{
			if (GameMode->IsUsingTowerMainMapLayout() && Slot.SourceTowerFloorNumber != INDEX_NONE)
			{
				const int32 TargetTowerFloorNumber = GameMode->ResolveTowerFloorNumberForActor(Actor);
				if (TargetTowerFloorNumber != INDEX_NONE && TargetTowerFloorNumber != Slot.SourceTowerFloorNumber)
				{
					return false;
				}
			}
		}
	}

	FT66CombatTargetHandle ResolvedHandle = Slot.TargetHandle;
	ResolvedHandle.Actor = Actor;
	if (ResolvedHandle.AimPoint.IsNearlyZero())
	{
		ResolvedHandle.AimPoint = SnapshotEntry->AimPoint;
	}
	if (ResolvedHandle.HitZoneType == ET66HitZoneType::None)
	{
		ResolvedHandle.HitZoneType = SnapshotEntry->HitZoneType;
	}
	if (ResolvedHandle.HitZoneName.IsNone())
	{
		ResolvedHandle.HitZoneName = SnapshotEntry->HitZoneName;
	}

	if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Actor))
	{
		if (Enemy->CurrentHP > 0)
		{
			Enemy->TakeDamageFromHeroHitZone(Slot.DamageAmount, ResolvedHandle, Slot.DamageSourceID, Slot.EventType);
			++Diagnostics.ArrivalDamageAppliedTotal;
			return true;
		}
		return false;
	}

	if (AT66MobBase* Mob = Cast<AT66MobBase>(Actor))
	{
		if (Mob->IsAliveAndActive())
		{
			Mob->TakeDamageFromHeroHitZone(Slot.DamageAmount, ResolvedHandle, Slot.DamageSourceID, Slot.EventType);
			++Diagnostics.ArrivalDamageAppliedTotal;
			return true;
		}
		return false;
	}

	if (AT66BossBase* Boss = Cast<AT66BossBase>(Actor))
	{
		if (Boss->IsAwakened() && Boss->IsAlive())
		{
			Boss->TakeDamageFromHeroHitZone(Slot.DamageAmount, ResolvedHandle, Slot.DamageSourceID, Slot.EventType);
			++Diagnostics.ArrivalDamageAppliedTotal;
			return true;
		}
	}

	return false;
}

void UT66OutgoingTravelerPoolSubsystem::TickSimulatedTravelers(const float DeltaTime)
{
	if (DenseSlots.Num() <= 0)
	{
		Diagnostics.LastSimulationMs = 0.0;
		Diagnostics.LastArrivalCollisionMs = 0.0;
		return;
	}

	const uint64 StartCycles = FPlatformTime::Cycles64();
	uint64 ArrivalCollisionCycles = 0;
	const float StepSeconds = FMath::Max(0.0f, DeltaTime);
	bool bAnySimulated = false;
	for (int32 DenseIndex = 0; DenseIndex < DenseSlots.Num();)
	{
		const int32 SlotIndex = DenseSlots[DenseIndex];
		if (!Slots.IsValidIndex(SlotIndex))
		{
			++DenseIndex;
			continue;
		}

		FT66OutgoingTravelerSlot& Slot = Slots[SlotIndex];
		if (!Slot.bActive || !Slot.bSimulated)
		{
			++DenseIndex;
			continue;
		}

		bAnySimulated = true;
		Slot.RemainingLifetimeSeconds -= StepSeconds;
		if (Slot.RemainingLifetimeSeconds <= 0.0f)
		{
			ReleaseSlot(SlotIndex, EReleaseReason::Expired);
			continue;
		}

		const FT66OutgoingTravelerTargetSnapshotEntry* SnapshotEntry = nullptr;
		FVector TargetPosition = Slot.TargetPosition;
		if (Slot.bTrackTarget)
		{
			SnapshotEntry = FindTargetSnapshotEntry(Slot);
			if (SnapshotEntry)
			{
				TargetPosition = SnapshotEntry->Location + Slot.TargetOffset;
				Slot.TargetPosition = TargetPosition;
				Slot.LastKnownTargetPosition = TargetPosition;
			}
			else
			{
				TargetPosition = Slot.LastKnownTargetPosition;
			}
		}

		const FVector ToTarget = TargetPosition - Slot.State.Position;
		const float Distance = ToTarget.Size();
		if (Distance <= FMath::Max(KINDA_SMALL_NUMBER, Slot.ArrivalRadius))
		{
			ResolveTravelerArrival(SlotIndex, SnapshotEntry, Slot.State.Position, ArrivalCollisionCycles);
			continue;
		}

		const float MoveDistance = Slot.Speed * StepSeconds;
		const FVector Direction = ToTarget / Distance;
		if (MoveDistance >= Distance)
		{
			Slot.State.Position = TargetPosition;
			ResolveTravelerArrival(SlotIndex, SnapshotEntry, Slot.State.Position, ArrivalCollisionCycles);
			continue;
		}

		Slot.State.Position += Direction * MoveDistance;
		Slot.State.Rotation = Direction.Rotation().Quaternion() * Slot.ProfileRotation;
		++DenseIndex;
	}

	const uint64 EndCycles = FPlatformTime::Cycles64();
	Diagnostics.LastSimulationMs = bAnySimulated ? FPlatformTime::ToMilliseconds64(EndCycles - StartCycles) : 0.0;
	Diagnostics.LastArrivalCollisionMs = bAnySimulated ? FPlatformTime::ToMilliseconds64(ArrivalCollisionCycles) : 0.0;
	if (bAnySimulated)
	{
		++Diagnostics.SimulationTickCount;
		const double SampleCount = static_cast<double>(Diagnostics.SimulationTickCount);
		Diagnostics.AverageSimulationMs += (Diagnostics.LastSimulationMs - Diagnostics.AverageSimulationMs) / SampleCount;
		Diagnostics.AverageArrivalCollisionMs += (Diagnostics.LastArrivalCollisionMs - Diagnostics.AverageArrivalCollisionMs) / SampleCount;
		Diagnostics.MaxSimulationMs = FMath::Max(Diagnostics.MaxSimulationMs, Diagnostics.LastSimulationMs);
		Diagnostics.MaxArrivalCollisionMs = FMath::Max(Diagnostics.MaxArrivalCollisionMs, Diagnostics.LastArrivalCollisionMs);
		bDirty = true;
	}
}

void UT66OutgoingTravelerPoolSubsystem::UploadLiveState()
{
	if (!NiagaraComponent)
	{
		return;
	}

	const uint64 StartCycles = FPlatformTime::Cycles64();
	const int32 LiveCount = DenseSlots.Num();
	PositionUpload.Reset(LiveCount);
	RotationUpload.Reset(LiveCount);
	ScaleUpload.Reset(LiveCount);
	ColorUpload.Reset(LiveCount);
	MeshIndexUpload.Reset(LiveCount);

	for (const int32 SlotIndex : DenseSlots)
	{
		const FT66OutgoingTravelerVisualState& State = Slots[SlotIndex].State;
		PositionUpload.Add(State.Position);
		RotationUpload.Add(State.Rotation);
		ScaleUpload.Add(State.Scale);
		ColorUpload.Add(State.Color);
		MeshIndexUpload.Add(State.MeshIndex);
	}

	const uint64 PackEndCycles = FPlatformTime::Cycles64();
	const bool bActivateAfterUpload = LiveCount > 0 && !NiagaraComponent->IsActive();
	if (bActivateAfterUpload)
	{
		NiagaraComponent->SetComponentTickEnabled(true);
	}

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
		NiagaraComponent,
		T66TravelerPositionsParam,
		PositionUpload);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(
		NiagaraComponent,
		T66TravelerRotationsParam,
		RotationUpload);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
		NiagaraComponent,
		T66TravelerScalesParam,
		ScaleUpload);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayColor(
		NiagaraComponent,
		T66TravelerColorsParam,
		ColorUpload);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayInt32(
		NiagaraComponent,
		T66TravelerMeshIndicesParam,
		MeshIndexUpload);
	NiagaraComponent->SetVariableInt(T66TravelerLiveCountParam, LiveCount);
	if (bActivateAfterUpload)
	{
		NiagaraComponent->Activate(true);
		NiagaraComponent->ReinitializeSystem();
	}

	Diagnostics.LiveCount = LiveCount;
	Diagnostics.PeakLiveCount = FMath::Max(Diagnostics.PeakLiveCount, LiveCount);
	Diagnostics.LastUploadedLiveCount = LiveCount;
	const uint64 EndCycles = FPlatformTime::Cycles64();
	Diagnostics.LastPackMs = FPlatformTime::ToMilliseconds64(PackEndCycles - StartCycles);
	Diagnostics.LastNiagaraArrayUploadMs = FPlatformTime::ToMilliseconds64(EndCycles - PackEndCycles);
	Diagnostics.LastUploadMs = FPlatformTime::ToMilliseconds64(EndCycles - StartCycles);
	++Diagnostics.UploadCount;
	const double UploadCount = static_cast<double>(Diagnostics.UploadCount);
	Diagnostics.AveragePackMs += (Diagnostics.LastPackMs - Diagnostics.AveragePackMs) / UploadCount;
	Diagnostics.AverageNiagaraArrayUploadMs += (Diagnostics.LastNiagaraArrayUploadMs - Diagnostics.AverageNiagaraArrayUploadMs) / UploadCount;
	Diagnostics.AverageUploadMs += (Diagnostics.LastUploadMs - Diagnostics.AverageUploadMs) / UploadCount;
	Diagnostics.MaxPackMs = FMath::Max(Diagnostics.MaxPackMs, Diagnostics.LastPackMs);
	Diagnostics.MaxNiagaraArrayUploadMs = FMath::Max(Diagnostics.MaxNiagaraArrayUploadMs, Diagnostics.LastNiagaraArrayUploadMs);
	Diagnostics.MaxUploadMs = FMath::Max(Diagnostics.MaxUploadMs, Diagnostics.LastUploadMs);
	bDirty = false;

	if (CVarT66OutgoingTravelerPoolVerbose.GetValueOnGameThread() != 0)
	{
		UE_LOG(LogT66OutgoingTravelerPool, Display,
			TEXT("[OutgoingTravelerPool] phase=Upload live=%d uploadMs=%.3f packMs=%.3f niagaraArrayMs=%.3f fired=%d canceled=%d dropped=%d suppressedMeshes=%d"),
			LiveCount,
			Diagnostics.LastUploadMs,
			Diagnostics.LastPackMs,
			Diagnostics.LastNiagaraArrayUploadMs,
			Diagnostics.FiredTotal,
			Diagnostics.CanceledTotal,
			Diagnostics.DroppedTotal,
			Diagnostics.MainMeshSuppressedTotal);
	}
}

void UT66OutgoingTravelerPoolSubsystem::SampleProofMetrics(const float DeltaTime)
{
	if (ManifestPath.IsEmpty())
	{
		return;
	}

	const float NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (ManifestSampleStartSeconds <= 0.f)
	{
		ManifestSampleStartSeconds = NowSeconds;
	}

	if (DeltaTime > SMALL_NUMBER)
	{
		const double Fps = 1.0 / static_cast<double>(DeltaTime);
		FpsSum += Fps;
		FpsMin = FpsMin <= 0.0 ? Fps : FMath::Min(FpsMin, Fps);
		++FpsSamples;
	}

	const uint32 GpuCycles = RHIGetGPUFrameCycles(0);
	if (GpuCycles > 0)
	{
		LastGpuFrameMs = FPlatformTime::ToMilliseconds(GpuCycles);
		GpuFrameMsSum += LastGpuFrameMs;
		++GpuFrameSamples;
	}
	LastDrawCalls = GNumDrawCallsRHI[0];
	MaxDrawCalls = FMath::Max(MaxDrawCalls, LastDrawCalls);

	const bool bAfterPoolActivity = FirstActivityTimeSeconds > 0.f
		&& Diagnostics.FiredTotal > 0
		&& (NowSeconds - FirstActivityTimeSeconds) >= 0.05f;
	const bool bBaselineWindow = !IsEnabled()
		&& Diagnostics.FiredTotal == 0
		&& (NowSeconds - ManifestSampleStartSeconds) >= 8.0f;
	if (!bManifestWritten
		&& FpsSamples >= 12
		&& (bAfterPoolActivity || bBaselineWindow))
	{
		WriteProofManifest(TEXT("sample-window-complete"));
		bManifestWritten = true;
	}
}

void UT66OutgoingTravelerPoolSubsystem::WriteProofManifest(const TCHAR* Reason)
{
	if (ManifestPath.IsEmpty())
	{
		return;
	}

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("tool"), TEXT("T66OutgoingTravelerPoolGameplay"));
	Root->SetStringField(TEXT("reason"), Reason ? Reason : TEXT("unspecified"));
	Root->SetNumberField(TEXT("capacity"), MaxOutgoingTravelers);
	Root->SetNumberField(TEXT("live_count"), Diagnostics.LiveCount);
	Root->SetNumberField(TEXT("peak_live_count"), Diagnostics.PeakLiveCount);
	Root->SetNumberField(TEXT("fired_total"), Diagnostics.FiredTotal);
	Root->SetNumberField(TEXT("canceled_total"), Diagnostics.CanceledTotal);
	Root->SetNumberField(TEXT("dropped_total"), Diagnostics.DroppedTotal);
	Root->SetNumberField(TEXT("simulated_fired_total"), Diagnostics.SimulatedFiredTotal);
	Root->SetNumberField(TEXT("simulated_arrived_total"), Diagnostics.SimulatedArrivedTotal);
	Root->SetNumberField(TEXT("simulated_expired_total"), Diagnostics.SimulatedExpiredTotal);
	Root->SetNumberField(TEXT("upload_count"), Diagnostics.UploadCount);
	Root->SetNumberField(TEXT("simulation_tick_count"), Diagnostics.SimulationTickCount);
	Root->SetNumberField(TEXT("target_snapshot_build_count"), Diagnostics.TargetSnapshotBuildCount);
	Root->SetNumberField(TEXT("last_target_snapshot_count"), Diagnostics.LastTargetSnapshotCount);
	Root->SetNumberField(TEXT("arrival_check_total"), Diagnostics.ArrivalCheckTotal);
	Root->SetNumberField(TEXT("arrival_damage_applied_total"), Diagnostics.ArrivalDamageAppliedTotal);
	Root->SetNumberField(TEXT("arrival_callback_total"), Diagnostics.ArrivalCallbackTotal);
	Root->SetNumberField(TEXT("arrival_fizzle_no_target_total"), Diagnostics.ArrivalFizzleNoTargetTotal);
	Root->SetNumberField(TEXT("last_uploaded_live_count"), Diagnostics.LastUploadedLiveCount);
	Root->SetNumberField(TEXT("last_upload_ms"), Diagnostics.LastUploadMs);
	Root->SetNumberField(TEXT("last_pack_ms"), Diagnostics.LastPackMs);
	Root->SetNumberField(TEXT("last_niagara_array_upload_ms"), Diagnostics.LastNiagaraArrayUploadMs);
	Root->SetNumberField(TEXT("last_simulation_ms"), Diagnostics.LastSimulationMs);
	Root->SetNumberField(TEXT("last_target_snapshot_ms"), Diagnostics.LastTargetSnapshotMs);
	Root->SetNumberField(TEXT("last_arrival_collision_ms"), Diagnostics.LastArrivalCollisionMs);
	Root->SetNumberField(TEXT("average_upload_ms"), Diagnostics.AverageUploadMs);
	Root->SetNumberField(TEXT("average_pack_ms"), Diagnostics.AveragePackMs);
	Root->SetNumberField(TEXT("average_niagara_array_upload_ms"), Diagnostics.AverageNiagaraArrayUploadMs);
	Root->SetNumberField(TEXT("average_simulation_ms"), Diagnostics.AverageSimulationMs);
	Root->SetNumberField(TEXT("average_target_snapshot_ms"), Diagnostics.AverageTargetSnapshotMs);
	Root->SetNumberField(TEXT("average_arrival_collision_ms"), Diagnostics.AverageArrivalCollisionMs);
	Root->SetNumberField(TEXT("max_upload_ms"), Diagnostics.MaxUploadMs);
	Root->SetNumberField(TEXT("max_pack_ms"), Diagnostics.MaxPackMs);
	Root->SetNumberField(TEXT("max_niagara_array_upload_ms"), Diagnostics.MaxNiagaraArrayUploadMs);
	Root->SetNumberField(TEXT("max_simulation_ms"), Diagnostics.MaxSimulationMs);
	Root->SetNumberField(TEXT("max_target_snapshot_ms"), Diagnostics.MaxTargetSnapshotMs);
	Root->SetNumberField(TEXT("max_arrival_collision_ms"), Diagnostics.MaxArrivalCollisionMs);
	Root->SetNumberField(TEXT("main_mesh_suppressed_total"), Diagnostics.MainMeshSuppressedTotal);
	Root->SetNumberField(TEXT("rough_fps_avg_from_tick_delta"), FpsSamples > 0 ? FpsSum / static_cast<double>(FpsSamples) : 0.0);
	Root->SetNumberField(TEXT("rough_fps_min_from_tick_delta"), FpsMin);
	Root->SetStringField(TEXT("rough_gpu_frame_ms_avg"), T66FormatMaybeNumber(
		GpuFrameSamples > 0 ? GpuFrameMsSum / static_cast<double>(GpuFrameSamples) : -1.0));
	Root->SetStringField(TEXT("rough_gpu_frame_ms_last"), T66FormatMaybeNumber(LastGpuFrameMs));
	Root->SetStringField(TEXT("draw_calls_last"), LastDrawCalls >= 0 ? FString::FromInt(LastDrawCalls) : TEXT("Unavailable"));
	Root->SetStringField(TEXT("draw_calls_max"), MaxDrawCalls >= 0 ? FString::FromInt(MaxDrawCalls) : TEXT("Unavailable"));
	Root->SetBoolField(TEXT("uses_niagara_array_path"), true);
	Root->SetStringField(TEXT("positions_parameter"), T66TravelerPositionsParam.ToString());
	Root->SetStringField(TEXT("rotations_parameter"), T66TravelerRotationsParam.ToString());
	Root->SetStringField(TEXT("scales_parameter"), T66TravelerScalesParam.ToString());
	Root->SetStringField(TEXT("colors_parameter"), T66TravelerColorsParam.ToString());
	Root->SetStringField(TEXT("mesh_indices_parameter"), T66TravelerMeshIndicesParam.ToString());
	Root->SetNumberField(TEXT("temporary_projectile_mesh_slot_count"), TemporaryProjectileMeshSlotCount);
	Root->SetNumberField(TEXT("traveler_visual_profile_slot_base"), TravelerVisualProfileSlotBase);
	Root->SetNumberField(TEXT("traveler_visual_profile_slot_count"), TravelerVisualProfileSlotCount);
	Root->SetBoolField(TEXT("niagara_component_exists"), NiagaraComponent != nullptr);
	Root->SetBoolField(TEXT("niagara_system_exists"), NiagaraSystem != nullptr);
	if (NiagaraSystem)
	{
		Root->SetBoolField(TEXT("niagara_system_ready_to_run"), NiagaraSystem->IsReadyToRun());
		Root->SetStringField(TEXT("niagara_system_path"), NiagaraSystem->GetPathName());
	}
	if (NiagaraComponent)
	{
		Root->SetBoolField(TEXT("niagara_component_registered"), NiagaraComponent->IsRegistered());
		Root->SetBoolField(TEXT("niagara_component_active"), NiagaraComponent->IsActive());
		Root->SetBoolField(TEXT("niagara_component_visible"), NiagaraComponent->IsVisible());
		Root->SetBoolField(TEXT("niagara_component_hidden_in_game"), NiagaraComponent->bHiddenInGame);
		Root->SetBoolField(TEXT("niagara_component_tick_enabled"), NiagaraComponent->IsComponentTickEnabled());
		Root->SetBoolField(TEXT("niagara_component_complete"), NiagaraComponent->IsComplete());
		Root->SetNumberField(TEXT("niagara_execution_state"), static_cast<int32>(NiagaraComponent->GetExecutionState()));
		Root->SetNumberField(TEXT("niagara_requested_execution_state"), static_cast<int32>(NiagaraComponent->GetRequestedExecutionState()));
		const FVector ComponentLocation = NiagaraComponent->GetComponentLocation();
		Root->SetNumberField(TEXT("niagara_component_location_x"), ComponentLocation.X);
		Root->SetNumberField(TEXT("niagara_component_location_y"), ComponentLocation.Y);
		Root->SetNumberField(TEXT("niagara_component_location_z"), ComponentLocation.Z);
		const FBoxSphereBounds ComponentBounds = NiagaraComponent->Bounds;
		Root->SetNumberField(TEXT("niagara_bounds_origin_x"), ComponentBounds.Origin.X);
		Root->SetNumberField(TEXT("niagara_bounds_origin_y"), ComponentBounds.Origin.Y);
		Root->SetNumberField(TEXT("niagara_bounds_origin_z"), ComponentBounds.Origin.Z);
		Root->SetNumberField(TEXT("niagara_bounds_radius"), ComponentBounds.SphereRadius);
		const auto SystemInstanceController = NiagaraComponent->GetSystemInstanceController();
		Root->SetBoolField(TEXT("niagara_system_instance_valid"), SystemInstanceController.IsValid());
		if (SystemInstanceController.IsValid())
		{
			Root->SetNumberField(TEXT("niagara_system_instance_age"), SystemInstanceController->GetAge());
			Root->SetBoolField(TEXT("niagara_system_instance_complete"), SystemInstanceController->IsComplete());
			Root->SetBoolField(TEXT("niagara_system_instance_pending_spawn"), SystemInstanceController->IsPendingSpawn());
			Root->SetBoolField(TEXT("niagara_system_instance_paused"), SystemInstanceController->IsPaused());
			Root->SetBoolField(TEXT("niagara_data_interfaces_initialized"), SystemInstanceController->GetAreDataInterfacesInitialized());
			const FBox FixedBounds = SystemInstanceController->GetSystemFixedBounds();
			Root->SetNumberField(TEXT("niagara_fixed_bounds_min_x"), FixedBounds.Min.X);
			Root->SetNumberField(TEXT("niagara_fixed_bounds_min_y"), FixedBounds.Min.Y);
			Root->SetNumberField(TEXT("niagara_fixed_bounds_min_z"), FixedBounds.Min.Z);
			Root->SetNumberField(TEXT("niagara_fixed_bounds_max_x"), FixedBounds.Max.X);
			Root->SetNumberField(TEXT("niagara_fixed_bounds_max_y"), FixedBounds.Max.Y);
			Root->SetNumberField(TEXT("niagara_fixed_bounds_max_z"), FixedBounds.Max.Z);
		}
	}

	FString JsonText;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		UE_LOG(LogT66OutgoingTravelerPool, Warning, TEXT("[OutgoingTravelerPool] phase=ManifestWrite status=serialize-failed path=%s"),
			*ManifestPath);
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(ManifestPath), true);
	const bool bWrote = FFileHelper::SaveStringToFile(JsonText, *ManifestPath);
	UE_LOG(LogT66OutgoingTravelerPool, Display,
		TEXT("[OutgoingTravelerPool] phase=ManifestWrite status=%s path=%s live=%d peak=%d fpsAvg=%.2f gpuMsAvg=%s drawCallsMax=%s"),
		bWrote ? TEXT("ok") : TEXT("write-failed"),
		*ManifestPath,
		Diagnostics.LiveCount,
		Diagnostics.PeakLiveCount,
		FpsSamples > 0 ? FpsSum / static_cast<double>(FpsSamples) : 0.0,
		*T66FormatMaybeNumber(GpuFrameSamples > 0 ? GpuFrameMsSum / static_cast<double>(GpuFrameSamples) : -1.0),
		MaxDrawCalls >= 0 ? *FString::FromInt(MaxDrawCalls) : TEXT("Unavailable"));
}
