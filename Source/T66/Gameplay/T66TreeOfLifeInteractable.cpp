// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TreeOfLifeInteractable.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

AT66TreeOfLifeInteractable::AT66TreeOfLifeInteractable()
{
	// Trunk: cylinder
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		VisualMesh->SetStaticMesh(Cylinder);
		VisualMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 2.0f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	}

	// Crown: sphere
	CrownMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrownMesh"));
	CrownMesh->SetupAttachment(RootComponent);
	CrownMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		CrownMesh->SetStaticMesh(Sphere);
		CrownMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.2f));
		CrownMesh->SetRelativeLocation(FVector(0.f, 0.f, 260.f));
	}

	// Default expected import locations (safe if missing).
	MeshBlack = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Trees/SM_Tree_Black.SM_Tree_Black")));
	MeshRed = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Trees/SM_Tree_Red.SM_Tree_Red")));
	MeshYellow = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Trees/SM_Tree_Yellow.SM_Tree_Yellow")));
	MeshWhite = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Trees/SM_Tree_White.SM_Tree_White")));

	ApplyRarityVisuals();
}

void AT66TreeOfLifeInteractable::ApplyRarityVisuals()
{
	auto PickMesh = [&]() -> const TSoftObjectPtr<UStaticMesh>&
	{
		switch (Rarity)
		{
			case ET66Rarity::Black: return MeshBlack;
			case ET66Rarity::Red: return MeshRed;
			case ET66Rarity::Yellow: return MeshYellow;
			case ET66Rarity::White: return MeshWhite;
			default: return MeshBlack;
		}
	};

	UStaticMesh* M = nullptr;
	const TSoftObjectPtr<UStaticMesh>& Ptr = PickMesh();
	if (!Ptr.IsNull())
	{
		M = Ptr.Get();
		if (!M)
		{
			M = Ptr.LoadSynchronous();
		}
	}

	if (M && VisualMesh)
	{
		VisualMesh->SetStaticMesh(M);
		VisualMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
		const float HalfHeight = FMath::Max(1.f, M->GetBounds().BoxExtent.Z);
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, HalfHeight));

		if (CrownMesh)
		{
			CrownMesh->SetVisibility(false, true);
		}
		return;
	}

	const FLinearColor R = FT66RarityUtil::GetRarityColor(Rarity);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, R);
	FT66VisualUtil::ApplyT66Color(CrownMesh, this, R);
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

