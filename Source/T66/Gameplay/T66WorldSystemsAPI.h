// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66OutgoingTravelerPoolSubsystem.h"

class AActor;

/**
 * Stable cross-stream gameplay-system contracts published during outgoing-traveler Phase 1.
 *
 * Outgoing travelers are implemented by UT66OutgoingTravelerPoolSubsystem:
 * - FireOutgoingTraveler(const FT66OutgoingTravelerFireParams&, FT66OutgoingTravelerHandle&)
 * - FireOutgoingTraveler(const FT66OutgoingTravelerFireParams&, FT66OutgoingTravelerHandle&, FT66OutgoingTravelerArrivalCallback)
 * - CancelOutgoingTraveler(FT66OutgoingTravelerHandle&)
 * - FT66OutgoingTravelerFireParams::TravelerVisualProfileID selects one of the single
 *   Niagara pool's per-instance visual slots. A slot carries commandlet-authored mesh,
 *   material instance, and texture; NAME_None uses the ProfileID fallback shape.
 *
 * Mob Loot is implemented by UT66MobLootSubsystem. It owns only active uncollected
 * world drops. Economy owns the collected sellable stack, caps, and gold crediting.
 */

enum class ET66MobLootCollectorType : uint8
{
	Player,
	Pet,
	System
};

struct T66_API FT66MobLootHandle
{
	int32 SlotIndex = INDEX_NONE;
	int32 Generation = 0;

	bool IsValid() const { return SlotIndex != INDEX_NONE && Generation != 0; }
	void Reset()
	{
		SlotIndex = INDEX_NONE;
		Generation = 0;
	}
};

struct T66_API FT66MobLootSpawnParams
{
	FVector Position = FVector::ZeroVector;
	int32 Quantity = 1;
	/** Total sell value represented by this world drop. If Economy uses 1g/unit, set this to Quantity. */
	int32 GoldValue = 1;
	FName SourceID = NAME_None;
	FLinearColor Color = FLinearColor::White;
	float Scale = 1.0f;
	float LifetimeSeconds = 120.0f;
};

struct T66_API FT66MobLootCollectResult
{
	int32 DropsCollected = 0;
	int32 QuantityCollected = 0;
	/** Total sell value collected across all drops consumed by this call, not a per-unit value. */
	int32 GoldValueCollected = 0;
};

struct T66_API FT66MobLootRenderArrays
{
	TArray<FVector> Positions;
	TArray<float> Scales;
	TArray<FLinearColor> Colors;
	TArray<int32> Quantities;

	void Reset()
	{
		Positions.Reset();
		Scales.Reset();
		Colors.Reset();
		Quantities.Reset();
	}
};

struct T66_API FT66MobLootCollectorRef
{
	TWeakObjectPtr<AActor> Collector;
	FName CollectorID = NAME_None;
	ET66MobLootCollectorType CollectorType = ET66MobLootCollectorType::System;

	bool HasIdentity() const
	{
		return Collector.IsValid() || !CollectorID.IsNone();
	}
};

struct T66_API FT66MobLootExclusionSphere
{
	FVector Center = FVector::ZeroVector;
	float Radius = 0.0f;
};

struct T66_API FT66MobLootQueryFilter
{
	FVector Origin = FVector::ZeroVector;
	/** 0 or less means no search-radius limit. */
	float SearchRadius = 0.0f;
	TArray<FT66MobLootExclusionSphere> ExclusionSpheres;
	int32 MaxCandidates = 1;
	bool bExcludeReservedByOthers = true;
};

struct T66_API FT66MobLootReservationResult
{
	bool bReserved = false;
	FT66MobLootHandle Handle;
	FVector Position = FVector::ZeroVector;
	int32 Quantity = 0;
	/** Total sell value represented by this reserved drop, not a per-unit value. */
	int32 GoldValue = 0;
};

class T66_API IT66MobLootRuntime
{
public:
	virtual ~IT66MobLootRuntime() = default;

	virtual bool SpawnMobLoot(const FT66MobLootSpawnParams& Params, FT66MobLootHandle& OutHandle) = 0;
	virtual bool QueryMobLootTarget(const FT66MobLootQueryFilter& Filter, FT66MobLootHandle& OutHandle, FVector& OutPosition) const = 0;
	virtual FT66MobLootReservationResult QueryAndReserveMobLoot(const FT66MobLootQueryFilter& Filter, const FT66MobLootCollectorRef& Collector) = 0;
	virtual bool ReserveMobLoot(const FT66MobLootHandle& Handle, const FT66MobLootCollectorRef& Collector, FT66MobLootReservationResult& OutReservation) = 0;
	virtual bool ReleaseMobLootReservation(const FT66MobLootHandle& Handle, const FT66MobLootCollectorRef& Collector) = 0;
	virtual FT66MobLootCollectResult CollectReservedMobLoot(const FT66MobLootHandle& Handle, const FT66MobLootCollectorRef& Collector, float MaxDistance = 0.0f) = 0;
	virtual FT66MobLootCollectResult CollectMobLootAt(const FVector& Center, float Radius, const FT66MobLootCollectorRef& Collector, int32 MaxDrops = INDEX_NONE) = 0;
	FT66MobLootCollectResult CollectMobLootAt(const FVector& Center, float Radius, int32 MaxDrops = INDEX_NONE)
	{
		return CollectMobLootAt(Center, Radius, FT66MobLootCollectorRef{}, MaxDrops);
	}

	/** Active uncollected world-drop entry count only. This is not Economy's collected sellable stack. */
	virtual int32 GetActiveMobLootDropCount() const = 0;
	virtual void PushMobLootRenderArrays(FT66MobLootRenderArrays& OutArrays) const = 0;
};
