// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66IdolAltar.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
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
	InteractTrigger->SetCollisionObjectType(ECC_WorldDynamic);
	InteractTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractTrigger->SetBoxExtent(MinimumInteractionExtent);
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
	VisualMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

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
	BaseRect->SetRelativeLocation(FVector(0.f, 0.f, 12.5f * VisualScaleMultiplier));
	BaseRect->SetRelativeScale3D(FVector(3.2f, 2.2f, 0.25f) * VisualScaleMultiplier);

	MidRect->SetRelativeLocation(FVector(0.f, 0.f, 36.f * VisualScaleMultiplier));
	MidRect->SetRelativeScale3D(FVector(2.2f, 1.5f, 0.22f) * VisualScaleMultiplier);

	TopRect->SetRelativeLocation(FVector(0.f, 0.f, 57.f * VisualScaleMultiplier));
	TopRect->SetRelativeScale3D(FVector(1.4f, 0.95f, 0.20f) * VisualScaleMultiplier);

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
			VisualMesh->SetRelativeScale3D(FVector(VisualScaleMultiplier));
			VisualMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
			FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh, M);
			VisualMesh->SetVisibility(true, true);

			// Hide placeholder stack.
			if (BaseRect) BaseRect->SetVisibility(false, true);
			if (MidRect) MidRect->SetVisibility(false, true);
			if (TopRect) TopRect->SetVisibility(false, true);
		}
	}

	FT66VisualUtil::SnapToGround(this, GetWorld());
	UpdateInteractionBounds();
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

void AT66IdolAltar::ConfigureVisualCollision(UPrimitiveComponent* Primitive, bool bEnableCollision) const
{
	if (!Primitive)
	{
		return;
	}

	if (bEnableCollision)
	{
		Primitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Primitive->SetCollisionObjectType(ECC_WorldDynamic);
		Primitive->SetCollisionResponseToAllChannels(ECR_Block);
		Primitive->SetCanEverAffectNavigation(false);
	}
	else
	{
		Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AT66IdolAltar::UpdateInteractionBounds()
{
	if (!InteractTrigger)
	{
		return;
	}

	const FTransform ActorTransform = GetActorTransform();
	FVector RequiredExtent = MinimumInteractionExtent.ComponentMax(FVector::ZeroVector);
	const FVector BoundsPadding = InteractionBoundsPadding.ComponentMax(FVector::ZeroVector);

	TArray<UPrimitiveComponent*> VisualPrimitives = { BaseRect, MidRect, TopRect, VisualMesh };
	for (UPrimitiveComponent* Primitive : VisualPrimitives)
	{
		if (!Primitive || !Primitive->IsRegistered())
		{
			continue;
		}

		const bool bVisible = Primitive->IsVisible() && !Primitive->bHiddenInGame;
		ConfigureVisualCollision(Primitive, bVisible);
		if (!bVisible)
		{
			continue;
		}

		const FBoxSphereBounds Bounds = Primitive->CalcBounds(Primitive->GetComponentTransform());
		const FVector LocalCenter = ActorTransform.InverseTransformPosition(Bounds.Origin);
		const FVector PrimitiveExtent(
			FMath::Abs(LocalCenter.X) + Bounds.SphereRadius + BoundsPadding.X,
			FMath::Abs(LocalCenter.Y) + Bounds.SphereRadius + BoundsPadding.Y,
			FMath::Abs(LocalCenter.Z) + Bounds.SphereRadius + BoundsPadding.Z);
		RequiredExtent.X = FMath::Max(RequiredExtent.X, PrimitiveExtent.X);
		RequiredExtent.Y = FMath::Max(RequiredExtent.Y, PrimitiveExtent.Y);
		RequiredExtent.Z = FMath::Max(RequiredExtent.Z, PrimitiveExtent.Z);
	}

	InteractTrigger->SetBoxExtent(RequiredExtent);
}

