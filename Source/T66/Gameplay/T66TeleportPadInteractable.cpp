// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TeleportPadInteractable.h"

#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "UObject/SoftObjectPath.h"

AT66TeleportPadInteractable::AT66TeleportPadInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/TeleportPad/TeleportPad.TeleportPad")));

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(420.f, 420.f, 240.f));
	}

	ApplyRarityVisuals();
}

void AT66TeleportPadInteractable::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh())
	{
		return;
	}

	if (!VisualMesh)
	{
		return;
	}

	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.18f, 0.78f, 1.0f, 1.0f));
}

bool AT66TeleportPadInteractable::Interact(APlayerController* PC)
{
	if (!PC)
	{
		return false;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return false;
	}

	AT66TeleportPadInteractable* DestinationPad = ChooseDestinationPad();
	if (!DestinationPad)
	{
		return false;
	}

	float PawnHalfHeight = 90.f;
	if (const ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		if (const UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
	}

	const FVector Destination = DestinationPad->GetActorLocation() + FVector(0.f, 0.f, PawnHalfHeight + 5.f);
	Pawn->SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics);
	return true;
}

AT66TeleportPadInteractable* AT66TeleportPadInteractable::ChooseDestinationPad() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<AT66TeleportPadInteractable*> CandidatePads;
	for (TActorIterator<AT66TeleportPadInteractable> It(World); It; ++It)
	{
		AT66TeleportPadInteractable* Pad = *It;
		if (!Pad || Pad == this || Pad->IsActorBeingDestroyed())
		{
			continue;
		}

		CandidatePads.Add(Pad);
	}

	if (CandidatePads.Num() <= 0)
	{
		return nullptr;
	}

	const int32 Index = FMath::RandRange(0, CandidatePads.Num() - 1);
	return CandidatePads[Index];
}
