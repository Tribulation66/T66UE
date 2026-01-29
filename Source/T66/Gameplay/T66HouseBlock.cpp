// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HouseBlock.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

AT66HouseBlock::AT66HouseBlock()
{
	PrimaryActorTick.bCanEverTick = false;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	RootComponent = VisualMesh;
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(6.f, 6.f, 4.f)); // big cube house
	}
}

void AT66HouseBlock::BeginPlay()
{
	Super::BeginPlay();
	ApplyVisuals();
}

void AT66HouseBlock::ApplyVisuals()
{
	UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor"));
	if (VisualMesh)
	{
		if (ColorMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this);
			if (Mat)
			{
				Mat->SetVectorParameterValue(FName("Color"), HouseColor);
				VisualMesh->SetMaterial(0, Mat);
			}
		}
	}
}

