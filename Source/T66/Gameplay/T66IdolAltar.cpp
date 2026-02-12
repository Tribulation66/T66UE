// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66IdolAltar.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPath.h"
#include "Gameplay/T66VisualUtil.h"

AT66IdolAltar::AT66IdolAltar()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractTrigger"));
	InteractTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractTrigger->SetBoxExtent(FVector(180.f, 180.f, 140.f));
	RootComponent = InteractTrigger;

	BaseRect = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseRect"));
	BaseRect->SetupAttachment(RootComponent);
	BaseRect->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MidRect = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MidRect"));
	MidRect->SetupAttachment(RootComponent);
	MidRect->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TopRect = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopRect"));
	TopRect->SetupAttachment(RootComponent);
	TopRect->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetVisibility(false, true);

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		BaseRect->SetStaticMesh(Cube);
		MidRect->SetStaticMesh(Cube);
		TopRect->SetStaticMesh(Cube);
	}

	// Default expected import location (safe if missing).
	AltarMeshOverride = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/SM_IdolAltar.SM_IdolAltar")));

	// Pyramid-like stacking: wide base, smaller mid, smallest top.
	// Grounded stack (bottom of base at Z=0).
	BaseRect->SetRelativeLocation(FVector(0.f, 0.f, 12.5f));
	BaseRect->SetRelativeScale3D(FVector(3.2f, 2.2f, 0.25f));

	MidRect->SetRelativeLocation(FVector(0.f, 0.f, 36.f));
	MidRect->SetRelativeScale3D(FVector(2.2f, 1.5f, 0.22f));

	TopRect->SetRelativeLocation(FVector(0.f, 0.f, 57.f));
	TopRect->SetRelativeScale3D(FVector(1.4f, 0.95f, 0.20f));

	ApplyVisuals();
}

void AT66IdolAltar::BeginPlay()
{
	Super::BeginPlay();

	// Swap to imported mesh if present.
	if (VisualMesh && !AltarMeshOverride.IsNull())
	{
		if (UStaticMesh* M = AltarMeshOverride.LoadSynchronous())
		{
			VisualMesh->SetStaticMesh(M);
			VisualMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
			const float HalfHeight = FMath::Max(1.f, M->GetBounds().BoxExtent.Z);
			VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, HalfHeight));
			VisualMesh->SetVisibility(true, true);

			// Hide placeholder stack.
			if (BaseRect) BaseRect->SetVisibility(false, true);
			if (MidRect) MidRect->SetVisibility(false, true);
			if (TopRect) TopRect->SetVisibility(false, true);
		}
	}

	// Robust: snap the altar to ground so it's never floating.
	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		const FVector Here = GetActorLocation();
		const FVector Start = Here + FVector(0.f, 0.f, 2000.f);
		const FVector End = Here - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void AT66IdolAltar::ApplyVisuals()
{
	const FLinearColor C0(0.10f, 0.10f, 0.12f, 1.f);
	const FLinearColor C1(0.12f, 0.12f, 0.16f, 1.f);
	const FLinearColor C2(0.14f, 0.14f, 0.20f, 1.f);

	if (VisualMesh && VisualMesh->IsVisible())
	{
		// Imported mesh: leave materials as-authored.
		return;
	}

	if (UMaterialInstanceDynamic* M = BaseRect->CreateAndSetMaterialInstanceDynamic(0))
	{
		M->SetVectorParameterValue(TEXT("BaseColor"), C0);
	}
	if (UMaterialInstanceDynamic* M = MidRect->CreateAndSetMaterialInstanceDynamic(0))
	{
		M->SetVectorParameterValue(TEXT("BaseColor"), C1);
	}
	if (UMaterialInstanceDynamic* M = TopRect->CreateAndSetMaterialInstanceDynamic(0))
	{
		M->SetVectorParameterValue(TEXT("BaseColor"), C2);
	}
}

