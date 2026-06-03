// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66RecruitableCompanion.h"

#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RecruitableCompanion, Log, All);

AT66RecruitableCompanion::AT66RecruitableCompanion()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetSphereRadius(150.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_Pawn);
	InteractionSphere->SetGenerateOverlapEvents(true);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = InteractionSphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		VisualMesh->SetStaticMesh(Cylinder);
		// About hero-sized cylinder (placeholder).
		VisualMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.05f));
		// Keep centered; SnapToGround() positions the actor so the cylinder sits on the floor.
		VisualMesh->SetRelativeLocation(FVector::ZeroVector);
	}

	UStaticMesh* CageCube = FT66VisualUtil::GetBasicShapeCube();
	auto ConfigureCageBar = [CageCube, this](UStaticMeshComponent* CageBar, const FVector& RelativeLocation, const FVector& RelativeScale)
	{
		if (!CageBar)
		{
			return;
		}

		CageBar->SetupAttachment(RootComponent);
		CageBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CageBar->SetGenerateOverlapEvents(false);
		CageBar->SetVisibility(false, true);
		CageBar->SetHiddenInGame(true, true);
		CageBar->SetRelativeLocation(RelativeLocation);
		CageBar->SetRelativeScale3D(RelativeScale);
		if (CageCube)
		{
			CageBar->SetStaticMesh(CageCube);
		}
	};

	CageBarFrontLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageBarFrontLeft"));
	CageBarFrontRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageBarFrontRight"));
	CageBarBackLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageBarBackLeft"));
	CageBarBackRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageBarBackRight"));
	CageTopBar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CageTopBar"));

	ConfigureCageBar(CageBarFrontLeft, FVector(72.f, 72.f, 92.f), FVector(0.07f, 0.07f, 1.85f));
	ConfigureCageBar(CageBarFrontRight, FVector(72.f, -72.f, 92.f), FVector(0.07f, 0.07f, 1.85f));
	ConfigureCageBar(CageBarBackLeft, FVector(-72.f, 72.f, 92.f), FVector(0.07f, 0.07f, 1.85f));
	ConfigureCageBar(CageBarBackRight, FVector(-72.f, -72.f, 92.f), FVector(0.07f, 0.07f, 1.85f));
	ConfigureCageBar(CageTopBar, FVector(0.f, 0.f, 188.f), FVector(1.55f, 1.55f, 0.06f));

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetVisibility(false, true);
}

void AT66RecruitableCompanion::BeginPlay()
{
	Super::BeginPlay();

	if (VisualMesh && VisualMesh->GetMaterial(0))
	{
		PlaceholderMaterial = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	// If we are still a placeholder cylinder, snap so it sits on ground.
	// If we have a character visual, treat actor origin as ground contact point (like companions).
	SnapToGround(bUsingCharacterVisual);

	ApplyCageColor(FLinearColor(0.95f, 0.58f, 0.12f, 1.f));
	SetCageVisualsVisible(bLockedInBossCage);
}

void AT66RecruitableCompanion::ApplyPlaceholderColor(const FLinearColor& Color)
{
	if (!VisualMesh) return;

	UMaterialInterface* ColorMat = FT66VisualUtil::GetFlatColorMaterial();
	if (!ColorMat) ColorMat = VisualMesh->GetMaterial(0);
	if (!ColorMat) return;

	PlaceholderMaterial = UMaterialInstanceDynamic::Create(ColorMat, this);
	if (!PlaceholderMaterial) return;

	VisualMesh->SetMaterial(0, PlaceholderMaterial);
	FT66VisualUtil::ConfigureFlatColorMaterial(PlaceholderMaterial, Color);
}

void AT66RecruitableCompanion::ApplyCageColor(const FLinearColor& Color)
{
	FT66VisualUtil::ApplyT66Color(CageBarFrontLeft, this, Color);
	FT66VisualUtil::ApplyT66Color(CageBarFrontRight, this, Color);
	FT66VisualUtil::ApplyT66Color(CageBarBackLeft, this, Color);
	FT66VisualUtil::ApplyT66Color(CageBarBackRight, this, Color);
	FT66VisualUtil::ApplyT66Color(CageTopBar, this, Color);
}

void AT66RecruitableCompanion::SetCageVisualsVisible(const bool bVisible)
{
	auto SetBarVisible = [bVisible](UStaticMeshComponent* CageBar)
	{
		if (!CageBar)
		{
			return;
		}

		CageBar->SetVisibility(bVisible, true);
		CageBar->SetHiddenInGame(!bVisible, true);
	};

	SetBarVisible(CageBarFrontLeft);
	SetBarVisible(CageBarFrontRight);
	SetBarVisible(CageBarBackLeft);
	SetBarVisible(CageBarBackRight);
	SetBarVisible(CageTopBar);
}

void AT66RecruitableCompanion::InitializeRecruit(const FCompanionData& InData)
{
	CompanionID = InData.CompanionID;
	CompanionData = InData;

	ApplyPlaceholderColor(InData.PlaceholderColor);

	bUsingCharacterVisual = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			// If a skeletal mesh visual exists, it hides the placeholder cylinder.
			bUsingCharacterVisual = Visuals->ApplyCharacterVisual(CompanionID, SkeletalMesh, VisualMesh, true, false, false, VisualMesh);
			if (!bUsingCharacterVisual && SkeletalMesh)
			{
				SkeletalMesh->SetVisibility(false, true);
			}
		}
	}

	// If we are using a character visual, actor origin should be ground contact.
	SnapToGround(bUsingCharacterVisual);
}

void AT66RecruitableCompanion::SetCagedForBossReward()
{
	bBossCageUnlockReward = true;
	bLockedInBossCage = true;
	bFreedFromBossCage = false;
	bUnlockGrantedFromBossCage = false;

	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	SetCageVisualsVisible(true);

	UE_LOG(LogT66RecruitableCompanion, Log, TEXT("[CompanionCage] Caged CompanionID=%s Actor=%s"),
		*CompanionID.ToString(),
		*GetNameSafe(this));
}

void AT66RecruitableCompanion::FreeFromBossCage()
{
	if (!bBossCageUnlockReward)
	{
		return;
	}

	bLockedInBossCage = false;
	bFreedFromBossCage = true;

	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	SetCageVisualsVisible(false);

	UE_LOG(LogT66RecruitableCompanion, Log, TEXT("[CompanionCage] Freed CompanionID=%s Actor=%s"),
		*CompanionID.ToString(),
		*GetNameSafe(this));
}

void AT66RecruitableCompanion::SnapToGround(bool bTreatOriginAsGroundContact)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FHitResult Hit;
	const FVector Here = GetActorLocation();
	const FVector Start = Here + FVector(0.f, 0.f, 2000.f);
	const FVector End = Here - FVector(0.f, 0.f, 6000.f);
	if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
	{
		return;
	}

	if (bTreatOriginAsGroundContact)
	{
		SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		const float HalfHeight = (VisualMesh ? VisualMesh->Bounds.BoxExtent.Z : 52.5f);
		SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight), false, nullptr, ETeleportType::TeleportPhysics);
	}
}

bool AT66RecruitableCompanion::Interact(APlayerController* PC)
{
	if (!PC) return false;

	if (bBossCageUnlockReward && !bFreedFromBossCage)
	{
		UE_LOG(LogT66RecruitableCompanion, Verbose, TEXT("[CompanionCage] InteractionBlocked Locked=1 CompanionID=%s Actor=%s"),
			*CompanionID.ToString(),
			*GetNameSafe(this));
		return false;
	}

	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC) return false;

	if (bBossCageUnlockReward && !bUnlockGrantedFromBossCage && !CompanionID.IsNone())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
			{
				const bool bNewlyUnlocked = Unlocks->UnlockCompanion(CompanionID);
				bUnlockGrantedFromBossCage = true;
				UE_LOG(LogT66RecruitableCompanion, Log, TEXT("[CompanionCage] InteractUnlock CompanionID=%s NewlyUnlocked=%d Actor=%s"),
					*CompanionID.ToString(),
					bNewlyUnlocked ? 1 : 0,
					*GetNameSafe(this));
			}
		}
	}

	// PlayerController owns the HUD-rendered world dialogue.
	T66PC->OpenWorldDialogueCompanion(this);
	return true;
}

