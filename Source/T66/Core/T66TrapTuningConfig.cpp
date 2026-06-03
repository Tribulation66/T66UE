// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66TrapTuningConfig.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/StaticMesh.h"
#include "Misc/ConfigCacheIni.h"
#include "NiagaraSystem.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66TrapAssets, Log, All);

namespace
{
	static constexpr const TCHAR* T66TrapConfigSection = TEXT("/Script/T66.T66TrapTuningConfig");

	static FString GetT66TrapConfigFilename()
	{
		FString ConfigFilename;
		FConfigCacheIni::LoadGlobalIniFile(ConfigFilename, TEXT("T66TrapTuning"));
		return ConfigFilename;
	}

	static TMap<FString, TSharedPtr<FStreamableHandle>>& GetTrapAssetAsyncLoadHandles()
	{
		static TMap<FString, TSharedPtr<FStreamableHandle>> ActiveLoads;
		return ActiveLoads;
	}

	static void QueueTrapAssetAsyncLoad(const FSoftObjectPath& ObjectPath)
	{
		if (!ObjectPath.IsValid() || ObjectPath.ResolveObject())
		{
			return;
		}
		if (!UAssetManager::IsInitialized())
		{
			return;
		}

		const FString Key = ObjectPath.ToString();
		TMap<FString, TSharedPtr<FStreamableHandle>>& ActiveLoads = GetTrapAssetAsyncLoadHandles();
		if (ActiveLoads.Contains(Key))
		{
			return;
		}

		TArray<FSoftObjectPath> AssetPaths;
		AssetPaths.Add(ObjectPath);
		ActiveLoads.Add(Key, UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetPaths, FStreamableDelegate()));
	}

	static void QueueConfiguredTrapAssetPreload(const FString& ObjectPathString)
	{
		const FString TrimmedPath = ObjectPathString.TrimStartAndEnd();
		if (TrimmedPath.IsEmpty())
		{
			return;
		}

		const FSoftObjectPath ObjectPath(TrimmedPath);
		QueueTrapAssetAsyncLoad(ObjectPath);
	}

	static void QueueConfiguredTrapAssetPreloads(const FT66TrapVisualAssetConfig& TrapAssets)
	{
		QueueConfiguredTrapAssetPreload(TrapAssets.WallArrowMesh);
		QueueConfiguredTrapAssetPreload(TrapAssets.ArrowProjectileMesh);
		QueueConfiguredTrapAssetPreload(TrapAssets.ArrowProjectileTrailNiagara);
		QueueConfiguredTrapAssetPreload(TrapAssets.ArrowProjectileFallbackTrailNiagara);
		QueueConfiguredTrapAssetPreload(TrapAssets.FloorFlameNiagara);
		QueueConfiguredTrapAssetPreload(TrapAssets.FloorSpikeMesh);
		QueueConfiguredTrapAssetPreload(TrapAssets.FloorSpikeClusterMesh);
		QueueConfiguredTrapAssetPreload(TrapAssets.FloorSpikeRiseBurstNiagara);
	}

	template <typename StructType>
	static void LoadTrapStructValue(const FString& ConfigFilename, const TCHAR* Key, StructType& Value)
	{
		if (!GConfig)
		{
			return;
		}

		FString RawValue;
		if (!GConfig->GetString(T66TrapConfigSection, Key, RawValue, ConfigFilename) || RawValue.IsEmpty())
		{
			return;
		}

		UScriptStruct* StructTypeInfo = StructType::StaticStruct();
		if (!StructTypeInfo)
		{
			return;
		}

		StructTypeInfo->ImportText(*RawValue, &Value, nullptr, PPF_None, GLog, StructTypeInfo->GetName());
	}

	template <typename AssetType>
	static AssetType* LoadConfiguredTrapAsset(const FString& ObjectPathString, const TCHAR* ConfigKey, const TCHAR* AssetTypeName)
	{
		static TSet<FString> WarnedFailures;

		const FString TrimmedPath = ObjectPathString.TrimStartAndEnd();
		const FString AssetPathForLog = TrimmedPath.IsEmpty() ? TEXT("<empty>") : TrimmedPath;
		const FString WarningKey = FString::Printf(TEXT("%s|%s|%s"), ConfigKey, AssetTypeName, *AssetPathForLog);

		auto WarnOnce = [&]()
		{
			if (!WarnedFailures.Contains(WarningKey))
			{
				WarnedFailures.Add(WarningKey);
				UE_LOG(
					LogT66TrapAssets,
					Warning,
					TEXT("Trap asset config %s is missing or invalid for %s path '%s'. Check Config/DefaultT66TrapTuning.ini before shipping."),
					ConfigKey,
					AssetTypeName,
					*AssetPathForLog);
			}
		};

		if (TrimmedPath.IsEmpty())
		{
			WarnOnce();
			return nullptr;
		}

		const FSoftObjectPath ObjectPath(TrimmedPath);
		if (!ObjectPath.IsValid())
		{
			WarnOnce();
			return nullptr;
		}

		if (AssetType* LoadedAsset = Cast<AssetType>(ObjectPath.ResolveObject()))
		{
			return LoadedAsset;
		}

		QueueTrapAssetAsyncLoad(ObjectPath);
		return nullptr;
	}

	static FT66TrapSpawnWindow MakeSpawnWindow(const int32 MinCount, const int32 MaxCount, const int32 Attempts, const float MinSpacing)
	{
		FT66TrapSpawnWindow Window;
		Window.SpawnCount = { MinCount, MaxCount };
		Window.SpawnAttempts = Attempts;
		Window.MinTrapSpacing = MinSpacing;
		return Window;
	}

	static FT66TrapFloorSpawnTuning MakeFloorSpawnTuning(const int32 MinCount, const int32 MaxCount)
	{
		FT66TrapFloorSpawnTuning Tuning;
		Tuning.TotalTrapCount = { MinCount, MaxCount };
		return Tuning;
	}

	static FT66WallProjectileTrapTuning MakeWallTuning(
		const int32 MinCount,
		const int32 MaxCount,
		const float FireInterval,
		const float Windup,
		const float DelayMin,
		const float DelayMax,
		const float DetectionRange,
		const float ProjectileSpeed,
		const int32 DamageHP,
		const float HeightOffset = 115.f,
		const float MinSpacing = 1200.f)
	{
		FT66WallProjectileTrapTuning Tuning;
		Tuning.Spawn = MakeSpawnWindow(MinCount, MaxCount, 20, MinSpacing);
		Tuning.HeightOffset = HeightOffset;
		Tuning.FireIntervalSeconds = FireInterval;
		Tuning.WindupDurationSeconds = Windup;
		Tuning.InitialFireDelaySeconds = { DelayMin, DelayMax };
		Tuning.DetectionRange = DetectionRange;
		Tuning.ProjectileSpeed = ProjectileSpeed;
		Tuning.DamageHP = DamageHP;
		return Tuning;
	}

	static FT66FloorBurstTrapTuning MakeFloorBurstTuning(
		const int32 MinCount,
		const int32 MaxCount,
		const float Radius,
		const float WarningDuration,
		const float ActiveDuration,
		const float CooldownDuration,
		const float DamageInterval,
		const float DelayMin,
		const float DelayMax,
		const int32 DamageHP,
		const float EdgePadding = 1800.f,
		const float HolePadding = 1900.f,
		const float MinSpacing = 1450.f)
	{
		FT66FloorBurstTrapTuning Tuning;
		Tuning.Spawn = MakeSpawnWindow(MinCount, MaxCount, 20, MinSpacing);
		Tuning.EdgePadding = EdgePadding;
		Tuning.HolePadding = HolePadding;
		Tuning.Radius = Radius;
		Tuning.WarningDurationSeconds = WarningDuration;
		Tuning.ActiveDurationSeconds = ActiveDuration;
		Tuning.CooldownDurationSeconds = CooldownDuration;
		Tuning.DamageIntervalSeconds = DamageInterval;
		Tuning.InitialCycleDelaySeconds = { DelayMin, DelayMax };
		Tuning.DamageHP = DamageHP;
		return Tuning;
	}

	static FT66AreaControlTrapTuning MakeAreaControlTuning(
		const int32 MinCount,
		const int32 MaxCount,
		const float Radius,
		const float WarningDuration,
		const float RiseDuration,
		const float RaisedDuration,
		const float RetractDuration,
		const float CooldownDuration,
		const float DamageInterval,
		const float DelayMin,
		const float DelayMax,
		const float SpikeHeight,
		const int32 SpikeCount,
		const int32 DamageHP,
		const float EdgePadding = 1500.f,
		const float HolePadding = 1650.f,
		const float MinSpacing = 1500.f)
	{
		FT66AreaControlTrapTuning Tuning;
		Tuning.Spawn = MakeSpawnWindow(MinCount, MaxCount, 20, MinSpacing);
		Tuning.EdgePadding = EdgePadding;
		Tuning.HolePadding = HolePadding;
		Tuning.Radius = Radius;
		Tuning.WarningDurationSeconds = WarningDuration;
		Tuning.RiseDurationSeconds = RiseDuration;
		Tuning.RaisedDurationSeconds = RaisedDuration;
		Tuning.RetractDurationSeconds = RetractDuration;
		Tuning.CooldownDurationSeconds = CooldownDuration;
		Tuning.DamageIntervalSeconds = DamageInterval;
		Tuning.InitialCycleDelaySeconds = { DelayMin, DelayMax };
		Tuning.SpikeHeight = SpikeHeight;
		Tuning.SpikeCount = SpikeCount;
		Tuning.DamageHP = DamageHP;
		return Tuning;
	}
}

UT66TrapTuningConfig::UT66TrapTuningConfig()
	: TowerFloor2(MakeFloorSpawnTuning(3, 4))
	, TowerFloor3(MakeFloorSpawnTuning(3, 5))
	, TowerFloor4(MakeFloorSpawnTuning(4, 5))
	, DungeonWallArrow(MakeWallTuning(1, 2, 2.60f, 0.40f, 0.35f, 1.15f, 5200.f, 2400.f, 12))
	, DungeonFloorFlame(MakeFloorBurstTuning(1, 2, 260.f, 0.80f, 1.15f, 3.00f, 0.35f, 0.80f, 2.10f, 10))
	, DungeonFloorSpikePatch(MakeAreaControlTuning(1, 2, 285.f, 0.90f, 0.25f, 1.05f, 0.25f, 2.85f, 0.35f, 1.00f, 2.15f, 165.f, 10, 11))
	, ForestThornVolley(MakeWallTuning(1, 2, 2.40f, 0.35f, 0.40f, 1.10f, 5200.f, 2300.f, 12))
	, ForestSporeBurst(MakeFloorBurstTuning(1, 2, 250.f, 0.85f, 1.00f, 2.80f, 0.35f, 0.95f, 2.00f, 11))
	, ForestBramblePatch(MakeAreaControlTuning(1, 2, 300.f, 0.95f, 0.25f, 1.05f, 0.30f, 2.70f, 0.35f, 1.10f, 2.20f, 170.f, 12, 12))
	, OceanHarpoonVolley(MakeWallTuning(1, 2, 2.20f, 0.35f, 0.35f, 1.00f, 5600.f, 2550.f, 13))
	, OceanSteamBurst(MakeFloorBurstTuning(1, 2, 275.f, 0.80f, 1.10f, 2.65f, 0.35f, 0.85f, 1.90f, 12))
	, OceanUrchinPatch(MakeAreaControlTuning(1, 2, 315.f, 0.85f, 0.22f, 1.10f, 0.28f, 2.55f, 0.35f, 0.95f, 2.00f, 180.f, 12, 13))
	, MartianShardVolley(MakeWallTuning(1, 2, 2.05f, 0.30f, 0.30f, 0.95f, 5800.f, 2680.f, 14))
	, MartianPlasmaBurst(MakeFloorBurstTuning(1, 2, 290.f, 0.75f, 1.15f, 2.45f, 0.30f, 0.80f, 1.80f, 13))
	, MartianCrystalPatch(MakeAreaControlTuning(1, 2, 330.f, 0.80f, 0.22f, 1.15f, 0.25f, 2.40f, 0.30f, 0.90f, 1.95f, 195.f, 13, 14))
	, HellSoulBoltVolley(MakeWallTuning(1, 3, 1.90f, 0.25f, 0.25f, 0.90f, 6200.f, 2825.f, 16))
	, HellEmberBurst(MakeFloorBurstTuning(1, 2, 315.f, 0.70f, 1.25f, 2.20f, 0.30f, 0.75f, 1.70f, 15))
	, HellBrimstonePatch(MakeAreaControlTuning(1, 2, 350.f, 0.78f, 0.20f, 1.20f, 0.25f, 2.20f, 0.30f, 0.80f, 1.80f, 210.f, 14, 16))
{
}

void UT66TrapTuningConfig::LoadFromConfig()
{
	const FString ConfigFilename = GetT66TrapConfigFilename();

	LoadTrapStructValue(ConfigFilename, TEXT("TowerFloor2"), TowerFloor2);
	LoadTrapStructValue(ConfigFilename, TEXT("TowerFloor3"), TowerFloor3);
	LoadTrapStructValue(ConfigFilename, TEXT("TowerFloor4"), TowerFloor4);

	LoadTrapStructValue(ConfigFilename, TEXT("TrapAssets"), TrapAssets);

	LoadTrapStructValue(ConfigFilename, TEXT("DungeonWallArrow"), DungeonWallArrow);
	LoadTrapStructValue(ConfigFilename, TEXT("DungeonFloorFlame"), DungeonFloorFlame);
	LoadTrapStructValue(ConfigFilename, TEXT("DungeonFloorSpikePatch"), DungeonFloorSpikePatch);

	LoadTrapStructValue(ConfigFilename, TEXT("ForestThornVolley"), ForestThornVolley);
	LoadTrapStructValue(ConfigFilename, TEXT("ForestSporeBurst"), ForestSporeBurst);
	LoadTrapStructValue(ConfigFilename, TEXT("ForestBramblePatch"), ForestBramblePatch);

	LoadTrapStructValue(ConfigFilename, TEXT("OceanHarpoonVolley"), OceanHarpoonVolley);
	LoadTrapStructValue(ConfigFilename, TEXT("OceanSteamBurst"), OceanSteamBurst);
	LoadTrapStructValue(ConfigFilename, TEXT("OceanUrchinPatch"), OceanUrchinPatch);

	LoadTrapStructValue(ConfigFilename, TEXT("MartianShardVolley"), MartianShardVolley);
	LoadTrapStructValue(ConfigFilename, TEXT("MartianPlasmaBurst"), MartianPlasmaBurst);
	LoadTrapStructValue(ConfigFilename, TEXT("MartianCrystalPatch"), MartianCrystalPatch);

	LoadTrapStructValue(ConfigFilename, TEXT("HellSoulBoltVolley"), HellSoulBoltVolley);
	LoadTrapStructValue(ConfigFilename, TEXT("HellEmberBurst"), HellEmberBurst);
	LoadTrapStructValue(ConfigFilename, TEXT("HellBrimstonePatch"), HellBrimstonePatch);
}

const FT66TrapFloorSpawnTuning* UT66TrapTuningConfig::FindFloorSpawnTuning(const int32 TowerFloorNumber) const
{
	switch (TowerFloorNumber)
	{
	case 2: return &TowerFloor2;
	case 3: return &TowerFloor3;
	case 4: return &TowerFloor4;
	default: return nullptr;
	}
}

const FT66WallProjectileTrapTuning* UT66TrapTuningConfig::FindWallProjectileTuning(const FName RegistryKey) const
{
	if (RegistryKey == TEXT("DungeonWallArrow")) return &DungeonWallArrow;
	if (RegistryKey == TEXT("ForestThornVolley")) return &ForestThornVolley;
	if (RegistryKey == TEXT("OceanHarpoonVolley")) return &OceanHarpoonVolley;
	if (RegistryKey == TEXT("MartianShardVolley")) return &MartianShardVolley;
	if (RegistryKey == TEXT("HellSoulBoltVolley")) return &HellSoulBoltVolley;
	return nullptr;
}

const FT66FloorBurstTrapTuning* UT66TrapTuningConfig::FindFloorBurstTuning(const FName RegistryKey) const
{
	if (RegistryKey == TEXT("DungeonFloorFlame")) return &DungeonFloorFlame;
	if (RegistryKey == TEXT("ForestSporeBurst")) return &ForestSporeBurst;
	if (RegistryKey == TEXT("OceanSteamBurst")) return &OceanSteamBurst;
	if (RegistryKey == TEXT("MartianPlasmaBurst")) return &MartianPlasmaBurst;
	if (RegistryKey == TEXT("HellEmberBurst")) return &HellEmberBurst;
	return nullptr;
}

const FT66AreaControlTrapTuning* UT66TrapTuningConfig::FindAreaControlTuning(const FName RegistryKey) const
{
	if (RegistryKey == TEXT("DungeonFloorSpikePatch")) return &DungeonFloorSpikePatch;
	if (RegistryKey == TEXT("ForestBramblePatch")) return &ForestBramblePatch;
	if (RegistryKey == TEXT("OceanUrchinPatch")) return &OceanUrchinPatch;
	if (RegistryKey == TEXT("MartianCrystalPatch")) return &MartianCrystalPatch;
	if (RegistryKey == TEXT("HellBrimstonePatch")) return &HellBrimstonePatch;
	return nullptr;
}

const FT66TrapVisualAssetConfig& UT66TrapTuningConfig::GetRuntimeTrapAssets()
{
	static UT66TrapTuningConfig RuntimeConfig;
	static bool bLoaded = false;

	if (!bLoaded)
	{
		RuntimeConfig.LoadFromConfig();
		QueueConfiguredTrapAssetPreloads(RuntimeConfig.TrapAssets);
		bLoaded = true;
	}

	return RuntimeConfig.TrapAssets;
}

UStaticMesh* UT66TrapTuningConfig::LoadConfiguredTrapStaticMesh(const FString& ObjectPathString, const TCHAR* ConfigKey)
{
	return LoadConfiguredTrapAsset<UStaticMesh>(ObjectPathString, ConfigKey, TEXT("UStaticMesh"));
}

UNiagaraSystem* UT66TrapTuningConfig::LoadConfiguredTrapNiagaraSystem(const FString& ObjectPathString, const TCHAR* ConfigKey)
{
	return LoadConfiguredTrapAsset<UNiagaraSystem>(ObjectPathString, ConfigKey, TEXT("UNiagaraSystem"));
}
