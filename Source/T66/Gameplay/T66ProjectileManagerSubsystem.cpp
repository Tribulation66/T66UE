// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ProjectileManagerSubsystem.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/Enemies/T66EnemyFamilyTypes.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66CombatShared.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66ProjectileManager, Log, All);

namespace
{
constexpr float T66ProjectileHiddenScale = 0.001f;
constexpr float T66ProjectileMuzzleClearMargin = 8.f;

double CyclesToMicroseconds(const uint64 Cycles)
{
	return FPlatformTime::ToMilliseconds64(Cycles) * 1000.0;
}

uint64 GManagedBossProjectileVFXBudgetFrame = MAX_uint64;
int32 GManagedBossProjectileVFXEmittedThisFrame = 0;

bool IsLightweightRangedSource(const AActor* SourceMob)
{
	const AT66MobBase* Mob = Cast<AT66MobBase>(SourceMob);
	return Mob && Mob->GetEnemyFamily() == ET66EnemyFamily::Ranged;
}

bool IsEnemyProjectilePeerBody(const AActor* Actor)
{
	return Cast<AT66EnemyBase>(Actor) || Cast<AT66MobBase>(Actor) || Cast<AT66BossBase>(Actor);
}

FColor QuantizeBossProjectileColor(const FLinearColor& Color)
{
	return Color.ToFColor(true);
}

float GetBossProjectileEffectsQualityScale()
{
	static IConsoleVariable* UseScalabilityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.VFX.BossProjectileUseEffectsScalability"));
	if (UseScalabilityCVar && UseScalabilityCVar->GetInt() == 0)
	{
		return 1.0f;
	}

	static IConsoleVariable* EffectsQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
	const int32 EffectsQuality = EffectsQualityCVar ? FMath::Clamp(EffectsQualityCVar->GetInt(), 0, 4) : 3;
	switch (EffectsQuality)
	{
	case 0:
		return 0.35f;
	case 1:
		return 0.55f;
	case 2:
		return 0.75f;
	case 4:
		return 1.15f;
	case 3:
	default:
		return 1.0f;
	}
}

bool TryConsumeManagedBossProjectileVFXBudget()
{
	if (GManagedBossProjectileVFXBudgetFrame != GFrameCounter)
	{
		GManagedBossProjectileVFXBudgetFrame = GFrameCounter;
		GManagedBossProjectileVFXEmittedThisFrame = 0;
	}

	static IConsoleVariable* BudgetCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.VFX.BossProjectileMaxPerFrame"));
	const int32 BaseBudget = BudgetCVar ? BudgetCVar->GetInt() : 28;
	if (BaseBudget <= 0)
	{
		++GManagedBossProjectileVFXEmittedThisFrame;
		return true;
	}

	const int32 ScaledBudget = FMath::Max(4, FMath::RoundToInt(static_cast<float>(BaseBudget) * GetBossProjectileEffectsQualityScale()));
	if (GManagedBossProjectileVFXEmittedThisFrame >= ScaledBudget)
	{
		return false;
	}

	++GManagedBossProjectileVFXEmittedThisFrame;
	return true;
}

const FName T66ManagedProjectileProfile_EnemySpit(TEXT("ManagedProjectile.EnemySpit"));
const FName T66ManagedProjectileProfile_EnemyWeb(TEXT("ManagedProjectile.EnemyWeb"));
const FName T66ManagedProjectileProfile_EnemyWebNiagara(TEXT("ManagedProjectile.EnemyWeb.NiagaraBody"));
const FName T66ManagedProjectileProfile_BossWebNeedle(TEXT("ManagedProjectile.Boss.WebNeedle"));

TMap<FString, TWeakObjectPtr<UNiagaraSystem>>& GetManagedBossProjectileSystemCache()
{
	static TMap<FString, TWeakObjectPtr<UNiagaraSystem>> Cache;
	return Cache;
}

TMap<FString, TSharedPtr<FStreamableHandle>>& GetManagedBossProjectileActiveLoads()
{
	static TMap<FString, TSharedPtr<FStreamableHandle>> ActiveLoads;
	return ActiveLoads;
}

#if !UE_BUILD_SHIPPING
void GetManagedBossProjectileAsyncDebug(
	int32& OutCacheEntries,
	int32& OutValidCachedSystems,
	int32& OutActiveLoadEntries,
	int32& OutValidLoadHandles,
	int32& OutLoadingLoadHandles)
{
	const TMap<FString, TWeakObjectPtr<UNiagaraSystem>>& Cache = GetManagedBossProjectileSystemCache();
	const TMap<FString, TSharedPtr<FStreamableHandle>>& ActiveLoads = GetManagedBossProjectileActiveLoads();

	OutCacheEntries = Cache.Num();
	OutValidCachedSystems = 0;
	for (const TPair<FString, TWeakObjectPtr<UNiagaraSystem>>& Pair : Cache)
	{
		if (Pair.Value.IsValid())
		{
			++OutValidCachedSystems;
		}
	}

	OutActiveLoadEntries = ActiveLoads.Num();
	OutValidLoadHandles = 0;
	OutLoadingLoadHandles = 0;
	for (const TPair<FString, TSharedPtr<FStreamableHandle>>& Pair : ActiveLoads)
	{
		if (Pair.Value.IsValid())
		{
			++OutValidLoadHandles;
			if (!Pair.Value->HasLoadCompleted())
			{
				++OutLoadingLoadHandles;
			}
		}
	}
}
#endif

struct FT66ManagedProjectileVisualProfileSpec
{
	FName ProfileID = NAME_None;
	ET66ManagedProjectileDelivery Delivery = ET66ManagedProjectileDelivery::EnemyProjectile;
	ET66TemporaryProjectileShape Shape = ET66TemporaryProjectileShape::Sphere;
	FLinearColor Color = FLinearColor::White;
	FVector VisualScale = FVector(1.f);
	FQuat RotationOffset = FQuat::Identity;
	ET66ManagedProjectileVisualBodyMode BodyMode = ET66ManagedProjectileVisualBodyMode::HISM;
	const TCHAR* BodySystemPath = nullptr;
	const TCHAR* TrailSystemPath = nullptr;
	const TCHAR* ImpactSystemPath = nullptr;
};

UNiagaraSystem* ResolveManagedBossProjectileSystem(const TCHAR* AssetPath)
{
	if (!AssetPath || !*AssetPath)
	{
		return nullptr;
	}

	TMap<FString, TWeakObjectPtr<UNiagaraSystem>>& Cache = GetManagedBossProjectileSystemCache();
	TMap<FString, TSharedPtr<FStreamableHandle>>& ActiveLoads = GetManagedBossProjectileActiveLoads();
	const FSoftObjectPath Path(AssetPath);
	const FString Key = Path.ToString();
	if (const TWeakObjectPtr<UNiagaraSystem>* Found = Cache.Find(Key))
	{
		if (Found->IsValid())
		{
			return Found->Get();
		}
	}

	if (UNiagaraSystem* Resolved = Cast<UNiagaraSystem>(Path.ResolveObject()))
	{
		Cache.Add(Key, Resolved);
		return Resolved;
	}

	if (!ActiveLoads.Contains(Key))
	{
		TArray<FSoftObjectPath> AssetPaths;
		AssetPaths.Add(Path);
		ActiveLoads.Add(Key, UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetPaths, FStreamableDelegate()));
	}

	return nullptr;
}

UStaticMesh* ResolveManagedProjectileShapeMesh(const ET66TemporaryProjectileShape Shape)
{
	switch (Shape)
	{
	case ET66TemporaryProjectileShape::Cone:
		return FT66VisualUtil::GetBasicShapeCone();
	case ET66TemporaryProjectileShape::Cylinder:
		return FT66VisualUtil::GetBasicShapeCylinder();
	case ET66TemporaryProjectileShape::Cube:
		return FT66VisualUtil::GetBasicShapeCube();
	case ET66TemporaryProjectileShape::Sphere:
	default:
		return FT66VisualUtil::GetBasicShapeSphere();
	}
}

UStaticMesh* GetManagedBossProjectileMesh(ET66BossAttackProfile AttackProfile);
FVector GetManagedBossProjectileScale(ET66BossAttackProfile AttackProfile);
FQuat GetManagedBossProjectileMeshRotationOffset(ET66BossAttackProfile AttackProfile);
const TCHAR* GetManagedBossProjectileTrailPath(ET66BossAttackProfile AttackProfile);
const TCHAR* GetManagedBossProjectileImpactPath(ET66BossAttackProfile AttackProfile);

const FT66ManagedProjectileVisualProfileSpec* FindManagedProjectileVisualProfile(const FName ProfileID)
{
	static const FT66ManagedProjectileVisualProfileSpec Profiles[] =
	{
		{
			T66ManagedProjectileProfile_EnemySpit,
			ET66ManagedProjectileDelivery::EnemyProjectile,
			ET66TemporaryProjectileShape::Sphere,
			FT66TemporaryProjectileSystem::HostileProjectileColor(),
			FVector(1.10f),
			FQuat::Identity,
			ET66ManagedProjectileVisualBodyMode::HISM,
			nullptr,
			nullptr,
			nullptr
		},
		{
			T66ManagedProjectileProfile_EnemyWeb,
			ET66ManagedProjectileDelivery::EnemyProjectile,
			ET66TemporaryProjectileShape::Cylinder,
			FLinearColor(0.86f, 0.91f, 0.98f, 1.f),
			FVector(0.80f, 0.80f, 0.22f),
			FRotator(90.f, 0.f, 0.f).Quaternion(),
			ET66ManagedProjectileVisualBodyMode::HISM,
			nullptr,
			nullptr,
			nullptr
		},
		{
			T66ManagedProjectileProfile_EnemyWebNiagara,
			ET66ManagedProjectileDelivery::EnemyProjectile,
			ET66TemporaryProjectileShape::Sphere,
			FLinearColor(0.74f, 0.84f, 1.f, 1.f),
			FVector(0.72f),
			FQuat::Identity,
			ET66ManagedProjectileVisualBodyMode::Niagara,
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_02.P_Cosmic_Projectile_02"),
			nullptr,
			nullptr
		},
		{
			T66ManagedProjectileProfile_BossWebNeedle,
			ET66ManagedProjectileDelivery::BossProjectile,
			ET66TemporaryProjectileShape::Cone,
			FLinearColor(0.78f, 0.68f, 1.0f, 1.f),
			FVector(0.22f, 0.22f, 0.92f),
			FRotator(-90.f, 0.f, 0.f).Quaternion(),
			ET66ManagedProjectileVisualBodyMode::HISM,
			nullptr,
			TEXT("/Game/Stylized_VFX_StPack/Particles/P_Laser_02.P_Laser_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Portal.P_Cosmic_Portal")
		},
	};

	for (const FT66ManagedProjectileVisualProfileSpec& Profile : Profiles)
	{
		if (Profile.ProfileID == ProfileID)
		{
			return &Profile;
		}
	}

	return nullptr;
}

FT66ProjectileVisualBucket MakeManagedProjectileBucketFromProfile(const FT66ManagedProjectileVisualProfileSpec& Profile)
{
	FT66ProjectileVisualBucket Bucket;
	Bucket.VisualProfileID = Profile.ProfileID;
	Bucket.Color = Profile.Color.ToFColor(true);
	Bucket.VisualScale = Profile.BodyMode == ET66ManagedProjectileVisualBodyMode::Niagara
		? FVector(T66ProjectileHiddenScale)
		: Profile.VisualScale;
	Bucket.RotationOffset = Profile.RotationOffset;
	Bucket.BodyMode = Profile.BodyMode;
	Bucket.BodyMesh = ResolveManagedProjectileShapeMesh(Profile.Shape);
	Bucket.BodySystem = ResolveManagedBossProjectileSystem(Profile.BodySystemPath);
	Bucket.TrailSystem = ResolveManagedBossProjectileSystem(Profile.TrailSystemPath);
	Bucket.ImpactSystem = ResolveManagedBossProjectileSystem(Profile.ImpactSystemPath);
	return Bucket;
}

FT66ProjectileVisualBucket MakeDefaultEnemySpitBucket()
{
	if (const FT66ManagedProjectileVisualProfileSpec* Profile = FindManagedProjectileVisualProfile(T66ManagedProjectileProfile_EnemySpit))
	{
		return MakeManagedProjectileBucketFromProfile(*Profile);
	}

	FT66ProjectileVisualBucket Bucket;
	Bucket.VisualProfileID = T66ManagedProjectileProfile_EnemySpit;
	Bucket.Color = FT66TemporaryProjectileSystem::HostileProjectileColor().ToFColor(true);
	Bucket.VisualScale = FVector(1.10f);
	Bucket.BodyMesh = FT66VisualUtil::GetBasicShapeSphere();
	return Bucket;
}

FT66ProjectileVisualBucket MakeLegacyBossProjectileBucket(
	const ET66BossAttackProfile AttackProfile,
	const FLinearColor& Color,
	const bool bOverflowBucket)
{
	FT66ProjectileVisualBucket Bucket;
	Bucket.VisualProfileID = FName(*FString::Printf(TEXT("ManagedProjectile.Boss.Legacy.%d.%s"),
		static_cast<int32>(AttackProfile),
		bOverflowBucket ? TEXT("Overflow") : TEXT("Exact")));
	Bucket.AttackProfile = AttackProfile;
	Bucket.Color = Color.ToFColor(true);
	Bucket.VisualScale = GetManagedBossProjectileScale(AttackProfile);
	Bucket.RotationOffset = GetManagedBossProjectileMeshRotationOffset(AttackProfile);
	Bucket.BodyMode = ET66ManagedProjectileVisualBodyMode::HISM;
	Bucket.BodyMesh = GetManagedBossProjectileMesh(AttackProfile);
	Bucket.TrailSystem = ResolveManagedBossProjectileSystem(GetManagedBossProjectileTrailPath(AttackProfile));
	Bucket.ImpactSystem = ResolveManagedBossProjectileSystem(GetManagedBossProjectileImpactPath(AttackProfile));
	Bucket.bOverflowBucket = bOverflowBucket;
	return Bucket;
}

const TCHAR* GetManagedBossProjectileTrailPath(const ET66BossAttackProfile AttackProfile)
{
	switch (AttackProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/P_Laser_02.P_Laser_02");
	case ET66BossAttackProfile::Juggernaut:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire");
	case ET66BossAttackProfile::Duelist:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_02.P_Cosmic_Projectile_02");
	case ET66BossAttackProfile::Gambler:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_03.P_Cosmic_Projectile_03");
	case ET66BossAttackProfile::Balanced:
	default:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_01.P_Weapon_01");
	}
}

const TCHAR* GetManagedBossProjectileImpactPath(const ET66BossAttackProfile AttackProfile)
{
	switch (AttackProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/P_Laser_02.P_Laser_02");
	case ET66BossAttackProfile::Juggernaut:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02");
	case ET66BossAttackProfile::Duelist:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_02.P_Cosmic_Projectile_02");
	case ET66BossAttackProfile::Gambler:
		return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Portal.P_Cosmic_Portal");
	case ET66BossAttackProfile::Balanced:
	default:
		return TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1");
	}
}

void PreloadManagedBossProjectileSystemsAsync()
{
	static bool bRequested = false;
	if (bRequested)
	{
		return;
	}
	bRequested = true;

	static const TCHAR* Paths[] =
	{
		TEXT("/Game/Stylized_VFX_StPack/Particles/P_Laser_02.P_Laser_02"),
		TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire"),
		TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_02.P_Cosmic_Projectile_02"),
		TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_03.P_Cosmic_Projectile_03"),
		TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_01.P_Weapon_01"),
		TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02"),
		TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Portal.P_Cosmic_Portal"),
		TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"),
	};

	for (const TCHAR* Path : Paths)
	{
		ResolveManagedBossProjectileSystem(Path);
	}
}

UStaticMesh* GetManagedBossProjectileMesh(const ET66BossAttackProfile AttackProfile)
{
	switch (AttackProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
	case ET66BossAttackProfile::Duelist:
		return FT66VisualUtil::GetBasicShapeCone();
	case ET66BossAttackProfile::Juggernaut:
		return FT66VisualUtil::GetBasicShapeCylinder();
	case ET66BossAttackProfile::Balanced:
	case ET66BossAttackProfile::Gambler:
	default:
		return FT66VisualUtil::GetBasicShapeSphere();
	}
}

FVector GetManagedBossProjectileScale(const ET66BossAttackProfile AttackProfile)
{
	switch (AttackProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
		return FVector(0.22f, 0.22f, 0.60f);
	case ET66BossAttackProfile::Juggernaut:
		return FVector(0.28f, 0.28f, 0.45f);
	case ET66BossAttackProfile::Duelist:
		return FVector(0.18f, 0.18f, 0.56f);
	case ET66BossAttackProfile::Gambler:
		return FVector(0.26f);
	case ET66BossAttackProfile::Balanced:
	default:
		return FVector(0.22f);
	}
}

FQuat GetManagedBossProjectileMeshRotationOffset(const ET66BossAttackProfile AttackProfile)
{
	if (AttackProfile == ET66BossAttackProfile::Sharpshooter || AttackProfile == ET66BossAttackProfile::Duelist)
	{
		return FRotator(-90.f, 0.f, 0.f).Quaternion();
	}
	return FQuat::Identity;
}

FLinearColor GetBossOverflowColor(const ET66BossAttackProfile AttackProfile)
{
	switch (AttackProfile)
	{
	case ET66BossAttackProfile::Sharpshooter:
		return FLinearColor(0.82f, 0.78f, 0.58f, 1.f);
	case ET66BossAttackProfile::Juggernaut:
		return FLinearColor(0.90f, 0.12f, 0.08f, 1.f);
	case ET66BossAttackProfile::Duelist:
		return FLinearColor(0.30f, 0.44f, 1.0f, 1.f);
	case ET66BossAttackProfile::Gambler:
		return FLinearColor(0.76f, 0.16f, 0.68f, 1.f);
	case ET66BossAttackProfile::Balanced:
	default:
		return FLinearColor(0.95f, 0.16f, 0.12f, 1.f);
	}
}

float SegmentSegmentDistanceSquared(
	const FVector& P1,
	const FVector& Q1,
	const FVector& P2,
	const FVector& Q2,
	float& OutS)
{
	const FVector D1 = Q1 - P1;
	const FVector D2 = Q2 - P2;
	const FVector R = P1 - P2;
	const float A = FVector::DotProduct(D1, D1);
	const float E = FVector::DotProduct(D2, D2);
	const float F = FVector::DotProduct(D2, R);

	float S = 0.f;
	float T = 0.f;
	if (A <= SMALL_NUMBER && E <= SMALL_NUMBER)
	{
		OutS = 0.f;
		return FVector::DistSquared(P1, P2);
	}
	if (A <= SMALL_NUMBER)
	{
		S = 0.f;
		T = FMath::Clamp(F / E, 0.f, 1.f);
	}
	else
	{
		const float C = FVector::DotProduct(D1, R);
		if (E <= SMALL_NUMBER)
		{
			T = 0.f;
			S = FMath::Clamp(-C / A, 0.f, 1.f);
		}
		else
		{
			const float B = FVector::DotProduct(D1, D2);
			const float Denom = A * E - B * B;
			if (!FMath::IsNearlyZero(Denom))
			{
				S = FMath::Clamp((B * F - C * E) / Denom, 0.f, 1.f);
			}
			else
			{
				S = 0.f;
			}

			T = (B * S + F) / E;
			if (T < 0.f)
			{
				T = 0.f;
				S = FMath::Clamp(-C / A, 0.f, 1.f);
			}
			else if (T > 1.f)
			{
				T = 1.f;
				S = FMath::Clamp((B - C) / A, 0.f, 1.f);
			}
		}
	}

	OutS = S;
	const FVector C1 = P1 + D1 * S;
	const FVector C2 = P2 + D2 * T;
	return FVector::DistSquared(C1, C2);
}
}

void FT66ProjectileManagerDiagnostics::Reset(const FString& Reason)
{
	const int32 NextResetCount = ResetCount + 1;
	*this = FT66ProjectileManagerDiagnostics{};
	ResetCount = NextResetCount;
	LastResetReason = Reason;
}

double FT66ProjectileManagerDiagnostics::GetManagerTickAvgUs() const
{
	return ManagerTickSamples > 0 ? ManagerTickTotalUs / static_cast<double>(ManagerTickSamples) : 0.0;
}

double FT66ProjectileManagerDiagnostics::GetHISMUpdateAvgUs() const
{
	return HISMUpdateSamples > 0 ? HISMUpdateTotalUs / static_cast<double>(HISMUpdateSamples) : 0.0;
}

FName UT66ProjectileManagerSubsystem::DefaultEnemySpitVisualProfileID()
{
	return T66ManagedProjectileProfile_EnemySpit;
}

FName UT66ProjectileManagerSubsystem::EnemyWebVisualProfileID()
{
	return T66ManagedProjectileProfile_EnemyWeb;
}

FName UT66ProjectileManagerSubsystem::EnemyWebNiagaraVisualProfileID()
{
	return T66ManagedProjectileProfile_EnemyWebNiagara;
}

FName UT66ProjectileManagerSubsystem::BossWebNeedleVisualProfileID()
{
	return T66ManagedProjectileProfile_BossWebNeedle;
}

void UT66ProjectileManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Projectiles.SetNum(MaxProjectiles);
	Diagnostics.Reset(TEXT("Initialize"));
	bInitialized = true;
	bShuttingDown = false;
	PreloadManagedBossProjectileSystemsAsync();
	UE_LOG(LogT66ProjectileManager, VeryVerbose, TEXT("UT66ProjectileManagerSubsystem initialized world=%s"), *GetNameSafe(GetWorld()));
}

void UT66ProjectileManagerSubsystem::Deinitialize()
{
	bShuttingDown = true;
	for (FT66ManagedProjectile& Projectile : Projectiles)
	{
		CleanupProjectileTrail(Projectile);
	}
	for (TObjectPtr<UHierarchicalInstancedStaticMeshComponent>& Component : ProjectileComponents)
	{
		if (Component)
		{
			Component->DestroyComponent();
			Component = nullptr;
		}
	}
	ProjectileComponents.Reset();
	ProjectileVisualBuckets.Reset();
	ManagedVisualBucketByProfileID.Reset();
	BossVisualBucketByKey.Reset();
	BossOverflowBucketByProfile.Reset();
	VisualBucketWarningsEmitted.Reset();
	if (RenderHost)
	{
		RenderHost->Destroy();
		RenderHost = nullptr;
	}
	RenderRoot = nullptr;
	bProjectileInstancesDirty = false;
	Projectiles.Reset();
	ActiveProjectileCount = 0;
	bInitialized = false;
	Super::Deinitialize();
}

#if !UE_BUILD_SHIPPING
FT66WorldRuntimeDebugSnapshot UT66ProjectileManagerSubsystem::GetWorldRuntimeDebugSnapshot() const
{
	FT66WorldRuntimeDebugSnapshot Snapshot;
	Snapshot.SystemName = TEXT("UT66ProjectileManagerSubsystem");

	int32 ValidRenderComponents = 0;
	int32 TotalRenderInstances = 0;
	for (const TObjectPtr<UHierarchicalInstancedStaticMeshComponent>& Component : ProjectileComponents)
	{
		if (Component)
		{
			++ValidRenderComponents;
			TotalRenderInstances += Component->GetInstanceCount();
		}
	}

	int32 ActiveSlots = 0;
	int32 ValidSourceRefs = 0;
	int32 ValidBodyComponents = 0;
	int32 ValidTrailComponents = 0;
	for (const FT66ManagedProjectile& Projectile : Projectiles)
	{
		if (!Projectile.bIsActive)
		{
			continue;
		}
		++ActiveSlots;
		if (Projectile.SourceMob.IsValid())
		{
			++ValidSourceRefs;
		}
		if (Projectile.BodyComponent.IsValid())
		{
			++ValidBodyComponents;
		}
		if (Projectile.TrailComponent.IsValid())
		{
			++ValidTrailComponents;
		}
	}

	int32 CacheEntries = 0;
	int32 ValidCachedSystems = 0;
	int32 ActiveLoadEntries = 0;
	int32 ValidLoadHandles = 0;
	int32 LoadingLoadHandles = 0;
	GetManagedBossProjectileAsyncDebug(CacheEntries, ValidCachedSystems, ActiveLoadEntries, ValidLoadHandles, LoadingLoadHandles);

	Snapshot.AddCounter(TEXT("active_projectile_count"), ActiveProjectileCount);
	Snapshot.AddCounter(TEXT("active_projectile_slots"), ActiveSlots);
	Snapshot.AddCounter(TEXT("projectile_slot_capacity"), Projectiles.Num());
	Snapshot.AddCounter(TEXT("valid_source_refs"), ValidSourceRefs);
	Snapshot.AddCounter(TEXT("valid_body_components"), ValidBodyComponents);
	Snapshot.AddCounter(TEXT("valid_trail_components"), ValidTrailComponents);
	Snapshot.AddCounter(TEXT("render_component_slots"), ProjectileComponents.Num());
	Snapshot.AddCounter(TEXT("valid_render_components"), ValidRenderComponents);
	Snapshot.AddCounter(TEXT("render_instance_count"), TotalRenderInstances);
	Snapshot.AddCounter(TEXT("visual_bucket_count"), ProjectileVisualBuckets.Num());
	Snapshot.AddCounter(TEXT("managed_visual_bucket_key_count"), ManagedVisualBucketByProfileID.Num());
	Snapshot.AddCounter(TEXT("boss_visual_bucket_key_count"), BossVisualBucketByKey.Num());
	Snapshot.AddCounter(TEXT("boss_overflow_bucket_key_count"), BossOverflowBucketByProfile.Num());
	Snapshot.AddCounter(TEXT("async_cache_entries"), CacheEntries);
	Snapshot.AddCounter(TEXT("async_cached_systems_valid"), ValidCachedSystems);
	Snapshot.AddCounter(TEXT("async_load_entries"), ActiveLoadEntries);
	Snapshot.AddCounter(TEXT("async_load_handles_valid"), ValidLoadHandles);
	Snapshot.AddCounter(TEXT("async_load_handles_loading"), LoadingLoadHandles);
	Snapshot.AddCounter(TEXT("known_timer_handles"), 0);
	Snapshot.AddCounter(TEXT("known_external_delegate_handles"), 0);
	Snapshot.AddFlag(TEXT("initialized"), bInitialized);
	Snapshot.AddFlag(TEXT("shutting_down"), bShuttingDown);
	Snapshot.AddFlag(TEXT("projectile_instances_dirty"), bProjectileInstancesDirty);
	Snapshot.AddFlag(TEXT("render_host_valid"), RenderHost != nullptr);
	Snapshot.AddFlag(TEXT("render_root_valid"), RenderRoot != nullptr);
	Snapshot.AddEvidence(TEXT("timers"), TEXT("No stored timer handles found; projectile lifetime is tick-driven."));
	Snapshot.AddEvidence(TEXT("delegates"), TEXT("No external delegate handle is stored by this subsystem."));
	Snapshot.AddEvidence(TEXT("async_loads"), TEXT("Managed boss projectile Niagara systems use the static streamable cache counted in async_* fields."));
	return Snapshot;
}
#endif

bool UT66ProjectileManagerSubsystem::FireProjectile(
	AActor* SourceMob,
	const FName SourceMobID,
	const FVector Origin,
	FVector Direction,
	const float Speed,
	const float Damage,
	const float Radius,
	const float Lifetime,
	const int32 ProjectileTypeIndex)
{
	FT66ManagedProjectileFireParams Params;
	Params.SourceActor = SourceMob;
	Params.SourceID = SourceMobID;
	Params.Origin = Origin;
	Params.Direction = Direction;
	Params.Speed = Speed;
	Params.Damage = Damage;
	Params.Radius = Radius;
	Params.Lifetime = Lifetime;
	Params.ProjectileTypeIndex = ProjectileTypeIndex;
	Params.Delivery = ET66ManagedProjectileDelivery::EnemyProjectile;
	Params.VisualProfileID = DefaultEnemySpitVisualProfileID();
	return AllocateProjectile(Params);
}

bool UT66ProjectileManagerSubsystem::FireManagedProjectile(const FT66ManagedProjectileFireParams& Params)
{
	FT66ManagedProjectileFireParams ManagedParams = Params;
	if (ManagedParams.Delivery == ET66ManagedProjectileDelivery::EnemyProjectile && ManagedParams.VisualProfileID.IsNone())
	{
		ManagedParams.VisualProfileID = DefaultEnemySpitVisualProfileID();
	}
	return AllocateProjectile(ManagedParams);
}

bool UT66ProjectileManagerSubsystem::FireBossProjectile(const FT66ManagedProjectileFireParams& Params)
{
	FT66ManagedProjectileFireParams BossParams = Params;
	BossParams.Delivery = ET66ManagedProjectileDelivery::BossProjectile;
	BossParams.Radius = 24.f * FMath::Max(1.f, BossParams.BossVisualScaleMultiplier);
	BossParams.Lifetime = 6.f;
	return AllocateProjectile(BossParams);
}

bool UT66ProjectileManagerSubsystem::AllocateProjectile(FT66ManagedProjectileFireParams Params)
{
	if (!IsInGameThread())
	{
		ensureMsgf(false, TEXT("UT66ProjectileManagerSubsystem projectile fire must run on the game thread."));
		return false;
	}
	if (!bInitialized || bShuttingDown || !Params.SourceActor || !Params.Direction.Normalize() || Params.Speed <= 0.f || Params.Lifetime <= 0.f)
	{
		++Diagnostics.DroppedFires;
		return false;
	}
	if (!EnsureRenderResources(Params.Origin))
	{
		++Diagnostics.DroppedFires;
		return false;
	}
	if (Params.Delivery == ET66ManagedProjectileDelivery::BossProjectile)
	{
		Params.ProjectileTypeIndex = Params.VisualProfileID.IsNone()
			? ResolveBossProjectileTypeIndex(Params)
			: ResolveManagedProjectileVisualTypeIndex(Params);
	}
	else if (!Params.VisualProfileID.IsNone())
	{
		Params.ProjectileTypeIndex = ResolveManagedProjectileVisualTypeIndex(Params);
	}

	UHierarchicalInstancedStaticMeshComponent* Component = GetProjectileComponent(Params.ProjectileTypeIndex);
	if (!Component)
	{
		++Diagnostics.DroppedFires;
		return false;
	}

	const int32 SlotIndex = FindFreeProjectileSlot();
	if (SlotIndex == INDEX_NONE)
	{
		++Diagnostics.DroppedFires;
		return false;
	}

	FT66ManagedProjectile& Projectile = Projectiles[SlotIndex];
	Projectile = FT66ManagedProjectile{};
	Projectile.bIsActive = true;
	Projectile.Position = Params.Origin;
	Projectile.PreviousPosition = Params.Origin;
	Projectile.Velocity = Params.Direction * Params.Speed;
	Projectile.LifetimeRemaining = Params.Lifetime;
	Projectile.Radius = FMath::Max(1.f, Params.Radius);
	Projectile.Damage = FMath::Max(0, FMath::RoundToInt(Params.Damage));
	Projectile.SourceMob = Params.SourceActor;
	Projectile.SourceMobID = Params.SourceID;
	Projectile.bSourceWasLightweight = IsLightweightRangedSource(Params.SourceActor);
	Projectile.Delivery = Params.Delivery;
	Projectile.BossAttackProfile = Params.BossAttackProfile;
	Projectile.BossPrimaryColor = Params.BossPrimaryColor;
	Projectile.BossSecondaryColor = Params.BossSecondaryColor;
	Projectile.bUseBossSecondaryTint = Params.bUseBossSecondaryTint;
	Projectile.BossVisualScaleMultiplier = FMath::Clamp(Params.BossVisualScaleMultiplier, 0.35f, 5.f);
	if (ProjectileVisualBuckets.IsValidIndex(Params.ProjectileTypeIndex))
	{
		const FT66ProjectileVisualBucket& Bucket = ProjectileVisualBuckets[Params.ProjectileTypeIndex];
		Projectile.VisualProfileID = Bucket.VisualProfileID;
		Projectile.VisualScale = Bucket.VisualScale * (Params.Delivery == ET66ManagedProjectileDelivery::BossProjectile ? Projectile.BossVisualScaleMultiplier : 1.f);
		Projectile.VisualRotationOffset = Bucket.RotationOffset;
		Projectile.VisualBodyMode = Bucket.BodyMode;
		Projectile.TrailSystem = Bucket.TrailSystem.Get();
		Projectile.ImpactSystem = Bucket.ImpactSystem.Get();
	}
	else
	{
		const FT66TemporaryProjectileVisualSpec Spec = FT66TemporaryProjectileSystem::MakeSpec(
			FT66TemporaryProjectileSystem::ProfileEnemySpit(),
			FT66TemporaryProjectileSystem::HostileProjectileColor());
		Projectile.VisualProfileID = DefaultEnemySpitVisualProfileID();
		Projectile.VisualScale = FVector(FMath::Max(0.01f, Spec.RelativeScale.GetMax()));
	}
	Projectile.ProjectileTypeIndex = Params.ProjectileTypeIndex;
	Projectile.InstanceIndex = SlotIndex;
	Projectile.SourceIgnoreDistance = ResolveSourceIgnoreDistance(Params.SourceActor, Projectile.Radius);

	++ActiveProjectileCount;
	++Diagnostics.ProjectilesFired;
	Diagnostics.ProjectilesActivePeak = FMath::Max(Diagnostics.ProjectilesActivePeak, ActiveProjectileCount);
	UpdateProjectileInstance(Projectile);
	SpawnProjectileBody(Projectile);
	SpawnBossTrail(Projectile);
	return true;
}

void UT66ProjectileManagerSubsystem::ResetProjectileDiagnostics(const TCHAR* Reason)
{
	Diagnostics.Reset(Reason ? FString(Reason) : FString(TEXT("Unknown")));
	bTerminalSummaryEmitted = false;
	for (int32 Index = 0; Index < Projectiles.Num(); ++Index)
	{
		if (Projectiles[Index].bIsActive)
		{
			CleanupProjectileTrail(Projectiles[Index]);
			Projectiles[Index].bIsActive = false;
			HideProjectileInstance(Projectiles[Index]);
		}
	}
	ActiveProjectileCount = 0;
	FlushProjectileInstanceUpdates();
}

void UT66ProjectileManagerSubsystem::EmitProjectileManagerSummary(const TCHAR* Reason, const bool bTerminal)
{
	if (bTerminal && bTerminalSummaryEmitted)
	{
		return;
	}
	if (bTerminal)
	{
		bTerminalSummaryEmitted = true;
	}

	UWorld* World = GetWorld();
	const float WorldTime = World ? World->GetTimeSeconds() : -1.f;
	FBoxSphereBounds Bounds(FSphere(FVector::ZeroVector, 0.f));
	bool bHasBounds = false;
	for (const TObjectPtr<UHierarchicalInstancedStaticMeshComponent>& Component : ProjectileComponents)
	{
		if (!Component)
		{
			continue;
		}
		if (!bHasBounds)
		{
			Bounds = Component->Bounds;
			bHasBounds = true;
		}
		else
		{
			Bounds = Bounds + Component->Bounds;
		}
	}
	UE_LOG(
		LogT66ProjectileManager,
		Log,
		TEXT("[ProjectileManagerSummary] Reason=%s Terminal=%d WorldTime=%.2f ResetCount=%d ResetReason=%s Active=%d Fired=%d ActivePeak=%d HitHero=%d Expired=%d HitWorld=%d DroppedFires=%d DroppedInvalidSource=%d VisualBucketOverflow=%d ApplyDamageReturnedFalse=%d VisualProfilesResolved=%d VisualProfileFallbacks=%d NiagaraBodiesSpawned=%d VisualBuckets=%d ManagerTickMaxUs=%.1f ManagerTickAvgUs=%.1f HISMUpdateMaxUs=%.1f HISMUpdateAvgUs=%.1f BoundsOrigin=%s BoundsExtent=%s LastWorldImpactActor=%s"),
		Reason ? Reason : TEXT("Unknown"),
		bTerminal ? 1 : 0,
		WorldTime,
		Diagnostics.ResetCount,
		*Diagnostics.LastResetReason,
		ActiveProjectileCount,
		Diagnostics.ProjectilesFired,
		Diagnostics.ProjectilesActivePeak,
		Diagnostics.ProjectilesHitHero,
		Diagnostics.ProjectilesExpired,
		Diagnostics.ProjectilesHitWorld,
		Diagnostics.DroppedFires,
		Diagnostics.DroppedInvalidSource,
		Diagnostics.VisualBucketOverflowCount,
		Diagnostics.ApplyDamageReturnedFalse,
		Diagnostics.VisualProfilesResolved,
		Diagnostics.VisualProfileFallbacks,
		Diagnostics.NiagaraBodiesSpawned,
		ProjectileVisualBuckets.Num(),
		Diagnostics.ManagerTickMaxUs,
		Diagnostics.GetManagerTickAvgUs(),
		Diagnostics.HISMUpdateMaxUs,
		Diagnostics.GetHISMUpdateAvgUs(),
		*Bounds.Origin.ToCompactString(),
		*Bounds.BoxExtent.ToCompactString(),
		*Diagnostics.LastWorldImpactActor);
}

#if !UE_BUILD_SHIPPING
bool UT66ProjectileManagerSubsystem::RunBossProjectileKillMidFlightProof()
{
	UWorld* World = GetWorld();
	AT66HeroBase* Hero = nullptr;
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			Hero = Cast<AT66HeroBase>(PC->GetPawn());
		}
	}

	UT66RunStateSubsystem* RunState = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	}

	ResetProjectileDiagnostics(TEXT("BossProjectileKillMidFlightProof"));

	bool bFireSucceeded = false;
	bool bSourceDestroyed = false;
	int32 ActiveAfterFire = 0;
	int32 FixedTicks = 0;
	int32 DroppedBeforeTick = Diagnostics.DroppedInvalidSource;
	int32 HitHeroBeforeTick = Diagnostics.ProjectilesHitHero;
	float HeroHPBeforeTick = RunState ? RunState->GetCurrentHP() : -1.0f;
	float HeroHPAfterTick = HeroHPBeforeTick;

	if (World && Hero && RunState)
	{
		const FVector HeroLocation = Hero->GetActorLocation();
		const FVector BossLocation = HeroLocation + FVector(2200.0f, 0.0f, 160.0f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
			AT66BossBase::StaticClass(),
			BossLocation,
			FRotator::ZeroRotator,
			SpawnParams);

		if (Boss)
		{
			Boss->BossID = FName(TEXT("BossProjectileKillMidFlightProof"));
			Boss->MaxHP = FMath::Max(Boss->MaxHP, 1000);
			Boss->CurrentHP = Boss->MaxHP;
			Boss->SetActorHiddenInGame(true);
			Boss->SetActorEnableCollision(false);
			Boss->SetActorTickEnabled(false);

			FT66ManagedProjectileFireParams FireParams;
			FireParams.SourceActor = Boss;
			FireParams.SourceID = Boss->BossID;
			FireParams.Origin = BossLocation + FVector(0.0f, 0.0f, 96.0f);
			FireParams.Direction = FVector::YAxisVector;
			FireParams.Speed = 720.0f;
			FireParams.Damage = 20.0f;
			FireParams.BossAttackProfile = ET66BossAttackProfile::Balanced;
			FireParams.BossPrimaryColor = FLinearColor(0.95f, 0.16f, 0.12f, 1.0f);
			FireParams.BossSecondaryColor = FLinearColor(1.0f, 0.72f, 0.18f, 1.0f);
			FireParams.bUseBossSecondaryTint = false;
			FireParams.BossVisualScaleMultiplier = 1.0f;

			bFireSucceeded = FireBossProjectile(FireParams);
			ActiveAfterFire = GetActiveProjectileCount();
			DroppedBeforeTick = Diagnostics.DroppedInvalidSource;
			HitHeroBeforeTick = Diagnostics.ProjectilesHitHero;
			HeroHPBeforeTick = RunState->GetCurrentHP();

			Boss->CurrentHP = 0;
			Boss->Destroy();
			bSourceDestroyed = true;

			for (FixedTicks = 0; FixedTicks < 4; ++FixedTicks)
			{
				Tick(1.0f / 60.0f);
			}

			HeroHPAfterTick = RunState->GetCurrentHP();
		}
	}

	const int32 DroppedDelta = Diagnostics.DroppedInvalidSource - DroppedBeforeTick;
	const int32 HitHeroAfterTick = Diagnostics.ProjectilesHitHero;
	const int32 HitHeroDelta = HitHeroAfterTick - HitHeroBeforeTick;
	const bool bPostDeathDamage = HitHeroDelta > 0 || (HeroHPAfterTick >= 0.0f && HeroHPBeforeTick >= 0.0f && HeroHPAfterTick < HeroHPBeforeTick - KINDA_SMALL_NUMBER);
	const bool bPass = bFireSucceeded
		&& bSourceDestroyed
		&& ActiveAfterFire > 0
		&& DroppedDelta >= 1
		&& !bPostDeathDamage;

	UE_LOG(
		LogT66ProjectileManager,
		Log,
		TEXT("[BossProjectileKillMidFlightProofSummary] Terminal=1 FireBossProjectile=%d SourceDestroyed=%d ActiveAfterFire=%d FixedTicks=%d DroppedInvalidSource=%d DroppedInvalidSourceDelta=%d ProjectilesHitHeroBefore=%d ProjectilesHitHeroAfter=%d PostDeathHitDelta=%d HeroHPBefore=%.1f HeroHPAfter=%.1f PostDeathDamage=%d Pass=%d"),
		bFireSucceeded ? 1 : 0,
		bSourceDestroyed ? 1 : 0,
		ActiveAfterFire,
		FixedTicks,
		Diagnostics.DroppedInvalidSource,
		DroppedDelta,
		HitHeroBeforeTick,
		HitHeroAfterTick,
		HitHeroDelta,
		HeroHPBeforeTick,
		HeroHPAfterTick,
		bPostDeathDamage ? 1 : 0,
		bPass ? 1 : 0);

	EmitProjectileManagerSummary(TEXT("BossProjectileKillMidFlightProof"), true);
	return bPass;
}

bool UT66ProjectileManagerSubsystem::RunManagedProjectileVisualProfileProof()
{
	UWorld* World = GetWorld();
	AT66HeroBase* Hero = nullptr;
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			Hero = Cast<AT66HeroBase>(PC->GetPawn());
		}
	}

	ResetProjectileDiagnostics(TEXT("ManagedProjectileVisualProfileProof"));

	int32 CountPerProfile = 16;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66ManagedProjectileVisualProofCount="), CountPerProfile))
	{
		CountPerProfile = FMath::Clamp(CountPerProfile, 1, MaxProjectiles / 3);
	}

	int32 FiredDefault = 0;
	int32 FiredWeb = 0;
	int32 FiredWebNiagara = 0;
	int32 FixedTicks = 0;
	AActor* SourceActor = nullptr;

	if (World && Hero)
	{
		const FVector HeroLocation = Hero->GetActorLocation();
		const FVector SourceLocation = HeroLocation + FVector(-3200.f, -3200.f, 12000.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SourceActor = World->SpawnActor<AActor>(AActor::StaticClass(), SourceLocation, FRotator::ZeroRotator, SpawnParams);
		if (SourceActor)
		{
			SourceActor->SetActorHiddenInGame(true);
			SourceActor->SetActorEnableCollision(false);
			SourceActor->SetActorTickEnabled(false);

			auto FireProfile = [&](const FName ProfileID, const int32 Row, int32& Counter)
			{
				for (int32 Index = 0; Index < CountPerProfile; ++Index)
				{
					FT66ManagedProjectileFireParams Params;
					Params.SourceActor = SourceActor;
					Params.SourceID = FName(TEXT("ManagedProjectileVisualProfileProof"));
					Params.VisualProfileID = ProfileID;
					Params.Origin = SourceLocation + FVector(0.f, static_cast<float>(Index) * 70.f, static_cast<float>(Row) * 90.f);
					Params.Direction = FVector::XAxisVector;
					Params.Speed = 360.f;
					Params.Damage = 0.f;
					Params.Radius = 12.f;
					Params.Lifetime = 3.f;
					Params.Delivery = ET66ManagedProjectileDelivery::EnemyProjectile;
					if (FireManagedProjectile(Params))
					{
						++Counter;
					}
				}
			};

			FireProfile(DefaultEnemySpitVisualProfileID(), 0, FiredDefault);
			FireProfile(EnemyWebVisualProfileID(), 1, FiredWeb);
			FireProfile(EnemyWebNiagaraVisualProfileID(), 2, FiredWebNiagara);

			for (FixedTicks = 0; FixedTicks < 30; ++FixedTicks)
			{
				Tick(1.f / 60.f);
			}
		}
	}

	if (SourceActor)
	{
		SourceActor->Destroy();
	}

	const bool bPass = FiredDefault == CountPerProfile
		&& FiredWeb == CountPerProfile
		&& FiredWebNiagara == CountPerProfile
		&& Diagnostics.ProjectilesActivePeak >= CountPerProfile * 3
		&& Diagnostics.DroppedFires == 0
		&& Diagnostics.VisualProfileFallbacks == 0
		&& Diagnostics.NiagaraBodiesSpawned > 0;

	UE_LOG(
		LogT66ProjectileManager,
		Log,
		TEXT("[ManagedProjectileVisualProfileProofSummary] Terminal=1 CountPerProfile=%d FiredDefault=%d FiredWebHISM=%d FiredWebNiagara=%d FixedTicks=%d ActivePeak=%d DroppedFires=%d VisualProfilesResolved=%d VisualProfileFallbacks=%d NiagaraBodiesSpawned=%d VisualBuckets=%d Pass=%d"),
		CountPerProfile,
		FiredDefault,
		FiredWeb,
		FiredWebNiagara,
		FixedTicks,
		Diagnostics.ProjectilesActivePeak,
		Diagnostics.DroppedFires,
		Diagnostics.VisualProfilesResolved,
		Diagnostics.VisualProfileFallbacks,
		Diagnostics.NiagaraBodiesSpawned,
		ProjectileVisualBuckets.Num(),
		bPass ? 1 : 0);

	EmitProjectileManagerSummary(TEXT("ManagedProjectileVisualProfileProof"), true);
	return bPass;
}

bool UT66ProjectileManagerSubsystem::RunBossProjectileVisualProfileProof()
{
	UWorld* World = GetWorld();
	AT66HeroBase* Hero = nullptr;
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			Hero = Cast<AT66HeroBase>(PC->GetPawn());
		}
	}

	ResetProjectileDiagnostics(TEXT("BossProjectileVisualProfileProof"));

	int32 RequestedCount = 32;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66BossProjectileVisualProofCount="), RequestedCount))
	{
		RequestedCount = FMath::Clamp(RequestedCount, 1, MaxProjectiles);
	}

	int32 FiredBossWebNeedle = 0;
	int32 FixedTicks = 0;
	AT66BossBase* Boss = nullptr;

	if (World && Hero)
	{
		const FVector HeroLocation = Hero->GetActorLocation();
		const FVector BossLocation = HeroLocation + FVector(-4200.f, 1600.f, 12000.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Boss = World->SpawnActor<AT66BossBase>(
			AT66BossBase::StaticClass(),
			BossLocation,
			FRotator::ZeroRotator,
			SpawnParams);

		if (Boss)
		{
			Boss->BossID = FName(TEXT("BossProjectileVisualProfileProof"));
			Boss->MaxHP = FMath::Max(Boss->MaxHP, 1000);
			Boss->CurrentHP = Boss->MaxHP;
			Boss->SetActorHiddenInGame(true);
			Boss->SetActorEnableCollision(false);
			Boss->SetActorTickEnabled(false);

			for (int32 Index = 0; Index < RequestedCount; ++Index)
			{
				FT66ManagedProjectileFireParams FireParams;
				FireParams.SourceActor = Boss;
				FireParams.SourceID = Boss->BossID;
				FireParams.VisualProfileID = BossWebNeedleVisualProfileID();
				FireParams.Origin = BossLocation + FVector(0.f, static_cast<float>(Index) * 54.f, 0.f);
				FireParams.Direction = FVector::XAxisVector;
				FireParams.Speed = 420.f;
				FireParams.Damage = 20.f;
				FireParams.BossAttackProfile = ET66BossAttackProfile::Sharpshooter;
				FireParams.BossPrimaryColor = FLinearColor(0.78f, 0.68f, 1.0f, 1.f);
				FireParams.BossSecondaryColor = FLinearColor(0.92f, 0.92f, 1.0f, 1.f);
				FireParams.bUseBossSecondaryTint = false;
				FireParams.BossVisualScaleMultiplier = 1.f;
				if (FireBossProjectile(FireParams))
				{
					++FiredBossWebNeedle;
				}
			}

			for (FixedTicks = 0; FixedTicks < 30; ++FixedTicks)
			{
				Tick(1.f / 60.f);
			}
		}
	}

	if (Boss)
	{
		Boss->Destroy();
	}

	const bool bPass = FiredBossWebNeedle == RequestedCount
		&& Diagnostics.ProjectilesActivePeak >= RequestedCount
		&& Diagnostics.DroppedFires == 0
		&& Diagnostics.VisualProfileFallbacks == 0
		&& ProjectileVisualBuckets.Num() >= 2;

	UE_LOG(
		LogT66ProjectileManager,
		Log,
		TEXT("[BossProjectileVisualProfileProofSummary] Terminal=1 RequestedCount=%d FiredBossWebNeedle=%d FixedTicks=%d ActivePeak=%d DroppedFires=%d VisualProfilesResolved=%d VisualProfileFallbacks=%d NiagaraBodiesSpawned=%d VisualBuckets=%d Pass=%d"),
		RequestedCount,
		FiredBossWebNeedle,
		FixedTicks,
		Diagnostics.ProjectilesActivePeak,
		Diagnostics.DroppedFires,
		Diagnostics.VisualProfilesResolved,
		Diagnostics.VisualProfileFallbacks,
		Diagnostics.NiagaraBodiesSpawned,
		ProjectileVisualBuckets.Num(),
		bPass ? 1 : 0);

	EmitProjectileManagerSummary(TEXT("BossProjectileVisualProfileProof"), true);
	return bPass;
}
#endif

void UT66ProjectileManagerSubsystem::Tick(const float DeltaTime)
{
	if (!IsTickable() || DeltaTime <= 0.f)
	{
		return;
	}

	const uint64 TickStartCycles = FPlatformTime::Cycles64();
	AT66HeroBase* Hero = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			Hero = Cast<AT66HeroBase>(PC->GetPawn());
		}
	}

	for (int32 Index = 0; Index < Projectiles.Num(); ++Index)
	{
		if (Projectiles[Index].bIsActive)
		{
			TickProjectile(Index, DeltaTime, Hero);
		}
	}

	FlushProjectileInstanceUpdates();
	AccumulateManagerTick(CyclesToMicroseconds(FPlatformTime::Cycles64() - TickStartCycles));
}

TStatId UT66ProjectileManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UT66ProjectileManagerSubsystem, STATGROUP_Tickables);
}

bool UT66ProjectileManagerSubsystem::IsTickable() const
{
	return bInitialized && !bShuttingDown && ActiveProjectileCount > 0 && GetWorld() && GetWorld()->IsGameWorld();
}

bool UT66ProjectileManagerSubsystem::IsTickableWhenPaused() const
{
	return false;
}

bool UT66ProjectileManagerSubsystem::IsTickableInEditor() const
{
	return false;
}

UWorld* UT66ProjectileManagerSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

bool UT66ProjectileManagerSubsystem::EnsureRenderResources(const FVector& AnchorLocation)
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return false;
	}
	if (!RenderHost)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RenderHost = World->SpawnActor<AActor>(AActor::StaticClass(), AnchorLocation, FRotator::ZeroRotator, SpawnParams);
		if (!RenderHost)
		{
			return false;
		}
		RenderHost->SetActorTickEnabled(false);
		RenderHost->SetActorEnableCollision(false);
		RenderRoot = NewObject<USceneComponent>(RenderHost, TEXT("ProjectileManagerRoot"), RF_Transient);
		if (!RenderRoot)
		{
			return false;
		}
		RenderRoot->SetMobility(EComponentMobility::Movable);
		RenderRoot->RegisterComponentWithWorld(World);
		RenderHost->SetRootComponent(RenderRoot);
	}
	return GetProjectileComponent(EnemySpitProjectileTypeIndex) != nullptr;
}

UHierarchicalInstancedStaticMeshComponent* UT66ProjectileManagerSubsystem::GetProjectileComponent(const int32 ProjectileTypeIndex)
{
	const int32 ResolvedTypeIndex = ProjectileTypeIndex >= 0 ? ProjectileTypeIndex : EnemySpitProjectileTypeIndex;
	if (ProjectileComponents.Num() <= ResolvedTypeIndex)
	{
		ProjectileComponents.SetNum(ResolvedTypeIndex + 1);
	}
	if (ProjectileComponents[ResolvedTypeIndex])
	{
		return ProjectileComponents[ResolvedTypeIndex];
	}

	UWorld* World = GetWorld();
	if (!World || !RenderHost || !RenderRoot)
	{
		return nullptr;
	}

	if (ProjectileVisualBuckets.IsValidIndex(ResolvedTypeIndex))
	{
		return CreateProjectileComponent(ResolvedTypeIndex, ProjectileVisualBuckets[ResolvedTypeIndex]);
	}

	if (ResolvedTypeIndex != EnemySpitProjectileTypeIndex)
	{
		return nullptr;
	}

	if (ProjectileVisualBuckets.Num() <= EnemySpitProjectileTypeIndex)
	{
		ProjectileVisualBuckets.SetNum(EnemySpitProjectileTypeIndex + 1);
	}
	ProjectileVisualBuckets[EnemySpitProjectileTypeIndex] = MakeDefaultEnemySpitBucket();
	ManagedVisualBucketByProfileID.Add(DefaultEnemySpitVisualProfileID(), EnemySpitProjectileTypeIndex);
	return CreateProjectileComponent(EnemySpitProjectileTypeIndex, ProjectileVisualBuckets[EnemySpitProjectileTypeIndex]);
}

int32 UT66ProjectileManagerSubsystem::ResolveManagedProjectileVisualTypeIndex(const FT66ManagedProjectileFireParams& Params)
{
	const FName RequestedProfileID = Params.VisualProfileID.IsNone()
		? DefaultEnemySpitVisualProfileID()
		: Params.VisualProfileID;
	if (const int32* Existing = ManagedVisualBucketByProfileID.Find(RequestedProfileID))
	{
		return *Existing;
	}

	const FT66ManagedProjectileVisualProfileSpec* Profile = FindManagedProjectileVisualProfile(RequestedProfileID);
	if (!Profile)
	{
		++Diagnostics.VisualProfileFallbacks;
		UE_LOG(
			LogT66ProjectileManager,
			Warning,
			TEXT("[ManagedProjectileVisualProfileFallback] RequestedProfile=%s FallbackProfile=%s"),
			*RequestedProfileID.ToString(),
			*DefaultEnemySpitVisualProfileID().ToString());
		FT66ManagedProjectileFireParams FallbackParams = Params;
		FallbackParams.VisualProfileID = DefaultEnemySpitVisualProfileID();
		FallbackParams.ProjectileTypeIndex = EnemySpitProjectileTypeIndex;
		return ResolveManagedProjectileVisualTypeIndex(FallbackParams);
	}

	const int32 NewIndex = RequestedProfileID == DefaultEnemySpitVisualProfileID()
		? EnemySpitProjectileTypeIndex
		: FMath::Max(ProjectileComponents.Num(), ProjectileVisualBuckets.Num());
	ProjectileComponents.SetNum(FMath::Max(ProjectileComponents.Num(), NewIndex + 1));
	ProjectileVisualBuckets.SetNum(FMath::Max(ProjectileVisualBuckets.Num(), NewIndex + 1));
	ProjectileVisualBuckets[NewIndex] = MakeManagedProjectileBucketFromProfile(*Profile);
	ManagedVisualBucketByProfileID.Add(RequestedProfileID, NewIndex);
	++Diagnostics.VisualProfilesResolved;
	return NewIndex;
}

int32 UT66ProjectileManagerSubsystem::ResolveBossProjectileTypeIndex(const FT66ManagedProjectileFireParams& Params)
{
	const FLinearColor EffectiveColor = Params.bUseBossSecondaryTint ? Params.BossSecondaryColor : Params.BossPrimaryColor;
	const FColor QuantizedColor = QuantizeBossProjectileColor(EffectiveColor);
	const FT66BossProjectileVisualKey Key{ Params.BossAttackProfile, QuantizedColor };
	if (const int32* Existing = BossVisualBucketByKey.Find(Key))
	{
		return *Existing;
	}

	if (BossVisualBucketByKey.Num() < MaxExactBossVisualBuckets)
	{
		const int32 NewIndex = FMath::Max(ProjectileComponents.Num(), ProjectileVisualBuckets.Num());
		ProjectileComponents.SetNum(NewIndex + 1);
		ProjectileVisualBuckets.SetNum(NewIndex + 1);
		ProjectileVisualBuckets[NewIndex] = MakeLegacyBossProjectileBucket(Params.BossAttackProfile, EffectiveColor, false);
		BossVisualBucketByKey.Add(Key, NewIndex);
		return NewIndex;
	}

	if (const int32* ExistingOverflow = BossOverflowBucketByProfile.Find(Params.BossAttackProfile))
	{
		return *ExistingOverflow;
	}

	const FString WarningKey = FString::Printf(TEXT("Overflow_%d"), static_cast<int32>(Params.BossAttackProfile));
	if (!VisualBucketWarningsEmitted.Contains(WarningKey))
	{
		VisualBucketWarningsEmitted.Add(WarningKey);
		++Diagnostics.VisualBucketOverflowCount;
		UE_LOG(
			LogT66ProjectileManager,
			Warning,
			TEXT("[ProjectileManagerVisualBucketOverflow] Profile=%d ExactBucketCap=%d UsingOverflow=1"),
			static_cast<int32>(Params.BossAttackProfile),
			MaxExactBossVisualBuckets);
	}

	const int32 NewOverflowIndex = FMath::Max(ProjectileComponents.Num(), ProjectileVisualBuckets.Num());
	ProjectileComponents.SetNum(NewOverflowIndex + 1);
	ProjectileVisualBuckets.SetNum(NewOverflowIndex + 1);
	ProjectileVisualBuckets[NewOverflowIndex] = MakeLegacyBossProjectileBucket(Params.BossAttackProfile, GetBossOverflowColor(Params.BossAttackProfile), true);
	BossOverflowBucketByProfile.Add(Params.BossAttackProfile, NewOverflowIndex);
	return NewOverflowIndex;
}

UHierarchicalInstancedStaticMeshComponent* UT66ProjectileManagerSubsystem::CreateProjectileComponent(
	const int32 ProjectileTypeIndex,
	const FT66ProjectileVisualBucket& Bucket)
{
	UWorld* World = GetWorld();
	if (!World || !RenderHost || !RenderRoot)
	{
		return nullptr;
	}
	if (ProjectileComponents.Num() <= ProjectileTypeIndex)
	{
		ProjectileComponents.SetNum(ProjectileTypeIndex + 1);
	}
	if (ProjectileComponents[ProjectileTypeIndex])
	{
		return ProjectileComponents[ProjectileTypeIndex];
	}

	FString SafeProfileName = Bucket.VisualProfileID.IsNone() ? FString(TEXT("Unnamed")) : Bucket.VisualProfileID.ToString();
	SafeProfileName.ReplaceInline(TEXT("."), TEXT("_"));
	SafeProfileName.ReplaceInline(TEXT(" "), TEXT("_"));
	const FName ComponentName = ProjectileTypeIndex == EnemySpitProjectileTypeIndex
		? FName(TEXT("ProjectileManagerHISM_EnemySpit"))
		: FName(*FString::Printf(TEXT("ProjectileManagerHISM_%d_%s%s"),
			ProjectileTypeIndex,
			*SafeProfileName,
			Bucket.bOverflowBucket ? TEXT("_Overflow") : TEXT("")));
	UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(RenderHost, ComponentName, RF_Transient);
	if (!Component)
	{
		return nullptr;
	}
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetupAttachment(RenderRoot);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetCastShadow(false);
	Component->SetCullDistances(0, 0);
	Component->bAutoRebuildTreeOnInstanceChanges = false;
	Component->SetStaticMesh(Bucket.BodyMesh.IsValid() ? Bucket.BodyMesh.Get() : FT66VisualUtil::GetBasicShapeSphere());
	if (UMaterialInterface* BaseMaterial = FT66VisualUtil::GetFlatColorMaterial())
	{
		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Component);
		FT66VisualUtil::ConfigureFlatColorMaterial(Material, FLinearColor::FromSRGBColor(Bucket.Color));
		Component->SetMaterial(0, Material);
	}
	Component->RegisterComponentWithWorld(World);
	PreallocateInstances(Component);
	ProjectileComponents[ProjectileTypeIndex] = Component;
	return Component;
}

void UT66ProjectileManagerSubsystem::PreallocateInstances(UHierarchicalInstancedStaticMeshComponent* Component)
{
	if (!Component || Component->GetInstanceCount() >= MaxProjectiles)
	{
		return;
	}

	const FTransform HiddenTransform(FQuat::Identity, FVector::ZeroVector, FVector(T66ProjectileHiddenScale));
	while (Component->GetInstanceCount() < MaxProjectiles)
	{
		Component->AddInstance(HiddenTransform, false);
	}
	Component->BuildTreeIfOutdated(true, true);
}

int32 UT66ProjectileManagerSubsystem::FindFreeProjectileSlot() const
{
	for (int32 Index = 0; Index < Projectiles.Num(); ++Index)
	{
		if (!Projectiles[Index].bIsActive)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

void UT66ProjectileManagerSubsystem::DeactivateProjectile(const int32 SlotIndex)
{
	if (!Projectiles.IsValidIndex(SlotIndex))
	{
		return;
	}
	FT66ManagedProjectile& Projectile = Projectiles[SlotIndex];
	if (!Projectile.bIsActive)
	{
		return;
	}
	Projectile.bIsActive = false;
	ActiveProjectileCount = FMath::Max(0, ActiveProjectileCount - 1);
	CleanupProjectileTrail(Projectile);
	HideProjectileInstance(Projectile);
}

void UT66ProjectileManagerSubsystem::CleanupProjectileTrail(FT66ManagedProjectile& Projectile)
{
	if (UNiagaraComponent* Body = Projectile.BodyComponent.Get())
	{
		Body->Deactivate();
		Body->DestroyComponent();
	}
	Projectile.BodyComponent.Reset();

	if (UNiagaraComponent* Trail = Projectile.TrailComponent.Get())
	{
		Trail->Deactivate();
		Trail->DestroyComponent();
	}
	Projectile.TrailComponent.Reset();
	Projectile.TrailSystem.Reset();
	Projectile.ImpactSystem.Reset();
}

void UT66ProjectileManagerSubsystem::HideProjectileInstance(const FT66ManagedProjectile& Projectile)
{
	UHierarchicalInstancedStaticMeshComponent* Component = GetProjectileComponent(Projectile.ProjectileTypeIndex);
	if (!Component || Projectile.InstanceIndex == INDEX_NONE)
	{
		return;
	}

	const uint64 HISMStartCycles = FPlatformTime::Cycles64();
	Component->UpdateInstanceTransform(Projectile.InstanceIndex, FTransform(FQuat::Identity, FVector::ZeroVector, FVector(T66ProjectileHiddenScale)), false, false, true);
	bProjectileInstancesDirty = true;
	AccumulateHISMUpdate(HISMStartCycles);
}

void UT66ProjectileManagerSubsystem::UpdateProjectileInstance(const FT66ManagedProjectile& Projectile)
{
	UHierarchicalInstancedStaticMeshComponent* Component = GetProjectileComponent(Projectile.ProjectileTypeIndex);
	if (!Component || Projectile.InstanceIndex == INDEX_NONE)
	{
		return;
	}

	const uint64 HISMStartCycles = FPlatformTime::Cycles64();
	Component->UpdateInstanceTransform(Projectile.InstanceIndex, MakeLocalTransform(Projectile), false, false, true);
	if (UNiagaraComponent* Body = Projectile.BodyComponent.Get())
	{
		Body->SetWorldLocationAndRotation(Projectile.Position, Projectile.Velocity.Rotation());
	}
	if (UNiagaraComponent* Trail = Projectile.TrailComponent.Get())
	{
		Trail->SetWorldLocationAndRotation(Projectile.Position, Projectile.Velocity.Rotation());
	}
	bProjectileInstancesDirty = true;
	AccumulateHISMUpdate(HISMStartCycles);
}

void UT66ProjectileManagerSubsystem::SpawnProjectileBody(FT66ManagedProjectile& Projectile)
{
	if (Projectile.VisualBodyMode != ET66ManagedProjectileVisualBodyMode::Niagara || !GetWorld())
	{
		return;
	}
	if (UNiagaraComponent* Existing = Projectile.BodyComponent.Get())
	{
		Existing->SetWorldLocationAndRotation(Projectile.Position, Projectile.Velocity.Rotation());
		return;
	}

	UNiagaraSystem* BodySystem = nullptr;
	if (ProjectileVisualBuckets.IsValidIndex(Projectile.ProjectileTypeIndex))
	{
		BodySystem = ProjectileVisualBuckets[Projectile.ProjectileTypeIndex].BodySystem.Get();
	}
	if (!BodySystem || !TryConsumeManagedBossProjectileVFXBudget())
	{
		return;
	}

	UNiagaraComponent* Body = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		BodySystem,
		Projectile.Position,
		Projectile.Velocity.Rotation(),
		FVector(0.50f),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
	if (Body)
	{
		Body->SetAutoDestroy(false);
		Projectile.BodyComponent = Body;
		++Diagnostics.NiagaraBodiesSpawned;
	}
}

void UT66ProjectileManagerSubsystem::SpawnBossTrail(FT66ManagedProjectile& Projectile)
{
	if (!RenderRoot || !GetWorld())
	{
		return;
	}
	UNiagaraSystem* TrailSystem = Projectile.TrailSystem.Get();
	if (!TrailSystem && Projectile.Delivery == ET66ManagedProjectileDelivery::BossProjectile)
	{
		TrailSystem = ResolveManagedBossProjectileSystem(GetManagedBossProjectileTrailPath(Projectile.BossAttackProfile));
	}
	if (!TrailSystem || !TryConsumeManagedBossProjectileVFXBudget())
	{
		return;
	}

	UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		TrailSystem,
		Projectile.Position,
		Projectile.Velocity.Rotation(),
		FVector(0.45f),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
	if (Trail)
	{
		Trail->SetAutoDestroy(false);
		Projectile.TrailComponent = Trail;
	}
}

void UT66ProjectileManagerSubsystem::SpawnBossImpact(const FT66ManagedProjectile& Projectile)
{
	if (Projectile.Delivery != ET66ManagedProjectileDelivery::BossProjectile
		&& !Projectile.ImpactSystem.IsValid())
	{
		return;
	}

	if (Projectile.Delivery == ET66ManagedProjectileDelivery::BossProjectile)
	{
		UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Boss.Projectile.Impact")), Projectile.Position, Projectile.SourceMob.Get());
	}
	UNiagaraSystem* ImpactSystem = Projectile.ImpactSystem.Get();
	if (!ImpactSystem || !GetWorld() || !TryConsumeManagedBossProjectileVFXBudget())
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ImpactSystem,
		Projectile.Position,
		Projectile.Velocity.Rotation(),
		FVector(0.55f),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
}

void UT66ProjectileManagerSubsystem::TickProjectile(const int32 SlotIndex, const float DeltaTime, AT66HeroBase* Hero)
{
	FT66ManagedProjectile& Projectile = Projectiles[SlotIndex];
	if (Projectile.Delivery == ET66ManagedProjectileDelivery::BossProjectile)
	{
		AT66BossBase* BossSource = Cast<AT66BossBase>(Projectile.SourceMob.Get());
		if (!BossSource || !BossSource->IsAlive())
		{
			++Diagnostics.DroppedInvalidSource;
			DeactivateProjectile(SlotIndex);
			return;
		}
	}

	Projectile.PreviousPosition = Projectile.Position;
	const FVector NewPosition = Projectile.Position + Projectile.Velocity * DeltaTime;
	const float SegmentLength = FVector::Dist(Projectile.PreviousPosition, NewPosition);

	FHitResult NonHeroHit;
	const bool bHasNonHeroImpact = FindNonHeroImpact(Projectile, Projectile.PreviousPosition, NewPosition, Hero, NonHeroHit);
	float NonHeroFraction = bHasNonHeroImpact ? FMath::Clamp(NonHeroHit.Time, 0.f, 1.f) : TNumericLimits<float>::Max();

	float HeroFraction = TNumericLimits<float>::Max();
	const bool bHasHeroHit = FindHeroHitFraction(Projectile.PreviousPosition, NewPosition, Hero, Projectile.Radius, HeroFraction);

	if (bHasNonHeroImpact && NonHeroFraction <= HeroFraction)
	{
		Projectile.Position = FMath::Lerp(Projectile.PreviousPosition, NewPosition, NonHeroFraction);
		if (Projectile.Delivery == ET66ManagedProjectileDelivery::EnemyProjectile)
		{
			if (UT66MobManagerSubsystem* MobManager = GetWorld() ? GetWorld()->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
			{
				MobManager->RecordManagedEnemyProjectileWorldImpact(Projectile.SourceMob.Get(), Projectile.SourceMobID, Projectile.bSourceWasLightweight, NonHeroHit.GetActor());
			}
		}
		Diagnostics.LastWorldImpactActor = GetNameSafe(NonHeroHit.GetActor());
		++Diagnostics.ProjectilesHitWorld;
		SpawnBossImpact(Projectile);
		DeactivateProjectile(SlotIndex);
		return;
	}

	if (bHasHeroHit && Hero)
	{
		Projectile.Position = FMath::Lerp(Projectile.PreviousPosition, NewPosition, HeroFraction);
		if (Projectile.Delivery == ET66ManagedProjectileDelivery::EnemyProjectile)
		{
			if (UT66MobManagerSubsystem* MobManager = GetWorld() ? GetWorld()->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
			{
				if (Hero->IsInSafeZone())
				{
					MobManager->RecordManagedEnemyProjectileSafeZoneReject(Projectile.SourceMob.Get(), Projectile.SourceMobID, Projectile.bSourceWasLightweight, Hero);
					DeactivateProjectile(SlotIndex);
					return;
				}
				MobManager->RecordManagedEnemyProjectileHeroHit(Projectile.SourceMob.Get(), Projectile.SourceMobID, Projectile.bSourceWasLightweight, Hero, Projectile.Damage);
			}
		}
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				AActor* SourceActor = Projectile.SourceMob.Get();
				const FName Delivery = Projectile.Delivery == ET66ManagedProjectileDelivery::BossProjectile
					? FName(TEXT("BossProjectile"))
					: FName(TEXT("EnemyProjectile"));
				if (Projectile.Delivery == ET66ManagedProjectileDelivery::BossProjectile && !SourceActor)
				{
					++Diagnostics.DroppedInvalidSource;
					DeactivateProjectile(SlotIndex);
					return;
				}
				if (!RunState->ApplyDamage(Projectile.Damage, SourceActor, Delivery, SourceActor))
				{
					++Diagnostics.ApplyDamageReturnedFalse;
				}
			}
		}
		++Diagnostics.ProjectilesHitHero;
		SpawnBossImpact(Projectile);
		DeactivateProjectile(SlotIndex);
		return;
	}

	Projectile.Position = NewPosition;
	Projectile.DistanceTraveled += SegmentLength;
	Projectile.LifetimeRemaining -= DeltaTime;
	if (Projectile.LifetimeRemaining <= 0.f)
	{
		++Diagnostics.ProjectilesExpired;
		DeactivateProjectile(SlotIndex);
		return;
	}

	UpdateProjectileInstance(Projectile);
}

bool UT66ProjectileManagerSubsystem::ResolveHeroCapsule(const AT66HeroBase* Hero, FVector& OutSegmentA, FVector& OutSegmentB, float& OutRadius) const
{
	const UCapsuleComponent* Capsule = Hero ? Hero->FindComponentByClass<UCapsuleComponent>() : nullptr;
	if (!Capsule)
	{
		return false;
	}

	const FVector Center = Capsule->GetComponentLocation();
	const FVector Up = Capsule->GetUpVector();
	const float Radius = Capsule->GetScaledCapsuleRadius();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float SegmentHalf = FMath::Max(0.f, HalfHeight - Radius);
	OutSegmentA = Center - Up * SegmentHalf;
	OutSegmentB = Center + Up * SegmentHalf;
	OutRadius = Radius;
	return OutRadius > 0.f;
}

bool UT66ProjectileManagerSubsystem::FindHeroHitFraction(
	const FVector& SegmentStart,
	const FVector& SegmentEnd,
	const AT66HeroBase* Hero,
	const float ProjectileRadius,
	float& OutFraction) const
{
	OutFraction = TNumericLimits<float>::Max();
	FVector CapsuleA;
	FVector CapsuleB;
	float CapsuleRadius = 0.f;
	if (!ResolveHeroCapsule(Hero, CapsuleA, CapsuleB, CapsuleRadius))
	{
		return false;
	}

	float SegmentFraction = 0.f;
	const float DistanceSq = SegmentSegmentDistanceSquared(SegmentStart, SegmentEnd, CapsuleA, CapsuleB, SegmentFraction);
	const float CombinedRadius = CapsuleRadius + ProjectileRadius;
	if (DistanceSq <= FMath::Square(CombinedRadius))
	{
		OutFraction = FMath::Clamp(SegmentFraction, 0.f, 1.f);
		return true;
	}
	return false;
}

bool UT66ProjectileManagerSubsystem::FindNonHeroImpact(
	const FT66ManagedProjectile& Projectile,
	const FVector& SegmentStart,
	const FVector& SegmentEnd,
	const AT66HeroBase* Hero,
	FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(T66ManagedEnemyProjectileSweep), false);
	if (Hero)
	{
		QueryParams.AddIgnoredActor(Hero);
	}
	AActor* Source = Projectile.SourceMob.Get();
	const bool bIgnoreSourceForMuzzleClear = Source && Projectile.DistanceTraveled < Projectile.SourceIgnoreDistance;
	const bool bIgnoreNonPawnOwner = Source && !Cast<APawn>(Source);
	const bool bIgnoreBossOwner = Source && Projectile.Delivery == ET66ManagedProjectileDelivery::BossProjectile;
	if (Source && (bIgnoreSourceForMuzzleClear || bIgnoreNonPawnOwner || bIgnoreBossOwner))
	{
		QueryParams.AddIgnoredActor(Source);
	}

	TArray<FHitResult> Hits;
	if (!World->SweepMultiByObjectType(Hits, SegmentStart, SegmentEnd, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(Projectile.Radius), QueryParams))
	{
		return false;
	}

	bool bFound = false;
	float BestTime = TNumericLimits<float>::Max();
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Hero)
		{
			continue;
		}
		if (const UPrimitiveComponent* HitComponent = Hit.GetComponent())
		{
			if (HitComponent->GetCollisionResponseToChannel(ECC_WorldDynamic) == ECR_Ignore)
			{
				continue;
			}
		}
		if (HitActor == Source && bIgnoreNonPawnOwner)
		{
			if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
			{
				MobManager->RecordManagedEnemyProjectileOwnerIgnored(Source, Projectile.SourceMobID, Projectile.bSourceWasLightweight);
			}
			continue;
		}
		if (IsEnemyProjectilePeerBody(HitActor))
		{
			continue;
		}
		const float HitTime = FMath::Clamp(Hit.Time, 0.f, 1.f);
		if (HitTime < BestTime)
		{
			BestTime = HitTime;
			OutHit = Hit;
			bFound = true;
		}
	}
	return bFound;
}

float UT66ProjectileManagerSubsystem::ResolveSourceIgnoreDistance(const AActor* SourceMob, const float ProjectileRadius) const
{
	const UCapsuleComponent* Capsule = SourceMob ? SourceMob->FindComponentByClass<UCapsuleComponent>() : nullptr;
	const float SourceRadius = Capsule ? Capsule->GetScaledCapsuleRadius() : 48.f;
	return FMath::Max(ProjectileRadius + SourceRadius + T66ProjectileMuzzleClearMargin, 64.f);
}

FTransform UT66ProjectileManagerSubsystem::MakeLocalTransform(const FT66ManagedProjectile& Projectile) const
{
	FQuat Rotation = Projectile.Velocity.IsNearlyZero()
		? FQuat::Identity
		: Projectile.Velocity.Rotation().Quaternion();
	if (Projectile.Delivery == ET66ManagedProjectileDelivery::BossProjectile)
	{
		Rotation = Rotation * Projectile.VisualRotationOffset;
	}
	else if (!Projectile.VisualRotationOffset.IsIdentity())
	{
		Rotation = Rotation * Projectile.VisualRotationOffset;
	}
	return FTransform(Rotation, Projectile.Position - GetRenderHostLocation(), Projectile.VisualScale);
}

FVector UT66ProjectileManagerSubsystem::GetRenderHostLocation() const
{
	return RenderHost ? RenderHost->GetActorLocation() : FVector::ZeroVector;
}

void UT66ProjectileManagerSubsystem::FlushProjectileInstanceUpdates()
{
	if (!bProjectileInstancesDirty)
	{
		return;
	}
	const uint64 HISMStartCycles = FPlatformTime::Cycles64();
	for (const TObjectPtr<UHierarchicalInstancedStaticMeshComponent>& Component : ProjectileComponents)
	{
		if (Component)
		{
			Component->BuildTreeIfOutdated(true, true);
		}
	}
	bProjectileInstancesDirty = false;
	AccumulateHISMUpdate(HISMStartCycles);
}

void UT66ProjectileManagerSubsystem::AccumulateManagerTick(const double ElapsedUs)
{
	Diagnostics.ManagerTickMaxUs = FMath::Max(Diagnostics.ManagerTickMaxUs, ElapsedUs);
	Diagnostics.ManagerTickTotalUs += ElapsedUs;
	++Diagnostics.ManagerTickSamples;
}

void UT66ProjectileManagerSubsystem::AccumulateHISMUpdate(const uint64 StartCycles)
{
	const double ElapsedUs = CyclesToMicroseconds(FPlatformTime::Cycles64() - StartCycles);
	Diagnostics.HISMUpdateMaxUs = FMath::Max(Diagnostics.HISMUpdateMaxUs, ElapsedUs);
	Diagnostics.HISMUpdateTotalUs += ElapsedUs;
	++Diagnostics.HISMUpdateSamples;
}
