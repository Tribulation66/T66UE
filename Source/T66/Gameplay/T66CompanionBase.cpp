// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66HeroSpeedSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Core/T66RunStateSubsystem.h"

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

	// Character fill light (disabled — emissive + SkyLight now handle brightness).
	CharacterFillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CharacterFillLight"));
	CharacterFillLight->SetupAttachment(RootComponent);
	CharacterFillLight->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	CharacterFillLight->SetIntensity(0.0f);                            // Disabled — was causing frontend overexposure
	CharacterFillLight->SetAttenuationRadius(300.f);
	CharacterFillLight->SetLightColor(FLinearColor(1.f, 0.98f, 0.95f));
	CharacterFillLight->SetCastShadows(false);
	CharacterFillLight->SetVisibility(false);
}

void AT66CompanionBase::BeginPlay()
{
	Super::BeginPlay();
	if (PlaceholderMesh && PlaceholderMesh->GetMaterial(0))
	{
		PlaceholderMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void AT66CompanionBase::InitializeCompanion(const FCompanionData& InData, FName SkinID)
{
	CompanionID = InData.CompanionID;
	CompanionData = InData;
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
	UMaterialInterface* ColorMaterial = FT66VisualUtil::GetPlaceholderColorMaterial();
	if (!ColorMaterial) ColorMaterial = PlaceholderMesh->GetMaterial(0);
	if (ColorMaterial)
	{
		PlaceholderMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
		if (PlaceholderMaterial)
		{
			PlaceholderMesh->SetMaterial(0, PlaceholderMaterial);
			PlaceholderMaterial->SetVectorParameterValue(FName("Color"), Color);
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
		UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		UT66HeroSpeedSubsystem* SpeedSub = GI ? GI->GetSubsystem<UT66HeroSpeedSubsystem>() : nullptr;
		if (SpeedSub)
		{
			const int32 NewState = SpeedSub->GetMovementAnimState(); // 0=Idle, 2=Run
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
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;
	APawn* Hero = PC->GetPawn();
	if (!Hero) return;

	// Target: hero location + offset in hero's local space (behind and to the side)
	FRotator HeroYaw = FRotator(0.f, Hero->GetActorRotation().Yaw, 0.f);
	FVector OffsetWorld = HeroYaw.RotateVector(FollowOffset);
	FVector TargetLoc = Hero->GetActorLocation() + OffsetWorld;

	FVector NewLoc = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, FollowSpeed);
	// Keep companion grounded (actor origin = ground contact point; visuals handle their own offsets).
	// Perf: run ground trace only every Nth tick.
	++GroundTraceTickCounter;
	if (GroundTraceTickCounter % GroundTraceEveryNTicks == 0)
	{
		FLagScopedScope LagScope(World, TEXT("CompanionBase::Tick (LineTrace ground)"), 2.0f);
		FHitResult Hit;
		const FVector Start = NewLoc + FVector(0.f, 0.f, 2000.f);
		const FVector End = NewLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			NewLoc.Z = Hit.ImpactPoint.Z;
		}
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

	// Heal the hero over time (up to max hearts).
	HealAccumSeconds += DeltaTime;
	float EffectiveInterval = HealIntervalSeconds;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			// Union tiers: Basic=10s, Good=5s, Medium=3s, Hyper=1s per heart.
			EffectiveInterval = FMath::Clamp(Ach->GetCompanionUnionHealingIntervalSeconds(CompanionID), 0.25f, 60.f);
		}
	}

	if (HealAccumSeconds >= EffectiveInterval)
	{
		// Avoid drift on variable frame times; handle large deltas safely.
		int32 Heals = 0;
		while (HealAccumSeconds >= EffectiveInterval && Heals < 8)
		{
			HealAccumSeconds -= EffectiveInterval;
			++Heals;
		}
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				if (RunState->GetCurrentHearts() < RunState->GetMaxHearts())
				{
					for (int32 H = 0; H < Heals; ++H)
					{
						RunState->HealHearts(HealAmountHearts);
					}
				}
			}
		}
	}
}
