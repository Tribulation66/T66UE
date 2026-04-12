// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

AT66StageCatchUpLootInteractable::AT66StageCatchUpLootInteractable()
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

void AT66StageCatchUpLootInteractable::ApplyRarityVisuals()
{
	if (!VisualMesh) return;
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.92f, 0.92f, 0.96f, 1.f));
}

bool AT66StageCatchUpLootInteractable::Interact(APlayerController* PC)
{
	if (bConsumed) return false;
	UWorld* World = GetWorld();
	if (!World) return false;

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66RngSubsystem* RngSub = T66GI ? T66GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	if (!T66GI || !RunState) return false;

	const int32 Count = FMath::Clamp(LootBagCount, 0, 999);
	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}
	FRandomStream LocalRng(FPlatformTime::Cycles());
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;
	const FT66RarityWeights Weights = PlayerExperience
		? PlayerExperience->GetDifficultyCatchUpLootBagRarityWeights(T66GI->SelectedDifficulty)
		: FT66RarityWeights{};

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = RngSub ? RngSub->RunFRandRange(0.f, 2.f * PI) : Rng.FRandRange(0.f, 2.f * PI);
		const float Radius = RngSub ? RngSub->RunFRandRange(0.f, 240.f) : Rng.FRandRange(0.f, 240.f);
		const FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

		const ET66Rarity BagRarity = RngSub ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(
				FName(TEXT("CatchUpLootBagRarity")),
				BagRarity,
				RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE,
				RngSub ? RngSub->GetLastRunPreDrawSeed() : 0,
				&Weights);
		}
		const FName ItemID = T66GI->GetRandomItemIDForLootRarityFromStream(BagRarity, Rng);

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
	RefreshInteractionPrompt();
	return true;
}
