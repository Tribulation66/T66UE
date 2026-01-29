// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ItemPickup.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AT66ItemPickup::AT66ItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetSphereRadius(80.f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RootComponent = SphereComponent;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere)
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.4f));
	}
}

void AT66ItemPickup::SetItemID(FName InItemID)
{
	ItemID = InItemID;
	// Red ball drop: always show red
	FLinearColor RedColor(1.f, 0.f, 0.f, 1.f);
	if (UMaterialInstanceDynamic* DynMat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynMat->SetVectorParameterValue(TEXT("BaseColor"), RedColor);
		DynMat->SetVectorParameterValue(TEXT("Color"), RedColor);
	}
}

void AT66ItemPickup::BeginPlay()
{
	Super::BeginPlay();
	if (!ItemID.IsNone())
	{
		SetItemID(ItemID);
	}
}
