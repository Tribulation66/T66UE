// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WeaponAltar.h"

#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66VisualUtil.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/SoftObjectPath.h"

AT66WeaponAltar::AT66WeaponAltar()
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
		TopRect->SetStaticMesh(Cube);
	}

	AltarMeshOverride = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/WeaponAltar/SM_WeaponAltar_Pixal3D.SM_WeaponAltar_Pixal3D")));

	BaseRect->SetRelativeLocation(FVector(0.f, 0.f, 14.f * VisualScaleMultiplier));
	BaseRect->SetRelativeScale3D(FVector(2.8f, 1.7f, 0.28f) * VisualScaleMultiplier);

	TopRect->SetRelativeLocation(FVector(0.f, 0.f, 44.f * VisualScaleMultiplier));
	TopRect->SetRelativeScale3D(FVector(1.45f, 1.05f, 0.32f) * VisualScaleMultiplier);

	ApplyVisuals();
}

void AT66WeaponAltar::BeginPlay()
{
	Super::BeginPlay();

	if (VisualMesh && !AltarMeshOverride.IsNull())
	{
		if (UStaticMesh* Mesh = AltarMeshOverride.LoadSynchronous())
		{
			VisualMesh->SetStaticMesh(Mesh);
			VisualMesh->SetRelativeScale3D(FVector(VisualScaleMultiplier));
			VisualMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
			FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh, Mesh);
			VisualMesh->SetVisibility(true, true);

			if (BaseRect) BaseRect->SetVisibility(false, true);
			if (TopRect) TopRect->SetVisibility(false, true);
		}
	}

	FT66VisualUtil::SnapToGround(this, GetWorld());
	UpdateInteractionBounds();
}

void AT66WeaponAltar::ApplyVisuals()
{
	if (VisualMesh && VisualMesh->IsVisible())
	{
		return;
	}

	if (UMaterialInstanceDynamic* Material = BaseRect ? BaseRect->CreateAndSetMaterialInstanceDynamic(0) : nullptr)
	{
		Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.09f, 0.08f, 0.10f, 1.f));
	}
	if (UMaterialInstanceDynamic* Material = TopRect ? TopRect->CreateAndSetMaterialInstanceDynamic(0) : nullptr)
	{
		Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.46f, 0.05f, 0.10f, 1.f));
	}
}

void AT66WeaponAltar::LinkToTowerGateFloor(const int32 FromFloorNumber)
{
	LinkedTowerGateFloorNumber = FromFloorNumber;
}

void AT66WeaponAltar::NotifySelectionCommitted()
{
	if (LinkedTowerGateFloorNumber == INDEX_NONE || RemainingSelections > 0)
	{
		return;
	}

	if (AT66GameMode* GameMode = GetWorld() ? Cast<AT66GameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		GameMode->NotifyTowerWeaponSelectionForGate(LinkedTowerGateFloorNumber);
	}
}

void AT66WeaponAltar::ConfigureVisualCollision(UPrimitiveComponent* Primitive, const bool bEnableCollision) const
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

void AT66WeaponAltar::UpdateInteractionBounds()
{
	if (!InteractTrigger)
	{
		return;
	}

	const FTransform ActorTransform = GetActorTransform();
	FVector RequiredExtent = MinimumInteractionExtent.ComponentMax(FVector::ZeroVector);
	const FVector BoundsPadding = InteractionBoundsPadding.ComponentMax(FVector::ZeroVector);

	TArray<UPrimitiveComponent*> VisualPrimitives = { BaseRect, TopRect, VisualMesh };
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
