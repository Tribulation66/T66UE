// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66PlayerController.h"
#include "Data/T66DataTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/T66VisualUtil.h"

namespace
{
	UStaticMesh* T66LoadGamblerDemonStandMesh()
	{
		static TObjectPtr<UStaticMesh> CachedMesh = nullptr;
		if (!CachedMesh)
		{
			CachedMesh = LoadObject<UStaticMesh>(
				nullptr,
				TEXT("/Game/Characters/NPCs/Gambler/QuadRetro/SM_Gambler_QuadRetro.SM_Gambler_QuadRetro"));
		}
		return CachedMesh.Get();
	}
}

AT66GamblerNPC::AT66GamblerNPC()
{
	NPCID = FName(TEXT("Gambler"));
	NPCName = NSLOCTEXT("T66.NPC", "Gambler", "Gambler");
	NPCColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.f);
	bPreserveVisualMeshMaterials = true;
}

void AT66GamblerNPC::BeginPlay()
{
	bPreserveVisualMeshMaterials = true;
	Super::BeginPlay();
	ApplyGamblerStaticVisual();
}

void AT66GamblerNPC::ApplyNPCData(const FHouseNPCData& Data)
{
	Super::ApplyNPCData(Data);
	WinGoldAmount = Data.GamblerWinGold;
}

bool AT66GamblerNPC::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC) return false;
	T66PC->OpenCasinoOverlay();
	return true;
}

void AT66GamblerNPC::ApplyGamblerStaticVisual()
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

	if (UStaticMesh* DemonStandMesh = T66LoadGamblerDemonStandMesh())
	{
		VisualMesh->EmptyOverrideMaterials();
		VisualMesh->SetStaticMesh(DemonStandMesh);
		VisualMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
		FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh, DemonStandMesh);
	}
}

