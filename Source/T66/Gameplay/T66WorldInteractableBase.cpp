// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WorldInteractableBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

static void ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
{
	if (!Mesh) return;

	// Prefer our placeholder material so the color parameter name is stable ("Color").
	if (UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor")))
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, Outer ? Outer : Mesh))
		{
			Mat->SetVectorParameterValue(FName("Color"), Color);
			Mesh->SetMaterial(0, Mat);
			return;
		}
	}

	// Fallback to whatever material is on the mesh.
	if (UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(FName("Color"), Color);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

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
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
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
	ApplyT66Color(VisualMesh, this, FT66RarityUtil::GetRarityColor(Rarity));
}

