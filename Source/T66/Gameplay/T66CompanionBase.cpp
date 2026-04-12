// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66HeroSpeedSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Core/T66RunStateSubsystem.h"

namespace
{
	APawn* T66ResolveCompanionFollowHero(const AActor* CompanionActor)
	{
		if (!CompanionActor)
		{
			return nullptr;
		}

		if (APawn* OwnerPawn = Cast<APawn>(CompanionActor->GetOwner()))
		{
			return OwnerPawn;
		}

		if (AController* OwnerController = Cast<AController>(CompanionActor->GetOwner()))
		{
			if (APawn* OwnerControlledPawn = OwnerController->GetPawn())
			{
				return OwnerControlledPawn;
			}
		}

		if (APawn* InstigatorPawn = CompanionActor->GetInstigator())
		{
			return InstigatorPawn;
		}

		UWorld* World = CompanionActor->GetWorld();
		if (APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr)
		{
			return PC->GetPawn();
		}

		return nullptr;
	}
}

AT66CompanionBase::AT66CompanionBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(CylinderFinder.Object);
		// Placeholder cylinder (used when no character visual exists).
		// NOTE: actor origin is treated as ground contact point in gameplay.
		const FVector GameplayScale(0.42f, 0.42f, 0.95f);
		PlaceholderMesh->SetRelativeScale3D(GameplayScale);
		PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * GameplayScale.Z));
	}

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetVisibility(false, true);

	ApplyCompanionScale();
}

void AT66CompanionBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyCompanionScale();
	bHasCachedGroundZ = false;
	CachedGroundZ = GetActorLocation().Z;

	if (PlaceholderMesh && PlaceholderMesh->GetMaterial(0))
	{
		PlaceholderMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		CachedHeroSpeedSubsystem = GI->GetSubsystem<UT66HeroSpeedSubsystem>();
		CachedAchievementsSubsystem = GI->GetSubsystem<UT66AchievementsSubsystem>();
		CachedRunStateSubsystem = GI->GetSubsystem<UT66RunStateSubsystem>();
		if (CachedAchievementsSubsystem)
		{
			CachedUnionStagesCleared = CachedAchievementsSubsystem->GetCompanionUnionStagesCleared(CompanionID);
			CachedAchievementsSubsystem->AchievementsUnlocked.AddDynamic(this, &AT66CompanionBase::HandleAchievementsUnlocked);
		}
	}

	CachedHeroPawn = T66ResolveCompanionFollowHero(this);
}

void AT66CompanionBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedAchievementsSubsystem)
	{
		CachedAchievementsSubsystem->AchievementsUnlocked.RemoveDynamic(this, &AT66CompanionBase::HandleAchievementsUnlocked);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66CompanionBase::HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs)
{
	if (CachedAchievementsSubsystem)
	{
		CachedUnionStagesCleared = CachedAchievementsSubsystem->GetCompanionUnionStagesCleared(CompanionID);
	}
}

void AT66CompanionBase::InitializeCompanion(const FCompanionData& InData, FName SkinID)
{
	CompanionID = InData.CompanionID;
	CompanionData = InData;
	ApplyCompanionScale();
	bHasCachedGroundZ = false;
	GroundTraceTickCounter = 0;
	if (CachedAchievementsSubsystem)
	{
		CachedUnionStagesCleared = CachedAchievementsSubsystem->GetCompanionUnionStagesCleared(CompanionID);
	}
	SetPlaceholderColor(InData.PlaceholderColor);

	// VisualID = Companion_01 or Companion_01_Beachgoer (from DT_CharacterVisuals).
	// In preview mode use alert animation and preview context (like hero selection).
	bUsingCharacterVisual = false;
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const FName VisualID = UT66CharacterVisualSubsystem::GetCompanionVisualID(CompanionID, SkinID.IsNone() ? FName(TEXT("Default")) : SkinID);
			const bool bUseAlertAnimation = bIsPreviewMode;
			const bool bIsPreviewContext = bIsPreviewMode;
			bUsingCharacterVisual = Visuals->ApplyCharacterVisual(VisualID, SkeletalMesh, PlaceholderMesh, true, bUseAlertAnimation, bIsPreviewContext);
			if (!bUsingCharacterVisual && SkeletalMesh)
			{
				SkeletalMesh->SetVisibility(false, true);
			}
			else if (!bIsPreviewMode)
			{
				UAnimationAsset* WalkRaw = nullptr;
				UAnimationAsset* RunRaw = nullptr;
				UAnimationAsset* AlertRaw = nullptr;
				Visuals->GetMovementAnimsForVisual(VisualID, WalkRaw, RunRaw, AlertRaw);
				CachedWalkAnim = WalkRaw;
				CachedRunAnim = RunRaw;
				CachedAlertAnim = AlertRaw;
				// Match hero: ApplyCharacterVisual just played LoopingAnim (walk), so store Walk (1).
				// Then when hero is Idle (0) we see a state change and play Alert.
				LastMovementAnimState = 1;
			}
		}
	}
}

void AT66CompanionBase::SetPlaceholderColor(FLinearColor Color)
{
	if (!PlaceholderMesh) return;
	UMaterialInterface* ColorMaterial = FT66VisualUtil::GetFlatColorMaterial();
	if (!ColorMaterial) ColorMaterial = PlaceholderMesh->GetMaterial(0);
	if (ColorMaterial)
	{
		PlaceholderMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
		if (PlaceholderMaterial)
		{
			PlaceholderMesh->SetMaterial(0, PlaceholderMaterial);
			FT66VisualUtil::ConfigureFlatColorMaterial(PlaceholderMaterial, Color);
		}
	}
}

void AT66CompanionBase::SetPreviewMode(bool bPreview)
{
	bIsPreviewMode = bPreview;
	// Make preview easier to see in UI (only affects placeholder).
	if (PlaceholderMesh && PlaceholderMesh->IsVisible())
	{
		const FVector GameplayScale(0.42f, 0.42f, 0.95f);
		const FVector PreviewScale(0.78f, 0.78f, 1.25f);
		const FVector NewScale = bIsPreviewMode ? PreviewScale : GameplayScale;
		PlaceholderMesh->SetRelativeScale3D(NewScale);
		PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * NewScale.Z));
	}
}

void AT66CompanionBase::SetLockedVisual(bool bLocked)
{
	bLockedVisual = bLocked;

	if (bLockedVisual)
	{
		// Locked silhouette: hide skeletal mesh and show a near-black placeholder cylinder.
		if (SkeletalMesh)
		{
			SkeletalMesh->SetVisibility(false, true);
			SkeletalMesh->SetHiddenInGame(true, true);
		}
		if (PlaceholderMesh)
		{
			PlaceholderMesh->SetVisibility(true, true);
			PlaceholderMesh->SetHiddenInGame(false, true);
		}
		SetPlaceholderColor(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f));
		return;
	}

	// Unlocked: revert to the normal visual selection.
	if (bUsingCharacterVisual && SkeletalMesh)
	{
		SkeletalMesh->SetHiddenInGame(false, true);
		SkeletalMesh->SetVisibility(true, true);
	}
	if (PlaceholderMesh)
	{
		const bool bShowPlaceholder = !bUsingCharacterVisual;
		PlaceholderMesh->SetVisibility(bShowPlaceholder, true);
		PlaceholderMesh->SetHiddenInGame(!bShowPlaceholder, true);
	}
	SetPlaceholderColor(CompanionData.PlaceholderColor);
}

void AT66CompanionBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsPreviewMode) return;

	// Animation: two states only — Alert (idle) and Run (any movement). Match hero.
	if (bUsingCharacterVisual && SkeletalMesh && SkeletalMesh->IsVisible() && (CachedAlertAnim || CachedRunAnim || CachedWalkAnim))
	{
		if (CachedHeroSpeedSubsystem)
		{
			const int32 NewState = CachedHeroSpeedSubsystem->GetMovementAnimState(); // 0=Idle, 2=Run
			if (NewState != LastMovementAnimState)
			{
				LastMovementAnimState = static_cast<uint8>(NewState);
				UAnimationAsset* ToPlay = nullptr;
				if (NewState == 0)
					ToPlay = CachedAlertAnim;
				else
					ToPlay = CachedRunAnim ? CachedRunAnim : CachedWalkAnim;
				if (ToPlay)
					SkeletalMesh->PlayAnimation(ToPlay, true);
			}
		}
	}

	// Follow the player's pawn (hero)
	UWorld* World = GetWorld();
	if (!World) return;
	APawn* Hero = CachedHeroPawn.Get();
	if (!Hero)
	{
		Hero = T66ResolveCompanionFollowHero(this);
		if (Hero)
		{
			CachedHeroPawn = Hero;
		}
	}
	if (!Hero) return;

	// Target: hero location + offset in hero's local space (behind and to the side)
	FRotator HeroYaw = FRotator(0.f, Hero->GetActorRotation().Yaw, 0.f);
	FVector OffsetWorld = HeroYaw.RotateVector(FollowOffset);
	FVector TargetLoc = Hero->GetActorLocation() + OffsetWorld;

	const FVector CurrentLoc = GetActorLocation();
	const FVector CurrentPlanar(CurrentLoc.X, CurrentLoc.Y, 0.f);
	const FVector TargetPlanar(TargetLoc.X, TargetLoc.Y, 0.f);
	const FVector NewPlanar = FMath::VInterpTo(CurrentPlanar, TargetPlanar, DeltaTime, FollowSpeed);

	FVector NewLoc = CurrentLoc;
	NewLoc.X = NewPlanar.X;
	NewLoc.Y = NewPlanar.Y;

	// Keep companion grounded. Only follow the hero in X/Y; the old path interpolated Z
	// toward the hero's capsule center and then snapped back to the trace result, which
	// caused the visible up/down bob in gameplay.
	++GroundTraceTickCounter;
	if (!bHasCachedGroundZ || GroundTraceTickCounter % GroundTraceEveryNTicks == 0)
	{
		FLagScopedScope LagScope(World, TEXT("CompanionBase::Tick (LineTrace ground)"), 2.0f);
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
		? FMath::FInterpTo(CurrentLoc.Z, CachedGroundZ, DeltaTime, GroundFollowSpeed)
		: CurrentLoc.Z;
	if (bHasCachedGroundZ && FMath::IsNearlyEqual(NewLoc.Z, CachedGroundZ, 0.5f))
	{
		NewLoc.Z = CachedGroundZ;
	}
	SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);

	// Face the hero
	FVector ToHero = Hero->GetActorLocation() - GetActorLocation();
	ToHero.Z = 0.f;
	if (ToHero.IsNearlyZero() == false)
	{
		FRotator NewRot = ToHero.Rotation();
		NewRot.Pitch = 0.f;
		NewRot.Roll = 0.f;
		SetActorRotation(NewRot);
	}

	// Heal the hero over time (numerical HP/s by Union tier: 5/10/20/20).
	float HealHPPerSecond = 5.f;
	if (CachedAchievementsSubsystem)
	{
		const int32 Stages = CachedUnionStagesCleared;
		if (Stages >= UT66AchievementsSubsystem::UnionTier_HyperStages) HealHPPerSecond = 20.f;
		else if (Stages >= UT66AchievementsSubsystem::UnionTier_MediumStages) HealHPPerSecond = 20.f;
		else if (Stages >= UT66AchievementsSubsystem::UnionTier_GoodStages) HealHPPerSecond = 10.f;
	}
	if (CachedRunStateSubsystem)
	{
		if (CachedRunStateSubsystem->GetCurrentHP() < CachedRunStateSubsystem->GetMaxHP() && HealHPPerSecond > 0.f)
		{
			CachedRunStateSubsystem->HealHP(HealHPPerSecond * DeltaTime);
		}
	}
}

void AT66CompanionBase::ApplyCompanionScale()
{
	SetActorScale3D(FVector(FMath::Max(0.1f, CompanionActorScale)));
}
