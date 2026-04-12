// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66TrapSubsystem.h"

#include "Core/T66ActorRegistrySubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66CircusInteractable.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/Traps/T66FloorFlameTrap.h"
#include "Gameplay/Traps/T66TrapBase.h"
#include "Gameplay/Traps/T66WallArrowTrap.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66TrapSubsystem, Log, All);

namespace
{
	template <typename T>
	void AddUniqueWeak(TArray<TWeakObjectPtr<T>>& Entries, T* Actor)
	{
		if (!Actor)
		{
			return;
		}

		Entries.RemoveAllSwap([](const TWeakObjectPtr<T>& Weak) { return !Weak.IsValid(); }, EAllowShrinking::No);
		Entries.AddUnique(Actor);
	}

	template <typename T>
	void RemoveWeak(TArray<TWeakObjectPtr<T>>& Entries, T* Actor)
	{
		if (!Actor)
		{
			return;
		}

		Entries.RemoveAllSwap([Actor](const TWeakObjectPtr<T>& Weak)
		{
			return !Weak.IsValid() || Weak.Get() == Actor;
		}, EAllowShrinking::No);
	}

	struct FUsedTrapLocation
	{
		FVector Location = FVector::ZeroVector;
		int32 FloorNumber = INDEX_NONE;
	};

	int32 GetDifficultyStep(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy:       return 0;
		case ET66Difficulty::Medium:     return 1;
		case ET66Difficulty::Hard:       return 2;
		case ET66Difficulty::VeryHard:   return 3;
		case ET66Difficulty::Impossible: return 4;
		default:                         return 1;
		}
	}

	float GetArrowTrapInterval(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy:       return 2.90f;
		case ET66Difficulty::Medium:     return 2.60f;
		case ET66Difficulty::Hard:       return 2.35f;
		case ET66Difficulty::VeryHard:   return 2.05f;
		case ET66Difficulty::Impossible: return 1.80f;
		default:                         return 2.60f;
		}
	}

	float GetFlameTrapCooldown(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy:       return 3.30f;
		case ET66Difficulty::Medium:     return 3.00f;
		case ET66Difficulty::Hard:       return 2.70f;
		case ET66Difficulty::VeryHard:   return 2.40f;
		case ET66Difficulty::Impossible: return 2.10f;
		default:                         return 3.00f;
		}
	}

	int32 ComputeTrapDamageHP(const int32 StageNum, const ET66Difficulty Difficulty, const float BaseDamage)
	{
		const float StageScalar = FMath::Pow(1.14f, static_cast<float>(FMath::Max(StageNum - 1, 0)));
		const float DifficultyScalar = 0.95f + (0.09f * static_cast<float>(GetDifficultyStep(Difficulty)));
		return FMath::Max(8, FMath::RoundToInt(BaseDamage * DifficultyScalar * StageScalar));
	}

	int32 GetActorFloorNumber(const AT66GameMode* GameMode, const AActor* Actor)
	{
		return (GameMode && Actor) ? GameMode->GetTowerFloorIndexForLocation(Actor->GetActorLocation()) : INDEX_NONE;
	}
}

UT66TrapSubsystem* UT66TrapSubsystem::Get(UWorld* World)
{
	return World ? World->GetSubsystem<UT66TrapSubsystem>() : nullptr;
}

bool UT66TrapSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UT66TrapSubsystem::RegisterTrap(AT66TrapBase* Trap)
{
	AddUniqueWeak(RegisteredTraps, Trap);
}

void UT66TrapSubsystem::UnregisterTrap(AT66TrapBase* Trap)
{
	RemoveWeak(RegisteredTraps, Trap);
}

void UT66TrapSubsystem::PruneTrackedActors()
{
	RegisteredTraps.RemoveAllSwap([](const TWeakObjectPtr<AT66TrapBase>& Weak) { return !Weak.IsValid(); }, EAllowShrinking::No);
	ManagedTrapActors.RemoveAllSwap([](const TWeakObjectPtr<AActor>& Weak) { return !Weak.IsValid(); }, EAllowShrinking::No);
}

void UT66TrapSubsystem::ClearManagedTrapActors()
{
	PruneTrackedActors();

	TArray<TWeakObjectPtr<AActor>> ActorsToDestroy = ManagedTrapActors;
	ManagedTrapActors.Reset();

	for (const TWeakObjectPtr<AActor>& WeakActor : ActorsToDestroy)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Actor->Destroy();
		}
	}
}

void UT66TrapSubsystem::SpawnTowerStageTraps(const T66TowerMapTerrain::FLayout& Layout, int32 StageNum, ET66Difficulty Difficulty, int32 RunSeed)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ClearManagedTrapActors();
	PruneTrackedActors();

	AT66GameMode* GameMode = World->GetAuthGameMode<AT66GameMode>();
	UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
	TArray<FUsedTrapLocation> UsedLocations;

	auto IsLocationClear = [&](const FVector& Location, const int32 FloorNumber, const float MinDistance) -> bool
	{
		for (const FUsedTrapLocation& Used : UsedLocations)
		{
			if (Used.FloorNumber != FloorNumber)
			{
				continue;
			}

			if (FVector::DistSquared2D(Location, Used.Location) < FMath::Square(MinDistance))
			{
				return false;
			}
		}

		auto IsSafeZoneTooClose = [&](const AActor* Actor, const float SafeZoneRadius) -> bool
		{
			if (!Actor)
			{
				return false;
			}

			const int32 ActorFloorNumber = GetActorFloorNumber(GameMode, Actor);
			if (ActorFloorNumber != INDEX_NONE && ActorFloorNumber != FloorNumber)
			{
				return false;
			}

			if (ActorFloorNumber == INDEX_NONE && FMath::Abs(Actor->GetActorLocation().Z - Location.Z) > 900.f)
			{
				return false;
			}

			return FVector::DistSquared2D(Location, Actor->GetActorLocation()) < FMath::Square(SafeZoneRadius);
		};

		if (Registry)
		{
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
			{
				const AT66HouseNPCBase* NPC = WeakNPC.Get();
				if (IsSafeZoneTooClose(NPC, (NPC ? NPC->GetSafeZoneRadius() : 0.f) + 500.f))
				{
					return false;
				}
			}

			for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Registry->GetCircuses())
			{
				const AT66CircusInteractable* Circus = WeakCircus.Get();
				if (IsSafeZoneTooClose(Circus, (Circus ? Circus->GetSafeZoneRadius() : 0.f) + 500.f))
				{
					return false;
				}
			}
		}

		return true;
	};

	int32 SpawnedArrowTraps = 0;
	int32 SpawnedFlameTraps = 0;

	const int32 ArrowTrapCountPerFloor = 1;
	const int32 FlameTrapCountPerFloor = 1;
	const int32 DifficultyStep = GetDifficultyStep(Difficulty);

	for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
	{
		if (!Floor.bGameplayFloor
			|| Floor.FloorNumber < Layout.FirstGameplayFloorNumber
			|| Floor.FloorNumber > Layout.LastGameplayFloorNumber)
		{
			continue;
		}

		for (int32 ArrowIndex = 0; ArrowIndex < ArrowTrapCountPerFloor; ++ArrowIndex)
		{
			FRandomStream FloorRng(RunSeed + StageNum * 2501 + Floor.FloorNumber * 131 + ArrowIndex * 17);
			for (int32 Attempt = 0; Attempt < 24; ++Attempt)
			{
				FVector WallLocation = FVector::ZeroVector;
				FVector WallNormal = FVector::ZeroVector;
				if (!T66TowerMapTerrain::TryGetWallSpawnLocation(
					World,
					Layout,
					Floor.Center + FVector(0.f, 0.f, 20.f),
					2500.f,
					Floor.BoundsHalfExtent + 1800.f,
					FloorRng,
					WallLocation,
					WallNormal))
				{
					continue;
				}

				WallLocation.Z = Floor.SurfaceZ + 115.f;
				if (!IsLocationClear(WallLocation, Floor.FloorNumber, 1100.f))
				{
					continue;
				}

				const FTransform SpawnTransform(WallNormal.Rotation(), WallLocation);
				AT66WallArrowTrap* ArrowTrap = World->SpawnActorDeferred<AT66WallArrowTrap>(
					AT66WallArrowTrap::StaticClass(),
					SpawnTransform,
					GameMode,
					nullptr,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if (!ArrowTrap)
				{
					continue;
				}

				ArrowTrap->SetTowerFloorNumber(Floor.FloorNumber);
				ArrowTrap->DamageHP = ComputeTrapDamageHP(StageNum, Difficulty, 12.f);
				ArrowTrap->ProjectileSpeed = 2400.f + static_cast<float>(DifficultyStep * 120);
				ArrowTrap->FireIntervalSeconds = GetArrowTrapInterval(Difficulty);
				ArrowTrap->WindupDurationSeconds = 0.40f;
				ArrowTrap->InitialFireDelaySeconds = FloorRng.FRandRange(0.35f, 1.15f);
				ArrowTrap->DetectionRange = FMath::Max(5000.f, Floor.BoundsHalfExtent * 1.10f);
				ArrowTrap->FinishSpawning(SpawnTransform);

				ManagedTrapActors.Add(ArrowTrap);
				UsedLocations.Add({ ArrowTrap->GetActorLocation(), Floor.FloorNumber });
				++SpawnedArrowTraps;
				break;
			}
		}

		for (int32 FlameIndex = 0; FlameIndex < FlameTrapCountPerFloor; ++FlameIndex)
		{
			FRandomStream FloorRng(RunSeed + StageNum * 2701 + Floor.FloorNumber * 149 + FlameIndex * 23);
			for (int32 Attempt = 0; Attempt < 24; ++Attempt)
			{
				FVector FlameLocation = FVector::ZeroVector;
				if (!T66TowerMapTerrain::TryGetRandomSurfaceLocationOnFloor(
					World,
					Layout,
					Floor.FloorNumber,
					FloorRng,
					FlameLocation,
					1800.f,
					1900.f))
				{
					continue;
				}

				FlameLocation.Z = Floor.SurfaceZ + 6.f;
				if (!IsLocationClear(FlameLocation, Floor.FloorNumber, 1450.f))
				{
					continue;
				}

				const FTransform SpawnTransform(FRotator::ZeroRotator, FlameLocation);
				AT66FloorFlameTrap* FlameTrap = World->SpawnActorDeferred<AT66FloorFlameTrap>(
					AT66FloorFlameTrap::StaticClass(),
					SpawnTransform,
					GameMode,
					nullptr,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if (!FlameTrap)
				{
					continue;
				}

				FlameTrap->SetTowerFloorNumber(Floor.FloorNumber);
				FlameTrap->Radius = 250.f + static_cast<float>(DifficultyStep * 18);
				FlameTrap->DamageHP = ComputeTrapDamageHP(StageNum, Difficulty, 10.f);
				FlameTrap->WarningDurationSeconds = 0.80f;
				FlameTrap->ActiveDurationSeconds = 1.15f;
				FlameTrap->CooldownDurationSeconds = GetFlameTrapCooldown(Difficulty);
				FlameTrap->DamageIntervalSeconds = 0.35f;
				FlameTrap->InitialCycleDelaySeconds = FloorRng.FRandRange(0.80f, 2.10f);
				FlameTrap->FinishSpawning(SpawnTransform);

				ManagedTrapActors.Add(FlameTrap);
				UsedLocations.Add({ FlameTrap->GetActorLocation(), Floor.FloorNumber });
				++SpawnedFlameTraps;
				break;
			}
		}
	}

	UE_LOG(
		LogT66TrapSubsystem,
		Log,
		TEXT("[Traps] Spawned %d wall arrow traps and %d floor flame traps for tower stage %d."),
		SpawnedArrowTraps,
		SpawnedFlameTraps,
		StageNum);
}
