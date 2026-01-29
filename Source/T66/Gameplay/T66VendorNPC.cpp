// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VendorNPC.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

AT66VendorNPC::AT66VendorNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetSphereRadius(150.f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RootComponent = SphereComponent;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cylinder)
	{
		VisualMesh->SetStaticMesh(Cylinder);
		VisualMesh->SetRelativeScale3D(FVector(2.f, 2.f, 3.f));
		UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (Mat)
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.3f, 0.5f, 0.8f, 1.f));
		}
	}
}

bool AT66VendorNPC::TrySellFirstItem()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;
	return RunState->SellFirstItem();
}
