// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66FloorFlameTrap.h"

#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

namespace
{
	UNiagaraSystem* LoadFloorFlameSystem()
	{
		static TObjectPtr<UNiagaraSystem> CachedSystem = nullptr;
		if (!CachedSystem)
		{
			CachedSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire"));
		}
		return CachedSystem.Get();
	}
}

AT66FloorFlameTrap::AT66FloorFlameTrap()
{
	TrapTypeID = FName(TEXT("FloorFlame"));

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	DamageZone = CreateDefaultSubobject<USphereComponent>(TEXT("DamageZone"));
	DamageZone->SetupAttachment(SceneRoot);
	DamageZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageZone->SetGenerateOverlapEvents(true);

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MarkerMesh->SetupAttachment(SceneRoot);
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkerMesh->SetCastShadow(false);
}

void AT66FloorFlameTrap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateMarkerVisuals();
}

void AT66FloorFlameTrap::UpdateMarkerVisuals()
{
	if (DamageZone)
	{
		DamageZone->SetSphereRadius(Radius);
	}

	if (!MarkerMesh)
	{
		return;
	}

	if (!MarkerMesh->GetStaticMesh())
	{
		if (UStaticMesh* CylinderMesh = FT66VisualUtil::GetBasicShapeCylinder())
		{
			MarkerMesh->SetStaticMesh(CylinderMesh);
		}
	}

	MarkerMesh->SetRelativeLocation(FVector(0.f, 0.f, -10.f));
	MarkerMesh->SetRelativeScale3D(FVector(Radius / 50.f, Radius / 50.f, 0.04f));
	FT66VisualUtil::ApplyT66Color(MarkerMesh, this, FLinearColor(0.15f, 0.04f, 0.02f, 1.f));
}

void AT66FloorFlameTrap::BeginPlay()
{
	Super::BeginPlay();

	UpdateMarkerVisuals();
	CachedFireSystem = LoadFloorFlameSystem();
	ScheduleNextCycle(InitialCycleDelaySeconds);
}

void AT66FloorFlameTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WarningTimerHandle);
		World->GetTimerManager().ClearTimer(ActiveTimerHandle);
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	if (ActiveFireComponent)
	{
		ActiveFireComponent->DestroyComponent();
		ActiveFireComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AT66FloorFlameTrap::ScheduleNextCycle(const float DelaySeconds)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WarningTimerHandle,
			this,
			&AT66FloorFlameTrap::BeginWarningCycle,
			FMath::Max(DelaySeconds, 0.05f),
			false);
	}
}

void AT66FloorFlameTrap::BeginWarningCycle()
{
	if (!IsTrapEnabled())
	{
		ScheduleNextCycle(CooldownDurationSeconds);
		return;
	}

	bWarningActive = true;
	bFlamesActive = false;
	WarningElapsed = 0.f;
	WarningVFXAccum = 0.f;
	SetActorTickEnabled(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ActiveTimerHandle,
			this,
			&AT66FloorFlameTrap::ActivateFlames,
			FMath::Max(WarningDurationSeconds, 0.05f),
			false);
	}
}

void AT66FloorFlameTrap::ActivateFlames()
{
	bWarningActive = false;
	bFlamesActive = true;
	SetActorTickEnabled(false);

	if (CachedFireSystem && GetWorld())
	{
		ActiveFireComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CachedFireSystem,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector(FMath::Max(Radius / 110.f, 1.0f)),
			false,
			true,
			ENCPoolMethod::None,
			true);
	}

	SpawnActivationBurst();
	ApplyDamagePulse();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&AT66FloorFlameTrap::ApplyDamagePulse,
			FMath::Max(DamageIntervalSeconds, 0.05f),
			true,
			FMath::Max(DamageIntervalSeconds, 0.05f));

		World->GetTimerManager().SetTimer(
			ActiveTimerHandle,
			this,
			&AT66FloorFlameTrap::DeactivateFlames,
			FMath::Max(ActiveDurationSeconds, 0.05f),
			false);
	}
}

void AT66FloorFlameTrap::DeactivateFlames()
{
	bFlamesActive = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	if (ActiveFireComponent)
	{
		ActiveFireComponent->Deactivate();
		ActiveFireComponent->DestroyComponent();
		ActiveFireComponent = nullptr;
	}

	ScheduleNextCycle(CooldownDurationSeconds);
}

void AT66FloorFlameTrap::ApplyDamagePulse()
{
	if (!bFlamesActive)
	{
		return;
	}

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || !DamageZone)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	DamageZone->GetOverlappingActors(OverlappingActors, AT66HeroBase::StaticClass());
	for (AActor* OverlappingActor : OverlappingActors)
	{
		AT66HeroBase* Hero = Cast<AT66HeroBase>(OverlappingActor);
		if (!IsHeroTargetable(Hero))
		{
			continue;
		}

		RunState->ApplyDamage(DamageHP, this);
	}
}

void AT66FloorFlameTrap::SpawnActivationBurst() const
{
	UT66PixelVFXSubsystem* PixelVFX = GetWorld() ? GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>() : nullptr;
	if (!PixelVFX)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	for (int32 Index = 0; Index < 24; ++Index)
	{
		const float Angle = (static_cast<float>(Index) / 24.f) * 2.f * PI;
		const float Dist = FMath::FRandRange(0.f, Radius * 0.75f);
		const float Height = FMath::FRandRange(20.f, 220.f);
		const FVector SpawnLocation(
			Origin.X + FMath::Cos(Angle) * Dist,
			Origin.Y + FMath::Sin(Angle) * Dist,
			Origin.Z + Height);

		PixelVFX->SpawnPixelAtLocation(
			SpawnLocation,
			FLinearColor(1.f, 0.42f, 0.10f, 0.95f),
			FVector2D(4.f, 4.f),
			ET66PixelVFXPriority::Medium);
	}
}

void AT66FloorFlameTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bWarningActive)
	{
		return;
	}

	WarningElapsed += DeltaSeconds;
	WarningVFXAccum += DeltaSeconds;
	if (WarningVFXAccum < 0.08f)
	{
		return;
	}
	WarningVFXAccum = 0.f;

	UT66PixelVFXSubsystem* PixelVFX = GetWorld() ? GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>() : nullptr;
	if (!PixelVFX)
	{
		return;
	}

	const float Progress = FMath::Clamp(WarningElapsed / FMath::Max(WarningDurationSeconds, 0.05f), 0.f, 1.f);
	const float PulseScale = 1.f + 0.08f * FMath::Sin(WarningElapsed * FMath::Lerp(3.f, 8.f, Progress) * PI);
	const float RingRadius = Radius * PulseScale;
	const FVector Origin = GetActorLocation();

	for (int32 Index = 0; Index < 10; ++Index)
	{
		const float Angle = (static_cast<float>(Index) / 10.f) * 2.f * PI + (WarningElapsed * 1.4f);
		const FVector SpawnLocation(
			Origin.X + FMath::Cos(Angle) * RingRadius,
			Origin.Y + FMath::Sin(Angle) * RingRadius,
			Origin.Z + 4.f);

		PixelVFX->SpawnPixelAtLocation(
			SpawnLocation,
			FLinearColor(1.f, 0.26f, 0.08f, 0.8f),
			FVector2D(3.4f, 3.4f),
			ET66PixelVFXPriority::Low);
	}
}
