// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66FloorFlameTrap.h"

#include "Gameplay/Traps/T66TrapDamageUtils.h"

#include "Core/T66PixelVFXSubsystem.h"
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
	TrapFamilyID = FName(TEXT("FloorBurst"));

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

	FLinearColor MarkerColor = IdleMarkerColor;
	if (bFlamesActive)
	{
		MarkerColor = ActiveColor;
	}
	else if (bWarningActive)
	{
		const float Progress = FMath::Clamp(WarningElapsed / FMath::Max(ScaleTrapDuration(WarningDurationSeconds), 0.05f), 0.f, 1.f);
		MarkerColor = FMath::Lerp(IdleMarkerColor, WarningColor, Progress);
	}

	FT66VisualUtil::ApplyT66Color(MarkerMesh, this, MarkerColor);
}

void AT66FloorFlameTrap::BeginPlay()
{
	Super::BeginPlay();

	UpdateMarkerVisuals();
	CachedFireSystem = bUseFireNiagaraVFX ? LoadFloorFlameSystem() : nullptr;
	if (UsesTimedActivation())
	{
		ScheduleNextCycle(InitialCycleDelaySeconds);
	}
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
			FMath::Max(ScaleTrapDuration(DelaySeconds), 0.05f),
			false);
	}
}

void AT66FloorFlameTrap::HandleTrapEnabledChanged()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(WarningTimerHandle);
		World->GetTimerManager().ClearTimer(ActiveTimerHandle);
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	bWarningActive = false;
	bFlamesActive = false;
	WarningElapsed = 0.f;
	WarningVFXAccum = 0.f;
	ActiveVFXAccum = 0.f;
	SetActorTickEnabled(false);

	if (ActiveFireComponent)
	{
		ActiveFireComponent->Deactivate();
		ActiveFireComponent->DestroyComponent();
		ActiveFireComponent = nullptr;
	}

	UpdateMarkerVisuals();

	if (bTrapEnabled && UsesTimedActivation())
	{
		ScheduleNextCycle(0.15f);
	}
	else
	{
		ResetTriggerLock();
	}
}

void AT66FloorFlameTrap::BeginWarningCycle()
{
	if (!IsTrapEnabled())
	{
		if (UsesTimedActivation())
		{
			ScheduleNextCycle(CooldownDurationSeconds);
		}
		else
		{
			ResetTriggerLock();
		}
		return;
	}

	bWarningActive = true;
	bFlamesActive = false;
	WarningElapsed = 0.f;
	WarningVFXAccum = 0.f;
	ActiveVFXAccum = 0.f;
	SetActorTickEnabled(true);
	UpdateMarkerVisuals();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ActiveTimerHandle,
			this,
			&AT66FloorFlameTrap::ActivateFlames,
			FMath::Max(ScaleTrapDuration(WarningDurationSeconds), 0.05f),
			false);
	}
}

void AT66FloorFlameTrap::ActivateFlames()
{
	bWarningActive = false;
	bFlamesActive = true;
	ActiveVFXAccum = 0.f;
	const bool bUseNiagaraThisCycle = ShouldUseFireNiagara();
	SetActorTickEnabled(!bUseNiagaraThisCycle);
	UpdateMarkerVisuals();

	if (bUseNiagaraThisCycle && GetWorld())
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
			FMath::Max(ScaleTrapDuration(DamageIntervalSeconds), 0.05f),
			true,
			FMath::Max(ScaleTrapDuration(DamageIntervalSeconds), 0.05f));

		World->GetTimerManager().SetTimer(
			ActiveTimerHandle,
			this,
			&AT66FloorFlameTrap::DeactivateFlames,
			FMath::Max(ScaleTrapDuration(ActiveDurationSeconds), 0.05f),
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

	SetActorTickEnabled(false);
	UpdateMarkerVisuals();
	if (UsesTimedActivation())
	{
		ScheduleNextCycle(CooldownDurationSeconds);
	}
	else
	{
		ResetTriggerLock();
	}
}

void AT66FloorFlameTrap::ApplyDamagePulse()
{
	if (!bFlamesActive)
	{
		return;
	}

	if (!DamageZone)
	{
		return;
	}

	FT66TrapDamageUtils::ApplyTrapDamageToOverlaps(this, DamageZone, DamageHP);

	if (!ShouldUseFireNiagara())
	{
		SpawnActivePulseBurst();
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
			BurstColor,
			FVector2D(4.f, 4.f),
			ET66PixelVFXPriority::Medium);
	}
}

void AT66FloorFlameTrap::SpawnActivePulseBurst() const
{
	UT66PixelVFXSubsystem* PixelVFX = GetWorld() ? GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>() : nullptr;
	if (!PixelVFX)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	for (int32 Index = 0; Index < 16; ++Index)
	{
		const float Angle = (static_cast<float>(Index) / 16.f) * 2.f * PI;
		const float Dist = Radius * FMath::FRandRange(0.15f, 0.90f);
		const FVector SpawnLocation(
			Origin.X + FMath::Cos(Angle) * Dist,
			Origin.Y + FMath::Sin(Angle) * Dist,
			Origin.Z + FMath::FRandRange(15.f, 95.f));

		PixelVFX->SpawnPixelAtLocation(
			SpawnLocation,
			ActiveColor,
			FVector2D(3.6f, 3.6f),
			ET66PixelVFXPriority::Low);
	}
}

bool AT66FloorFlameTrap::ShouldUseFireNiagara() const
{
	return bUseFireNiagaraVFX && CachedFireSystem != nullptr;
}

void AT66FloorFlameTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bWarningActive && !(bFlamesActive && !ShouldUseFireNiagara()))
	{
		return;
	}

	if (bWarningActive)
	{
		WarningElapsed += DeltaSeconds;
		WarningVFXAccum += DeltaSeconds;
		UpdateMarkerVisuals();
	}

	if (bFlamesActive && !ShouldUseFireNiagara())
	{
		ActiveVFXAccum += DeltaSeconds;
	}

	const bool bEmitWarning = bWarningActive && WarningVFXAccum >= 0.08f;
	const bool bEmitActive = bFlamesActive && !ShouldUseFireNiagara() && ActiveVFXAccum >= 0.08f;
	if (!bEmitWarning && !bEmitActive)
	{
		return;
	}

	UT66PixelVFXSubsystem* PixelVFX = GetWorld() ? GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>() : nullptr;
	if (!PixelVFX)
	{
		return;
	}

	const FVector Origin = GetActorLocation();

	if (bEmitWarning)
	{
		WarningVFXAccum = 0.f;
		const float Progress = FMath::Clamp(WarningElapsed / FMath::Max(ScaleTrapDuration(WarningDurationSeconds), 0.05f), 0.f, 1.f);
		const float PulseScale = 1.f + 0.08f * FMath::Sin(WarningElapsed * FMath::Lerp(3.f, 8.f, Progress) * PI);
		const float RingRadius = Radius * PulseScale;

		for (int32 Index = 0; Index < 10; ++Index)
		{
			const float Angle = (static_cast<float>(Index) / 10.f) * 2.f * PI + (WarningElapsed * 1.4f);
			const FVector SpawnLocation(
				Origin.X + FMath::Cos(Angle) * RingRadius,
				Origin.Y + FMath::Sin(Angle) * RingRadius,
				Origin.Z + 4.f);

			PixelVFX->SpawnPixelAtLocation(
				SpawnLocation,
				WarningColor,
				FVector2D(3.4f, 3.4f),
				ET66PixelVFXPriority::Low);
		}
	}

	if (bEmitActive)
	{
		ActiveVFXAccum = 0.f;
		for (int32 Index = 0; Index < 8; ++Index)
		{
			const float Angle = (static_cast<float>(Index) / 8.f) * 2.f * PI + (GetWorld()->TimeSeconds * 1.8f);
			const FVector SpawnLocation(
				Origin.X + FMath::Cos(Angle) * Radius * 0.65f,
				Origin.Y + FMath::Sin(Angle) * Radius * 0.65f,
				Origin.Z + FMath::FRandRange(15.f, 90.f));

			PixelVFX->SpawnPixelAtLocation(
				SpawnLocation,
				ActiveColor,
				FVector2D(3.2f, 3.2f),
				ET66PixelVFXPriority::Low);
		}
	}
}

bool AT66FloorFlameTrap::CanAcceptExternalTrigger() const
{
	return AT66TrapBase::CanAcceptExternalTrigger() && !bWarningActive && !bFlamesActive;
}

bool AT66FloorFlameTrap::HandleTrapTriggered(AActor* TriggeringActor)
{
	(void)TriggeringActor;

	if (bWarningActive || bFlamesActive)
	{
		return false;
	}

	BeginWarningCycle();
	return true;
}
