// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66RecruitableCompanion.h"

#include "Core/T66CharacterVisualSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

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
}

void AT66RecruitableCompanion::ApplyPlaceholderColor(const FLinearColor& Color)
{
	if (!VisualMesh) return;

	UMaterialInterface* ColorMat = FT66VisualUtil::GetPlaceholderColorMaterial();
	if (!ColorMat) ColorMat = VisualMesh->GetMaterial(0);
	if (!ColorMat) return;

	PlaceholderMaterial = UMaterialInstanceDynamic::Create(ColorMat, this);
	if (!PlaceholderMaterial) return;

	VisualMesh->SetMaterial(0, PlaceholderMaterial);
	PlaceholderMaterial->SetVectorParameterValue(FName("Color"), Color);
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
			bUsingCharacterVisual = Visuals->ApplyCharacterVisual(CompanionID, SkeletalMesh, VisualMesh, true);
			if (!bUsingCharacterVisual && SkeletalMesh)
			{
				SkeletalMesh->SetVisibility(false, true);
			}
		}
	}

	// If we are using a character visual, actor origin should be ground contact.
	SnapToGround(bUsingCharacterVisual);
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
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC) return false;

	// PlayerController owns the HUD-rendered world dialogue.
	T66PC->OpenWorldDialogueCompanion(this);
	return true;
}

