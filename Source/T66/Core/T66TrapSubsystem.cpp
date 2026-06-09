// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66TrapSubsystem.h"

#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66StageProgressionSubsystem.h"
#include "Core/T66TowerTuningConfig.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66NPCBase.h"
#include "Gameplay/T66SafeZoneComponent.h"
#include "Gameplay/Traps/T66FloorFlameTrap.h"
#include "Gameplay/Traps/T66FloorSpikePatchTrap.h"
#include "Gameplay/Traps/T66ObstacleTrap.h"
#include "Gameplay/Traps/T66TrapBase.h"
#include "Gameplay/Traps/T66TrapPressurePlate.h"
#include "Gameplay/Traps/T66WallArrowTrap.h"
#include "Engine/World.h"
#include "Subsystems/SubsystemCollection.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66TrapSubsystem, Log, All);

namespace
{
	static constexpr bool T66EnableTowerTrapSpawns = true;
	static const FName TrapFamilyWallProjectile(TEXT("WallProjectile"));
	static const FName TrapFamilyFloorBurst(TEXT("FloorBurst"));
	static const FName TrapFamilyAreaControl(TEXT("AreaControl"));
	static const FName TrapFamilyObstacle(TEXT("Obstacle"));

	enum class ET66TrapSpawnSurface : uint8
	{
		MazeWall,
		TileCenter
	};

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

	struct FT66TrapVisualStyle
	{
		FLinearColor BaseColor = FLinearColor::White;
		FLinearColor WarningColor = FLinearColor::White;
		FLinearColor AccentColor = FLinearColor::White;
		FLinearColor BurstColor = FLinearColor::White;
		FVector MeshScale = FVector(0.32f);
		bool bUseFireNiagaraVFX = false;
	};

	struct FT66TrapRegistryEntry
	{
		FName RegistryKey = NAME_None;
		FName FamilyKey = NAME_None;
		ET66TrapSpawnSurface SpawnSurface = ET66TrapSpawnSurface::TileCenter;
		TSubclassOf<AT66TrapBase> TrapClass = nullptr;
		ET66TrapActivationMode ActivationMode = ET66TrapActivationMode::Timed;
		ET66TrapTriggerTarget TriggerTargetMode = ET66TrapTriggerTarget::HeroesAndEnemies;
		bool bSpawnPressurePlate = false;
		FT66TrapVisualStyle VisualStyle;
	};

	struct FT66ResolvedTrapSpawnRequest
	{
		const FT66TrapRegistryEntry* Entry = nullptr;
		int32 MinCount = 0;
		int32 MaxCount = 0;
		int32 DesiredCount = 0;
	};

	FT66TrapVisualStyle MakeTrapStyle(
		const FLinearColor& BaseColor,
		const FLinearColor& WarningColor,
		const FLinearColor& AccentColor,
		const FLinearColor& BurstColor,
		const FVector& MeshScale = FVector(0.32f),
		const bool bUseFireNiagaraVFX = false)
	{
		FT66TrapVisualStyle Style;
		Style.BaseColor = BaseColor;
		Style.WarningColor = WarningColor;
		Style.AccentColor = AccentColor;
		Style.BurstColor = BurstColor;
		Style.MeshScale = MeshScale;
		Style.bUseFireNiagaraVFX = bUseFireNiagaraVFX;
		return Style;
	}

	FT66TrapRegistryEntry MakeRegistryEntry(
		const FName RegistryKey,
		const FName FamilyKey,
		const ET66TrapSpawnSurface SpawnSurface,
		TSubclassOf<AT66TrapBase> TrapClass,
		const FT66TrapVisualStyle& VisualStyle,
		const ET66TrapActivationMode ActivationMode = ET66TrapActivationMode::Timed,
		const ET66TrapTriggerTarget TriggerTargetMode = ET66TrapTriggerTarget::HeroesAndEnemies,
		const bool bSpawnPressurePlate = false)
	{
		FT66TrapRegistryEntry Entry;
		Entry.RegistryKey = RegistryKey;
		Entry.FamilyKey = FamilyKey;
		Entry.SpawnSurface = SpawnSurface;
		Entry.TrapClass = TrapClass;
		Entry.ActivationMode = ActivationMode;
		Entry.TriggerTargetMode = TriggerTargetMode;
		Entry.bSpawnPressurePlate = bSpawnPressurePlate;
		Entry.VisualStyle = VisualStyle;
		return Entry;
	}

	const TArray<FT66TrapRegistryEntry>& GetTrapRegistry()
	{
		static const TArray<FT66TrapRegistryEntry> Registry = []
		{
			TArray<FT66TrapRegistryEntry> Entries;
			Entries.Reserve(19);

			Entries.Add(MakeRegistryEntry(
				TEXT("DungeonWallArrow"),
				TrapFamilyWallProjectile,
				ET66TrapSpawnSurface::MazeWall,
				AT66WallArrowTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.66f, 0.54f, 0.22f, 1.f),
					FLinearColor(1.00f, 0.82f, 0.24f, 1.f),
					FLinearColor(1.00f, 0.92f, 0.42f, 1.f),
					FLinearColor(1.00f, 0.80f, 0.28f, 0.95f),
					FVector(0.32f))));
			Entries.Add(MakeRegistryEntry(
				TEXT("DungeonFloorFlame"),
				TrapFamilyFloorBurst,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorFlameTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.16f, 0.04f, 0.02f, 1.f),
					FLinearColor(1.00f, 0.26f, 0.08f, 0.85f),
					FLinearColor(1.00f, 0.42f, 0.10f, 0.95f),
					FLinearColor(1.00f, 0.48f, 0.20f, 0.95f),
					FVector(0.32f),
					true)));
			Entries.Add(MakeRegistryEntry(
				TEXT("DungeonFloorSpikePatch"),
				TrapFamilyAreaControl,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorSpikePatchTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.12f, 0.09f, 0.09f, 1.f),
					FLinearColor(0.92f, 0.22f, 0.18f, 0.92f),
					FLinearColor(0.82f, 0.82f, 0.86f, 1.f),
					FLinearColor(1.00f, 0.55f, 0.24f, 0.95f))));

			Entries.Add(MakeRegistryEntry(
				TEXT("ObstacleSweeperArm"),
				TrapFamilyObstacle,
				ET66TrapSpawnSurface::TileCenter,
				AT66SweeperArmTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.14f, 0.16f, 0.18f, 1.f),
					FLinearColor(1.00f, 0.82f, 0.22f, 1.f),
					FLinearColor(1.00f, 0.72f, 0.18f, 1.f),
					FLinearColor(1.00f, 0.84f, 0.28f, 0.95f))));
			Entries.Add(MakeRegistryEntry(
				TEXT("ObstacleFloorBumper"),
				TrapFamilyObstacle,
				ET66TrapSpawnSurface::TileCenter,
				AT66BumperTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.08f, 0.16f, 0.20f, 1.f),
					FLinearColor(0.18f, 0.88f, 1.00f, 1.f),
					FLinearColor(0.22f, 0.72f, 1.00f, 1.f),
					FLinearColor(0.56f, 0.94f, 1.00f, 0.95f))));
			Entries.Add(MakeRegistryEntry(
				TEXT("ObstacleWallBumper"),
				TrapFamilyObstacle,
				ET66TrapSpawnSurface::MazeWall,
				AT66WallBumperTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.10f, 0.14f, 0.10f, 1.f),
					FLinearColor(0.42f, 1.00f, 0.34f, 1.f),
					FLinearColor(0.38f, 0.92f, 0.24f, 1.f),
					FLinearColor(0.66f, 1.00f, 0.42f, 0.95f))));
			Entries.Add(MakeRegistryEntry(
				TEXT("ObstacleCeilingHammer"),
				TrapFamilyObstacle,
				ET66TrapSpawnSurface::TileCenter,
				AT66CeilingHammerTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.13f, 0.12f, 0.16f, 1.f),
					FLinearColor(1.00f, 0.36f, 0.22f, 1.f),
					FLinearColor(0.94f, 0.28f, 0.18f, 1.f),
					FLinearColor(1.00f, 0.52f, 0.24f, 0.95f))));

			Entries.Add(MakeRegistryEntry(
				TEXT("ForestThornVolley"),
				TrapFamilyWallProjectile,
				ET66TrapSpawnSurface::MazeWall,
				AT66WallArrowTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.12f, 0.28f, 0.12f, 1.f),
					FLinearColor(0.42f, 0.88f, 0.30f, 1.f),
					FLinearColor(0.68f, 1.00f, 0.52f, 1.f),
					FLinearColor(0.74f, 0.96f, 0.48f, 0.95f),
					FVector(0.34f))));
			Entries.Add(MakeRegistryEntry(
				TEXT("ForestSporeBurst"),
				TrapFamilyFloorBurst,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorFlameTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.05f, 0.12f, 0.05f, 1.f),
					FLinearColor(0.50f, 0.90f, 0.30f, 0.85f),
					FLinearColor(0.74f, 1.00f, 0.44f, 0.95f),
					FLinearColor(0.94f, 0.96f, 0.42f, 0.95f),
					FVector(0.32f),
					false)));
			Entries.Add(MakeRegistryEntry(
				TEXT("ForestBramblePatch"),
				TrapFamilyAreaControl,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorSpikePatchTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.08f, 0.11f, 0.06f, 1.f),
					FLinearColor(0.54f, 0.98f, 0.34f, 0.90f),
					FLinearColor(0.30f, 0.48f, 0.14f, 1.f),
					FLinearColor(0.82f, 1.00f, 0.52f, 0.95f))));

			Entries.Add(MakeRegistryEntry(
				TEXT("OceanHarpoonVolley"),
				TrapFamilyWallProjectile,
				ET66TrapSpawnSurface::MazeWall,
				AT66WallArrowTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.08f, 0.20f, 0.32f, 1.f),
					FLinearColor(0.30f, 0.86f, 1.00f, 1.f),
					FLinearColor(0.60f, 0.94f, 1.00f, 1.f),
					FLinearColor(0.70f, 1.00f, 1.00f, 0.95f),
					FVector(0.33f))));
			Entries.Add(MakeRegistryEntry(
				TEXT("OceanSteamBurst"),
				TrapFamilyFloorBurst,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorFlameTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.04f, 0.14f, 0.18f, 1.f),
					FLinearColor(0.30f, 0.82f, 0.96f, 0.85f),
					FLinearColor(0.54f, 0.96f, 1.00f, 0.95f),
					FLinearColor(0.88f, 0.98f, 1.00f, 0.95f),
					FVector(0.32f),
					false)));
			Entries.Add(MakeRegistryEntry(
				TEXT("OceanUrchinPatch"),
				TrapFamilyAreaControl,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorSpikePatchTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.05f, 0.10f, 0.16f, 1.f),
					FLinearColor(0.40f, 0.94f, 1.00f, 0.90f),
					FLinearColor(0.28f, 0.24f, 0.48f, 1.f),
					FLinearColor(0.76f, 0.98f, 1.00f, 0.95f))));

			Entries.Add(MakeRegistryEntry(
				TEXT("MartianShardVolley"),
				TrapFamilyWallProjectile,
				ET66TrapSpawnSurface::MazeWall,
				AT66WallArrowTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.30f, 0.10f, 0.20f, 1.f),
					FLinearColor(1.00f, 0.38f, 0.60f, 1.f),
					FLinearColor(1.00f, 0.70f, 0.28f, 1.f),
					FLinearColor(1.00f, 0.54f, 0.40f, 0.95f),
					FVector(0.35f))));
			Entries.Add(MakeRegistryEntry(
				TEXT("MartianPlasmaBurst"),
				TrapFamilyFloorBurst,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorFlameTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.16f, 0.05f, 0.10f, 1.f),
					FLinearColor(0.96f, 0.26f, 0.56f, 0.85f),
					FLinearColor(1.00f, 0.54f, 0.24f, 0.95f),
					FLinearColor(1.00f, 0.76f, 0.46f, 0.95f),
					FVector(0.32f),
					false)));
			Entries.Add(MakeRegistryEntry(
				TEXT("MartianCrystalPatch"),
				TrapFamilyAreaControl,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorSpikePatchTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.14f, 0.04f, 0.08f, 1.f),
					FLinearColor(1.00f, 0.34f, 0.60f, 0.90f),
					FLinearColor(0.86f, 0.56f, 0.94f, 1.f),
					FLinearColor(1.00f, 0.66f, 0.34f, 0.95f))));

			Entries.Add(MakeRegistryEntry(
				TEXT("HellSoulBoltVolley"),
				TrapFamilyWallProjectile,
				ET66TrapSpawnSurface::MazeWall,
				AT66WallArrowTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.32f, 0.06f, 0.02f, 1.f),
					FLinearColor(1.00f, 0.18f, 0.10f, 1.f),
					FLinearColor(1.00f, 0.64f, 0.22f, 1.f),
					FLinearColor(1.00f, 0.74f, 0.26f, 0.95f),
					FVector(0.36f))));
			Entries.Add(MakeRegistryEntry(
				TEXT("HellEmberBurst"),
				TrapFamilyFloorBurst,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorFlameTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.18f, 0.03f, 0.01f, 1.f),
					FLinearColor(1.00f, 0.14f, 0.08f, 0.88f),
					FLinearColor(1.00f, 0.42f, 0.08f, 0.95f),
					FLinearColor(1.00f, 0.72f, 0.20f, 0.95f),
					FVector(0.32f),
					true)));
			Entries.Add(MakeRegistryEntry(
				TEXT("HellBrimstonePatch"),
				TrapFamilyAreaControl,
				ET66TrapSpawnSurface::TileCenter,
				AT66FloorSpikePatchTrap::StaticClass(),
				MakeTrapStyle(
					FLinearColor(0.18f, 0.04f, 0.02f, 1.f),
					FLinearColor(1.00f, 0.16f, 0.08f, 0.92f),
					FLinearColor(0.42f, 0.12f, 0.04f, 1.f),
					FLinearColor(1.00f, 0.72f, 0.22f, 0.95f))));

			return Entries;
		}();

		return Registry;
	}

	const FT66TrapRegistryEntry* FindTrapRegistryEntry(const FName RegistryKey)
	{
		for (const FT66TrapRegistryEntry& Entry : GetTrapRegistry())
		{
			if (Entry.RegistryKey == RegistryKey)
			{
				return &Entry;
			}
		}

		return nullptr;
	}

	FName GetTowerFloorRoleName(const T66TowerMapTerrain::ET66TowerFloorRole Role)
	{
		switch (Role)
		{
		case T66TowerMapTerrain::ET66TowerFloorRole::Start:
			return TEXT("Start");
		case T66TowerMapTerrain::ET66TowerFloorRole::Mob:
			return TEXT("Mob");
		case T66TowerMapTerrain::ET66TowerFloorRole::Boss:
			return TEXT("Boss");
		default:
			return NAME_None;
		}
	}

	const TArray<FName>& GetTrapPoolForTowerFloor(const T66TowerMapTerrain::FFloor& Floor)
	{
		static const TArray<FName> Empty;

		const UT66TowerTuningConfig& TowerTuning = UT66TowerTuningConfig::GetRuntimeConfig();
		if (const FT66TowerFloorTrapPoolTuning* TrapPool = TowerTuning.FindTrapPoolForFloor(Floor.FloorNumber, GetTowerFloorRoleName(Floor.FloorRole)))
		{
			return TrapPool->TrapPool;
		}

		return Empty;
	}

	bool IsTrapTowerFloor(const T66TowerMapTerrain::FLayout& Layout, const T66TowerMapTerrain::FFloor& Floor)
	{
		if (!Floor.bMobFloor)
		{
			return false;
		}

		return GetTrapPoolForTowerFloor(Floor).Num() > 0;
	}

	int32 RollSpawnCount(const FT66IntRange& Range, FRandomStream& Rng)
	{
		const int32 MinCount = FMath::Max(0, Range.Min);
		const int32 MaxCount = FMath::Max(MinCount, Range.Max);
		return (MinCount == MaxCount) ? MinCount : Rng.RandRange(MinCount, MaxCount);
	}

	float RollFloatRange(const FT66FloatRange& Range, FRandomStream& Rng)
	{
		const float MinValue = FMath::Min(Range.Min, Range.Max);
		const float MaxValue = FMath::Max(Range.Min, Range.Max);
		return FMath::IsNearlyEqual(MinValue, MaxValue) ? MinValue : Rng.FRandRange(MinValue, MaxValue);
	}

	int32 ComputeTrapDamageHP(const int32 StageNum, const int32 BaseDamage)
	{
		const float StageScalar = FMath::Pow(1.14f, static_cast<float>(FMath::Max(StageNum - 1, 0)));
		return FMath::Max(8, FMath::RoundToInt(static_cast<float>(BaseDamage) * StageScalar));
	}

	int32 GetActorFloorNumber(const AT66GameMode* GameMode, const AActor* Actor)
	{
		return (GameMode && Actor) ? GameMode->GetTowerFloorIndexForLocation(Actor->GetActorLocation()) : INDEX_NONE;
	}

	void ApplyWallProjectileStyle(AT66WallArrowTrap* Trap, const FT66TrapVisualStyle& Style)
	{
		if (!Trap)
		{
			return;
		}

		Trap->TrapVisualScale = Style.MeshScale;
		Trap->TrapTint = Style.BaseColor;
		Trap->WindupColor = Style.WarningColor;
		Trap->ProjectileTint = Style.AccentColor;
		Trap->ProjectileTrailColor = Style.BurstColor;
	}

	void ApplyFloorBurstStyle(AT66FloorFlameTrap* Trap, const FT66TrapVisualStyle& Style)
	{
		if (!Trap)
		{
			return;
		}

		Trap->bUseFireNiagaraVFX = Style.bUseFireNiagaraVFX;
		Trap->IdleMarkerColor = Style.BaseColor;
		Trap->WarningColor = Style.WarningColor;
		Trap->ActiveColor = Style.AccentColor;
		Trap->BurstColor = Style.BurstColor;
	}

	void ApplyAreaControlStyle(AT66FloorSpikePatchTrap* Trap, const FT66TrapVisualStyle& Style)
	{
		if (!Trap)
		{
			return;
		}

		Trap->IdleMarkerColor = Style.BaseColor;
		Trap->WarningColor = Style.WarningColor;
		Trap->SpikeColor = Style.AccentColor;
		Trap->BurstColor = Style.BurstColor;
	}

	void ApplyObstacleStyle(AT66ObstacleTrapBase* Trap, const FT66TrapVisualStyle& Style)
	{
		if (!Trap)
		{
			return;
		}

		Trap->BaseColor = Style.BaseColor;
		Trap->AccentColor = Style.AccentColor;
	}

	void ApplyObstacleTuning(AT66ObstacleTrapBase* Trap, const FT66ObstacleTrapTuning& Tuning)
	{
		if (!Trap)
		{
			return;
		}

		Trap->LaunchXY = Tuning.LaunchXY;
		Trap->LaunchZ = Tuning.LaunchZ;
		Trap->ReactionCooldownSeconds = Tuning.CooldownSeconds;

		if (AT66SweeperArmTrap* Sweeper = Cast<AT66SweeperArmTrap>(Trap))
		{
			Sweeper->ArmLength = Tuning.PrimarySize;
			Sweeper->ArmThickness = Tuning.SecondarySize;
			Sweeper->ArmHeight = Tuning.Height;
			Sweeper->RotationSpeedDegPerSecond = Tuning.SpeedOrPeriod;
		}
		else if (AT66BumperTrap* Bumper = Cast<AT66BumperTrap>(Trap))
		{
			Bumper->Radius = Tuning.PrimarySize;
			Bumper->TravelDistance = Tuning.SecondarySize;
			Bumper->Height = Tuning.Height;
			Bumper->CyclePeriodSeconds = FMath::Max(0.20f, Tuning.SpeedOrPeriod);
		}
		else if (AT66WallBumperTrap* WallBumper = Cast<AT66WallBumperTrap>(Trap))
		{
			WallBumper->Width = Tuning.PrimarySize;
			WallBumper->TravelDistance = Tuning.SecondarySize;
			WallBumper->Height = Tuning.Height;
			WallBumper->CyclePeriodSeconds = FMath::Max(0.20f, Tuning.SpeedOrPeriod);
		}
		else if (AT66CeilingHammerTrap* Hammer = Cast<AT66CeilingHammerTrap>(Trap))
		{
			Hammer->HammerLength = Tuning.PrimarySize;
			Hammer->HammerHeadSize = Tuning.SecondarySize;
			Hammer->HangHeight = Tuning.Height;
			Hammer->SwingPeriodSeconds = FMath::Max(0.20f, Tuning.SpeedOrPeriod);
		}
	}
}

UT66TrapSubsystem* UT66TrapSubsystem::Get(UWorld* World)
{
	return World ? World->GetSubsystem<UT66TrapSubsystem>() : nullptr;
}

void UT66TrapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CachedTuning.LoadFromConfig();
}

bool UT66TrapSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UT66TrapSubsystem::RegisterTrap(AT66TrapBase* Trap)
{
	AddUniqueWeak(RegisteredTraps, Trap);
	ApplyCurrentProgressionToTrap(Trap);
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
	ActiveTowerFloorNumber = INDEX_NONE;

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

bool UT66TrapSubsystem::AreTowerTrapSpawnsEnabled() const
{
	return T66EnableTowerTrapSpawns;
}

void UT66TrapSubsystem::SetActiveTowerFloor(const int32 InActiveFloorNumber)
{
	PruneTrackedActors();

	if (ActiveTowerFloorNumber == InActiveFloorNumber)
	{
		return;
	}

	ActiveTowerFloorNumber = InActiveFloorNumber;
	for (const TWeakObjectPtr<AT66TrapBase>& WeakTrap : RegisteredTraps)
	{
		AT66TrapBase* Trap = WeakTrap.Get();
		if (!Trap)
		{
			continue;
		}

		const int32 TrapFloorNumber = Trap->GetTowerFloorNumber();
		const bool bShouldEnable = TrapFloorNumber == INDEX_NONE
			|| (ActiveTowerFloorNumber != INDEX_NONE && TrapFloorNumber == ActiveTowerFloorNumber);
		Trap->SetTrapEnabled(bShouldEnable);
	}
}

void UT66TrapSubsystem::RefreshRegisteredTrapProgression()
{
	PruneTrackedActors();
	for (const TWeakObjectPtr<AT66TrapBase>& WeakTrap : RegisteredTraps)
	{
		if (AT66TrapBase* Trap = WeakTrap.Get())
		{
			ApplyCurrentProgressionToTrap(Trap);
		}
	}
}

void UT66TrapSubsystem::ApplyCurrentProgressionToTrap(AT66TrapBase* Trap) const
{
	if (!Trap)
	{
		return;
	}

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66StageProgressionSubsystem* StageProgression = GameInstance ? GameInstance->GetSubsystem<UT66StageProgressionSubsystem>() : nullptr;
	if (!StageProgression)
	{
		Trap->ApplyProgressionScalars(1.0f, 1.0f);
		return;
	}

	StageProgression->RefreshSnapshot(false);
	const FT66StageProgressionSnapshot& Snapshot = StageProgression->GetCurrentSnapshot();
	Trap->ApplyProgressionScalars(Snapshot.TrapDamageScalar, Snapshot.TrapSpeedScalar);
}

void UT66TrapSubsystem::SpawnTowerStageTraps(const T66TowerMapTerrain::FLayout& Layout, const int32 StageNum, ET66Difficulty Difficulty, const int32 RunSeed)
{
	(void)Difficulty;

	if (!AreTowerTrapSpawnsEnabled())
	{
		ClearManagedTrapActors();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ClearManagedTrapActors();
	PruneTrackedActors();
	CachedTuning.LoadFromConfig();

	AT66GameMode* GameMode = World->GetAuthGameMode<AT66GameMode>();
	UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
	TArray<FUsedTrapLocation> UsedLocations;
	TMap<FName, int32> SpawnCounts;
	TMap<int32, int32> TrapCountsByFloor;

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
			for (const TWeakObjectPtr<AT66NPCBase>& WeakNPC : Registry->GetNPCs())
			{
				const AT66NPCBase* NPC = WeakNPC.Get();
				if (IsSafeZoneTooClose(NPC, (NPC ? NPC->GetSafeZoneRadius() : 0.f) + 500.f))
				{
					return false;
				}
			}
			for (const TWeakObjectPtr<UT66SafeZoneComponent>& WeakSafeZone : Registry->GetSafeZones())
			{
				const UT66SafeZoneComponent* SafeZone = WeakSafeZone.Get();
				const AActor* Owner = SafeZone ? SafeZone->GetOwner() : nullptr;
				if (IsSafeZoneTooClose(Owner, (SafeZone ? SafeZone->GetSafeZoneRadius() : 0.f) + 500.f))
				{
					return false;
				}
			}

		}

		return true;
	};

	auto RegisterManagedTrap = [&](AT66TrapBase* TrapActor, const int32 FloorNumber)
	{
		if (!TrapActor)
		{
			return;
		}

		AddUniqueWeak(ManagedTrapActors, static_cast<AActor*>(TrapActor));
		UsedLocations.Add({ TrapActor->GetActorLocation(), FloorNumber });
		SpawnCounts.FindOrAdd(TrapActor->GetTrapTypeID())++;
		if (FloorNumber != INDEX_NONE)
		{
			TrapCountsByFloor.FindOrAdd(FloorNumber)++;
		}
	};

	auto RegisterManagedActor = [&](AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}

		AddUniqueWeak(ManagedTrapActors, Actor);
	};

	auto SpawnPressurePlateForTrap = [&](AT66TrapBase* TrapActor, const FT66TrapRegistryEntry& Entry, const int32 FloorNumber)
	{
		if (!TrapActor)
		{
			return;
		}

		const bool bShouldSpawnPressurePlate = Entry.bSpawnPressurePlate || Entry.FamilyKey == TrapFamilyAreaControl;
		if (!bShouldSpawnPressurePlate)
		{
			return;
		}

		const FTransform PlateTransform(FRotator::ZeroRotator, TrapActor->GetActorLocation());
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = TrapActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66TrapPressurePlate* PressurePlate = World->SpawnActor<AT66TrapPressurePlate>(
			AT66TrapPressurePlate::StaticClass(),
			PlateTransform,
			SpawnParams);
		if (!PressurePlate)
		{
			return;
		}

		PressurePlate->SetTowerFloorNumber(FloorNumber);
		PressurePlate->SetTriggerTargetMode(Entry.TriggerTargetMode);
		PressurePlate->SetIdleColor(Entry.VisualStyle.BaseColor);
		PressurePlate->SetPressedColor(Entry.VisualStyle.WarningColor);
		PressurePlate->AddLinkedTrap(TrapActor);
		RegisterManagedActor(PressurePlate);
	};

	auto BuildSpawnRequestsForFloor =
		[&](const int32 TowerFloorNumber, const TArray<FName>& TrapPool, FRandomStream& FloorCountRng, TArray<FT66ResolvedTrapSpawnRequest, TInlineAllocator<8>>& OutRequests)
	{
		OutRequests.Reset();

		int32 MinimumTotal = 0;
		int32 MaximumTotal = 0;

		for (const FName TrapKey : TrapPool)
		{
			const FT66TrapRegistryEntry* Entry = FindTrapRegistryEntry(TrapKey);
			if (!Entry || !Entry->TrapClass)
			{
				continue;
			}

			FT66IntRange SpawnRange = { 0, 0 };
			bool bHasTuning = false;

			if (Entry->FamilyKey == TrapFamilyWallProjectile)
			{
				if (const FT66WallProjectileTrapTuning* Tuning = CachedTuning.FindWallProjectileTuning(TrapKey))
				{
					SpawnRange = Tuning->Spawn.SpawnCount;
					bHasTuning = true;
				}
			}
			else if (Entry->FamilyKey == TrapFamilyFloorBurst)
			{
				if (const FT66FloorBurstTrapTuning* Tuning = CachedTuning.FindFloorBurstTuning(TrapKey))
				{
					SpawnRange = Tuning->Spawn.SpawnCount;
					bHasTuning = true;
				}
			}
			else if (Entry->FamilyKey == TrapFamilyAreaControl)
			{
				if (const FT66AreaControlTrapTuning* Tuning = CachedTuning.FindAreaControlTuning(TrapKey))
				{
					SpawnRange = Tuning->Spawn.SpawnCount;
					bHasTuning = true;
				}
			}
			else if (Entry->FamilyKey == TrapFamilyObstacle)
			{
				if (const FT66ObstacleTrapTuning* Tuning = CachedTuning.FindObstacleTuning(TrapKey))
				{
					SpawnRange = Tuning->Spawn.SpawnCount;
					bHasTuning = true;
				}
			}

			if (!bHasTuning)
			{
				continue;
			}

			FT66ResolvedTrapSpawnRequest Request;
			Request.Entry = Entry;
			Request.MinCount = FMath::Max(0, SpawnRange.Min);
			Request.MaxCount = FMath::Max(Request.MinCount, SpawnRange.Max);
			Request.DesiredCount = Request.MinCount;
			MinimumTotal += Request.MinCount;
			MaximumTotal += Request.MaxCount;
			OutRequests.Add(Request);
		}

		if (OutRequests.Num() <= 0)
		{
			return;
		}

		const FT66TrapFloorSpawnTuning* FloorTuning = CachedTuning.FindFloorSpawnTuning(TowerFloorNumber);
		const FT66IntRange TotalTrapRange = FloorTuning
			? FloorTuning->TotalTrapCount
			: FT66IntRange{ MinimumTotal, MaximumTotal };
		const int32 TargetTotal = FMath::Clamp(
			RollSpawnCount(TotalTrapRange, FloorCountRng),
			MinimumTotal,
			MaximumTotal);

		int32 RemainingSlots = TargetTotal - MinimumTotal;
		while (RemainingSlots > 0)
		{
			TArray<int32, TInlineAllocator<8>> EligibleIndices;
			for (int32 RequestIndex = 0; RequestIndex < OutRequests.Num(); ++RequestIndex)
			{
				if (OutRequests[RequestIndex].DesiredCount < OutRequests[RequestIndex].MaxCount)
				{
					EligibleIndices.Add(RequestIndex);
				}
			}

			if (EligibleIndices.Num() <= 0)
			{
				break;
			}

			const int32 ChosenIndex = EligibleIndices[FloorCountRng.RandRange(0, EligibleIndices.Num() - 1)];
			++OutRequests[ChosenIndex].DesiredCount;
			--RemainingSlots;
		}
	};

	auto SpawnObstacleTrapInRoom =
		[&](
			const T66TowerMapTerrain::FFloor& Floor,
			const T66TowerMapTerrain::FRoom& Room,
			const FT66TrapRegistryEntry& Entry,
			const FT66ObstacleTrapTuning& Tuning,
			FRandomStream& SpawnRng,
			const float PaddingScale,
			const float MinSpacingScale) -> bool
	{
		const float FootprintRadius = FMath::Max(0.0f, Tuning.FootprintRadius);
		const float ClampedPaddingScale = FMath::Clamp(PaddingScale, 0.0f, 1.0f);
		const float EdgePadding = FMath::Max(FootprintRadius * 0.35f, FMath::Max(Tuning.EdgePadding, FootprintRadius * 1.10f) * ClampedPaddingScale);
		const float HolePadding = FMath::Max(FootprintRadius * 0.50f, FMath::Max(Tuning.HolePadding, FootprintRadius * 1.20f) * ClampedPaddingScale);
		const float WallPadding = FMath::Max(FootprintRadius * 0.25f, FMath::Max(Layout.PlacementCellSize * 0.35f, FootprintRadius * 0.80f) * ClampedPaddingScale);
		const float MinTrapSpacing = FMath::Max(0.0f, Tuning.Spawn.MinTrapSpacing * FMath::Max(0.0f, MinSpacingScale));
		const int32 SpawnAttempts = FMath::Max(Tuning.Spawn.SpawnAttempts, 12);

		for (int32 Attempt = 0; Attempt < SpawnAttempts; ++Attempt)
		{
			FVector SpawnLocation = FVector::ZeroVector;
			if (!T66TowerMapTerrain::TryGetRoomSurfaceLocation(
				World,
				Layout,
				Floor,
				Room,
				SpawnRng,
				SpawnLocation,
				EdgePadding,
				HolePadding,
				WallPadding))
			{
				continue;
			}

			SpawnLocation.Z = Floor.SurfaceZ + 6.f;
			if (!IsLocationClear(SpawnLocation, Floor.FloorNumber, MinTrapSpacing))
			{
				continue;
			}

			const FRotator SpawnRotation(0.f, SpawnRng.FRandRange(0.f, 360.f), 0.f);
			const FTransform SpawnTransform(SpawnRotation, SpawnLocation);
			AT66ObstacleTrapBase* ObstacleTrap = World->SpawnActorDeferred<AT66ObstacleTrapBase>(
				Entry.TrapClass.Get(),
				SpawnTransform,
				GameMode,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (!ObstacleTrap)
			{
				continue;
			}

			ObstacleTrap->SetTrapTypeID(Entry.RegistryKey);
			ObstacleTrap->SetTrapFamilyID(Entry.FamilyKey);
			ObstacleTrap->SetTowerFloorNumber(Floor.FloorNumber);
			ObstacleTrap->SetActivationMode(Entry.ActivationMode);
			ObstacleTrap->SetTriggerTargetMode(Entry.TriggerTargetMode);
			ObstacleTrap->SetDamagesHeroes(false);
			ObstacleTrap->SetDamagesEnemies(false);
			ApplyObstacleTuning(ObstacleTrap, Tuning);
			ApplyObstacleStyle(ObstacleTrap, Entry.VisualStyle);
			if (AT66CeilingHammerTrap* Hammer = Cast<AT66CeilingHammerTrap>(ObstacleTrap))
			{
				Hammer->InitialPhaseSeconds = RollFloatRange(Tuning.InitialPhaseSeconds, SpawnRng);
			}
			else if (AT66BumperTrap* Bumper = Cast<AT66BumperTrap>(ObstacleTrap))
			{
				Bumper->InitialPhaseSeconds = RollFloatRange(Tuning.InitialPhaseSeconds, SpawnRng);
			}
			else if (AT66WallBumperTrap* WallBumper = Cast<AT66WallBumperTrap>(ObstacleTrap))
			{
				WallBumper->InitialPhaseSeconds = RollFloatRange(Tuning.InitialPhaseSeconds, SpawnRng);
			}
			ObstacleTrap->FinishSpawning(SpawnTransform);

			RegisterManagedTrap(ObstacleTrap, Floor.FloorNumber);
			return true;
		}

		return false;
	};

	const UT66TowerTuningConfig& TowerTuning = UT66TowerTuningConfig::GetRuntimeConfig();

	for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
	{
		if (!IsTrapTowerFloor(Layout, Floor))
		{
			continue;
		}

		const int32 TowerFloorNumber = Floor.FloorNumber;
		const TArray<FName>& TrapPool = GetTrapPoolForTowerFloor(Floor);
		UE_LOG(
			LogT66TrapSubsystem,
			Verbose,
			TEXT("[Traps][FloorScan] Floor=%d bMob=%d Pool=%d"),
			TowerFloorNumber,
			Floor.bMobFloor ? 1 : 0,
			TrapPool.Num());
		if (TrapPool.Num() <= 0)
		{
			continue;
		}

		TArray<const FT66TrapRegistryEntry*, TInlineAllocator<8>> RoomPlaceableObstacleEntries;
		for (const FName TrapKey : TrapPool)
		{
			const FT66TrapRegistryEntry* Entry = FindTrapRegistryEntry(TrapKey);
			if (Entry
				&& Entry->TrapClass
				&& Entry->FamilyKey == TrapFamilyObstacle
				&& Entry->SpawnSurface == ET66TrapSpawnSurface::TileCenter
				&& CachedTuning.FindObstacleTuning(Entry->RegistryKey))
			{
				RoomPlaceableObstacleEntries.Add(Entry);
			}
		}

		struct FRoomTrapPlan
		{
			const T66TowerMapTerrain::FRoom* Room = nullptr;
			int32 MinCount = 0;
			int32 MaxCount = 0;
			int32 DesiredCount = 0;
		};

		TArray<FRoomTrapPlan, TInlineAllocator<16>> RoomTrapPlans;
		FRandomStream RoomCountRng(RunSeed + StageNum * 2903 + Floor.FloorNumber * 521 + 149);
		for (const T66TowerMapTerrain::FRoom& Room : Floor.Rooms)
		{
			const FT66TowerRoomRuleTuning* RoomRule = TowerTuning.FindRoomRule(Room.RoomRuleID);
			if (!RoomRule || RoomRule->TrapSlots.Max <= 0)
			{
				continue;
			}

			FRoomTrapPlan& Plan = RoomTrapPlans.AddDefaulted_GetRef();
			Plan.Room = &Room;
			Plan.MinCount = FMath::Max(0, RoomRule->TrapSlots.Min);
			Plan.MaxCount = FMath::Max(Plan.MinCount, RoomRule->TrapSlots.Max);
			Plan.DesiredCount = RoomCountRng.RandRange(Plan.MinCount, Plan.MaxCount);
		}

		bool bHasTwoTrapRoom = false;
		int32 FirstExpandableRoomIndex = INDEX_NONE;
		for (int32 PlanIndex = 0; PlanIndex < RoomTrapPlans.Num(); ++PlanIndex)
		{
			const FRoomTrapPlan& Plan = RoomTrapPlans[PlanIndex];
			if (Plan.DesiredCount > Plan.MinCount)
			{
				bHasTwoTrapRoom = true;
			}
			if (FirstExpandableRoomIndex == INDEX_NONE && Plan.MaxCount > Plan.MinCount)
			{
				FirstExpandableRoomIndex = PlanIndex;
			}
		}
		if (!bHasTwoTrapRoom && FirstExpandableRoomIndex != INDEX_NONE)
		{
			RoomTrapPlans[FirstExpandableRoomIndex].DesiredCount = RoomTrapPlans[FirstExpandableRoomIndex].MaxCount;
		}

		if (RoomTrapPlans.Num() > 0 && RoomPlaceableObstacleEntries.Num() > 0)
		{
			auto TrySpawnRoomTrapSlot =
				[&](
					const T66TowerMapTerrain::FFloor& RoomFloor,
					const FRoomTrapPlan& Plan,
					const int32 TrapIndex,
					const int32 PassIndex,
					const float PaddingScale,
					const float MinSpacingScale) -> bool
			{
				if (!Plan.Room)
				{
					return false;
				}

				FRandomStream RoomTrapRng(
					RunSeed
					+ StageNum * 3301
					+ RoomFloor.FloorNumber * 1009
					+ Plan.Room->RoomId * 131
					+ TrapIndex * 37
					+ PassIndex * 733);
				const int32 StartEntryIndex = RoomTrapRng.RandRange(0, RoomPlaceableObstacleEntries.Num() - 1);
				for (int32 EntryOffset = 0; EntryOffset < RoomPlaceableObstacleEntries.Num(); ++EntryOffset)
				{
					const FT66TrapRegistryEntry* Entry = RoomPlaceableObstacleEntries[(StartEntryIndex + EntryOffset) % RoomPlaceableObstacleEntries.Num()];
					const FT66ObstacleTrapTuning* Tuning = Entry ? CachedTuning.FindObstacleTuning(Entry->RegistryKey) : nullptr;
					if (Entry && Tuning && SpawnObstacleTrapInRoom(RoomFloor, *Plan.Room, *Entry, *Tuning, RoomTrapRng, PaddingScale, MinSpacingScale))
					{
						return true;
					}
				}

				return false;
			};

			int32 RoomTrapExpectedMin = 0;
			int32 RoomTrapExpectedMax = 0;
			int32 RoomTrapDesired = 0;
			int32 RoomTrapSpawned = 0;
			int32 RoomTrapRoomsWithSpawn = 0;
			for (int32 PlanIndex = 0; PlanIndex < RoomTrapPlans.Num(); ++PlanIndex)
			{
				const FRoomTrapPlan& Plan = RoomTrapPlans[PlanIndex];
				if (!Plan.Room || Plan.DesiredCount <= 0)
				{
					continue;
				}

				RoomTrapExpectedMin += Plan.MinCount;
				RoomTrapExpectedMax += Plan.MaxCount;
				RoomTrapDesired += Plan.DesiredCount;
				int32 SpawnedInRoom = 0;
				for (int32 TrapIndex = 0; TrapIndex < Plan.DesiredCount; ++TrapIndex)
				{
					bool bSpawnedSlot = false;
					constexpr float PaddingScales[] = { 1.00f, 0.70f, 0.45f };
					constexpr float MinSpacingScales[] = { 1.00f, 0.70f, 0.45f };
					for (int32 PassIndex = 0; PassIndex < UE_ARRAY_COUNT(PaddingScales); ++PassIndex)
					{
						if (TrySpawnRoomTrapSlot(Floor, Plan, TrapIndex, PassIndex, PaddingScales[PassIndex], MinSpacingScales[PassIndex]))
						{
							bSpawnedSlot = true;
							break;
						}
					}

					if (!bSpawnedSlot && SpawnedInRoom <= 0)
					{
						bSpawnedSlot = TrySpawnRoomTrapSlot(Floor, Plan, TrapIndex, 97, 0.25f, 0.0f);
					}

					if (bSpawnedSlot)
					{
						++SpawnedInRoom;
						++RoomTrapSpawned;
					}
				}
				if (SpawnedInRoom > 0)
				{
					++RoomTrapRoomsWithSpawn;
				}
			}

			const bool bRoomTrapProofPass = RoomTrapSpawned >= RoomTrapDesired && RoomTrapRoomsWithSpawn == RoomTrapPlans.Num();
			if (bRoomTrapProofPass)
			{
				UE_LOG(
					LogT66TrapSubsystem,
					Log,
					TEXT("[T66Proof][TowerRoomTrapSummary] Stage=%d Floor=%d Result=PASS Rooms=%d RoomsWithTrap=%d Desired=%d Spawned=%d ExpectedRange=%d-%d Rule=RoomTrapSlots"),
					StageNum,
					Floor.FloorNumber,
					RoomTrapPlans.Num(),
					RoomTrapRoomsWithSpawn,
					RoomTrapDesired,
					RoomTrapSpawned,
					RoomTrapExpectedMin,
					RoomTrapExpectedMax);
			}
			else
			{
				UE_LOG(
					LogT66TrapSubsystem,
					Warning,
					TEXT("[T66Proof][TowerRoomTrapSummary] Stage=%d Floor=%d Result=FAIL Rooms=%d RoomsWithTrap=%d Desired=%d Spawned=%d ExpectedRange=%d-%d Rule=RoomTrapSlots"),
					StageNum,
					Floor.FloorNumber,
					RoomTrapPlans.Num(),
					RoomTrapRoomsWithSpawn,
					RoomTrapDesired,
					RoomTrapSpawned,
					RoomTrapExpectedMin,
					RoomTrapExpectedMax);
			}
			continue;
		}

		FRandomStream FloorCountRng(RunSeed + StageNum * 2903 + Floor.FloorNumber * 311 + 97);
		TArray<FT66ResolvedTrapSpawnRequest, TInlineAllocator<8>> SpawnRequests;
		BuildSpawnRequestsForFloor(TowerFloorNumber, TrapPool, FloorCountRng, SpawnRequests);
		UE_LOG(
			LogT66TrapSubsystem,
			Verbose,
			TEXT("[Traps][FloorRequests] Floor=%d Requests=%d"),
			TowerFloorNumber,
			SpawnRequests.Num());

		for (const FT66ResolvedTrapSpawnRequest& SpawnRequest : SpawnRequests)
		{
			const FT66TrapRegistryEntry* Entry = SpawnRequest.Entry;
			if (!Entry || !Entry->TrapClass || SpawnRequest.DesiredCount <= 0)
			{
				continue;
			}

			const FName TrapKey = Entry->RegistryKey;
			const int32 KeySeed = static_cast<int32>(GetTypeHash(TrapKey) & 0x7fffffff);

			if (Entry->FamilyKey == TrapFamilyWallProjectile)
			{
				const FT66WallProjectileTrapTuning* Tuning = CachedTuning.FindWallProjectileTuning(TrapKey);
				if (!Tuning)
				{
					continue;
				}

				const int32 DesiredCount = SpawnRequest.DesiredCount;
				for (int32 SpawnIndex = 0; SpawnIndex < DesiredCount; ++SpawnIndex)
				{
					FRandomStream SpawnRng(RunSeed + StageNum * 2501 + Floor.FloorNumber * 131 + KeySeed + SpawnIndex * 17);
					for (int32 Attempt = 0; Attempt < Tuning->Spawn.SpawnAttempts; ++Attempt)
					{
						FVector SpawnLocation = FVector::ZeroVector;
						FVector WallNormal = FVector::ZeroVector;
						if (!T66TowerMapTerrain::TryGetMazeWallSpawnLocation(World, Layout, Floor.FloorNumber, SpawnRng, SpawnLocation, WallNormal))
						{
							continue;
						}

						SpawnLocation.Z = Floor.SurfaceZ + Tuning->HeightOffset;
						if (!IsLocationClear(SpawnLocation, Floor.FloorNumber, Tuning->Spawn.MinTrapSpacing))
						{
							continue;
						}

						const FTransform SpawnTransform(WallNormal.Rotation(), SpawnLocation);
						AT66WallArrowTrap* ArrowTrap = World->SpawnActorDeferred<AT66WallArrowTrap>(
							Entry->TrapClass.Get(),
							SpawnTransform,
							GameMode,
							nullptr,
							ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
						if (!ArrowTrap)
						{
							continue;
						}

						ArrowTrap->SetTrapTypeID(Entry->RegistryKey);
						ArrowTrap->SetTrapFamilyID(Entry->FamilyKey);
						ArrowTrap->SetTowerFloorNumber(Floor.FloorNumber);
						ArrowTrap->SetActivationMode(Entry->ActivationMode);
						ArrowTrap->SetTriggerTargetMode(Entry->TriggerTargetMode);
						ArrowTrap->DamageHP = ComputeTrapDamageHP(StageNum, Tuning->DamageHP);
						ArrowTrap->ProjectileSpeed = Tuning->ProjectileSpeed;
						ArrowTrap->FireIntervalSeconds = Tuning->FireIntervalSeconds;
						ArrowTrap->WindupDurationSeconds = Tuning->WindupDurationSeconds;
						ArrowTrap->InitialFireDelaySeconds = RollFloatRange(Tuning->InitialFireDelaySeconds, SpawnRng);
						ArrowTrap->DetectionRange = FMath::Max(Tuning->DetectionRange, Floor.BoundsHalfExtent * 0.75f);
						ApplyWallProjectileStyle(ArrowTrap, Entry->VisualStyle);
						ArrowTrap->FinishSpawning(SpawnTransform);

						RegisterManagedTrap(ArrowTrap, Floor.FloorNumber);
						break;
					}
				}
			}
			else if (Entry->FamilyKey == TrapFamilyFloorBurst)
			{
				const FT66FloorBurstTrapTuning* Tuning = CachedTuning.FindFloorBurstTuning(TrapKey);
				if (!Tuning)
				{
					continue;
				}

				const int32 DesiredCount = SpawnRequest.DesiredCount;
				for (int32 SpawnIndex = 0; SpawnIndex < DesiredCount; ++SpawnIndex)
				{
					FRandomStream SpawnRng(RunSeed + StageNum * 2701 + Floor.FloorNumber * 149 + KeySeed + SpawnIndex * 23);
					for (int32 Attempt = 0; Attempt < Tuning->Spawn.SpawnAttempts; ++Attempt)
					{
						FVector SpawnLocation = FVector::ZeroVector;
						if (!T66TowerMapTerrain::TryGetFloorTileCenterSpawnLocation(
							World,
							Layout,
							Floor.FloorNumber,
							SpawnRng,
							SpawnLocation,
							Tuning->EdgePadding,
							Tuning->HolePadding,
							FMath::Max(550.f, Layout.PlacementCellSize * 0.30f)))
						{
							continue;
						}

						SpawnLocation.Z = Floor.SurfaceZ + 6.f;
						if (!IsLocationClear(SpawnLocation, Floor.FloorNumber, Tuning->Spawn.MinTrapSpacing))
						{
							continue;
						}

						const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
						AT66FloorFlameTrap* FloorBurstTrap = World->SpawnActorDeferred<AT66FloorFlameTrap>(
							Entry->TrapClass.Get(),
							SpawnTransform,
							GameMode,
							nullptr,
							ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
						if (!FloorBurstTrap)
						{
							continue;
						}

						FloorBurstTrap->SetTrapTypeID(Entry->RegistryKey);
						FloorBurstTrap->SetTrapFamilyID(Entry->FamilyKey);
						FloorBurstTrap->SetTowerFloorNumber(Floor.FloorNumber);
						FloorBurstTrap->SetActivationMode(Entry->ActivationMode);
						FloorBurstTrap->SetTriggerTargetMode(Entry->TriggerTargetMode);
						FloorBurstTrap->Radius = Tuning->Radius;
						FloorBurstTrap->DamageHP = ComputeTrapDamageHP(StageNum, Tuning->DamageHP);
						FloorBurstTrap->WarningDurationSeconds = Tuning->WarningDurationSeconds;
						FloorBurstTrap->ActiveDurationSeconds = Tuning->ActiveDurationSeconds;
						FloorBurstTrap->CooldownDurationSeconds = Tuning->CooldownDurationSeconds;
						FloorBurstTrap->DamageIntervalSeconds = Tuning->DamageIntervalSeconds;
						FloorBurstTrap->InitialCycleDelaySeconds = RollFloatRange(Tuning->InitialCycleDelaySeconds, SpawnRng);
						ApplyFloorBurstStyle(FloorBurstTrap, Entry->VisualStyle);
						FloorBurstTrap->FinishSpawning(SpawnTransform);

						RegisterManagedTrap(FloorBurstTrap, Floor.FloorNumber);
						break;
					}
				}
			}
			else if (Entry->FamilyKey == TrapFamilyAreaControl)
			{
				const FT66AreaControlTrapTuning* Tuning = CachedTuning.FindAreaControlTuning(TrapKey);
				if (!Tuning)
				{
					continue;
				}

				const int32 DesiredCount = SpawnRequest.DesiredCount;
				for (int32 SpawnIndex = 0; SpawnIndex < DesiredCount; ++SpawnIndex)
				{
					FRandomStream SpawnRng(RunSeed + StageNum * 2909 + Floor.FloorNumber * 173 + KeySeed + SpawnIndex * 29);
					for (int32 Attempt = 0; Attempt < Tuning->Spawn.SpawnAttempts; ++Attempt)
					{
						FVector SpawnLocation = FVector::ZeroVector;
						if (!T66TowerMapTerrain::TryGetFloorTileCenterSpawnLocation(
							World,
							Layout,
							Floor.FloorNumber,
							SpawnRng,
							SpawnLocation,
							Tuning->EdgePadding,
							Tuning->HolePadding,
							FMath::Max(600.f, Layout.PlacementCellSize * 0.33f)))
						{
							continue;
						}

						SpawnLocation.Z = Floor.SurfaceZ + 4.f;
						if (!IsLocationClear(SpawnLocation, Floor.FloorNumber, Tuning->Spawn.MinTrapSpacing))
						{
							continue;
						}

						const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
						AT66FloorSpikePatchTrap* SpikePatchTrap = World->SpawnActorDeferred<AT66FloorSpikePatchTrap>(
							Entry->TrapClass.Get(),
							SpawnTransform,
							GameMode,
							nullptr,
							ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
						if (!SpikePatchTrap)
						{
							continue;
						}

						SpikePatchTrap->SetTrapTypeID(Entry->RegistryKey);
						SpikePatchTrap->SetTrapFamilyID(Entry->FamilyKey);
						SpikePatchTrap->SetTowerFloorNumber(Floor.FloorNumber);
						SpikePatchTrap->SetActivationMode(ET66TrapActivationMode::Triggered);
						SpikePatchTrap->SetTriggerTargetMode(ET66TrapTriggerTarget::HeroesAndEnemies);
						SpikePatchTrap->Radius = Tuning->Radius;
						SpikePatchTrap->WarningDurationSeconds = Tuning->WarningDurationSeconds;
						SpikePatchTrap->RiseDurationSeconds = Tuning->RiseDurationSeconds;
						SpikePatchTrap->RaisedDurationSeconds = Tuning->RaisedDurationSeconds;
						SpikePatchTrap->RetractDurationSeconds = Tuning->RetractDurationSeconds;
						SpikePatchTrap->CooldownDurationSeconds = Tuning->CooldownDurationSeconds;
						SpikePatchTrap->DamageIntervalSeconds = Tuning->DamageIntervalSeconds;
						SpikePatchTrap->InitialCycleDelaySeconds = RollFloatRange(Tuning->InitialCycleDelaySeconds, SpawnRng);
						SpikePatchTrap->SpikeHeight = Tuning->SpikeHeight;
						SpikePatchTrap->SpikeCount = Tuning->SpikeCount;
						SpikePatchTrap->DamageHP = ComputeTrapDamageHP(StageNum, Tuning->DamageHP);
						ApplyAreaControlStyle(SpikePatchTrap, Entry->VisualStyle);
						SpikePatchTrap->FinishSpawning(SpawnTransform);

						RegisterManagedTrap(SpikePatchTrap, Floor.FloorNumber);
						SpawnPressurePlateForTrap(SpikePatchTrap, *Entry, Floor.FloorNumber);
						break;
					}
				}
			}
			else if (Entry->FamilyKey == TrapFamilyObstacle)
			{
				const FT66ObstacleTrapTuning* Tuning = CachedTuning.FindObstacleTuning(TrapKey);
				if (!Tuning)
				{
					continue;
				}

				const int32 DesiredCount = SpawnRequest.DesiredCount;
				int32 SpawnedForRequest = 0;
				int32 LocationFailureCount = 0;
				int32 ClearFailureCount = 0;
				int32 SpawnFailureCount = 0;
				for (int32 SpawnIndex = 0; SpawnIndex < DesiredCount; ++SpawnIndex)
				{
					FRandomStream SpawnRng(RunSeed + StageNum * 3301 + Floor.FloorNumber * 197 + KeySeed + SpawnIndex * 31);
					for (int32 Attempt = 0; Attempt < Tuning->Spawn.SpawnAttempts; ++Attempt)
					{
						FVector SpawnLocation = FVector::ZeroVector;
						FRotator SpawnRotation = FRotator::ZeroRotator;
						if (Entry->SpawnSurface == ET66TrapSpawnSurface::MazeWall)
						{
							FVector WallNormal = FVector::ZeroVector;
							if (!T66TowerMapTerrain::TryGetMazeWallSpawnLocation(
								World,
								Layout,
								Floor.FloorNumber,
								SpawnRng,
								SpawnLocation,
								WallNormal,
								Tuning->EdgePadding))
							{
								++LocationFailureCount;
								continue;
							}
							SpawnRotation = WallNormal.Rotation();
						}
						else
						{
							if (!T66TowerMapTerrain::TryGetObstacleTrapSpawnLocation(
								World,
								Layout,
								Floor.FloorNumber,
								SpawnRng,
								SpawnLocation,
								Tuning->FootprintRadius,
								Tuning->EdgePadding,
								Tuning->HolePadding))
							{
								++LocationFailureCount;
								continue;
							}
							SpawnRotation = FRotator(0.f, SpawnRng.FRandRange(0.f, 360.f), 0.f);
						}

						SpawnLocation.Z = Floor.SurfaceZ + 6.f;
						if (!IsLocationClear(SpawnLocation, Floor.FloorNumber, Tuning->Spawn.MinTrapSpacing))
						{
							++ClearFailureCount;
							continue;
						}

						const FTransform SpawnTransform(SpawnRotation, SpawnLocation);
						AT66ObstacleTrapBase* ObstacleTrap = World->SpawnActorDeferred<AT66ObstacleTrapBase>(
							Entry->TrapClass.Get(),
							SpawnTransform,
							GameMode,
							nullptr,
							ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
						if (!ObstacleTrap)
						{
							++SpawnFailureCount;
							continue;
						}

						ObstacleTrap->SetTrapTypeID(Entry->RegistryKey);
						ObstacleTrap->SetTrapFamilyID(Entry->FamilyKey);
						ObstacleTrap->SetTowerFloorNumber(Floor.FloorNumber);
						ObstacleTrap->SetActivationMode(Entry->ActivationMode);
						ObstacleTrap->SetTriggerTargetMode(Entry->TriggerTargetMode);
						ObstacleTrap->SetDamagesHeroes(false);
						ObstacleTrap->SetDamagesEnemies(false);
						ApplyObstacleTuning(ObstacleTrap, *Tuning);
						ApplyObstacleStyle(ObstacleTrap, Entry->VisualStyle);
						if (AT66CeilingHammerTrap* Hammer = Cast<AT66CeilingHammerTrap>(ObstacleTrap))
						{
							Hammer->InitialPhaseSeconds = RollFloatRange(Tuning->InitialPhaseSeconds, SpawnRng);
						}
						else if (AT66BumperTrap* Bumper = Cast<AT66BumperTrap>(ObstacleTrap))
						{
							Bumper->InitialPhaseSeconds = RollFloatRange(Tuning->InitialPhaseSeconds, SpawnRng);
						}
						else if (AT66WallBumperTrap* WallBumper = Cast<AT66WallBumperTrap>(ObstacleTrap))
						{
							WallBumper->InitialPhaseSeconds = RollFloatRange(Tuning->InitialPhaseSeconds, SpawnRng);
						}
						ObstacleTrap->FinishSpawning(SpawnTransform);

						RegisterManagedTrap(ObstacleTrap, Floor.FloorNumber);
						++SpawnedForRequest;
						break;
					}
				}
				if (SpawnedForRequest < DesiredCount)
				{
					UE_LOG(
						LogT66TrapSubsystem,
						Verbose,
						TEXT("[Traps][ObstacleSpawnMiss] Floor=%d Type=%s Desired=%d Spawned=%d LocationFailures=%d ClearFailures=%d SpawnFailures=%d"),
						Floor.FloorNumber,
						*TrapKey.ToString(),
						DesiredCount,
						SpawnedForRequest,
						LocationFailureCount,
						ClearFailureCount,
						SpawnFailureCount);
				}
			}
		}
	}

	int32 TotalSpawned = 0;
	for (const TPair<FName, int32>& Pair : SpawnCounts)
	{
		TotalSpawned += Pair.Value;
		UE_LOG(LogT66TrapSubsystem, Log, TEXT("[Traps] %s x%d"), *Pair.Key.ToString(), Pair.Value);
	}

	TArray<int32> TrapFloorNumbers;
	TrapCountsByFloor.GetKeys(TrapFloorNumbers);
	TrapFloorNumbers.Sort();

	FString TrapFloorSummary;
	for (const int32 TrapFloorNumber : TrapFloorNumbers)
	{
		if (!TrapFloorSummary.IsEmpty())
		{
			TrapFloorSummary += TEXT(", ");
		}

		TrapFloorSummary += FString::Printf(TEXT("Floor %d x%d"), TrapFloorNumber, TrapCountsByFloor.FindRef(TrapFloorNumber));
	}
	if (TrapFloorSummary.IsEmpty())
	{
		TrapFloorSummary = TEXT("none");
	}

	UE_LOG(
		LogT66TrapSubsystem,
		Log,
		TEXT("[Traps] Spawned %d floor-driven traps for tower stage %d on %s."),
		TotalSpawned,
		StageNum,
		*TrapFloorSummary);
}

