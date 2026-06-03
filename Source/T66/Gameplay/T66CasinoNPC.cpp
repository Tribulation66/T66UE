// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CasinoNPC.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Data/T66DataTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/T66VisualUtil.h"

AT66CasinoNPC::AT66CasinoNPC()
{
	NPCID = FName(TEXT("CasinoNPC"));
	NPCName = NSLOCTEXT("T66.NPC", "CasinoNPC", "Casino");
	NPCColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.f);
	bPreserveVisualMeshMaterials = true;
}

void AT66CasinoNPC::BeginPlay()
{
	bPreserveVisualMeshMaterials = true;
	Super::BeginPlay();
	ApplyCasinoNPCStaticVisual();
}

void AT66CasinoNPC::ApplyNPCData(const FT66NPCData& Data)
{
	Super::ApplyNPCData(Data);
	WinGoldAmount = Data.CasinoWinGold;
}

bool AT66CasinoNPC::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC) return false;
	T66PC->OpenCasinoOverlay();
	return true;
}

void AT66CasinoNPC::ApplyCasinoNPCStaticVisual()
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


