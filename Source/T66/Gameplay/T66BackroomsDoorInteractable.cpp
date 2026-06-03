// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BackroomsDoorInteractable.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	static const TCHAR* T66BackroomsDoorTexturePath = TEXT("/Game/World/Backrooms/Textures/T_Backrooms_Door.T_Backrooms_Door");

	static void T66ApplyBackroomsDoorMaterial(UStaticMeshComponent* Mesh, UObject* Outer)
	{
		if (!Mesh)
		{
			return;
		}

		UTexture* DoorTexture = LoadObject<UTexture>(nullptr, T66BackroomsDoorTexturePath);
		UMaterialInterface* BaseMaterial = FT66VisualUtil::GetFlatColorMaterial();
		if (BaseMaterial && DoorTexture)
		{
			if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : Mesh))
			{
				Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), DoorTexture);
				Material->SetTextureParameterValue(TEXT("BaseColorTexture"), DoorTexture);
				Material->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
				Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
				Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
				Mesh->SetMaterial(0, Material);
				return;
			}
		}

		FT66VisualUtil::ApplyT66Color(Mesh, Outer, FLinearColor(0.95f, 0.74f, 0.18f, 1.f));
	}
}

AT66BackroomsDoorInteractable::AT66BackroomsDoorInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(330.f, 190.f, 310.f));
	}

	if (VisualMesh)
	{
		VisualMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 170.f));
		VisualMesh->SetRelativeScale3D(FVector(3.2f, 0.12f, 3.4f));
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		VisualMesh->SetCollisionObjectType(ECC_WorldStatic);
		VisualMesh->SetCollisionResponseToAllChannels(ECR_Block);
	}

	ApplyRarityVisuals();
}

void AT66BackroomsDoorInteractable::InitializeBackroomsDoor(const bool bInExitDoor, const bool bInVisualOnlyClosedDoor)
{
	bExitDoor = bInExitDoor;
	bVisualOnlyClosedDoor = bInVisualOnlyClosedDoor;
	bConsumed = bVisualOnlyClosedDoor;

	if (TriggerBox)
	{
		TriggerBox->SetCollisionEnabled(bVisualOnlyClosedDoor ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly);
		TriggerBox->SetGenerateOverlapEvents(!bVisualOnlyClosedDoor);
	}
}

void AT66BackroomsDoorInteractable::SetDoorConsumed()
{
	bConsumed = true;
	if (TriggerBox)
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TriggerBox->SetGenerateOverlapEvents(false);
	}
}

bool AT66BackroomsDoorInteractable::Interact(APlayerController* PC)
{
	if (bConsumed || bVisualOnlyClosedDoor)
	{
		return false;
	}

	AT66HeroBase* Hero = PC ? Cast<AT66HeroBase>(PC->GetPawn()) : Cast<AT66HeroBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Hero)
	{
		return false;
	}

	if (AT66GameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr)
	{
		GameMode->HandleBackroomsDoorInteracted(this, Hero);
		return true;
	}

	return false;
}

void AT66BackroomsDoorInteractable::ApplyRarityVisuals()
{
	T66ApplyBackroomsDoorMaterial(VisualMesh, this);
}

bool AT66BackroomsDoorInteractable::ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const
{
	return !bVisualOnlyClosedDoor && Super::ShouldShowInteractionPrompt(LocalHero);
}

FText AT66BackroomsDoorInteractable::BuildInteractionPromptTargetName() const
{
	return bExitDoor
		? NSLOCTEXT("T66.Backrooms", "BackroomsExitDoorPrompt", "Exit Door")
		: NSLOCTEXT("T66.Backrooms", "BackroomsEntryDoorPrompt", "Backrooms Door");
}
