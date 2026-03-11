// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66ChestMimicEnemy.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

AT66ChestInteractable::AT66ChestInteractable()
{
	RarityMeshes.Add(ET66Rarity::Black, TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Chests/Black/ChestBlack.ChestBlack"))));
	RarityMeshes.Add(ET66Rarity::Red, TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Chests/Red/ChestRed.ChestRed"))));
	RarityMeshes.Add(ET66Rarity::Yellow, TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Chests/Yellow/ChestYellow.ChestYellow"))));
	RarityMeshes.Add(ET66Rarity::White, TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Chests/White/ChestWhite.ChestWhite"))));

	ApplyRarityVisuals();
}

void AT66ChestInteractable::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh()) return;

	const FLinearColor BodyC = bIsMimic ? FLinearColor(0.35f, 0.10f, 0.55f, 1.f) : FT66RarityUtil::GetRarityColor(Rarity);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, BodyC);
}

bool AT66ChestInteractable::Interact(APlayerController* PC)
{
	if (bConsumed) return false;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;

	if (bIsMimic)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66ChestMimicEnemy>(AT66ChestMimicEnemy::StaticClass(), GetActorLocation(), GetActorRotation(), SpawnParams);
		bConsumed = true;
		Destroy();
		return true;
	}

	int32 Gold = 50;
	switch (Rarity)
	{
	case ET66Rarity::Black:  Gold = 50; break;
	case ET66Rarity::Red:    Gold = 150; break;
	case ET66Rarity::Yellow: Gold = 300; break;
	case ET66Rarity::White:  Gold = 600; break;
	default:                 Gold = 50; break;
	}

	RunState->AddGold(Gold);
	bConsumed = true;
	Destroy();
	return true;
}
