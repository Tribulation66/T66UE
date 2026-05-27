// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66HeroSpeedSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Core/T66RunStateSubsystem.h"
#include "Misc/CommandLine.h"

namespace
{
	static const FVector T66CompanionGameplayPlaceholderScale(0.42f, 0.42f, 1.00f);
	static const FVector T66CompanionPreviewPlaceholderScale(0.78f, 0.78f, 1.32f);

	bool T66CompanionAnimDebugEnabled()
	{
		static const bool bEnabled = FParse::Param(FCommandLine::Get(), TEXT("T66CompanionAnimDebug"));
		return bEnabled;
	}

	FString T66CompanionAnimDebugAssetName(const UAnimationAsset* Anim)
	{
		return Anim ? Anim->GetPathName() : FString(TEXT("(null)"));
	}

	FString T66CompanionAnimDebugMeshName(const USkeletalMeshComponent* Mesh)
	{
		const USkeletalMesh* SkeletalAsset = Mesh ? Mesh->GetSkeletalMeshAsset() : nullptr;
		return SkeletalAsset ? SkeletalAsset->GetPathName() : FString(TEXT("(null)"));
	}

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
		PlaceholderMesh->SetRelativeScale3D(T66CompanionGameplayPlaceholderScale);
		PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * T66CompanionGameplayPlaceholderScale.Z));
	}

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetVisibility(false, true);

	ApplyCompanionScale();
}

float AT66CompanionBase::GetHealingPerSecondForUnionStages(const int32 UnionStagesCleared)
{
	if (UnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_HyperStages)
	{
		return 20.f;
	}
	if (UnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_MediumStages)
	{
		return 20.f;
	}
	if (UnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_GoodStages)
	{
		return 10.f;
	}
	return 5.f;
}

float AT66CompanionBase::GetHealingAmountForDifficulty(const ET66Difficulty Difficulty)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:
		return 5.f;
	case ET66Difficulty::Medium:
		return 6.f;
	case ET66Difficulty::Hard:
		return 6.f;
	case ET66Difficulty::VeryHard:
		return 5.f;
	case ET66Difficulty::Impossible:
		return 4.f;
	default:
		return 5.f;
	}
}

float AT66CompanionBase::GetHealingIntervalSecondsForDifficulty(const ET66Difficulty Difficulty)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:
		return 1.0f;
	case ET66Difficulty::Medium:
		return 1.5f;
	case ET66Difficulty::Hard:
		return 2.0f;
	case ET66Difficulty::VeryHard:
		return 2.5f;
	case ET66Difficulty::Impossible:
		return 3.0f;
	default:
		return 1.0f;
	}
}

float AT66CompanionBase::GetHealingPerSecondForDifficulty(const ET66Difficulty Difficulty)
{
	const float Interval = GetHealingIntervalSecondsForDifficulty(Difficulty);
	return Interval > 0.f ? GetHealingAmountForDifficulty(Difficulty) / Interval : 0.f;
}

void AT66CompanionBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyCompanionScale();
	CompanionHealAccumulatorSeconds = 0.f;
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
	ActiveSkinID = SkinID.IsNone() ? FName(TEXT("Default")) : SkinID;
	ApplyCompanionScale();
	CompanionHealAccumulatorSeconds = 0.f;
	bHasCachedGroundZ = false;
	GroundTraceTickCounter = 0;
	bCompanionAnimDebugLoggedGuardFailure = false;
	bCompanionAnimDebugLoggedMissingStateSource = false;
	bCompanionAnimDebugLoggedNullSelection = false;
	if (CachedAchievementsSubsystem)
	{
		CachedUnionStagesCleared = CachedAchievementsSubsystem->GetCompanionUnionStagesCleared(CompanionID);
	}
	SetPlaceholderColor(InData.PlaceholderColor);

	// VisualID = Companion_01 or Companion_01_Beachgoer (from DT_CharacterVisuals).
	// In preview mode use alert animation and preview context (like hero selection).
	ApplyCurrentCharacterVisual();
}

bool AT66CompanionBase::ApplyCurrentCharacterVisual()
{
	bUsingCharacterVisual = false;
	bUsingStaticCharacterVisual = false;
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const FName VisualID = UT66CharacterVisualSubsystem::GetCompanionVisualID(CompanionID, ActiveSkinID.IsNone() ? FName(TEXT("Default")) : ActiveSkinID);
			const bool bUseIdleAnimation = bIsPreviewMode;
			const bool bIsPreviewContext = bIsPreviewMode;
			bUsingCharacterVisual = Visuals->ApplyCharacterVisual(VisualID, SkeletalMesh, PlaceholderMesh, true, bUseIdleAnimation, bIsPreviewContext, PlaceholderMesh);
			bUsingStaticCharacterVisual = bUsingCharacterVisual
				&& PlaceholderMesh
				&& PlaceholderMesh->IsVisible()
				&& (!SkeletalMesh || !SkeletalMesh->IsVisible());
			if (!bUsingCharacterVisual && SkeletalMesh)
			{
				SkeletalMesh->SetVisibility(false, true);
			}
			else if (!bIsPreviewMode)
			{
				UAnimationAsset* WalkRaw = nullptr;
				UAnimationAsset* JumpRaw = nullptr;
				UAnimationAsset* IdleRaw = nullptr;
				UAnimationAsset* RollRaw = nullptr;
				Visuals->GetMovementAnimsForVisual(VisualID, WalkRaw, JumpRaw, IdleRaw, RollRaw);
				CachedWalkAnim = WalkRaw;
				CachedJumpAnim = JumpRaw;
				CachedIdleAnim = IdleRaw;
				CachedRollAnim = RollRaw;
				// Match hero: force the first movement-state tick to refresh the clip.
				LastMovementAnimState = 255;
			}
			if (T66CompanionAnimDebugEnabled())
			{
				UE_LOG(LogTemp, Display,
					TEXT("[CompanionAnimDebug] ApplyVisual CompanionID=%s Skin=%s VisualID=%s Preview=%d UsingCharacter=%d StaticFallback=%d MeshVisible=%d MeshHidden=%d AnimMode=%d Mesh=%s Idle=%s Walk=%s Jump=%s Roll=%s"),
					*CompanionID.ToString(),
					*ActiveSkinID.ToString(),
					*VisualID.ToString(),
					bIsPreviewMode ? 1 : 0,
					bUsingCharacterVisual ? 1 : 0,
					bUsingStaticCharacterVisual ? 1 : 0,
					(SkeletalMesh && SkeletalMesh->IsVisible()) ? 1 : 0,
					(SkeletalMesh && SkeletalMesh->bHiddenInGame) ? 1 : 0,
					SkeletalMesh ? static_cast<int32>(SkeletalMesh->GetAnimationMode()) : -1,
					*T66CompanionAnimDebugMeshName(SkeletalMesh),
					*T66CompanionAnimDebugAssetName(CachedIdleAnim),
					*T66CompanionAnimDebugAssetName(CachedWalkAnim),
					*T66CompanionAnimDebugAssetName(CachedJumpAnim),
					*T66CompanionAnimDebugAssetName(CachedRollAnim));
			}
		}
	}
	return bUsingCharacterVisual;
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
	const bool bWasPreviewMode = bIsPreviewMode;
	bIsPreviewMode = bPreview;
	if (T66CompanionAnimDebugEnabled())
	{
		UE_LOG(LogTemp, Display,
			TEXT("[CompanionAnimDebug] SetPreviewMode CompanionID=%s WasPreview=%d NewPreview=%d UsingCharacter=%d StaticFallback=%d WillReapply=%d Idle=%s Walk=%s Jump=%s Roll=%s"),
			*CompanionID.ToString(),
			bWasPreviewMode ? 1 : 0,
			bIsPreviewMode ? 1 : 0,
			bUsingCharacterVisual ? 1 : 0,
			bUsingStaticCharacterVisual ? 1 : 0,
			bUsingStaticCharacterVisual ? 1 : 0,
			*T66CompanionAnimDebugAssetName(CachedIdleAnim),
			*T66CompanionAnimDebugAssetName(CachedWalkAnim),
			*T66CompanionAnimDebugAssetName(CachedJumpAnim),
			*T66CompanionAnimDebugAssetName(CachedRollAnim));
	}
	if (bUsingStaticCharacterVisual)
	{
		ApplyCurrentCharacterVisual();
		return;
	}

	// Make preview easier to see in UI (only affects placeholder).
	if (PlaceholderMesh && PlaceholderMesh->IsVisible())
	{
		const FVector NewScale = bIsPreviewMode ? T66CompanionPreviewPlaceholderScale : T66CompanionGameplayPlaceholderScale;
		PlaceholderMesh->SetRelativeScale3D(NewScale);
		PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * NewScale.Z));
	}
}

void AT66CompanionBase::SetLockedVisual(bool bLocked)
{
	bLockedVisual = bLocked;

	if (bLockedVisual)
	{
		// Locked silhouette: hide skeletal visuals and tint the visible static fallback black.
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
	if (bUsingStaticCharacterVisual)
	{
		ApplyCurrentCharacterVisual();
		return;
	}

	if (bUsingCharacterVisual && !bUsingStaticCharacterVisual && SkeletalMesh)
	{
		SkeletalMesh->SetHiddenInGame(false, true);
		SkeletalMesh->SetVisibility(true, true);
	}
	if (PlaceholderMesh)
	{
		const bool bShowPlaceholder = !bUsingCharacterVisual || bUsingStaticCharacterVisual;
		PlaceholderMesh->SetVisibility(bShowPlaceholder, true);
		PlaceholderMesh->SetHiddenInGame(!bShowPlaceholder, true);
	}
	if (!bUsingStaticCharacterVisual)
	{
		SetPlaceholderColor(CompanionData.PlaceholderColor);
	}
}

void AT66CompanionBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsPreviewMode) return;

	// Animation: mirror the hero's idle/walk/jump/roll movement state.
	const bool bAnimGuardPasses = bUsingCharacterVisual && SkeletalMesh && SkeletalMesh->IsVisible() && (CachedIdleAnim || CachedJumpAnim || CachedWalkAnim || CachedRollAnim);
	if (!bAnimGuardPasses && T66CompanionAnimDebugEnabled() && !bCompanionAnimDebugLoggedGuardFailure)
	{
		bCompanionAnimDebugLoggedGuardFailure = true;
		UE_LOG(LogTemp, Warning,
			TEXT("[CompanionAnimDebug] GuardFailed CompanionID=%s Preview=%d UsingCharacter=%d StaticFallback=%d HasSkeletal=%d MeshVisible=%d MeshHidden=%d AnimMode=%d Mesh=%s Idle=%s Walk=%s Jump=%s Roll=%s"),
			*CompanionID.ToString(),
			bIsPreviewMode ? 1 : 0,
			bUsingCharacterVisual ? 1 : 0,
			bUsingStaticCharacterVisual ? 1 : 0,
			SkeletalMesh ? 1 : 0,
			(SkeletalMesh && SkeletalMesh->IsVisible()) ? 1 : 0,
			(SkeletalMesh && SkeletalMesh->bHiddenInGame) ? 1 : 0,
			SkeletalMesh ? static_cast<int32>(SkeletalMesh->GetAnimationMode()) : -1,
			*T66CompanionAnimDebugMeshName(SkeletalMesh),
			*T66CompanionAnimDebugAssetName(CachedIdleAnim),
			*T66CompanionAnimDebugAssetName(CachedWalkAnim),
			*T66CompanionAnimDebugAssetName(CachedJumpAnim),
			*T66CompanionAnimDebugAssetName(CachedRollAnim));
	}
	if (bAnimGuardPasses)
	{
		if (CachedHeroSpeedSubsystem)
		{
			const int32 NewState = CachedHeroSpeedSubsystem->GetMovementAnimState(); // 0=Idle, 1=Walk, 2=Jump, 3=Roll
			if (NewState != LastMovementAnimState)
			{
				LastMovementAnimState = static_cast<uint8>(NewState);
				UAnimationAsset* ToPlay = nullptr;
				bool bLoop = true;
				if (NewState == 0)
				{
					ToPlay = CachedIdleAnim;
				}
				else if (NewState == 2)
				{
					ToPlay = CachedJumpAnim ? CachedJumpAnim : (CachedWalkAnim ? CachedWalkAnim : CachedIdleAnim);
					bLoop = false;
				}
				else if (NewState == 3)
				{
					ToPlay = CachedRollAnim ? CachedRollAnim : (CachedWalkAnim ? CachedWalkAnim : CachedIdleAnim);
					bLoop = false;
				}
				else
				{
					ToPlay = CachedWalkAnim ? CachedWalkAnim : CachedIdleAnim;
				}
				if (ToPlay)
				{
					if (T66CompanionAnimDebugEnabled())
					{
						UE_LOG(LogTemp, Display,
							TEXT("[CompanionAnimDebug] PlayStateChange CompanionID=%s Source=HeroSpeedSubsystem State=%d Selected=%s Loop=%d BeforePlaying=%d Position=%.3f AnimMode=%d Mesh=%s"),
							*CompanionID.ToString(),
							NewState,
							*T66CompanionAnimDebugAssetName(ToPlay),
							bLoop ? 1 : 0,
							SkeletalMesh->IsPlaying() ? 1 : 0,
							SkeletalMesh->GetPosition(),
							static_cast<int32>(SkeletalMesh->GetAnimationMode()),
							*T66CompanionAnimDebugMeshName(SkeletalMesh));
					}
					SkeletalMesh->PlayAnimation(ToPlay, bLoop);
					if (T66CompanionAnimDebugEnabled())
					{
						UE_LOG(LogTemp, Display,
							TEXT("[CompanionAnimDebug] PlayStateApplied CompanionID=%s Source=HeroSpeedSubsystem State=%d Selected=%s AfterPlaying=%d Position=%.3f AnimMode=%d PlayRate=%.3f"),
							*CompanionID.ToString(),
							NewState,
							*T66CompanionAnimDebugAssetName(ToPlay),
							SkeletalMesh->IsPlaying() ? 1 : 0,
							SkeletalMesh->GetPosition(),
							static_cast<int32>(SkeletalMesh->GetAnimationMode()),
							SkeletalMesh->GetPlayRate());
					}
				}
				else if (T66CompanionAnimDebugEnabled() && !bCompanionAnimDebugLoggedNullSelection)
				{
					bCompanionAnimDebugLoggedNullSelection = true;
					UE_LOG(LogTemp, Warning,
						TEXT("[CompanionAnimDebug] NullSelection CompanionID=%s State=%d Idle=%s Walk=%s Jump=%s Roll=%s"),
						*CompanionID.ToString(),
						NewState,
						*T66CompanionAnimDebugAssetName(CachedIdleAnim),
						*T66CompanionAnimDebugAssetName(CachedWalkAnim),
						*T66CompanionAnimDebugAssetName(CachedJumpAnim),
						*T66CompanionAnimDebugAssetName(CachedRollAnim));
				}
			}
		}
		else if (T66CompanionAnimDebugEnabled() && !bCompanionAnimDebugLoggedMissingStateSource)
		{
			bCompanionAnimDebugLoggedMissingStateSource = true;
			UE_LOG(LogTemp, Warning,
				TEXT("[CompanionAnimDebug] MissingHeroSpeedSubsystem CompanionID=%s UsingCharacter=%d MeshVisible=%d Idle=%s Walk=%s Jump=%s Roll=%s"),
				*CompanionID.ToString(),
				bUsingCharacterVisual ? 1 : 0,
				(SkeletalMesh && SkeletalMesh->IsVisible()) ? 1 : 0,
				*T66CompanionAnimDebugAssetName(CachedIdleAnim),
				*T66CompanionAnimDebugAssetName(CachedWalkAnim),
				*T66CompanionAnimDebugAssetName(CachedJumpAnim),
				*T66CompanionAnimDebugAssetName(CachedRollAnim));
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

	// Heal the hero in fixed difficulty bands. Unity is progression only.
	ET66Difficulty SelectedDifficulty = ET66Difficulty::Easy;
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
	{
		SelectedDifficulty = T66GI->SelectedDifficulty;
	}
	const float HealAmount = GetHealingAmountForDifficulty(SelectedDifficulty);
	const float HealIntervalSeconds = GetHealingIntervalSecondsForDifficulty(SelectedDifficulty);
	if (CachedRunStateSubsystem)
	{
		if (CachedRunStateSubsystem->GetCurrentHP() >= CachedRunStateSubsystem->GetMaxHP())
		{
			CompanionHealAccumulatorSeconds = 0.f;
			return;
		}

		CompanionHealAccumulatorSeconds += DeltaTime;
		if (HealAmount > 0.f
			&& HealIntervalSeconds > 0.f
			&& CompanionHealAccumulatorSeconds >= HealIntervalSeconds)
		{
			CompanionHealAccumulatorSeconds = FMath::Fmod(CompanionHealAccumulatorSeconds, HealIntervalSeconds);
			CachedRunStateSubsystem->HealHPFromCompanion(HealAmount);
		}
	}
}

void AT66CompanionBase::ApplyCompanionScale()
{
	SetActorScale3D(FVector(FMath::Max(0.1f, CompanionActorScale)));
}
