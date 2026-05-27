// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossProjectile.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66CombatShared.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CoreMisc.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66BossProjectile, Log, All);

namespace
{
	FString T66BossProjectileVectorForLog(const FVector& Vector)
	{
		return Vector.ToCompactString();
	}

	static TAutoConsoleVariable<int32> CVarT66BossProjectileVFXMaxPerFrame(
		TEXT("T66.VFX.BossProjectileMaxPerFrame"),
		28,
		TEXT("Max boss projectile Niagara trail/impact spawns per frame. Values <= 0 disable this cap."));

	static TAutoConsoleVariable<int32> CVarT66BossProjectileVFXUseEffectsScalability(
		TEXT("T66.VFX.BossProjectileUseEffectsScalability"),
		1,
		TEXT("Scale boss projectile VFX caps by sg.EffectsQuality."));

	uint64 GBossProjectileVFXBudgetFrame = MAX_uint64;
	int32 GBossProjectileVFXEmittedThisFrame = 0;

	float T66GetBossProjectileEffectsQualityScale()
	{
		if (CVarT66BossProjectileVFXUseEffectsScalability.GetValueOnGameThread() == 0)
		{
			return 1.0f;
		}

		IConsoleVariable* EffectsQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
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

	bool T66TryConsumeBossProjectileVFXBudget()
	{
		if (GBossProjectileVFXBudgetFrame != GFrameCounter)
		{
			GBossProjectileVFXBudgetFrame = GFrameCounter;
			GBossProjectileVFXEmittedThisFrame = 0;
		}

		const int32 BaseBudget = CVarT66BossProjectileVFXMaxPerFrame.GetValueOnGameThread();
		if (BaseBudget <= 0)
		{
			++GBossProjectileVFXEmittedThisFrame;
			return true;
		}

		const int32 ScaledBudget = FMath::Max(4, FMath::RoundToInt(static_cast<float>(BaseBudget) * T66GetBossProjectileEffectsQualityScale()));
		if (GBossProjectileVFXEmittedThisFrame >= ScaledBudget)
		{
			return false;
		}

		++GBossProjectileVFXEmittedThisFrame;
		return true;
	}

	UNiagaraSystem* T66ResolveBossProjectileSystem(const TCHAR* AssetPath)
	{
		if (!AssetPath || !*AssetPath)
		{
			return nullptr;
		}

		static TMap<FString, TWeakObjectPtr<UNiagaraSystem>> Cache;
		static TMap<FString, TSharedPtr<FStreamableHandle>> ActiveLoads;
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

	const TCHAR* T66GetBossProjectileTrailPath(const ET66BossAttackProfile AttackProfile)
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

	const TCHAR* T66GetBossProjectileImpactPath(const ET66BossAttackProfile AttackProfile)
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

	void T66PreloadBossProjectileSystemsAsync()
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
			T66ResolveBossProjectileSystem(Path);
		}
	}

	UStaticMesh* T66GetBossProjectileMesh(const ET66BossAttackProfile AttackProfile)
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

	FVector T66GetBossProjectileScale(const ET66BossAttackProfile AttackProfile)
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
}

AT66BossProjectile::AT66BossProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 6.0f;
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		T66PreloadBossProjectileSystemsAsync();
	}

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionSphere->SetSphereRadius(24.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	RootComponent = CollisionSphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetCastShadow(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 900.f;
	ProjectileMovement->MaxSpeed = 900.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	UpdateVisualStyle();
}

void AT66BossProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	T66CombatDebugDraw::DrawDamageSphere(CollisionSphere, TEXT("Boss Projectile Damage"), DamageHearts > 0);
}

void AT66BossProjectile::ConfigureVisualStyle(
	const ET66BossAttackProfile InAttackProfile,
	const FLinearColor& InPrimaryColor,
	const FLinearColor& InSecondaryColor,
	const bool bInUseSecondaryTint)
{
	AttackProfile = InAttackProfile;
	PrimaryColor = InPrimaryColor;
	SecondaryColor = InSecondaryColor;
	bUseSecondaryTint = bInUseSecondaryTint;
	UpdateVisualStyle();
}

void AT66BossProjectile::SetVisualScaleMultiplier(const float InMultiplier)
{
	VisualScaleMultiplier = FMath::Clamp(InMultiplier, 0.35f, 5.f);
	UpdateVisualStyle();
}

void AT66BossProjectile::UpdateVisualStyle()
{
	if (!VisualMesh)
	{
		return;
	}

	if (UStaticMesh* Mesh = T66GetBossProjectileMesh(AttackProfile))
	{
		VisualMesh->SetStaticMesh(Mesh);
	}

	VisualMesh->SetRelativeScale3D(T66GetBossProjectileScale(AttackProfile) * VisualScaleMultiplier);
	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(24.f * FMath::Max(1.f, VisualScaleMultiplier));
	}
	if (AttackProfile == ET66BossAttackProfile::Sharpshooter || AttackProfile == ET66BossAttackProfile::Duelist)
	{
		VisualMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	}
	else
	{
		VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}

	FT66VisualUtil::ApplyT66Color(VisualMesh, this, bUseSecondaryTint ? SecondaryColor : PrimaryColor);
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		T66PreloadBossProjectileSystemsAsync();
		CachedTrailSystem = T66ResolveBossProjectileSystem(T66GetBossProjectileTrailPath(AttackProfile));
		CachedImpactSystem = T66ResolveBossProjectileSystem(T66GetBossProjectileImpactPath(AttackProfile));
	}
}

void AT66BossProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66BossProjectile::OnSphereOverlap);
	UpdateVisualStyle();

	if (CachedTrailSystem && T66TryConsumeBossProjectileVFXBudget())
	{
		TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CachedTrailSystem,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(0.45f),
			EAttachLocation::KeepRelativeOffset,
			false,
			ENCPoolMethod::AutoRelease,
			true,
			true);
		if (TrailComponent)
		{
			TrailComponent->SetAutoDestroy(false);
		}
	}
}

void AT66BossProjectile::SetTargetLocation(const FVector& TargetLoc, const float Speed)
{
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	const FVector Dir = (TargetLoc - GetActorLocation()).GetSafeNormal();
	ProjectileMovement->Velocity = Dir * Speed;

	UE_LOG(
		LogT66BossProjectile,
		Log,
		TEXT("[ProjectileFired] Type=BossProjectile Projectile=%s Owner=%s Loc=%s SphereLoc=%s Target=%s Direction=%s Speed=%.1f Radius=%.1f Visual=%s VisualVisible=%d"),
		*GetName(),
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
		*T66BossProjectileVectorForLog(GetActorLocation()),
		CollisionSphere ? *T66BossProjectileVectorForLog(CollisionSphere->GetComponentLocation()) : TEXT("None"),
		*T66BossProjectileVectorForLog(TargetLoc),
		*T66BossProjectileVectorForLog(Dir),
		Speed,
		CollisionSphere ? CollisionSphere->GetScaledSphereRadius() : 0.f,
		VisualMesh ? *VisualMesh->GetName() : TEXT("None"),
		(VisualMesh && VisualMesh->IsVisible() && !VisualMesh->bHiddenInGame) ? 1 : 0);
}

void AT66BossProjectile::SpawnImpactEffect()
{
	UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Boss.Projectile.Impact")), GetActorLocation(), GetOwner());

	if (!CachedImpactSystem || !GetWorld() || !T66TryConsumeBossProjectileVFXBudget())
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CachedImpactSystem,
		GetActorLocation(),
		GetActorRotation(),
		FVector(0.55f),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
}

void AT66BossProjectile::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OtherBodyIndex;

	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero)
	{
		UE_LOG(
			LogT66BossProjectile,
			Verbose,
			TEXT("[ProjectileImpact] Type=BossProjectile Result=NonHero Projectile=%s Owner=%s OtherActor=%s OtherComp=%s ProjectileLoc=%s SphereLoc=%s OverlapComp=%s bFromSweep=%d SweepLoc=%s ImpactPoint=%s Velocity=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			OtherActor ? *OtherActor->GetName() : TEXT("None"),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			*T66BossProjectileVectorForLog(GetActorLocation()),
			CollisionSphere ? *T66BossProjectileVectorForLog(CollisionSphere->GetComponentLocation()) : TEXT("None"),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OverlappedComponent),
			bFromSweep ? 1 : 0,
			*T66BossProjectileVectorForLog(SweepResult.Location),
			*T66BossProjectileVectorForLog(SweepResult.ImpactPoint),
			ProjectileMovement ? *T66BossProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
		SpawnImpactEffect();
		Destroy();
		return;
	}

	if (!T66CombatShared::IsHeroHurtboxComponent(Hero, OtherComp))
	{
		UE_LOG(
			LogT66BossProjectile,
			Log,
			TEXT("[ProjectileImpact] Type=BossProjectile Result=RejectedNonHeroHurtbox Projectile=%s Owner=%s Hero=%s OtherComp=%s ProjectileLoc=%s SphereLoc=%s HeroLoc=%s HeroDist2D=%.1f SphereToHeroDist2D=%.1f OverlapComp=%s bFromSweep=%d SweepLoc=%s ImpactPoint=%s Velocity=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			*Hero->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			*T66BossProjectileVectorForLog(GetActorLocation()),
			CollisionSphere ? *T66BossProjectileVectorForLog(CollisionSphere->GetComponentLocation()) : TEXT("None"),
			*T66BossProjectileVectorForLog(Hero->GetActorLocation()),
			FVector::Dist2D(GetActorLocation(), Hero->GetActorLocation()),
			CollisionSphere ? FVector::Dist2D(CollisionSphere->GetComponentLocation(), Hero->GetActorLocation()) : -1.f,
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OverlappedComponent),
			bFromSweep ? 1 : 0,
			*T66BossProjectileVectorForLog(SweepResult.Location),
			*T66BossProjectileVectorForLog(SweepResult.ImpactPoint),
			ProjectileMovement ? *T66BossProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		const int32 DamageHP = FMath::Max(1, DamageHearts) * 20;
		UE_LOG(
			LogT66BossProjectile,
			Log,
			TEXT("[ProjectileImpact] Type=BossProjectile Result=HeroHit Projectile=%s Owner=%s OwnerClass=%s Hero=%s HeroComp=%s DamageHP=%d ProjectileLoc=%s SphereLoc=%s HeroLoc=%s HeroDist2D=%.1f SphereToHeroDist2D=%.1f bFromSweep=%d SweepLoc=%s ImpactPoint=%s Normal=%s Velocity=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			GetOwner() && GetOwner()->GetClass() ? *GetOwner()->GetClass()->GetName() : TEXT("None"),
			*Hero->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			DamageHP,
			*T66BossProjectileVectorForLog(GetActorLocation()),
			CollisionSphere ? *T66BossProjectileVectorForLog(CollisionSphere->GetComponentLocation()) : TEXT("None"),
			*T66BossProjectileVectorForLog(Hero->GetActorLocation()),
			FVector::Dist2D(GetActorLocation(), Hero->GetActorLocation()),
			CollisionSphere ? FVector::Dist2D(CollisionSphere->GetComponentLocation(), Hero->GetActorLocation()) : -1.f,
			bFromSweep ? 1 : 0,
			*T66BossProjectileVectorForLog(SweepResult.Location),
			*T66BossProjectileVectorForLog(SweepResult.ImpactPoint),
			*T66BossProjectileVectorForLog(SweepResult.ImpactNormal),
			ProjectileMovement ? *T66BossProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
		RunState->ApplyDamage(DamageHP, GetOwner(), FName(TEXT("BossProjectile")), this);
	}

	SpawnImpactEffect();
	Destroy();
}
