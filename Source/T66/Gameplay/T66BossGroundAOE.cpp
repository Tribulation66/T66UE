// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossGroundAOE.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	UNiagaraSystem* T66LoadBossAOEImpactSystem(const TCHAR* AssetPath)
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

	const TCHAR* T66GetBossAOEImpactPath(const ET66BossAttackProfile AttackProfile)
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
}

AT66BossGroundAOE::AT66BossGroundAOE()
{
	PrimaryActorTick.bCanEverTick = true;

	DamageZone = CreateDefaultSubobject<USphereComponent>(TEXT("DamageZone"));
	DamageZone->SetSphereRadius(300.f);
	DamageZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageZone->SetGenerateOverlapEvents(false);
	RootComponent = DamageZone;

	WarningMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WarningMesh"));
	WarningMesh->SetupAttachment(RootComponent);
	WarningMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WarningMesh->SetCastShadow(false);

	ImpactMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImpactMesh"));
	ImpactMesh->SetupAttachment(RootComponent);
	ImpactMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ImpactMesh->SetCastShadow(false);
	ImpactMesh->SetVisibility(false);

	if (UStaticMesh* CylinderMesh = FT66VisualUtil::GetBasicShapeCylinder())
	{
		WarningMesh->SetStaticMesh(CylinderMesh);
		ImpactMesh->SetStaticMesh(CylinderMesh);
	}

	InitialLifeSpan = 8.f;
}

void AT66BossGroundAOE::ConfigureVisualStyle(
	const ET66BossAttackProfile InAttackProfile,
	const FLinearColor& InTelegraphColor,
	const FLinearColor& InImpactColor)
{
	AttackProfile = InAttackProfile;
	TelegraphColor = InTelegraphColor;
	ImpactColor = InImpactColor;
	CachedImpactVFX = T66LoadBossAOEImpactSystem(T66GetBossAOEImpactPath(AttackProfile));
}

void AT66BossGroundAOE::RefreshVisualState(const float WarningAlpha)
{
	const float RadiusScale = Radius / 50.f;
	if (WarningMesh)
	{
		const float Pulse = 1.f + 0.08f * FMath::Sin(WarningElapsed * FMath::Lerp(2.f, 8.f, WarningAlpha) * PI);
		WarningMesh->SetRelativeScale3D(FVector(RadiusScale * Pulse, RadiusScale * Pulse, 0.025f + 0.01f * WarningAlpha));
		WarningMesh->SetRelativeLocation(FVector(0.f, 0.f, 2.5f));
		WarningMesh->AddLocalRotation(FRotator(0.f, 50.f * GetWorld()->GetDeltaSeconds(), 0.f));
		FT66VisualUtil::ApplyT66Color(WarningMesh, this, TelegraphColor);
	}

	if (ImpactMesh)
	{
		const float PillarHeightScale = FMath::Lerp(0.05f, 2.6f, WarningAlpha);
		ImpactMesh->SetRelativeScale3D(FVector(RadiusScale * 0.38f, RadiusScale * 0.38f, PillarHeightScale));
		ImpactMesh->SetRelativeLocation(FVector(0.f, 0.f, 100.f * PillarHeightScale));
		FT66VisualUtil::ApplyT66Color(ImpactMesh, this, ImpactColor);
	}
}

void AT66BossGroundAOE::BeginPlay()
{
	Super::BeginPlay();
	DamageZone->SetSphereRadius(Radius);
	WarningElapsed = 0.f;

	if (!CachedImpactVFX)
	{
		CachedImpactVFX = T66LoadBossAOEImpactSystem(T66GetBossAOEImpactPath(AttackProfile));
	}

	RefreshVisualState(0.f);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ActivateTimerHandle, this, &AT66BossGroundAOE::ActivateDamage, WarningDurationSeconds, false);
	}
}

void AT66BossGroundAOE::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bDamageActivated)
	{
		WarningElapsed += DeltaSeconds;
		RefreshVisualState(FMath::Clamp(WarningElapsed / FMath::Max(0.1f, WarningDurationSeconds), 0.f, 1.f));
		return;
	}

	const float LingerAlpha = FMath::Clamp(WarningElapsed / FMath::Max(0.1f, PillarLingerSeconds), 0.f, 1.f);
	RefreshVisualState(1.f - LingerAlpha);
	WarningElapsed += DeltaSeconds;
}

void AT66BossGroundAOE::ActivateDamage()
{
	bDamageActivated = true;
	WarningElapsed = 0.f;
	UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Boss.AOE.Impact")), GetActorLocation(), GetOwner());

	if (WarningMesh)
	{
		WarningMesh->SetVisibility(false);
	}
	if (ImpactMesh)
	{
		ImpactMesh->SetVisibility(true);
	}

	DamageZone->SetGenerateOverlapEvents(true);
	DamageZone->UpdateOverlaps();

	const FName UltimateSourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	if (bDamageEnemies)
	{
		TArray<AActor*> Overlapping;
		DamageZone->GetOverlappingActors(Overlapping);
		for (AActor* Actor : Overlapping)
		{
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Actor))
			{
				if (E->CurrentHP > 0)
				{
					E->TakeDamageFromHero(DamageHP, UltimateSourceID, NAME_None);
				}
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Actor))
			{
				if (B->IsAwakened() && B->IsAlive())
				{
					B->TakeDamageFromHeroHit(DamageHP, UltimateSourceID, NAME_None);
				}
			}
		}
	}
	else
	{
		TArray<AActor*> Overlapping;
		DamageZone->GetOverlappingActors(Overlapping, AT66HeroBase::StaticClass());
		for (AActor* Actor : Overlapping)
		{
			AT66HeroBase* Hero = Cast<AT66HeroBase>(Actor);
			if (!Hero || Hero->IsInSafeZone())
			{
				continue;
			}

			UWorld* World = GetWorld();
			UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
			UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
			if (RunState)
			{
				RunState->ApplyDamage(DamageHP, GetOwner());
			}
		}
	}

	DamageZone->SetGenerateOverlapEvents(false);

	if (CachedImpactVFX && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CachedImpactVFX,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector(FMath::Max(0.65f, Radius / 180.f)),
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AT66BossGroundAOE::DestroySelf, PillarLingerSeconds, false);
	}
}

void AT66BossGroundAOE::DestroySelf()
{
	Destroy();
}
