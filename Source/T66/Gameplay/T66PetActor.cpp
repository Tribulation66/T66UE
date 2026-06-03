// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PetActor.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66MobLootSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	float T66PetPlanarDistanceSq(const FVector& A, const FVector& B)
	{
		const FVector APlanar(A.X, A.Y, 0.f);
		const FVector BPlanar(B.X, B.Y, 0.f);
		return FVector::DistSquared(APlanar, BPlanar);
	}
}

AT66PetActor::AT66PetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	SetRootComponent(Root);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetMobility(EComponentMobility::Movable);
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(0.26f, 0.26f, 0.26f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 26.f));
	}
}

void AT66PetActor::BeginPlay()
{
	Super::BeginPlay();
	CachedHeroPawn = ResolveFollowHero();
	CachedGroundZ = GetActorLocation().Z;
	UpdateMovementTuning();
	ApplyPetVisuals();
}

void AT66PetActor::InitializePet(const FPetData& InData, FName SkinID)
{
	PetData = InData;
	PetID = PetData.PetID.IsNone() ? PetData.SourceBossID : PetData.PetID;
	ActiveSkinID = SkinID.IsNone() ? FName(TEXT("Default")) : SkinID;
	UpdateMovementTuning();
	ApplyPetVisuals();
}

void AT66PetActor::ApplyPetVisuals()
{
	if (!VisualMesh)
	{
		return;
	}

	if (!PetData.CaptureVisualMesh.IsNull())
	{
		if (UStaticMesh* PetMesh = PetData.CaptureVisualMesh.Get())
		{
			VisualMesh->SetStaticMesh(PetMesh);
			VisualMesh->SetRelativeScale3D(FVector(0.5f));
			FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh, PetMesh);
		}
	}

	FT66VisualUtil::ApplyT66Color(VisualMesh, this, PetData.PlaceholderColor);
}

float AT66PetActor::ComputeFetchSpeedForBondStagesForAutomation(const int32 BondStages) const
{
	const float BaseSpeed = FMath::Max(1.f, PetData.BaseFetchSpeed);
	const float DataMultiplier = 1.f + static_cast<float>(FMath::Max(0, BondStages)) * FMath::Max(0.f, PetData.BondFetchSpeedPerStage);
	const float BondMultiplier = FMath::Clamp(DataMultiplier, 1.f, FMath::Max(1.f, PetData.MaxBondFetchSpeedMultiplier));
	return BaseSpeed * BondMultiplier;
}

void AT66PetActor::UpdateMovementTuning()
{
	int32 BondStages = 0;
	if (!PetID.IsNone())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				BondStages = Achievements->GetPetBondStagesCleared(PetID);
			}
		}
	}
	CurrentFollowSpeed = ComputeFetchSpeedForBondStagesForAutomation(BondStages);
	ReturnFollowSpeed = CurrentFollowSpeed;
}

void AT66PetActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TryFollowFoundationMobLootTarget(DeltaSeconds))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* Hero = CachedHeroPawn.Get();
	if (!Hero)
	{
		Hero = ResolveFollowHero();
		CachedHeroPawn = Hero;
	}
	if (!Hero)
	{
		return;
	}

	const FRotator HeroYaw(0.f, Hero->GetActorRotation().Yaw, 0.f);
	const FVector TargetLoc = Hero->GetActorLocation() + HeroYaw.RotateVector(FollowOffset);
	const FVector CurrentLoc = GetActorLocation();
	const FVector CurrentPlanar(CurrentLoc.X, CurrentLoc.Y, 0.f);
	const FVector TargetPlanar(TargetLoc.X, TargetLoc.Y, 0.f);
	const FVector NewPlanar = FMath::VInterpTo(CurrentPlanar, TargetPlanar, DeltaSeconds, ReturnFollowSpeed);

	FVector NewLoc = CurrentLoc;
	NewLoc.X = NewPlanar.X;
	NewLoc.Y = NewPlanar.Y;

	++GroundTraceTickCounter;
	if (!bHasCachedGroundZ || GroundTraceTickCounter % GroundTraceEveryNTicks == 0)
	{
		FHitResult Hit;
		const FVector TraceOrigin(NewLoc.X, NewLoc.Y, CurrentLoc.Z);
		const FVector Start = TraceOrigin + FVector(0.f, 0.f, 2000.f);
		const FVector End = TraceOrigin - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic) ||
			World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			CachedGroundZ = Hit.ImpactPoint.Z;
			bHasCachedGroundZ = true;
		}
	}

	NewLoc.Z = bHasCachedGroundZ
		? FMath::FInterpTo(CurrentLoc.Z, CachedGroundZ, DeltaSeconds, GroundFollowSpeed)
		: CurrentLoc.Z;
	++TotalMobLootMoveAttempts;
	LastMobLootMoveDistance = FVector::Dist(CurrentLoc, NewLoc);
	SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);

	FVector ToHero = Hero->GetActorLocation() - GetActorLocation();
	ToHero.Z = 0.f;
	if (!ToHero.IsNearlyZero())
	{
		FRotator NewRot = ToHero.Rotation();
		NewRot.Pitch = 0.f;
		NewRot.Roll = 0.f;
		SetActorRotation(NewRot);
	}
}

bool AT66PetActor::TryFollowFoundationMobLootTarget(float DeltaSeconds)
{
	if (!bMobLootCollectionEnabled)
	{
		if (bHasReservedMobLootTarget)
		{
			if (UWorld* World = GetWorld())
			{
				ReleaseReservedMobLootReservation(World->GetSubsystem<UT66MobLootSubsystem>());
			}
			else
			{
				ClearReservedMobLootTarget();
			}
		}
		return false;
	}

	UWorld* World = GetWorld();
	UT66MobLootSubsystem* MobLoot = World ? World->GetSubsystem<UT66MobLootSubsystem>() : nullptr;
	if (!MobLoot)
	{
		ClearReservedMobLootTarget();
		return false;
	}
	if (!UT66MobLootSubsystem::IsEnabled())
	{
		ReleaseReservedMobLootReservation(MobLoot);
		return false;
	}

	if (!bHasReservedMobLootTarget && !TryReserveMobLootTarget(*MobLoot))
	{
		return false;
	}

	ReservedMobLootAgeSeconds += FMath::Max(0.f, DeltaSeconds);
	if (TryCollectReservedMobLoot(*MobLoot))
	{
		return true;
	}

	MoveTowardReservedMobLootTarget(DeltaSeconds);
	if (TryCollectReservedMobLoot(*MobLoot))
	{
		return true;
	}

	if (ReservedMobLootAgeSeconds >= FMath::Max(0.1f, MobLootReservationMaxAgeSeconds) ||
		ReservedMobLootNoProgressSeconds >= FMath::Max(0.1f, MobLootNoProgressReleaseSeconds))
	{
		ReleaseReservedMobLootReservation(MobLoot);
	}
	return true;
}

bool AT66PetActor::PumpMobLootCollectionForAutomation(const float DeltaSeconds)
{
	return TryFollowFoundationMobLootTarget(DeltaSeconds);
}

bool AT66PetActor::TryReserveMobLootTarget(UT66MobLootSubsystem& MobLoot)
{
	FT66MobLootQueryFilter Filter;
	Filter.Origin = GetActorLocation();
	Filter.SearchRadius = FMath::Max(0.f, MobLootSearchRadius);
	Filter.MaxCandidates = 1;
	Filter.bExcludeReservedByOthers = true;
	// Future hero-radius ignores are added here; keep the ExclusionSpheres field explicitly on the pet query path.
	Filter.ExclusionSpheres.Reset();

	const FT66MobLootReservationResult Reservation = MobLoot.QueryAndReserveMobLoot(Filter, BuildMobLootCollectorRef());
	if (!Reservation.bReserved || !Reservation.Handle.IsValid())
	{
		return false;
	}

	bHasReservedMobLootTarget = true;
	ReservedMobLootHandle = Reservation.Handle;
	ReservedMobLootTargetLocation = Reservation.Position;
	ReservedMobLootAgeSeconds = 0.f;
	ReservedMobLootNoProgressSeconds = 0.f;
	ReservedMobLootLastDistanceSq = T66PetPlanarDistanceSq(GetActorLocation(), ReservedMobLootTargetLocation);
	return true;
}

bool AT66PetActor::MoveTowardReservedMobLootTarget(const float DeltaSeconds)
{
	if (!bHasReservedMobLootTarget)
	{
		return false;
	}

	UWorld* World = GetWorld();
	const FVector CurrentLoc = GetActorLocation();
	const FVector CurrentPlanar(CurrentLoc.X, CurrentLoc.Y, 0.f);
	const FVector TargetPlanar(ReservedMobLootTargetLocation.X, ReservedMobLootTargetLocation.Y, 0.f);
	const float FetchUnitsPerSecond = FMath::Max(1.f, CurrentFollowSpeed * FMath::Max(1.f, MobLootFetchUnitsPerSpeedPoint));
	const FVector NewPlanar = FMath::VInterpConstantTo(CurrentPlanar, TargetPlanar, DeltaSeconds, FetchUnitsPerSecond);

	FVector NewLoc = CurrentLoc;
	NewLoc.X = NewPlanar.X;
	NewLoc.Y = NewPlanar.Y;

	++GroundTraceTickCounter;
	if (World && (!bHasCachedGroundZ || GroundTraceTickCounter % GroundTraceEveryNTicks == 0))
	{
		FHitResult Hit;
		const FVector TraceOrigin(NewLoc.X, NewLoc.Y, CurrentLoc.Z);
		const FVector Start = TraceOrigin + FVector(0.f, 0.f, 2000.f);
		const FVector End = TraceOrigin - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic) ||
			World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			CachedGroundZ = Hit.ImpactPoint.Z;
			bHasCachedGroundZ = true;
		}
	}

	NewLoc.Z = bHasCachedGroundZ
		? FMath::FInterpTo(CurrentLoc.Z, CachedGroundZ, DeltaSeconds, GroundFollowSpeed)
		: CurrentLoc.Z;
	++TotalMobLootMoveAttempts;
	LastMobLootMoveDistance = FVector::Dist(CurrentLoc, NewLoc);
	SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);

	FVector ToTarget = ReservedMobLootTargetLocation - GetActorLocation();
	ToTarget.Z = 0.f;
	if (!ToTarget.IsNearlyZero())
	{
		FRotator NewRot = ToTarget.Rotation();
		NewRot.Pitch = 0.f;
		NewRot.Roll = 0.f;
		SetActorRotation(NewRot);
	}

	const float NewDistanceSq = T66PetPlanarDistanceSq(GetActorLocation(), ReservedMobLootTargetLocation);
	const float ProgressEpsilonSq = FMath::Square(FMath::Max(0.f, MobLootProgressEpsilon));
	if (NewDistanceSq + ProgressEpsilonSq < ReservedMobLootLastDistanceSq)
	{
		ReservedMobLootNoProgressSeconds = 0.f;
		ReservedMobLootLastDistanceSq = NewDistanceSq;
	}
	else
	{
		ReservedMobLootNoProgressSeconds += FMath::Max(0.f, DeltaSeconds);
	}
	return true;
}

bool AT66PetActor::TryCollectReservedMobLoot(UT66MobLootSubsystem& MobLoot)
{
	if (!bHasReservedMobLootTarget || !ReservedMobLootHandle.IsValid())
	{
		return false;
	}

	if (T66PetPlanarDistanceSq(GetActorLocation(), ReservedMobLootTargetLocation) > FMath::Square(FMath::Max(1.f, MobLootCollectDistance)))
	{
		return false;
	}

	// The pet walks on ground while Mob Loot may retain source/capsule height; use the pet's planar range gate above.
	const FT66MobLootCollectResult CollectResult =
		MobLoot.CollectReservedMobLoot(ReservedMobLootHandle, BuildMobLootCollectorRef(), 0.f);
	if (CollectResult.DropsCollected <= 0)
	{
		return false;
	}

	HandleMobLootCollectedForEconomyStack(CollectResult);
	ClearReservedMobLootTarget();
	return true;
}

void AT66PetActor::ReleaseReservedMobLootReservation(UT66MobLootSubsystem* MobLoot)
{
	if (!bHasReservedMobLootTarget || !ReservedMobLootHandle.IsValid())
	{
		ClearReservedMobLootTarget();
		return;
	}

	if (MobLoot && MobLoot->ReleaseMobLootReservation(ReservedMobLootHandle, BuildMobLootCollectorRef()))
	{
		++TotalMobLootReservationsReleased;
	}
	ClearReservedMobLootTarget();
}

void AT66PetActor::ClearReservedMobLootTarget()
{
	bHasReservedMobLootTarget = false;
	ReservedMobLootHandle.Reset();
	ReservedMobLootTargetLocation = FVector::ZeroVector;
	ReservedMobLootAgeSeconds = 0.f;
	ReservedMobLootNoProgressSeconds = 0.f;
	ReservedMobLootLastDistanceSq = TNumericLimits<float>::Max();
}

void AT66PetActor::HandleMobLootCollectedForEconomyStack(const FT66MobLootCollectResult& CollectResult)
{
	LastMobLootCollectResult = CollectResult;
	TotalMobLootDropsCollected += CollectResult.DropsCollected;
	TotalMobLootQuantityCollected += CollectResult.QuantityCollected;
	TotalMobLootSellValueCollected += CollectResult.GoldValueCollected;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->AddCollectedMobLootFromCollection(CollectResult, BuildMobLootCollectorRef());
		}
	}
}

FT66MobLootCollectorRef AT66PetActor::BuildMobLootCollectorRef() const
{
	FT66MobLootCollectorRef Collector;
	Collector.Collector = const_cast<AT66PetActor*>(this);
	Collector.CollectorID = PetID.IsNone()
		? FName(TEXT("ActivePet"))
		: FName(*FString::Printf(TEXT("Pet.%s"), *PetID.ToString()));
	Collector.CollectorType = ET66MobLootCollectorType::Pet;
	return Collector;
}

void AT66PetActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bHasReservedMobLootTarget)
	{
		UWorld* World = GetWorld();
		ReleaseReservedMobLootReservation(World ? World->GetSubsystem<UT66MobLootSubsystem>() : nullptr);
	}
	Super::EndPlay(EndPlayReason);
}

APawn* AT66PetActor::ResolveFollowHero() const
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		return OwnerPawn;
	}
	if (const AController* OwnerController = Cast<AController>(GetOwner()))
	{
		return OwnerController->GetPawn();
	}
	if (APawn* InstigatorPawn = GetInstigator())
	{
		return InstigatorPawn;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			return PC->GetPawn();
		}
	}
	return nullptr;
}
