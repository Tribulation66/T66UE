// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VendorNPC.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"

AT66VendorNPC::AT66VendorNPC()
{
	NPCID = FName(TEXT("VendorNPC"));
	NPCName = NSLOCTEXT("T66.NPC", "VendorNPC", "Vendor");
	NPCColor = FLinearColor(0.16f, 0.58f, 0.35f, 1.f);
	bPreserveVisualMeshMaterials = true;
}

void AT66VendorNPC::BeginPlay()
{
	bPreserveVisualMeshMaterials = true;
	Super::BeginPlay();
	ApplyVendorNPCStaticVisual();
}

bool AT66VendorNPC::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC)
	{
		return false;
	}

	return T66PC->OpenVendorNPC(this);
}

void AT66VendorNPC::ApplyVendorNPCStaticVisual()
{
	if (!VisualMesh)
	{
		return;
	}

	if (SkeletalMesh)
	{
		SkeletalMesh->SetVisibility(false, true);
		SkeletalMesh->SetHiddenInGame(true, true);
	}

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			if (Visuals->ApplyCharacterVisual(NPCID, SkeletalMesh, nullptr, false, false, false, VisualMesh))
			{
				return;
			}
		}
	}

	VisualMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.05f));
	VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
	FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh);
}
