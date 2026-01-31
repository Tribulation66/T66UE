// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageBoostLootInteractable.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66Rarity.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

AT66StageBoostLootInteractable::AT66StageBoostLootInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
	Rarity = ET66Rarity::White;

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(140.f, 140.f, 140.f));
	}
	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
		VisualMesh->SetRelativeScale3D(FVector(1.6f, 1.6f, 1.6f));
		if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
		{
			VisualMesh->SetStaticMesh(Sphere);
		}
	}
	ApplyRarityVisuals();
}

void AT66StageBoostLootInteractable::ApplyRarityVisuals()
{
	if (!VisualMesh) return;
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.92f, 0.92f, 0.96f, 1.f));
}

bool AT66StageBoostLootInteractable::Interact(APlayerController* PC)
{
	if (bConsumed) return false;
	UWorld* World = GetWorld();
	if (!World) return false;

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return false;

	const int32 Count = FMath::Clamp(LootBagCount, 0, 999);
	FRandomStream Rng(FPlatformTime::Cycles());

	for (int32 i = 0; i < Count; ++i)
	{
		// Small circle scatter so bags aren't stacked.
		const float Angle = Rng.FRandRange(0.f, 2.f * PI);
		const float Radius = Rng.FRandRange(0.f, 240.f);
		const FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

		const int32 LuckStat = RunState->GetLuckStat();
		const ET66Rarity BagRarity = FT66RarityUtil::RollRarityWithLuck(Rng, LuckStat);
		const FName ItemID = T66GI->GetRandomItemIDForLootRarity(BagRarity);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66LootBagPickup* Loot = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), GetActorLocation() + Offset, FRotator::ZeroRotator, SpawnParams);
		if (Loot)
		{
			Loot->SetLootRarity(BagRarity);
			Loot->SetItemID(ItemID);
		}
	}

	bConsumed = true;
	ApplyRarityVisuals();
	return true;
}

