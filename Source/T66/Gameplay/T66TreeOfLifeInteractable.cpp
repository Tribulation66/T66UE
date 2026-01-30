// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TreeOfLifeInteractable.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

static void ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
{
	if (!Mesh) return;
	if (UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor")))
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, Outer ? Outer : Mesh))
		{
			Mat->SetVectorParameterValue(FName("Color"), Color);
			Mesh->SetMaterial(0, Mat);
			return;
		}
	}
	if (UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(FName("Color"), Color);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

AT66TreeOfLifeInteractable::AT66TreeOfLifeInteractable()
{
	// Trunk: cylinder
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		VisualMesh->SetStaticMesh(Cylinder);
		VisualMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 2.0f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	}

	// Crown: sphere
	CrownMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrownMesh"));
	CrownMesh->SetupAttachment(RootComponent);
	CrownMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		CrownMesh->SetStaticMesh(Sphere);
		CrownMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.2f));
		CrownMesh->SetRelativeLocation(FVector(0.f, 0.f, 260.f));
	}
	ApplyRarityVisuals();
}

void AT66TreeOfLifeInteractable::ApplyRarityVisuals()
{
	const FLinearColor R = FT66RarityUtil::GetRarityColor(Rarity);
	ApplyT66Color(VisualMesh, this, R);
	ApplyT66Color(CrownMesh, this, R);
}

bool AT66TreeOfLifeInteractable::Interact(APlayerController* PC)
{
	if (bConsumed) return false;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;

	int32 DeltaHearts = 1;
	switch (Rarity)
	{
	case ET66Rarity::Black:  DeltaHearts = 1; break;
	case ET66Rarity::Red:    DeltaHearts = 3; break;
	case ET66Rarity::Yellow: DeltaHearts = 5; break;
	case ET66Rarity::White:  DeltaHearts = 10; break;
	default:                 DeltaHearts = 1; break;
	}

	RunState->AddMaxHearts(DeltaHearts);
	bConsumed = true;
	Destroy();
	return true;
}

