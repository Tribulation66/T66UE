// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WorldInteractableBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/SoftObjectPath.h"

AT66WorldInteractableBase::AT66WorldInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetBoxExtent(FVector(140.f, 140.f, 200.f));
	RootComponent = TriggerBox;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
	}
	ApplyRarityVisuals();
}

void AT66WorldInteractableBase::SetRarity(ET66Rarity InRarity)
{
	Rarity = InRarity;
	ApplyRarityVisuals();
}

void AT66WorldInteractableBase::ApplyRarityVisuals()
{
	if (!VisualMesh) return;
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FT66RarityUtil::GetRarityColor(Rarity));
}

bool AT66WorldInteractableBase::TryApplyImportedMesh()
{
	if (!VisualMesh) return false;

	UStaticMesh* M = nullptr;

	if (RarityMeshes.Num() > 0)
	{
		const TSoftObjectPtr<UStaticMesh>* Ptr = RarityMeshes.Find(Rarity);
		if (Ptr && !Ptr->IsNull())
		{
			M = Ptr->Get();
			if (!M) M = Ptr->LoadSynchronous();
		}
	}

	if (!M && !SingleMesh.IsNull())
	{
		M = SingleMesh.Get();
		if (!M) M = SingleMesh.LoadSynchronous();
	}

	if (!M) return false;

	VisualMesh->EmptyOverrideMaterials();
	VisualMesh->SetStaticMesh(M);
	VisualMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
	const FBoxSphereBounds B = M->GetBounds();
	const float BottomZ = (B.Origin.Z - B.BoxExtent.Z);
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -BottomZ));
	return true;
}

