// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MobLootSubsystem.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Core/T66GameInstance.h"
#include "Core/T66ShelvedFeatureGate.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66VisualUtil.h"
#include "Dom/JsonObject.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Stats/Stats.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MobLoot, Log, All);

namespace
{
	static TAutoConsoleVariable<int32> CVarT66MobLootEnabled(
		TEXT("T66.MobLoot.Enabled"),
		0,
		TEXT("Enables the actorless Mob Loot world-drop subsystem when Mob Loot is not shelved."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66MobLootRenderEnabled(
		TEXT("T66.MobLoot.RenderEnabled"),
		1,
		TEXT("Enables the persistent Niagara renderer for actorless Mob Loot drops."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66MobLootVerbose(
		TEXT("T66.MobLoot.Verbose"),
		0,
		TEXT("Logs Mob Loot allocation, collection, and upload diagnostics."),
		ECVF_Default);

	const TCHAR* T66MobLootPoolAssetPath =
		TEXT("/Game/VFX/Foundation/MobLoot/NS_MobLootPool.NS_MobLootPool");
	const FName T66MobLootPositionsParam(TEXT("User.MobLootPositions"));
	const FName T66MobLootScalesParam(TEXT("User.MobLootScales"));
	const FName T66MobLootColorsParam(TEXT("User.MobLootColors"));
	const FName T66MobLootQuantitiesParam(TEXT("User.MobLootQuantities"));
	const FName T66MobLootLiveCountParam(TEXT("User.MobLootLiveCount"));

	FObjectKey MakeCollectorKey(const FT66MobLootCollectorRef& Collector)
	{
		return Collector.Collector.IsValid()
			? FObjectKey(Collector.Collector.Get())
			: FObjectKey();
	}

	bool HasCollectorIdentity(const FT66MobLootCollectorRef& Collector)
	{
		return Collector.Collector.IsValid() || !Collector.CollectorID.IsNone();
	}

	ET66Difficulty GetSelectedMobLootDifficulty(const AActor* SourceActor)
	{
		const UWorld* World = SourceActor ? SourceActor->GetWorld() : nullptr;
		const UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
		return T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	}
}

int32 UT66MobLootSubsystem::GetDifficultyDropIndex(const ET66Difficulty Difficulty)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Impossible:
		return 4;
	case ET66Difficulty::VeryHard:
		return 3;
	case ET66Difficulty::Hard:
		return 2;
	case ET66Difficulty::Medium:
		return 1;
	case ET66Difficulty::Easy:
	default:
		return 0;
	}
}

int32 UT66MobLootSubsystem::GetDeathMobLootQuantityForDifficulty(
	const ET66Difficulty Difficulty,
	const bool bIsMiniBoss)
{
	const int32 DifficultyIndex = GetDifficultyDropIndex(Difficulty);
	return bIsMiniBoss
		? 4 + (2 * DifficultyIndex)
		: 1 + DifficultyIndex;
}

void UT66MobLootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeSlots();
}

void UT66MobLootSubsystem::Deinitialize()
{
	if (NiagaraComponent)
	{
		NiagaraComponent->DeactivateImmediate();
		NiagaraComponent->DestroyComponent();
		NiagaraComponent = nullptr;
	}
	if (FallbackVisualComponent)
	{
		FallbackVisualComponent->DestroyComponent();
		FallbackVisualComponent = nullptr;
	}
	if (RenderHost)
	{
		RenderHost->Destroy();
		RenderHost = nullptr;
	}

	RenderRoot = nullptr;
	NiagaraSystem = nullptr;
	Slots.Reset();
	FreeSlots.Reset();
	DenseSlots.Reset();
	PositionUpload.Reset();
	ScaleUpload.Reset();
	ColorUpload.Reset();
	QuantityUpload.Reset();
	bInitializedSlots = false;
	Super::Deinitialize();
}

void UT66MobLootSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsEnabled())
	{
		return;
	}

	const uint64 StartCycles = FPlatformTime::Cycles64();
	TickExpirations(DeltaTime);
	const uint64 EndCycles = FPlatformTime::Cycles64();

	Diagnostics.LastTickMs = FPlatformTime::ToMilliseconds64(EndCycles - StartCycles);
	++Diagnostics.TickCount;
	const double TickCount = static_cast<double>(Diagnostics.TickCount);
	Diagnostics.AverageTickMs += (Diagnostics.LastTickMs - Diagnostics.AverageTickMs) / TickCount;
	Diagnostics.MaxTickMs = FMath::Max(Diagnostics.MaxTickMs, Diagnostics.LastTickMs);

	if (CVarT66MobLootRenderEnabled.GetValueOnGameThread() != 0 && EnsureNiagaraComponent() && bDirty)
	{
		UploadLiveState();
	}
}

TStatId UT66MobLootSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UT66MobLootSubsystem, STATGROUP_Tickables);
}

bool UT66MobLootSubsystem::IsTickable() const
{
	return !IsTemplate();
}

bool UT66MobLootSubsystem::SpawnMobLoot(const FT66MobLootSpawnParams& Params, FT66MobLootHandle& OutHandle)
{
	OutHandle.Reset();
	if (!IsEnabled())
	{
		return false;
	}

	InitializeSlots();
	if (FreeSlots.Num() <= 0)
	{
		++Diagnostics.DroppedWhenFullTotal;
		UE_LOG(LogT66MobLoot, Warning, TEXT("[MobLoot] phase=Spawn status=pool-full capacity=%d"), MaxMobLootDrops);
		return false;
	}

	const int32 Quantity = FMath::Max(0, Params.Quantity);
	if (Quantity <= 0)
	{
		return false;
	}

	const int32 SlotIndex = FreeSlots.Pop(EAllowShrinking::No);
	FT66MobLootSlot& Slot = Slots[SlotIndex];
	Slot.Position = Params.Position;
	Slot.SourceID = Params.SourceID;
	Slot.Color = Params.Color;
	if (Slot.Color.A <= 0.0f)
	{
		Slot.Color.A = 1.0f;
	}
	Slot.Scale = FMath::Max(0.01f, Params.Scale);
	Slot.RemainingLifetimeSeconds = FMath::Max(0.0f, Params.LifetimeSeconds);
	Slot.Quantity = Quantity;
	Slot.GoldValue = FMath::Max(0, Params.GoldValue);
	Slot.bReserved = false;
	Slot.ReservedCollectorKey = FObjectKey();
	Slot.ReservedCollectorID = NAME_None;
	Slot.ReservedCollectorType = ET66MobLootCollectorType::System;
	Slot.bActive = true;
	Slot.Generation = Slot.Generation == MAX_int32 ? 1 : Slot.Generation + 1;
	if (Slot.Generation == 0)
	{
		Slot.Generation = 1;
	}
	Slot.DenseIndex = DenseSlots.Add(SlotIndex);

	OutHandle.SlotIndex = SlotIndex;
	OutHandle.Generation = Slot.Generation;

	Diagnostics.LiveWorldDropCount = DenseSlots.Num();
	Diagnostics.PeakLiveWorldDropCount = FMath::Max(Diagnostics.PeakLiveWorldDropCount, Diagnostics.LiveWorldDropCount);
	++Diagnostics.SpawnedDropTotal;
	bDirty = true;

	if (CVarT66MobLootVerbose.GetValueOnGameThread() != 0)
	{
		UE_LOG(LogT66MobLoot, Display,
			TEXT("[MobLoot] phase=Spawn slot=%d generation=%d live=%d quantity=%d goldTotal=%d source=%s pos=%s"),
			OutHandle.SlotIndex,
			OutHandle.Generation,
			Diagnostics.LiveWorldDropCount,
			Slot.Quantity,
			Slot.GoldValue,
			*Slot.SourceID.ToString(),
			*Slot.Position.ToCompactString());
	}
	return true;
}

bool UT66MobLootSubsystem::QueryMobLootTarget(
	const FT66MobLootQueryFilter& Filter,
	FT66MobLootHandle& OutHandle,
	FVector& OutPosition) const
{
	OutHandle.Reset();
	OutPosition = FVector::ZeroVector;

	const uint64 StartCycles = FPlatformTime::Cycles64();
	float BestDistanceSq = TNumericLimits<float>::Max();
	int32 BestSlotIndex = INDEX_NONE;
	const float SearchRadiusSq = Filter.SearchRadius > 0.0f
		? FMath::Square(Filter.SearchRadius)
		: TNumericLimits<float>::Max();

	for (const int32 SlotIndex : DenseSlots)
	{
		if (!Slots.IsValidIndex(SlotIndex))
		{
			continue;
		}

		const FT66MobLootSlot& Slot = Slots[SlotIndex];
		if (!IsEligibleForQuery(Slot, Filter, nullptr))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(Filter.Origin, Slot.Position);
		if (DistanceSq > SearchRadiusSq || DistanceSq >= BestDistanceSq)
		{
			continue;
		}

		BestDistanceSq = DistanceSq;
		BestSlotIndex = SlotIndex;
	}

	const uint64 EndCycles = FPlatformTime::Cycles64();
	const_cast<UT66MobLootSubsystem*>(this)->Diagnostics.LastQueryMs =
		FPlatformTime::ToMilliseconds64(EndCycles - StartCycles);

	if (!Slots.IsValidIndex(BestSlotIndex))
	{
		return false;
	}

	const FT66MobLootSlot& BestSlot = Slots[BestSlotIndex];
	OutHandle.SlotIndex = BestSlotIndex;
	OutHandle.Generation = BestSlot.Generation;
	OutPosition = BestSlot.Position;
	return true;
}

FT66MobLootReservationResult UT66MobLootSubsystem::QueryAndReserveMobLoot(
	const FT66MobLootQueryFilter& Filter,
	const FT66MobLootCollectorRef& Collector)
{
	FT66MobLootHandle Handle;
	FVector Position = FVector::ZeroVector;
	if (!QueryMobLootTarget(Filter, Handle, Position))
	{
		return FT66MobLootReservationResult{};
	}

	FT66MobLootReservationResult Result;
	ReserveMobLoot(Handle, Collector, Result);
	return Result;
}

bool UT66MobLootSubsystem::ReserveMobLoot(
	const FT66MobLootHandle& Handle,
	const FT66MobLootCollectorRef& Collector,
	FT66MobLootReservationResult& OutReservation)
{
	OutReservation = FT66MobLootReservationResult{};
	if (!IsHandleCurrent(Handle) || !HasCollectorIdentity(Collector))
	{
		++Diagnostics.ReservationDeniedTotal;
		return false;
	}

	FT66MobLootSlot& Slot = Slots[Handle.SlotIndex];
	if (Slot.bReserved && !IsCollectorMatch(Slot, Collector))
	{
		++Diagnostics.ReservationDeniedTotal;
		return false;
	}

	const bool bWasReserved = Slot.bReserved;
	Slot.bReserved = true;
	Slot.ReservedCollectorKey = MakeCollectorKey(Collector);
	Slot.ReservedCollectorID = Collector.CollectorID;
	Slot.ReservedCollectorType = Collector.CollectorType;
	if (!bWasReserved)
	{
		++Diagnostics.ReservationTotal;
	}
	bDirty = true;

	OutReservation = BuildReservationResult(Slot, Handle, true);
	return true;
}

bool UT66MobLootSubsystem::ReleaseMobLootReservation(
	const FT66MobLootHandle& Handle,
	const FT66MobLootCollectorRef& Collector)
{
	if (!IsHandleCurrent(Handle))
	{
		return false;
	}

	FT66MobLootSlot& Slot = Slots[Handle.SlotIndex];
	if (!Slot.bReserved || !IsCollectorMatch(Slot, Collector))
	{
		return false;
	}

	Slot.bReserved = false;
	Slot.ReservedCollectorKey = FObjectKey();
	Slot.ReservedCollectorID = NAME_None;
	Slot.ReservedCollectorType = ET66MobLootCollectorType::System;
	++Diagnostics.ReservationReleaseTotal;
	bDirty = true;
	return true;
}

FT66MobLootCollectResult UT66MobLootSubsystem::CollectReservedMobLoot(
	const FT66MobLootHandle& Handle,
	const FT66MobLootCollectorRef& Collector,
	const float MaxDistance)
{
	FT66MobLootCollectResult Result;
	const uint64 StartCycles = FPlatformTime::Cycles64();
	if (!IsHandleCurrent(Handle))
	{
		Diagnostics.LastCollectMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
		return Result;
	}

	FT66MobLootSlot& Slot = Slots[Handle.SlotIndex];
	if (!Slot.bReserved || !IsCollectorMatch(Slot, Collector))
	{
		Diagnostics.LastCollectMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
		return Result;
	}

	if (MaxDistance > 0.0f && Collector.Collector.IsValid())
	{
		const float MaxDistanceSq = FMath::Square(MaxDistance);
		if (FVector::DistSquared(Collector.Collector->GetActorLocation(), Slot.Position) > MaxDistanceSq)
		{
			Diagnostics.LastCollectMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
			return Result;
		}
	}

	ReleaseSlot(Handle.SlotIndex, EReleaseReason::Collected, &Result);
	Diagnostics.LastCollectMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
	return Result;
}

FT66MobLootCollectResult UT66MobLootSubsystem::CollectMobLootAt(
	const FVector& Center,
	const float Radius,
	const FT66MobLootCollectorRef& Collector,
	const int32 MaxDrops)
{
	FT66MobLootCollectResult Result;
	const uint64 StartCycles = FPlatformTime::Cycles64();
	const float RadiusSq = FMath::Square(FMath::Max(0.0f, Radius));
	const int32 DropLimit = MaxDrops == INDEX_NONE ? MAX_int32 : FMath::Max(0, MaxDrops);

	for (int32 DenseIndex = 0; DenseIndex < DenseSlots.Num() && Result.DropsCollected < DropLimit;)
	{
		const int32 SlotIndex = DenseSlots[DenseIndex];
		if (!Slots.IsValidIndex(SlotIndex))
		{
			++DenseIndex;
			continue;
		}

		const FT66MobLootSlot& Slot = Slots[SlotIndex];
		if (!Slot.bActive)
		{
			++DenseIndex;
			continue;
		}

		if (Slot.bReserved && !IsCollectorMatch(Slot, Collector))
		{
			++DenseIndex;
			continue;
		}

		if (FVector::DistSquared(Center, Slot.Position) > RadiusSq)
		{
			++DenseIndex;
			continue;
		}

		ReleaseSlot(SlotIndex, EReleaseReason::Collected, &Result);
	}

	Diagnostics.LastCollectMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
	return Result;
}

int32 UT66MobLootSubsystem::GetActiveMobLootDropCount() const
{
	return DenseSlots.Num();
}

void UT66MobLootSubsystem::PushMobLootRenderArrays(FT66MobLootRenderArrays& OutArrays) const
{
	OutArrays.Reset();
	OutArrays.Positions.Reserve(DenseSlots.Num());
	OutArrays.Scales.Reserve(DenseSlots.Num());
	OutArrays.Colors.Reserve(DenseSlots.Num());
	OutArrays.Quantities.Reserve(DenseSlots.Num());

	for (const int32 SlotIndex : DenseSlots)
	{
		if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex].bActive)
		{
			continue;
		}

		const FT66MobLootSlot& Slot = Slots[SlotIndex];
		FLinearColor RenderColor = Slot.Color;
		if (Slot.bReserved)
		{
			RenderColor.A *= 0.72f;
		}
		OutArrays.Positions.Add(Slot.Position);
		OutArrays.Scales.Add(Slot.Scale);
		OutArrays.Colors.Add(RenderColor);
		OutArrays.Quantities.Add(Slot.Quantity);
	}
}

bool UT66MobLootSubsystem::SpawnMobLootFromNonBossDeath(
	AActor* SourceActor,
	const FName SourceID,
	const bool bIsMiniBoss,
	FT66MobLootHandle& OutHandle)
{
	OutHandle.Reset();
	if (!SourceActor)
	{
		return false;
	}

	const ET66Difficulty Difficulty = GetSelectedMobLootDifficulty(SourceActor);
	const int32 Quantity = FMath::Max(0, GetDeathMobLootQuantityForDifficulty(Difficulty, bIsMiniBoss));
	if (Quantity <= 0)
	{
		return false;
	}

	FT66MobLootSpawnParams Params;
	Params.Position = SourceActor->GetActorLocation();
	Params.Quantity = Quantity;
	Params.GoldValue = Quantity * MobLootGoldPerUnit;
	Params.SourceID = SourceID;
	Params.Color = bIsMiniBoss
		? FLinearColor(1.0f, 0.86f, 0.28f, 1.0f)
		: FLinearColor(0.90f, 0.72f, 0.18f, 1.0f);
	Params.Scale = bIsMiniBoss ? 1.35f : 1.0f;
	Params.LifetimeSeconds = 120.0f;
	return SpawnMobLoot(Params, OutHandle);
}

bool UT66MobLootSubsystem::IsEnabled()
{
	return FT66ShelvedFeatureGate::IsMobLootEnabled()
		&& CVarT66MobLootEnabled.GetValueOnGameThread() != 0;
}

#if !UE_BUILD_SHIPPING
void UT66MobLootSubsystem::ClearAllMobLootForAutomation()
{
	for (int32 DenseIndex = 0; DenseIndex < DenseSlots.Num();)
	{
		const int32 SlotIndex = DenseSlots[DenseIndex];
		ReleaseSlot(SlotIndex, EReleaseReason::Cleared, nullptr);
	}
	bDirty = true;
}
#endif

void UT66MobLootSubsystem::InitializeSlots()
{
	if (bInitializedSlots)
	{
		return;
	}

	Slots.SetNum(MaxMobLootDrops);
	FreeSlots.Reset(MaxMobLootDrops);
	for (int32 Index = MaxMobLootDrops - 1; Index >= 0; --Index)
	{
		FreeSlots.Add(Index);
	}
	DenseSlots.Reserve(MaxMobLootDrops);
	PositionUpload.Reserve(MaxMobLootDrops);
	ScaleUpload.Reserve(MaxMobLootDrops);
	ColorUpload.Reserve(MaxMobLootDrops);
	QuantityUpload.Reserve(MaxMobLootDrops);
	bInitializedSlots = true;
}

bool UT66MobLootSubsystem::EnsureNiagaraComponent()
{
	if (NiagaraComponent && NiagaraComponent->IsRegistered() && EnsureFallbackVisualComponent())
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (!RenderHost)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("T66MobLootPoolHost");
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;
		RenderHost = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
		if (RenderHost)
		{
			RenderHost->SetActorHiddenInGame(false);
			RenderHost->SetActorEnableCollision(false);
			RenderRoot = NewObject<USceneComponent>(RenderHost, TEXT("MobLootPoolRoot"), RF_Transient);
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

	EnsureFallbackVisualComponent();

	if (!NiagaraSystem)
	{
		NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, T66MobLootPoolAssetPath);
		if (!NiagaraSystem)
		{
			UE_LOG(LogT66MobLoot, Warning, TEXT("[MobLoot] Missing Niagara system %s; using static visible fallback."), T66MobLootPoolAssetPath);
			return FallbackVisualComponent != nullptr;
		}
	}

	NiagaraComponent = NewObject<UNiagaraComponent>(RenderHost, TEXT("MobLootPoolNiagara"), RF_Transient);
	if (!NiagaraComponent)
	{
		return FallbackVisualComponent != nullptr;
	}

	NiagaraComponent->SetAsset(NiagaraSystem);
	NiagaraComponent->bAutoActivate = true;
	NiagaraComponent->SetUsingAbsoluteLocation(true);
	NiagaraComponent->SetUsingAbsoluteRotation(true);
	NiagaraComponent->SetUsingAbsoluteScale(true);
	NiagaraComponent->SetupAttachment(RenderRoot);
	NiagaraComponent->RegisterComponent();
	NiagaraComponent->Activate(true);
	bDirty = true;

	UE_LOG(LogT66MobLoot, Display, TEXT("[MobLoot] phase=EnsureNiagara status=ready asset=%s capacity=%d"),
		T66MobLootPoolAssetPath,
		MaxMobLootDrops);
	return true;
}

bool UT66MobLootSubsystem::IsHandleCurrent(const FT66MobLootHandle& Handle) const
{
	return Handle.IsValid()
		&& Slots.IsValidIndex(Handle.SlotIndex)
		&& Slots[Handle.SlotIndex].bActive
		&& Slots[Handle.SlotIndex].Generation == Handle.Generation;
}

bool UT66MobLootSubsystem::IsCollectorMatch(
	const FT66MobLootSlot& Slot,
	const FT66MobLootCollectorRef& Collector) const
{
	if (!Slot.bReserved)
	{
		return false;
	}

	if (Slot.ReservedCollectorType != Collector.CollectorType)
	{
		return false;
	}

	const FObjectKey CollectorKey = MakeCollectorKey(Collector);
	const bool bHasSlotKey = Slot.ReservedCollectorKey.ResolveObjectPtr() != nullptr;
	if (bHasSlotKey || Collector.Collector.IsValid())
	{
		return Slot.ReservedCollectorKey == CollectorKey;
	}

	return !Slot.ReservedCollectorID.IsNone()
		&& Slot.ReservedCollectorID == Collector.CollectorID;
}

bool UT66MobLootSubsystem::IsEligibleForQuery(
	const FT66MobLootSlot& Slot,
	const FT66MobLootQueryFilter& Filter,
	const FT66MobLootCollectorRef* Collector) const
{
	if (!Slot.bActive)
	{
		return false;
	}

	if (Slot.bReserved)
	{
		const bool bReservedByThisCollector = Collector && IsCollectorMatch(Slot, *Collector);
		if (Filter.bExcludeReservedByOthers && !bReservedByThisCollector)
		{
			return false;
		}
	}

	for (const FT66MobLootExclusionSphere& Sphere : Filter.ExclusionSpheres)
	{
		if (Sphere.Radius > 0.0f && FVector::DistSquared(Sphere.Center, Slot.Position) <= FMath::Square(Sphere.Radius))
		{
			return false;
		}
	}
	return true;
}

FT66MobLootReservationResult UT66MobLootSubsystem::BuildReservationResult(
	const FT66MobLootSlot& Slot,
	const FT66MobLootHandle& Handle,
	const bool bReserved) const
{
	FT66MobLootReservationResult Result;
	Result.bReserved = bReserved;
	Result.Handle = Handle;
	Result.Position = Slot.Position;
	Result.Quantity = Slot.Quantity;
	Result.GoldValue = Slot.GoldValue;
	return Result;
}

void UT66MobLootSubsystem::ReleaseSlot(
	const int32 SlotIndex,
	const EReleaseReason Reason,
	FT66MobLootCollectResult* InOutCollectResult)
{
	if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex].bActive)
	{
		return;
	}

	FT66MobLootSlot& Slot = Slots[SlotIndex];
	if (InOutCollectResult)
	{
		++InOutCollectResult->DropsCollected;
		InOutCollectResult->QuantityCollected += Slot.Quantity;
		InOutCollectResult->GoldValueCollected += Slot.GoldValue;

		++Diagnostics.CollectedDropTotal;
		Diagnostics.QuantityCollectedTotal += Slot.Quantity;
		Diagnostics.GoldValueCollectedTotal += Slot.GoldValue;
	}

	const int32 DenseIndex = Slot.DenseIndex;
	const int32 LastDenseIndex = DenseSlots.Num() - 1;
	if (DenseIndex != LastDenseIndex)
	{
		const int32 MovedSlotIndex = DenseSlots[LastDenseIndex];
		DenseSlots[DenseIndex] = MovedSlotIndex;
		Slots[MovedSlotIndex].DenseIndex = DenseIndex;
	}
	DenseSlots.Pop(EAllowShrinking::No);

	Slot.Position = FVector::ZeroVector;
	Slot.SourceID = NAME_None;
	Slot.Color = FLinearColor::White;
	Slot.Scale = 1.0f;
	Slot.RemainingLifetimeSeconds = 0.0f;
	Slot.Quantity = 0;
	Slot.GoldValue = 0;
	Slot.bReserved = false;
	Slot.ReservedCollectorKey = FObjectKey();
	Slot.ReservedCollectorID = NAME_None;
	Slot.ReservedCollectorType = ET66MobLootCollectorType::System;
	Slot.bActive = false;
	Slot.DenseIndex = INDEX_NONE;
	Slot.Generation = Slot.Generation == MAX_int32 ? 1 : Slot.Generation + 1;
	FreeSlots.Add(SlotIndex);

	if (Reason == EReleaseReason::Expired)
	{
		++Diagnostics.ExpiredDropTotal;
	}

	Diagnostics.LiveWorldDropCount = DenseSlots.Num();
	bDirty = true;
}

void UT66MobLootSubsystem::TickExpirations(const float DeltaTime)
{
	if (DenseSlots.Num() <= 0)
	{
		return;
	}

	const float StepSeconds = FMath::Max(0.0f, DeltaTime);
	for (int32 DenseIndex = 0; DenseIndex < DenseSlots.Num();)
	{
		const int32 SlotIndex = DenseSlots[DenseIndex];
		if (!Slots.IsValidIndex(SlotIndex))
		{
			++DenseIndex;
			continue;
		}

		FT66MobLootSlot& Slot = Slots[SlotIndex];
		if (!Slot.bActive || Slot.RemainingLifetimeSeconds <= 0.0f)
		{
			++DenseIndex;
			continue;
		}

		Slot.RemainingLifetimeSeconds = FMath::Max(0.0f, Slot.RemainingLifetimeSeconds - StepSeconds);
		if (Slot.RemainingLifetimeSeconds <= 0.0f)
		{
			ReleaseSlot(SlotIndex, EReleaseReason::Expired, nullptr);
			continue;
		}

		++DenseIndex;
	}
}

void UT66MobLootSubsystem::UploadLiveState()
{
	const uint64 StartCycles = FPlatformTime::Cycles64();
	const int32 LiveCount = DenseSlots.Num();
	PositionUpload.Reset(LiveCount);
	ScaleUpload.Reset(LiveCount);
	ColorUpload.Reset(LiveCount);
	QuantityUpload.Reset(LiveCount);

	for (const int32 SlotIndex : DenseSlots)
	{
		if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex].bActive)
		{
			continue;
		}

		const FT66MobLootSlot& Slot = Slots[SlotIndex];
		FLinearColor RenderColor = Slot.Color;
		if (Slot.bReserved)
		{
			RenderColor.A *= 0.72f;
		}
		PositionUpload.Add(Slot.Position);
		ScaleUpload.Add(Slot.Scale);
		ColorUpload.Add(RenderColor);
		QuantityUpload.Add(Slot.Quantity);
	}

	const uint64 PackEndCycles = FPlatformTime::Cycles64();
	if (NiagaraComponent)
	{
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
			NiagaraComponent,
			T66MobLootPositionsParam,
			PositionUpload);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(
			NiagaraComponent,
			T66MobLootScalesParam,
			ScaleUpload);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayColor(
			NiagaraComponent,
			T66MobLootColorsParam,
			ColorUpload);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayInt32(
			NiagaraComponent,
			T66MobLootQuantitiesParam,
			QuantityUpload);
		NiagaraComponent->SetVariableInt(T66MobLootLiveCountParam, LiveCount);
	}
	UploadFallbackVisualState();

	Diagnostics.LiveWorldDropCount = LiveCount;
	Diagnostics.PeakLiveWorldDropCount = FMath::Max(Diagnostics.PeakLiveWorldDropCount, LiveCount);
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

	if (CVarT66MobLootVerbose.GetValueOnGameThread() != 0)
	{
		UE_LOG(LogT66MobLoot, Display,
			TEXT("[MobLoot] phase=Upload liveWorldDrops=%d uploadMs=%.3f packMs=%.3f niagaraArrayMs=%.3f spawned=%d collected=%d expired=%d"),
			LiveCount,
			Diagnostics.LastUploadMs,
			Diagnostics.LastPackMs,
			Diagnostics.LastNiagaraArrayUploadMs,
			Diagnostics.SpawnedDropTotal,
			Diagnostics.CollectedDropTotal,
			Diagnostics.ExpiredDropTotal);
	}
}

bool UT66MobLootSubsystem::EnsureFallbackVisualComponent()
{
	if (FallbackVisualComponent && FallbackVisualComponent->IsRegistered())
	{
		return true;
	}

	if (!RenderHost || !RenderRoot)
	{
		return false;
	}

	FallbackVisualComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(RenderHost, TEXT("MobLootVisibleFallback"), RF_Transient);
	if (!FallbackVisualComponent)
	{
		return false;
	}

	FallbackVisualComponent->SetMobility(EComponentMobility::Movable);
	FallbackVisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FallbackVisualComponent->SetGenerateOverlapEvents(false);
	FallbackVisualComponent->SetCastShadow(false);
	FallbackVisualComponent->bReceivesDecals = false;
	FallbackVisualComponent->NumCustomDataFloats = 0;
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		FallbackVisualComponent->SetStaticMesh(Sphere);
	}
	if (UMaterialInterface* ColorMat = FT66VisualUtil::GetFlatColorMaterial())
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, RenderHost))
		{
			FT66VisualUtil::ConfigureFlatColorMaterial(Mat, FLinearColor(1.0f, 0.78f, 0.16f, 1.0f));
			FallbackVisualComponent->SetMaterial(0, Mat);
		}
	}
	FallbackVisualComponent->SetupAttachment(RenderRoot);
	FallbackVisualComponent->RegisterComponent();
	return true;
}

void UT66MobLootSubsystem::UploadFallbackVisualState()
{
	if (!FallbackVisualComponent)
	{
		return;
	}

	FallbackVisualComponent->ClearInstances();
	for (int32 Index = 0; Index < PositionUpload.Num(); ++Index)
	{
		const float SlotScale = ScaleUpload.IsValidIndex(Index) ? FMath::Max(0.01f, ScaleUpload[Index]) : 1.0f;
		const FVector Position = PositionUpload[Index] + FVector(0.f, 0.f, 32.f * SlotScale);
		const FTransform InstanceTransform(
			FQuat::Identity,
			Position,
			FVector(0.34f * SlotScale, 0.34f * SlotScale, 0.22f * SlotScale));
		FallbackVisualComponent->AddInstance(InstanceTransform, true);
	}
	FallbackVisualComponent->SetHiddenInGame(PositionUpload.Num() <= 0, true);
	FallbackVisualComponent->SetVisibility(PositionUpload.Num() > 0, true);
}

#if !UE_BUILD_SHIPPING
int32 UT66MobLootSubsystem::GetVisibleMobLootInstanceCountForAutomation() const
{
	return FallbackVisualComponent ? FallbackVisualComponent->GetInstanceCount() : 0;
}

FT66WorldRuntimeDebugSnapshot UT66MobLootSubsystem::GetWorldRuntimeDebugSnapshot() const
{
	FT66WorldRuntimeDebugSnapshot Snapshot;
	Snapshot.SystemName = TEXT("UT66MobLootSubsystem");
	Snapshot.AddCounter(TEXT("active_drop_count"), GetActiveMobLootDropCount());
	Snapshot.AddCounter(TEXT("slot_capacity"), Slots.Num());
	Snapshot.AddCounter(TEXT("free_slot_count"), FreeSlots.Num());
	Snapshot.AddCounter(TEXT("dense_slot_count"), DenseSlots.Num());
	Snapshot.AddCounter(TEXT("position_upload_count"), PositionUpload.Num());
	Snapshot.AddCounter(TEXT("scale_upload_count"), ScaleUpload.Num());
	Snapshot.AddCounter(TEXT("color_upload_count"), ColorUpload.Num());
	Snapshot.AddCounter(TEXT("quantity_upload_count"), QuantityUpload.Num());
	Snapshot.AddCounter(TEXT("fallback_instance_count"), FallbackVisualComponent ? FallbackVisualComponent->GetInstanceCount() : 0);
	Snapshot.AddCounter(TEXT("known_timer_handles"), 0);
	Snapshot.AddCounter(TEXT("known_external_delegate_handles"), 0);
	Snapshot.AddCounter(TEXT("async_load_handles_valid"), 0);
	Snapshot.AddFlag(TEXT("initialized_slots"), bInitializedSlots);
	Snapshot.AddFlag(TEXT("dirty"), bDirty);
	Snapshot.AddFlag(TEXT("render_host_valid"), RenderHost != nullptr);
	Snapshot.AddFlag(TEXT("render_root_valid"), RenderRoot != nullptr);
	Snapshot.AddFlag(TEXT("niagara_component_valid"), NiagaraComponent != nullptr);
	Snapshot.AddFlag(TEXT("niagara_component_registered"), NiagaraComponent ? NiagaraComponent->IsRegistered() : false);
	Snapshot.AddFlag(TEXT("niagara_component_active"), NiagaraComponent ? NiagaraComponent->IsActive() : false);
	Snapshot.AddFlag(TEXT("niagara_system_valid"), NiagaraSystem != nullptr);
	Snapshot.AddFlag(TEXT("fallback_visual_component_valid"), FallbackVisualComponent != nullptr);
	Snapshot.AddFlag(TEXT("fallback_visual_component_registered"), FallbackVisualComponent ? FallbackVisualComponent->IsRegistered() : false);
	Snapshot.AddEvidence(TEXT("timers"), TEXT("No stored timer handles found; expiration work is tick-driven."));
	Snapshot.AddEvidence(TEXT("delegates"), TEXT("No external delegate handle is stored by this subsystem."));
	Snapshot.AddEvidence(TEXT("async_loads"), TEXT("No async load handle is owned by this subsystem."));
	return Snapshot;
}
#endif
