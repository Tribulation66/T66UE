// Copyright Tribulation 66. All Rights Reserved.

#include "PerformanceSystem/T66OutgoingTravelerStressHarnessActor.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66GameInstance.h"
#include "Camera/PlayerCameraManager.h"
#include "Dom/JsonObject.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66OutgoingTravelerPoolSubsystem.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RHI.h"
#include "RenderTimer.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66OutgoingTravelerStress, Log, All);

namespace
{
	constexpr int32 T66OutgoingTravelerStressSchemaVersion = 4;

	bool ParseCommandLineBool(const TCHAR* Key, const bool DefaultValue)
	{
		int32 Value = DefaultValue ? 1 : 0;
		if (FParse::Value(FCommandLine::Get(), Key, Value))
		{
			return Value != 0;
		}
		return FParse::Param(FCommandLine::Get(), Key) || DefaultValue;
	}

	FString FormatMaybeNumber(const AT66OutgoingTravelerStressHarnessActor::FMetricAccumulator& Metric)
	{
		return Metric.Count > 0 ? FString::Printf(TEXT("%.4f"), Metric.Average()) : TEXT("Unavailable");
	}

	void SetMetricObject(
		const TSharedRef<FJsonObject>& Root,
		const TCHAR* FieldName,
		const AT66OutgoingTravelerStressHarnessActor::FMetricAccumulator& Metric)
	{
		const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("sample_count"), Metric.Count);
		if (Metric.Count > 0)
		{
			Object->SetNumberField(TEXT("avg"), Metric.Average());
			Object->SetNumberField(TEXT("min"), Metric.Min);
			Object->SetNumberField(TEXT("max"), Metric.Max);
		}
		else
		{
			Object->SetStringField(TEXT("avg"), TEXT("Unavailable"));
			Object->SetStringField(TEXT("min"), TEXT("Unavailable"));
			Object->SetStringField(TEXT("max"), TEXT("Unavailable"));
		}
		Root->SetObjectField(FieldName, Object);
	}

	FLinearColor TravelerVisualProfileStressColor(const FName VisualProfileID)
	{
		const FString Name = VisualProfileID.ToString();
		if (Name.Contains(TEXT(".Fire.")))
		{
			return FLinearColor(1.0f, 0.18f, 0.03f, 1.0f);
		}
		if (Name.Contains(TEXT(".Ice.")))
		{
			return FLinearColor(0.35f, 0.82f, 1.0f, 1.0f);
		}
		if (Name.Contains(TEXT(".Electricity.")))
		{
			return FLinearColor(1.0f, 0.92f, 0.10f, 1.0f);
		}
		if (Name.Contains(TEXT(".Nature.")))
		{
			return FLinearColor(0.20f, 0.95f, 0.34f, 1.0f);
		}
		return FT66TemporaryProjectileSystem::HeroProjectileColor();
	}

	bool DoesVisualProfileMatchFamilyFilter(const FName VisualProfileID, const FString& FamilyFilter)
	{
		if (FamilyFilter.IsEmpty() || FamilyFilter.Equals(TEXT("all"), ESearchCase::IgnoreCase))
		{
			return true;
		}

		const FString Name = VisualProfileID.ToString();
		if (FamilyFilter.Equals(TEXT("additive"), ESearchCase::IgnoreCase))
		{
			return Name.Contains(TEXT(".Fire.")) || Name.Contains(TEXT(".Electricity."));
		}
		if (FamilyFilter.Equals(TEXT("translucent"), ESearchCase::IgnoreCase))
		{
			return Name.Contains(TEXT(".Ice.")) || Name.Contains(TEXT(".Nature."));
		}
		return Name.Contains(FamilyFilter, ESearchCase::IgnoreCase);
	}
}

AT66OutgoingTravelerStressHarnessActor::AT66OutgoingTravelerStressHarnessActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AT66OutgoingTravelerStressHarnessActor::FMetricAccumulator::Add(const double Value)
{
	if (!FMath::IsFinite(Value))
	{
		return;
	}

	++Count;
	Sum += Value;
	Min = Count == 1 ? Value : FMath::Min(Min, Value);
	Max = Count == 1 ? Value : FMath::Max(Max, Value);
}

double AT66OutgoingTravelerStressHarnessActor::FMetricAccumulator::Average() const
{
	return Count > 0 ? Sum / static_cast<double>(Count) : 0.0;
}

void AT66OutgoingTravelerStressHarnessActor::StartFromCommandLine(APlayerController* InOwnerController)
{
	OwnerController = InOwnerController;

	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressCount="), RequestedCount);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressTargetCount="), TargetCount);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressGridColumns="), GridColumns);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressSpacing="), GridSpacing);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressSpawnDistance="), SpawnDistance);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressTravelerSpeed="), TravelerSpeed);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressVisualScaleMultiplier="), VisualScaleMultiplier);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressProofCameraDistance="), ProofCameraDistance);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressProofCameraFOV="), ProofCameraFOV);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressProofLaneCenterX="), ProofLaneCenter.X);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressProofLaneCenterY="), ProofLaneCenter.Y);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressProofLaneCenterZ="), ProofLaneCenter.Z);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressProofLaneYaw="), ProofLaneYaw);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressProofLanePitch="), ProofLanePitch);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressWarmupSeconds="), WarmupSeconds);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressSampleSeconds="), SampleSeconds);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressManifest="), ManifestPath);
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerStressVisualProfileFamily="), VisualProfileFamilyFilter);
	bUsePool = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressUsePool="), true);
	bDisableCollisionForRenderIsolation = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressDisableCollision="), true);
	bUseTargetSnapshot = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressTargetSnapshot="), false);
	bEnableArrivalCollision = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressArrivalCollision="), false);
	bBindArrivalCallback = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressBindArrivalCallback="), false);
	bUseMixedVisualProfiles = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressMixedVisualProfiles="), false);
	bUseProofCamera = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressProofCamera="), false);
	bHideHeroForProof = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressHideHeroForProof="), false);
	bUseFixedProofLane = ParseCommandLineBool(TEXT("T66OutgoingTravelerStressFixedProofLane="), true);
	if (bEnableArrivalCollision)
	{
		bUseTargetSnapshot = true;
	}
	if (bBindArrivalCallback)
	{
		bUseTargetSnapshot = true;
	}

	RequestedCount = FMath::Clamp(RequestedCount, 1, UT66OutgoingTravelerPoolSubsystem::MaxOutgoingTravelers);
	TargetCount = FMath::Clamp(TargetCount, 1, 512);
	GridSpacing = FMath::Clamp(GridSpacing, 2.0f, 200.0f);
	SpawnDistance = FMath::Clamp(SpawnDistance, 250.0f, 8000.0f);
	TravelerSpeed = FMath::Clamp(TravelerSpeed, 1.0f, 20000.0f);
	VisualScaleMultiplier = FMath::Clamp(VisualScaleMultiplier, 0.1f, 12.0f);
	ProofCameraDistance = FMath::Clamp(ProofCameraDistance, 120.0f, 8000.0f);
	ProofCameraFOV = FMath::Clamp(ProofCameraFOV, 5.0f, 120.0f);
	WarmupSeconds = FMath::Clamp(WarmupSeconds, 0.0f, 30.0f);
	SampleSeconds = FMath::Clamp(SampleSeconds, 0.25f, 60.0f);
	if (GridColumns <= 0)
	{
		GridColumns = FMath::Clamp(FMath::CeilToInt(FMath::Sqrt(static_cast<float>(RequestedCount))), 1, 160);
	}
	else
	{
		GridColumns = FMath::Clamp(GridColumns, 1, 512);
	}

	if (IConsoleVariable* PoolEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.OutgoingTravelerPool.Enabled")))
	{
		PoolEnabledCVar->Set(bUsePool ? 1 : 0, ECVF_SetByCode);
	}

	if (ManifestPath.IsEmpty())
	{
		ManifestPath = FPaths::Combine(
			FPaths::ProjectSavedDir(),
			TEXT("PerformanceSystem"),
			TEXT("OutgoingTravelerStress"),
			FString::Printf(TEXT("%s_%d.json"), bUsePool ? TEXT("pooled") : TEXT("individual_mesh"), RequestedCount));
	}
	ManifestPath = FPaths::ConvertRelativePathToFull(ManifestPath);

	SpawnTravelers();
	StartSeconds = FPlatformTime::Seconds();
	bStarted = true;

	UE_LOG(LogT66OutgoingTravelerStress, Display,
		TEXT("[OutgoingTravelerStress] started config=%s requested=%d spawned=%d manifest=%s"),
		bUsePool ? TEXT("pooled") : TEXT("individual_mesh"),
		RequestedCount,
		SpawnedCount,
		*ManifestPath);
}

void AT66OutgoingTravelerStressHarnessActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bStarted || bManifestWritten)
	{
		return;
	}

	const double ElapsedSeconds = FPlatformTime::Seconds() - StartSeconds;
	if (ElapsedSeconds >= WarmupSeconds)
	{
		SampleFrame(DeltaSeconds);
	}

	if (ElapsedSeconds >= WarmupSeconds + SampleSeconds)
	{
		WriteManifest(TEXT("sample-window-complete"));
		bManifestWritten = true;
	}
}

void AT66OutgoingTravelerStressHarnessActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bStarted && !bManifestWritten)
	{
		WriteManifest(TEXT("endplay-before-sample-complete"));
		bManifestWritten = true;
	}
	const FString PrimaryManifestPath = ManifestPath;
	DestroyTravelers();
	if (ACameraActor* CameraActor = ProofCameraActor.Get())
	{
		CameraActor->Destroy();
	}
	ProofCameraActor.Reset();
	if (bStarted && bUsePool && !PrimaryManifestPath.IsEmpty())
	{
		ManifestPath = PrimaryManifestPath + TEXT(".cleanup.json");
		WriteManifest(TEXT("cleanup-after-destroy"));
		ManifestPath = PrimaryManifestPath;
	}
	Super::EndPlay(EndPlayReason);
}

void AT66OutgoingTravelerStressHarnessActor::SpawnStressTargets(const FVector& Center, const FVector& Right, const FVector& Up)
{
	UWorld* World = GetWorld();
	if (!World || TargetEnemies.Num() > 0)
	{
		return;
	}

	const int32 TargetColumns = FMath::Clamp(FMath::CeilToInt(FMath::Sqrt(static_cast<float>(TargetCount))), 1, 64);
	const int32 TargetRows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(TargetCount) / static_cast<float>(TargetColumns)));
	const float TargetSpacing = 180.0f;
	const FVector TargetOrigin = Center
		- Right * (static_cast<float>(TargetColumns - 1) * TargetSpacing * 0.5f)
		- Up * (static_cast<float>(TargetRows - 1) * TargetSpacing * 0.5f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TargetEnemies.Reserve(TargetCount);
	for (int32 Index = 0; Index < TargetCount; ++Index)
	{
		const int32 Column = Index % TargetColumns;
		const int32 Row = Index / TargetColumns;
		const FVector Location = TargetOrigin
			+ Right * (static_cast<float>(Column) * TargetSpacing)
			+ Up * (static_cast<float>(Row) * TargetSpacing);

		AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
			AT66EnemyBase::StaticClass(),
			Location,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!Enemy)
		{
			continue;
		}

		Enemy->MobID = FName(TEXT("OutgoingTravelerStressTarget"));
		Enemy->MaxHP = 100000000;
		Enemy->CurrentHP = Enemy->MaxHP;
		Enemy->SetActorHiddenInGame(true);
		Enemy->SetActorEnableCollision(false);
		Enemy->SetActorTickEnabled(false);
		TargetEnemies.Add(Enemy);
	}
}

void AT66OutgoingTravelerStressHarnessActor::ConfigureProofCamera(const FVector& GridCenter, const FVector& ViewForward)
{
	UWorld* World = GetWorld();
	if (!World || !OwnerController)
	{
		return;
	}

	const FVector SafeForward = ViewForward.GetSafeNormal();
	if (SafeForward.IsNearlyZero())
	{
		return;
	}

	const FVector CameraLocation = GridCenter - SafeForward * ProofCameraDistance;
	const FRotator CameraRotation = (GridCenter - CameraLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerController;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACameraActor* CameraActor = World->SpawnActor<ACameraActor>(
		ACameraActor::StaticClass(),
		CameraLocation,
		CameraRotation,
		SpawnParams);
	if (!CameraActor)
	{
		return;
	}

	CameraActor->GetCameraComponent()->SetFieldOfView(ProofCameraFOV);
	ProofCameraActor = CameraActor;
	OwnerController->SetViewTarget(CameraActor);

	if (bHideHeroForProof)
	{
		if (APawn* Pawn = OwnerController->GetPawn())
		{
			Pawn->SetActorHiddenInGame(true);
		}
	}

	UE_LOG(LogT66OutgoingTravelerStress, Display,
		TEXT("[OutgoingTravelerStress] proof_camera enabled location=%s rotation=%s target=%s fov=%.1f hideHero=%d"),
		*CameraLocation.ToCompactString(),
		*CameraRotation.ToCompactString(),
		*GridCenter.ToCompactString(),
		ProofCameraFOV,
		bHideHeroForProof ? 1 : 0);
}

void AT66OutgoingTravelerStressHarnessActor::SpawnTravelers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const double SpawnStartSeconds = FPlatformTime::Seconds();
	FRotator CameraRotation = OwnerController && OwnerController->PlayerCameraManager
		? OwnerController->PlayerCameraManager->GetCameraRotation()
		: FRotator(-20.0f, 0.0f, 0.0f);
	FVector CameraLocation = OwnerController && OwnerController->PlayerCameraManager
		? OwnerController->PlayerCameraManager->GetCameraLocation()
		: FVector(-2400.0f, 0.0f, 800.0f);
	if (bUseProofCamera && bUseFixedProofLane)
	{
		CameraRotation = FRotator(ProofLanePitch, ProofLaneYaw, 0.0f);
		CameraLocation = ProofLaneCenter - CameraRotation.Vector().GetSafeNormal() * SpawnDistance;
	}
	const FVector Forward = CameraRotation.Vector().GetSafeNormal();
	const FVector Right = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);
	const FVector Up = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Z);
	const int32 Rows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(RequestedCount) / static_cast<float>(GridColumns)));
	const FVector GridCenter = (bUseProofCamera && bUseFixedProofLane)
		? ProofLaneCenter
		: CameraLocation + Forward * SpawnDistance;
	const FVector Origin = GridCenter
		- Right * (static_cast<float>(GridColumns - 1) * GridSpacing * 0.5f)
		- Up * (static_cast<float>(Rows - 1) * GridSpacing * 0.5f);
	if (bUsePool && bUseTargetSnapshot)
	{
		SpawnStressTargets(GridCenter, Right, Up);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerController ? Cast<AActor>(OwnerController) : this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Projectiles.Reserve(RequestedCount);
	TravelerHandles.Reserve(RequestedCount);

	if (bUsePool)
	{
		UT66OutgoingTravelerPoolSubsystem* Pool = World->GetSubsystem<UT66OutgoingTravelerPoolSubsystem>();
		if (!Pool)
		{
			FailedSpawnCount = RequestedCount;
			SpawnTimeMs = (FPlatformTime::Seconds() - SpawnStartSeconds) * 1000.0;
			return;
		}

		TArray<FName> KnownVisualProfiles;
		if (bUseMixedVisualProfiles)
		{
			UT66OutgoingTravelerPoolSubsystem::AppendKnownTravelerVisualProfileIDs(KnownVisualProfiles);
			if (!VisualProfileFamilyFilter.IsEmpty() && !VisualProfileFamilyFilter.Equals(TEXT("all"), ESearchCase::IgnoreCase))
			{
				KnownVisualProfiles = KnownVisualProfiles.FilterByPredicate(
					[this](const FName& VisualProfileID)
					{
						return DoesVisualProfileMatchFamilyFilter(VisualProfileID, VisualProfileFamilyFilter);
					});
			}
			VisualProfilesUsed.Reset();
		}

		for (int32 Index = 0; Index < RequestedCount; ++Index)
		{
			const int32 Column = Index % GridColumns;
			const int32 Row = Index / GridColumns;
			const FVector Location = Origin + Right * (static_cast<float>(Column) * GridSpacing) + Up * (static_cast<float>(Row) * GridSpacing);
			AT66EnemyBase* TargetEnemy = TargetEnemies.Num() > 0
				? TargetEnemies[Index % TargetEnemies.Num()].Get()
				: nullptr;
			const FVector TargetLocation = TargetEnemy
				? TargetEnemy->GetActorLocation() + FVector(0.0f, 0.0f, 80.0f)
				: Location + Forward * 100000.0f;
			const float ArrivalDistance = bEnableArrivalCollision
				? FMath::Lerp(800.0f, 5600.0f, static_cast<float>(Index % 257) / 256.0f)
				: 100000.0f;
			const FVector StartLocation = bEnableArrivalCollision && TargetEnemy
				? TargetLocation - Forward * ArrivalDistance + Right * (static_cast<float>(Column) * GridSpacing * 0.1f)
				: Location;

			FT66OutgoingTravelerFireParams Params;
			Params.StartPosition = StartLocation;
			Params.TargetPosition = TargetLocation;
			Params.ProfileID = FT66TemporaryProjectileSystem::ProfileHeroAOE();
			Params.Color = FT66TemporaryProjectileSystem::HeroProjectileColor();
			if (KnownVisualProfiles.Num() > 0)
			{
				const FName VisualProfileID = KnownVisualProfiles[Index % KnownVisualProfiles.Num()];
				Params.TravelerVisualProfileID = VisualProfileID;
				Params.Color = TravelerVisualProfileStressColor(VisualProfileID);
				VisualProfilesUsed.AddUnique(VisualProfileID);
			}
			Params.ScaleMultiplier = VisualScaleMultiplier;
			Params.Speed = TravelerSpeed;
			Params.LifetimeSeconds = WarmupSeconds + SampleSeconds + 10.0f;
			Params.bTrackTarget = bUseTargetSnapshot && TargetEnemy != nullptr;
			Params.bEnableArrivalCollision = bEnableArrivalCollision;
			Params.bApplyDamageOnArrival = bEnableArrivalCollision || bBindArrivalCallback;
			Params.DamageAmount = (bEnableArrivalCollision || bBindArrivalCallback) ? 1 : 0;
			Params.ArrivalRadius = bEnableArrivalCollision ? 24.0f : 0.0f;
			Params.DamageSourceID = FName(TEXT("OutgoingTravelerStress"));
			Params.EventType = bBindArrivalCallback
				? FName(TEXT("ArrivalCallback"))
				: (bEnableArrivalCollision ? FName(TEXT("ArrivalCollision")) : NAME_None);
			if (TargetEnemy)
			{
				Params.TargetHandle = TargetEnemy->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body);
				Params.TargetPosition = Params.TargetHandle.AimPoint.IsNearlyZero()
					? TargetLocation
					: Params.TargetHandle.AimPoint;
				Params.TargetOffset = Params.TargetPosition - TargetEnemy->GetActorLocation();
			}

			FT66OutgoingTravelerHandle Handle;
			FT66OutgoingTravelerArrivalCallback OnArrived;
			if (bBindArrivalCallback)
			{
				OnArrived.BindWeakLambda(this, [this](const FT66OutgoingTravelerArrivalEvent& Event)
				{
					++ArrivalCallbackCount;
					if (Event.bHitLiveTarget)
					{
						++ArrivalCallbackLiveTargetCount;
					}
					if (Event.bTargetLostOrDead)
					{
						++ArrivalCallbackTargetLostCount;
					}
				});
			}

			const bool bFired = bBindArrivalCallback
				? Pool->FireOutgoingTraveler(Params, Handle, OnArrived)
				: Pool->FireOutgoingTraveler(Params, Handle);
			if (!bFired)
			{
				++FailedSpawnCount;
				continue;
			}

			TravelerHandles.Add(Handle);
			++ActorlessTravelerCount;
			++SpawnedCount;
		}

		if (bUseProofCamera)
		{
			ConfigureProofCamera(GridCenter, Forward);
		}

		SpawnTimeMs = (FPlatformTime::Seconds() - SpawnStartSeconds) * 1000.0;
		return;
	}

	for (int32 Index = 0; Index < RequestedCount; ++Index)
	{
		const int32 Column = Index % GridColumns;
		const int32 Row = Index / GridColumns;
		const FVector Location = Origin + Right * (static_cast<float>(Column) * GridSpacing) + Up * (static_cast<float>(Row) * GridSpacing);
		AT66HeroProjectile* Projectile = World->SpawnActor<AT66HeroProjectile>(
			AT66HeroProjectile::StaticClass(),
			Location,
			Forward.Rotation(),
			SpawnParams);
		if (!Projectile)
		{
			++FailedSpawnCount;
			continue;
		}

		Projectile->SetVisualOnly(true);
		Projectile->Damage = 0;
		Projectile->SetLifeSpan(WarmupSeconds + SampleSeconds + 10.0f);
		if (bDisableCollisionForRenderIsolation)
		{
			Projectile->SetActorEnableCollision(false);
			if (Projectile->CollisionSphere)
			{
				Projectile->CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
		Projectile->ConfigureTemporaryProjectileVisual(
			FT66TemporaryProjectileSystem::ProfileHeroAOE(),
			FT66TemporaryProjectileSystem::HeroProjectileColor(),
			VisualScaleMultiplier,
			NAME_None,
			FLinearColor::Transparent,
			1.0f);
		Projectile->SetProjectileSpeed(1.0f);
		Projectile->SetTargetLocation(Location + Forward * 100000.0f);

		if (Projectile->VisualMesh && Projectile->VisualMesh->IsVisible())
		{
			++MainMeshVisibleCount;
		}
		else
		{
			++MainMeshSuppressedCountAtSpawn;
		}
		Projectiles.Add(Projectile);
		++SpawnedCount;
	}

	if (bUseProofCamera)
	{
		ConfigureProofCamera(GridCenter, Forward);
	}

	SpawnTimeMs = (FPlatformTime::Seconds() - SpawnStartSeconds) * 1000.0;
}

void AT66OutgoingTravelerStressHarnessActor::SampleFrame(const float DeltaSeconds)
{
	if (DeltaSeconds > SMALL_NUMBER)
	{
		const double FrameValueMs = static_cast<double>(DeltaSeconds) * 1000.0;
		FrameMs.Add(FrameValueMs);
		Fps.Add(1000.0 / FrameValueMs);
	}

	if (GGameThreadTime > 0)
	{
		GameThreadMs.Add(FPlatformTime::ToMilliseconds(GGameThreadTime));
	}

	const uint32 GpuCycles = RHIGetGPUFrameCycles(0);
	if (GpuCycles > 0)
	{
		GpuFrameMs.Add(FPlatformTime::ToMilliseconds(GpuCycles));
	}

	DrawCalls.Add(static_cast<double>(GNumDrawCallsRHI[0]));

	if (bUsePool)
	{
		if (const UWorld* World = GetWorld())
		{
			if (const UT66OutgoingTravelerPoolSubsystem* Pool = World->GetSubsystem<UT66OutgoingTravelerPoolSubsystem>())
			{
				const FT66OutgoingTravelerPoolDiagnostics& PoolDiagnostics = Pool->GetDiagnostics();
				if (PoolDiagnostics.UploadCount > 0 && PoolDiagnostics.FiredTotal > 0)
				{
					PoolUploadMs.Add(PoolDiagnostics.LastUploadMs);
					PoolPackMs.Add(PoolDiagnostics.LastPackMs);
					PoolNiagaraArrayUploadMs.Add(PoolDiagnostics.LastNiagaraArrayUploadMs);
					PoolSimulationMs.Add(PoolDiagnostics.LastSimulationMs);
					PoolTargetSnapshotMs.Add(PoolDiagnostics.LastTargetSnapshotMs);
					PoolArrivalCollisionMs.Add(PoolDiagnostics.LastArrivalCollisionMs);
				}
			}
		}
	}
}

void AT66OutgoingTravelerStressHarnessActor::WriteManifest(const TCHAR* Reason)
{
	const UWorld* World = GetWorld();
	const UT66OutgoingTravelerPoolSubsystem* Pool = World ? World->GetSubsystem<UT66OutgoingTravelerPoolSubsystem>() : nullptr;
	const FT66OutgoingTravelerPoolDiagnostics PoolDiagnostics = Pool ? Pool->GetDiagnostics() : FT66OutgoingTravelerPoolDiagnostics{};

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("SchemaVersion"), T66OutgoingTravelerStressSchemaVersion);
	Root->SetStringField(TEXT("tool"), TEXT("T66OutgoingTravelerStressHarness"));
	Root->SetStringField(TEXT("reason"), Reason ? Reason : TEXT("unspecified"));
	Root->SetStringField(TEXT("configuration"), bUsePool ? TEXT("pooled") : TEXT("individual_mesh"));
	Root->SetBoolField(TEXT("uses_real_actor_path"), !bUsePool);
	Root->SetBoolField(TEXT("uses_actorless_pool_path"), bUsePool);
	Root->SetStringField(TEXT("actor_class"), bUsePool ? TEXT("None") : TEXT("AT66HeroProjectile"));
	Root->SetStringField(TEXT("visual_route"), bUsePool
		? TEXT("UT66OutgoingTravelerPoolSubsystem::FireOutgoingTraveler(actorless)")
		: TEXT("AT66HeroProjectile::ConfigureTemporaryProjectileVisual"));
	Root->SetBoolField(TEXT("uses_fire_outgoing_traveler"), bUsePool);
	Root->SetBoolField(TEXT("collision_disabled_for_render_isolation"), bDisableCollisionForRenderIsolation);
	Root->SetBoolField(TEXT("target_snapshot_enabled"), bUseTargetSnapshot);
	Root->SetBoolField(TEXT("arrival_collision_enabled"), bEnableArrivalCollision);
	Root->SetBoolField(TEXT("arrival_callback_bound"), bBindArrivalCallback);
	Root->SetBoolField(TEXT("mixed_visual_profiles_enabled"), bUseMixedVisualProfiles);
	Root->SetStringField(TEXT("visual_profile_family_filter"), VisualProfileFamilyFilter.IsEmpty() ? TEXT("all") : VisualProfileFamilyFilter);
	Root->SetBoolField(TEXT("uses_single_niagara_system_visual_selector"), bUsePool && bUseMixedVisualProfiles);
	Root->SetNumberField(TEXT("visual_profile_slot_base"), UT66OutgoingTravelerPoolSubsystem::TravelerVisualProfileSlotBase);
	Root->SetNumberField(TEXT("visual_profile_slot_count"), UT66OutgoingTravelerPoolSubsystem::TravelerVisualProfileSlotCount);
	TArray<TSharedPtr<FJsonValue>> VisualProfileValues;
	VisualProfileValues.Reserve(VisualProfilesUsed.Num());
	for (const FName& VisualProfileID : VisualProfilesUsed)
	{
		VisualProfileValues.Add(MakeShared<FJsonValueString>(VisualProfileID.ToString()));
	}
	Root->SetArrayField(TEXT("visual_profiles_used"), VisualProfileValues);
	Root->SetNumberField(TEXT("visual_profiles_used_count"), VisualProfilesUsed.Num());
	Root->SetNumberField(TEXT("arrival_callback_count"), ArrivalCallbackCount);
	Root->SetNumberField(TEXT("arrival_callback_live_target_count"), ArrivalCallbackLiveTargetCount);
	Root->SetNumberField(TEXT("arrival_callback_target_lost_count"), ArrivalCallbackTargetLostCount);
	Root->SetBoolField(TEXT("arrival_callback_suppressed_pool_damage"), !bBindArrivalCallback || PoolDiagnostics.ArrivalDamageAppliedTotal == 0);
	Root->SetNumberField(TEXT("requested_live_count"), RequestedCount);
	Root->SetNumberField(TEXT("target_count_requested"), TargetCount);
	Root->SetNumberField(TEXT("target_count_spawned"), TargetEnemies.Num());
	Root->SetNumberField(TEXT("spawned_count"), SpawnedCount);
	Root->SetNumberField(TEXT("failed_spawn_count"), FailedSpawnCount);
	Root->SetNumberField(TEXT("main_mesh_visible_count_at_spawn"), MainMeshVisibleCount);
	Root->SetNumberField(TEXT("main_mesh_suppressed_count_at_spawn"), MainMeshSuppressedCountAtSpawn);
	Root->SetNumberField(TEXT("actorless_traveler_count"), ActorlessTravelerCount);
	Root->SetNumberField(TEXT("grid_columns"), GridColumns);
	Root->SetNumberField(TEXT("grid_spacing"), GridSpacing);
	Root->SetNumberField(TEXT("spawn_distance"), SpawnDistance);
	Root->SetNumberField(TEXT("traveler_speed"), TravelerSpeed);
	Root->SetNumberField(TEXT("visual_scale_multiplier"), VisualScaleMultiplier);
	Root->SetBoolField(TEXT("proof_camera_enabled"), bUseProofCamera);
	Root->SetBoolField(TEXT("proof_camera_hide_hero"), bHideHeroForProof);
	Root->SetBoolField(TEXT("proof_camera_fixed_lane"), bUseProofCamera && bUseFixedProofLane);
	Root->SetNumberField(TEXT("proof_camera_distance"), ProofCameraDistance);
	Root->SetNumberField(TEXT("proof_camera_fov"), ProofCameraFOV);
	Root->SetNumberField(TEXT("proof_lane_center_x"), ProofLaneCenter.X);
	Root->SetNumberField(TEXT("proof_lane_center_y"), ProofLaneCenter.Y);
	Root->SetNumberField(TEXT("proof_lane_center_z"), ProofLaneCenter.Z);
	Root->SetNumberField(TEXT("proof_lane_yaw"), ProofLaneYaw);
	Root->SetNumberField(TEXT("proof_lane_pitch"), ProofLanePitch);
	Root->SetNumberField(TEXT("warmup_seconds"), WarmupSeconds);
	Root->SetNumberField(TEXT("sample_seconds"), SampleSeconds);
	Root->SetNumberField(TEXT("spawn_time_ms"), SpawnTimeMs);

	SetMetricObject(Root, TEXT("fps"), Fps);
	SetMetricObject(Root, TEXT("frame_ms"), FrameMs);
	SetMetricObject(Root, TEXT("game_thread_ms"), GameThreadMs);
	SetMetricObject(Root, TEXT("gpu_frame_ms"), GpuFrameMs);
	SetMetricObject(Root, TEXT("draw_calls"), DrawCalls);
	SetMetricObject(Root, TEXT("pool_last_upload_ms_sampled"), PoolUploadMs);
	SetMetricObject(Root, TEXT("pool_last_pack_ms_sampled"), PoolPackMs);
	SetMetricObject(Root, TEXT("pool_last_niagara_array_upload_ms_sampled"), PoolNiagaraArrayUploadMs);
	SetMetricObject(Root, TEXT("pool_last_simulation_ms_sampled"), PoolSimulationMs);
	SetMetricObject(Root, TEXT("pool_last_target_snapshot_ms_sampled"), PoolTargetSnapshotMs);
	SetMetricObject(Root, TEXT("pool_last_arrival_collision_ms_sampled"), PoolArrivalCollisionMs);

	const TSharedRef<FJsonObject> PoolObject = MakeShared<FJsonObject>();
	PoolObject->SetNumberField(TEXT("capacity"), UT66OutgoingTravelerPoolSubsystem::MaxOutgoingTravelers);
	PoolObject->SetNumberField(TEXT("live_count"), PoolDiagnostics.LiveCount);
	PoolObject->SetNumberField(TEXT("peak_live_count"), PoolDiagnostics.PeakLiveCount);
	PoolObject->SetNumberField(TEXT("fired_total"), PoolDiagnostics.FiredTotal);
	PoolObject->SetNumberField(TEXT("canceled_total"), PoolDiagnostics.CanceledTotal);
	PoolObject->SetNumberField(TEXT("dropped_total"), PoolDiagnostics.DroppedTotal);
	PoolObject->SetNumberField(TEXT("simulated_fired_total"), PoolDiagnostics.SimulatedFiredTotal);
	PoolObject->SetNumberField(TEXT("simulated_arrived_total"), PoolDiagnostics.SimulatedArrivedTotal);
	PoolObject->SetNumberField(TEXT("simulated_expired_total"), PoolDiagnostics.SimulatedExpiredTotal);
	PoolObject->SetNumberField(TEXT("upload_count"), PoolDiagnostics.UploadCount);
	PoolObject->SetNumberField(TEXT("simulation_tick_count"), PoolDiagnostics.SimulationTickCount);
	PoolObject->SetNumberField(TEXT("target_snapshot_build_count"), PoolDiagnostics.TargetSnapshotBuildCount);
	PoolObject->SetNumberField(TEXT("last_target_snapshot_count"), PoolDiagnostics.LastTargetSnapshotCount);
	PoolObject->SetNumberField(TEXT("arrival_check_total"), PoolDiagnostics.ArrivalCheckTotal);
	PoolObject->SetNumberField(TEXT("arrival_damage_applied_total"), PoolDiagnostics.ArrivalDamageAppliedTotal);
	PoolObject->SetNumberField(TEXT("arrival_callback_total"), PoolDiagnostics.ArrivalCallbackTotal);
	PoolObject->SetNumberField(TEXT("arrival_fizzle_no_target_total"), PoolDiagnostics.ArrivalFizzleNoTargetTotal);
	PoolObject->SetNumberField(TEXT("last_uploaded_live_count"), PoolDiagnostics.LastUploadedLiveCount);
	PoolObject->SetNumberField(TEXT("last_upload_ms"), PoolDiagnostics.LastUploadMs);
	PoolObject->SetNumberField(TEXT("last_pack_ms"), PoolDiagnostics.LastPackMs);
	PoolObject->SetNumberField(TEXT("last_niagara_array_upload_ms"), PoolDiagnostics.LastNiagaraArrayUploadMs);
	PoolObject->SetNumberField(TEXT("last_simulation_ms"), PoolDiagnostics.LastSimulationMs);
	PoolObject->SetNumberField(TEXT("last_target_snapshot_ms"), PoolDiagnostics.LastTargetSnapshotMs);
	PoolObject->SetNumberField(TEXT("last_arrival_collision_ms"), PoolDiagnostics.LastArrivalCollisionMs);
	PoolObject->SetNumberField(TEXT("average_upload_ms"), PoolDiagnostics.AverageUploadMs);
	PoolObject->SetNumberField(TEXT("average_pack_ms"), PoolDiagnostics.AveragePackMs);
	PoolObject->SetNumberField(TEXT("average_niagara_array_upload_ms"), PoolDiagnostics.AverageNiagaraArrayUploadMs);
	PoolObject->SetNumberField(TEXT("average_simulation_ms"), PoolDiagnostics.AverageSimulationMs);
	PoolObject->SetNumberField(TEXT("average_target_snapshot_ms"), PoolDiagnostics.AverageTargetSnapshotMs);
	PoolObject->SetNumberField(TEXT("average_arrival_collision_ms"), PoolDiagnostics.AverageArrivalCollisionMs);
	PoolObject->SetNumberField(TEXT("max_upload_ms"), PoolDiagnostics.MaxUploadMs);
	PoolObject->SetNumberField(TEXT("max_pack_ms"), PoolDiagnostics.MaxPackMs);
	PoolObject->SetNumberField(TEXT("max_niagara_array_upload_ms"), PoolDiagnostics.MaxNiagaraArrayUploadMs);
	PoolObject->SetNumberField(TEXT("max_simulation_ms"), PoolDiagnostics.MaxSimulationMs);
	PoolObject->SetNumberField(TEXT("max_target_snapshot_ms"), PoolDiagnostics.MaxTargetSnapshotMs);
	PoolObject->SetNumberField(TEXT("max_arrival_collision_ms"), PoolDiagnostics.MaxArrivalCollisionMs);
	PoolObject->SetNumberField(TEXT("main_mesh_suppressed_total"), PoolDiagnostics.MainMeshSuppressedTotal);
	Root->SetObjectField(TEXT("pool_diagnostics"), PoolObject);

	FString JsonText;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		UE_LOG(LogT66OutgoingTravelerStress, Warning, TEXT("[OutgoingTravelerStress] manifest serialize failed path=%s"), *ManifestPath);
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(ManifestPath), true);
	const bool bSaved = FFileHelper::SaveStringToFile(JsonText, *ManifestPath);
	UE_LOG(LogT66OutgoingTravelerStress, Display,
		TEXT("[OutgoingTravelerStress] manifest status=%s config=%s requested=%d spawned=%d fpsAvg=%s gameThreadAvg=%s gpuAvg=%s drawCallsAvg=%s path=%s"),
		bSaved ? TEXT("ok") : TEXT("failed"),
		bUsePool ? TEXT("pooled") : TEXT("individual_mesh"),
		RequestedCount,
		SpawnedCount,
		*FormatMaybeNumber(Fps),
		*FormatMaybeNumber(GameThreadMs),
		*FormatMaybeNumber(GpuFrameMs),
		*FormatMaybeNumber(DrawCalls),
		*ManifestPath);
}

void AT66OutgoingTravelerStressHarnessActor::DestroyTravelers()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66OutgoingTravelerPoolSubsystem* Pool = World->GetSubsystem<UT66OutgoingTravelerPoolSubsystem>())
		{
			for (FT66OutgoingTravelerHandle& Handle : TravelerHandles)
			{
				Pool->CancelOutgoingTraveler(Handle);
			}
		}
	}
	TravelerHandles.Reset();

	for (const TWeakObjectPtr<AT66HeroProjectile>& ProjectilePtr : Projectiles)
	{
		if (AT66HeroProjectile* Projectile = ProjectilePtr.Get())
		{
			Projectile->Destroy();
		}
	}
	Projectiles.Reset();
	DestroyStressTargets();
}

void AT66OutgoingTravelerStressHarnessActor::DestroyStressTargets()
{
	for (const TWeakObjectPtr<AT66EnemyBase>& EnemyPtr : TargetEnemies)
	{
		if (AT66EnemyBase* Enemy = EnemyPtr.Get())
		{
			Enemy->Destroy();
		}
	}
	TargetEnemies.Reset();
}
