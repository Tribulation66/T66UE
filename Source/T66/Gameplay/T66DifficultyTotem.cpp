// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66DifficultyTotem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

AT66DifficultyTotem::AT66DifficultyTotem()
{
	PrimaryActorTick.bCanEverTick = false;
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		VisualMesh->SetStaticMesh(Cube);
		// Tall skinny rectangle
		VisualMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 4.0f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	}
}

bool AT66DifficultyTotem::Interact(APlayerController* PC)
{
	if (bConsumed) return false;
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;

	// Track activation order for the "taller each time" visual rule.
	const int32 ActivationIndex = RunState->RegisterTotemActivated(); // 1-based

	// Bible (locked): Black +0.5, Red +1, Yellow +3, White +5.
	float Delta = 0.5f;
	switch (Rarity)
	{
	case ET66Rarity::Black:  Delta = 0.5f; break;
	case ET66Rarity::Red:    Delta = 1.0f; break;
	case ET66Rarity::Yellow: Delta = 3.0f; break;
	case ET66Rarity::White:  Delta = 5.0f; break;
	default:                 Delta = 0.5f; break;
	}
	RunState->AddDifficultySkulls(Delta);
	bConsumed = true;

	// One-time interact: disable overlap and make the activated totem taller based on activation order.
	if (TriggerBox)
	{
		TriggerBox->SetGenerateOverlapEvents(false);
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (VisualMesh)
	{
		const float BaseZScale = 4.0f;
		const float ExtraPerActivation = 2.0f; // activation #2 => +2, #3 => +4, etc.
		const float NewZScale = BaseZScale + ExtraPerActivation * static_cast<float>(FMath::Max(0, ActivationIndex - 1));
		VisualMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, NewZScale));
		// Keep base planted on the ground (cube half-height = 50 * scaleZ).
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * NewZScale));
	}
	return true;
}

