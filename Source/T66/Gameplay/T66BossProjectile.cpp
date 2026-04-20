// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossProjectile.h"

#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	UNiagaraSystem* T66LoadBossProjectileSystem(const TCHAR* AssetPath)
	{
		if (!AssetPath || !*AssetPath)
		{
			return nullptr;
		}

		static TMap<FString, TWeakObjectPtr<UNiagaraSystem>> Cache;
		const FString Key(AssetPath);
		if (const TWeakObjectPtr<UNiagaraSystem>* Found = Cache.Find(Key))
		{
			if (Found->IsValid())
			{
				return Found->Get();
			}
		}

		UNiagaraSystem* Loaded = FindObject<UNiagaraSystem>(nullptr, AssetPath);
		if (!Loaded)
		{
			Loaded = LoadObject<UNiagaraSystem>(nullptr, AssetPath);
		}

		Cache.Add(Key, Loaded);
		return Loaded;
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
		case ET66BossAttackProfile::Vendor:
			return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_02.P_Weapon_02");
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
		case ET66BossAttackProfile::Vendor:
			return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_02.P_Weapon_02");
		case ET66BossAttackProfile::Gambler:
			return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Portal.P_Cosmic_Portal");
		case ET66BossAttackProfile::Balanced:
		default:
			return TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1");
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
		case ET66BossAttackProfile::Vendor:
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
		case ET66BossAttackProfile::Vendor:
			return FVector(0.24f, 0.24f, 0.52f);
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
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 6.0f;

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

	VisualMesh->SetRelativeScale3D(T66GetBossProjectileScale(AttackProfile));
	if (AttackProfile == ET66BossAttackProfile::Sharpshooter || AttackProfile == ET66BossAttackProfile::Duelist)
	{
		VisualMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	}
	else
	{
		VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}

	FT66VisualUtil::ApplyT66Color(VisualMesh, this, bUseSecondaryTint ? SecondaryColor : PrimaryColor);
	CachedTrailSystem = T66LoadBossProjectileSystem(T66GetBossProjectileTrailPath(AttackProfile));
	CachedImpactSystem = T66LoadBossProjectileSystem(T66GetBossProjectileImpactPath(AttackProfile));
}

void AT66BossProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66BossProjectile::OnSphereOverlap);
	UpdateVisualStyle();

	if (CachedTrailSystem)
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
}

void AT66BossProjectile::SpawnImpactEffect()
{
	if (!CachedImpactSystem || !GetWorld())
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
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero)
	{
		SpawnImpactEffect();
		Destroy();
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		const int32 DamageHP = FMath::Max(1, DamageHearts) * 20;
		RunState->ApplyDamage(DamageHP, GetOwner());
	}

	SpawnImpactEffect();
	Destroy();
}
