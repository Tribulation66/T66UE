// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66FloorSpikePatchTrap.h"

#include "Gameplay/Traps/T66TrapDamageUtils.h"

#include "Core/T66PixelVFXSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	UStaticMesh* LoadFloorSpikeMesh()
	{
		static TSoftObjectPtr<UStaticMesh> Mesh(FSoftObjectPath(TEXT("/Game/Stylized_VFX_StPack/Meshes/SM_Spike.SM_Spike")));
		return Mesh.LoadSynchronous();
	}

	UStaticMesh* LoadFloorSpikeClusterMesh()
	{
		static TSoftObjectPtr<UStaticMesh> Mesh(FSoftObjectPath(TEXT("/Game/Stylized_VFX_StPack/Meshes/SM_Spikes.SM_Spikes")));
		return Mesh.LoadSynchronous();
	}

	UNiagaraSystem* LoadFloorSpikeBurstSystem()
	{
		static TSoftObjectPtr<UNiagaraSystem> System(FSoftObjectPath(TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02")));
		return System.LoadSynchronous();
	}
}

AT66FloorSpikePatchTrap::AT66FloorSpikePatchTrap()
{
	TrapTypeID = FName(TEXT("FloorSpikePatch"));
	TrapFamilyID = FName(TEXT("AreaControl"));
	ActivationMode = ET66TrapActivationMode::Triggered;
	TriggerTargetMode = ET66TrapTriggerTarget::HeroesAndEnemies;

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

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(SceneRoot);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseMesh->SetCastShadow(true);

	SpikeInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SpikeInstances"));
	SpikeInstances->SetupAttachment(SceneRoot);
	SpikeInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpikeInstances->SetCastShadow(true);
}

void AT66FloorSpikePatchTrap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildSpikes();
	UpdateMarkerVisuals();
	UpdateSpikeTransforms(0.f);
}

void AT66FloorSpikePatchTrap::BeginPlay()
{
	Super::BeginPlay();
	RebuildSpikes();
	CachedRiseBurstSystem = LoadFloorSpikeBurstSystem();
	UpdateMarkerVisuals();
	UpdateSpikeTransforms(0.f);
	if (UsesTimedActivation())
	{
		ScheduleNextCycle(InitialCycleDelaySeconds);
	}
}

void AT66FloorSpikePatchTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WarningTimerHandle);
		World->GetTimerManager().ClearTimer(RaiseTimerHandle);
		World->GetTimerManager().ClearTimer(ActiveTimerHandle);
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66FloorSpikePatchTrap::ScheduleNextCycle(const float DelaySeconds)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WarningTimerHandle,
			this,
			&AT66FloorSpikePatchTrap::BeginWarningCycle,
			FMath::Max(ScaleTrapDuration(DelaySeconds), 0.05f),
			false);
	}
}

void AT66FloorSpikePatchTrap::HandleTrapEnabledChanged()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(WarningTimerHandle);
		World->GetTimerManager().ClearTimer(RaiseTimerHandle);
		World->GetTimerManager().ClearTimer(ActiveTimerHandle);
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	bWarningActive = false;
	bRising = false;
	bRaised = false;
	bRetracting = false;
	PhaseElapsed = 0.f;
	WarningVFXAccum = 0.f;
	SetActorTickEnabled(false);
	UpdateSpikeTransforms(0.f);
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

void AT66FloorSpikePatchTrap::BeginWarningCycle()
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
	bRising = false;
	bRaised = false;
	bRetracting = false;
	PhaseElapsed = 0.f;
	WarningVFXAccum = 0.f;
	SetActorTickEnabled(true);
	UpdateMarkerVisuals();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RaiseTimerHandle,
			this,
			&AT66FloorSpikePatchTrap::BeginRiseCycle,
			FMath::Max(ScaleTrapDuration(WarningDurationSeconds), 0.05f),
			false);
	}
}

void AT66FloorSpikePatchTrap::BeginRiseCycle()
{
	bWarningActive = false;
	bRising = true;
	bRetracting = false;
	bRaised = false;
	PhaseElapsed = 0.f;
	UpdateMarkerVisuals();
	SpawnRiseNiagaraBurst();
	SpawnRiseBurst();
	SetActorTickEnabled(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ActiveTimerHandle,
			this,
			&AT66FloorSpikePatchTrap::ActivatePatch,
			FMath::Max(ScaleTrapDuration(RiseDurationSeconds), 0.05f),
			false);
	}
}

void AT66FloorSpikePatchTrap::ActivatePatch()
{
	bRising = false;
	bRaised = true;
	bRetracting = false;
	PhaseElapsed = 0.f;
	UpdateSpikeTransforms(1.f);
	UpdateMarkerVisuals();
	ApplyDamagePulse();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&AT66FloorSpikePatchTrap::ApplyDamagePulse,
			FMath::Max(ScaleTrapDuration(DamageIntervalSeconds), 0.05f),
			true,
			FMath::Max(ScaleTrapDuration(DamageIntervalSeconds), 0.05f));

		World->GetTimerManager().SetTimer(
			ActiveTimerHandle,
			this,
			&AT66FloorSpikePatchTrap::BeginRetractCycle,
			FMath::Max(ScaleTrapDuration(RaisedDurationSeconds), 0.05f),
			false);
	}
}

void AT66FloorSpikePatchTrap::BeginRetractCycle()
{
	bWarningActive = false;
	bRising = false;
	bRaised = false;
	bRetracting = true;
	PhaseElapsed = 0.f;
	UpdateMarkerVisuals();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
		World->GetTimerManager().SetTimer(
			ActiveTimerHandle,
			this,
			&AT66FloorSpikePatchTrap::FinishRetractCycle,
			FMath::Max(ScaleTrapDuration(RetractDurationSeconds), 0.05f),
			false);
	}
}

void AT66FloorSpikePatchTrap::FinishRetractCycle()
{
	bRetracting = false;
	UpdateSpikeTransforms(0.f);
	UpdateMarkerVisuals();
	SetActorTickEnabled(false);
	if (UsesTimedActivation())
	{
		ScheduleNextCycle(CooldownDurationSeconds);
	}
	else
	{
		ResetTriggerLock();
	}
}

void AT66FloorSpikePatchTrap::ApplyDamagePulse()
{
	if (!bRaised)
	{
		return;
	}

	if (!DamageZone)
	{
		return;
	}

	FT66TrapDamageUtils::ApplyTrapDamageToOverlaps(this, DamageZone, DamageHP);
}

bool AT66FloorSpikePatchTrap::ShouldTickForAnimation() const
{
	return bWarningActive || bRising || bRetracting;
}

void AT66FloorSpikePatchTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ShouldTickForAnimation())
	{
		return;
	}

	PhaseElapsed += DeltaSeconds;

	if (bWarningActive)
	{
		WarningVFXAccum += DeltaSeconds;
		UpdateMarkerVisuals();
		if (WarningVFXAccum >= 0.08f)
		{
			WarningVFXAccum = 0.f;
			UT66PixelVFXSubsystem* PixelVFX = GetWorld() ? GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>() : nullptr;
			if (PixelVFX)
			{
				const FVector Origin = GetActorLocation();
				const float Progress = FMath::Clamp(PhaseElapsed / FMath::Max(ScaleTrapDuration(WarningDurationSeconds), 0.05f), 0.f, 1.f);
				const float RingRadius = Radius * (1.f + 0.05f * FMath::Sin(PhaseElapsed * 8.f));
				for (int32 Index = 0; Index < 12; ++Index)
				{
					const float Angle = (static_cast<float>(Index) / 12.f) * 2.f * PI;
					const FVector SpawnLocation(
						Origin.X + FMath::Cos(Angle) * RingRadius,
						Origin.Y + FMath::Sin(Angle) * RingRadius,
						Origin.Z + 4.f + Progress * 4.f);

					PixelVFX->SpawnPixelAtLocation(
						SpawnLocation,
						WarningColor,
						FVector2D(3.6f, 3.6f),
						ET66PixelVFXPriority::Low);
				}
			}
		}
	}

	if (bRising)
	{
		const float RaisedAlpha = FMath::Clamp(PhaseElapsed / FMath::Max(ScaleTrapDuration(RiseDurationSeconds), 0.05f), 0.f, 1.f);
		UpdateSpikeTransforms(RaisedAlpha);
	}
	else if (bRetracting)
	{
		const float RaisedAlpha = 1.f - FMath::Clamp(PhaseElapsed / FMath::Max(ScaleTrapDuration(RetractDurationSeconds), 0.05f), 0.f, 1.f);
		UpdateSpikeTransforms(RaisedAlpha);
	}
}

void AT66FloorSpikePatchTrap::RebuildSpikes()
{
	if (DamageZone)
	{
		DamageZone->SetSphereRadius(Radius);
	}

	if (MarkerMesh && !MarkerMesh->GetStaticMesh())
	{
		MarkerMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCylinder());
	}

	if (BaseMesh && !BaseMesh->GetStaticMesh())
	{
		BaseMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCylinder());
	}

	if (SpikeInstances && !SpikeInstances->GetStaticMesh())
	{
		if (UStaticMesh* SpikeMesh = LoadFloorSpikeMesh())
		{
			SpikeInstances->SetStaticMesh(SpikeMesh);
		}
		else if (UStaticMesh* SpikeClusterMesh = LoadFloorSpikeClusterMesh())
		{
			SpikeInstances->SetStaticMesh(SpikeClusterMesh);
		}
		else if (UStaticMesh* ConeMesh = FT66VisualUtil::GetBasicShapeCone())
		{
			SpikeInstances->SetStaticMesh(ConeMesh);
		}
	}

	if (!SpikeInstances || !SpikeInstances->GetStaticMesh())
	{
		return;
	}

	SpikeDynamicMaterials.Reset();
	SpikeInstances->ClearInstances();
	SpikeCount = FMath::Max(3, SpikeCount);
	for (int32 Index = 0; Index < SpikeCount; ++Index)
	{
		SpikeInstances->AddInstance(FTransform::Identity);
	}
}

void AT66FloorSpikePatchTrap::UpdateMarkerVisuals()
{
	if (BaseMesh)
	{
		BaseMesh->SetRelativeLocation(FVector(0.f, 0.f, -7.f));
		BaseMesh->SetRelativeScale3D(FVector(Radius / 48.f, Radius / 48.f, 0.05f));
		FT66VisualUtil::ApplyT66Color(BaseMesh, this, FLinearColor(0.07f, 0.06f, 0.06f, 1.f));
	}

	if (MarkerMesh)
	{
		MarkerMesh->SetRelativeLocation(FVector(0.f, 0.f, -11.f));
		MarkerMesh->SetRelativeScale3D(FVector((Radius / 50.f) * 1.04f, (Radius / 50.f) * 1.04f, 0.02f));

		FLinearColor MarkerColor = IdleMarkerColor;
		if (bWarningActive)
		{
			const float Progress = FMath::Clamp(PhaseElapsed / FMath::Max(ScaleTrapDuration(WarningDurationSeconds), 0.05f), 0.f, 1.f);
			MarkerColor = FMath::Lerp(IdleMarkerColor, WarningColor, Progress);
		}
		else if (bRaised || bRising || bRetracting)
		{
			MarkerColor = BurstColor;
		}

		FT66VisualUtil::ApplyT66Color(MarkerMesh, this, MarkerColor);
	}

	if (SpikeInstances)
	{
		if (SpikeInstances->GetStaticMesh() != FT66VisualUtil::GetBasicShapeCone())
		{
			SpikeDynamicMaterials.Reset();
			SpikeInstances->EmptyOverrideMaterials();
			return;
		}

		if (SpikeDynamicMaterials.Num() != SpikeInstances->GetNumMaterials())
		{
			SpikeDynamicMaterials.Reset();
			for (int32 MaterialIndex = 0; MaterialIndex < SpikeInstances->GetNumMaterials(); ++MaterialIndex)
			{
				if (UMaterialInstanceDynamic* Material = SpikeInstances->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
				{
					SpikeDynamicMaterials.Add(Material);
				}
			}
		}

		for (UMaterialInstanceDynamic* Material : SpikeDynamicMaterials)
		{
			if (Material)
			{
				FT66VisualUtil::ConfigureFlatColorMaterial(Material, SpikeColor);
			}
		}
	}
}

void AT66FloorSpikePatchTrap::UpdateSpikeTransforms(const float RaisedAlpha)
{
	if (!SpikeInstances || SpikeInstances->GetInstanceCount() <= 0)
	{
		return;
	}

	const int32 InstanceCount = SpikeInstances->GetInstanceCount();
	const bool bUsingImportedSpikeMesh = SpikeInstances->GetStaticMesh() != FT66VisualUtil::GetBasicShapeCone();
	const float HiddenZ = bUsingImportedSpikeMesh ? (-SpikeHeight * 0.88f) : (-SpikeHeight * 0.55f);
	const float VisibleZ = bUsingImportedSpikeMesh ? (SpikeHeight * 0.10f) : (SpikeHeight * 0.35f);
	const float BaseXYScale = bUsingImportedSpikeMesh ? 0.52f : 0.40f;
	const float BaseZScale = bUsingImportedSpikeMesh ? (SpikeHeight / 185.f) : (SpikeHeight / 100.f);
	for (int32 Index = 0; Index < InstanceCount; ++Index)
	{
		const float Angle = ((static_cast<float>(Index) + 0.5f) / static_cast<float>(InstanceCount)) * 2.f * PI + (static_cast<float>(Index % 3) * 0.16f);
		const float Dist = Radius * (Index % 3 == 0 ? 0.34f : (Index % 3 == 1 ? 0.56f : 0.78f));
		const float ScaleJitter = FMath::Lerp(0.92f, 1.14f, static_cast<float>(Index % 4) / 3.f);
		const FVector Location(
			FMath::Cos(Angle) * Dist,
			FMath::Sin(Angle) * Dist,
			FMath::Lerp(HiddenZ, VisibleZ, RaisedAlpha));
		const FRotator Rotation = bUsingImportedSpikeMesh
			? FRotator(0.f, FMath::RadiansToDegrees(Angle) + (Index % 2 == 0 ? 14.f : -14.f), 0.f)
			: FRotator(0.f, FMath::RadiansToDegrees(Angle), -90.f);
		const FVector Scale(
			BaseXYScale * ScaleJitter,
			BaseXYScale * ScaleJitter,
			FMath::Max(0.10f, RaisedAlpha) * BaseZScale);
		SpikeInstances->UpdateInstanceTransform(Index, FTransform(Rotation, Location, Scale), false, true, true);
	}

	SpikeInstances->MarkRenderStateDirty();
}

void AT66FloorSpikePatchTrap::SpawnRiseBurst() const
{
	UT66PixelVFXSubsystem* PixelVFX = GetWorld() ? GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>() : nullptr;
	if (!PixelVFX)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	for (int32 Index = 0; Index < 20; ++Index)
	{
		const float Angle = (static_cast<float>(Index) / 20.f) * 2.f * PI;
		const float Dist = Radius * FMath::FRandRange(0.20f, 0.85f);
		const FVector SpawnLocation(
			Origin.X + FMath::Cos(Angle) * Dist,
			Origin.Y + FMath::Sin(Angle) * Dist,
			Origin.Z + FMath::FRandRange(10.f, SpikeHeight * 0.7f));

		PixelVFX->SpawnPixelAtLocation(
			SpawnLocation,
			BurstColor,
			FVector2D(3.8f, 3.8f),
			ET66PixelVFXPriority::Medium);
	}
}

void AT66FloorSpikePatchTrap::SpawnRiseNiagaraBurst() const
{
	if (!CachedRiseBurstSystem || !GetWorld())
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CachedRiseBurstSystem,
		GetActorLocation(),
		FRotator::ZeroRotator,
		FVector(FMath::Max(Radius / 170.f, 0.95f)),
		false,
		true,
		ENCPoolMethod::AutoRelease,
		true);
}

bool AT66FloorSpikePatchTrap::CanAcceptExternalTrigger() const
{
	return AT66TrapBase::CanAcceptExternalTrigger() && !bWarningActive && !bRising && !bRaised && !bRetracting;
}

bool AT66FloorSpikePatchTrap::HandleTrapTriggered(AActor* TriggeringActor)
{
	(void)TriggeringActor;

	if (bWarningActive || bRising || bRaised || bRetracting)
	{
		return false;
	}

	BeginWarningCycle();
	return true;
}
