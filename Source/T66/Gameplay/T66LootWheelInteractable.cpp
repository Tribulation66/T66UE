// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LootWheelInteractable.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66BoostInteractable.h"
#include "Gameplay/T66PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	enum class ET66LootWheelRewardType : uint8
	{
		Gold,
		Item,
		Boost,
	};

	static ET66ItemRarity T66LootRarityToItemRarity(const ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Black: return ET66ItemRarity::Black;
		case ET66Rarity::Red: return ET66ItemRarity::Red;
		case ET66Rarity::Yellow: return ET66ItemRarity::Yellow;
		case ET66Rarity::White: return ET66ItemRarity::White;
		default: return ET66ItemRarity::Black;
		}
	}

	static ET66LootWheelRewardType T66RollLootWheelReward(const FT66LootWheelRewardWeights& Weights, FRandomStream& Rng)
	{
		const float GoldWeight = FMath::Max(0.f, Weights.Gold);
		const float ItemWeight = FMath::Max(0.f, Weights.Item);
		const float BoostWeight = FMath::Max(0.f, Weights.Boost);
		const float Total = GoldWeight + ItemWeight + BoostWeight;
		if (Total <= KINDA_SMALL_NUMBER)
		{
			return ET66LootWheelRewardType::Gold;
		}

		const float Roll = Rng.FRandRange(0.f, Total);
		if (Roll < GoldWeight)
		{
			return ET66LootWheelRewardType::Gold;
		}
		if (Roll < GoldWeight + ItemWeight)
		{
			return ET66LootWheelRewardType::Item;
		}
		return ET66LootWheelRewardType::Boost;
	}

	static const TArray<ET66HeroStatType>& T66GetLootWheelBoostStatPool()
	{
		static const TArray<ET66HeroStatType> StatPool = {
			ET66HeroStatType::Damage,
			ET66HeroStatType::AttackSpeed,
			ET66HeroStatType::AttackScale,
			ET66HeroStatType::Armor,
			ET66HeroStatType::Evasion,
			ET66HeroStatType::Luck,
			ET66HeroStatType::Speed,
			ET66HeroStatType::Accuracy,
		};
		return StatPool;
	}
}

AT66LootWheelInteractable::AT66LootWheelInteractable()
{
	PrimaryActorTick.bCanEverTick = true;
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/LootWheel/LootWheel_Pixal3D.LootWheel_Pixal3D")));

	if (VisualMesh)
	{
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	ApplyRarityVisuals();
}

void AT66LootWheelInteractable::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (VisualMesh)
	{
		VisualMesh->AddRelativeRotation(FRotator(0.f, 90.f * DeltaSeconds, 0.f));
	}
}

bool AT66LootWheelInteractable::Interact(APlayerController* PC)
{
	if (!PC || bConsumed)
	{
		return false;
	}

	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66RngSubsystem* RngSub = T66GI ? T66GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return false;
	}

	FRandomStream LocalRng(FMath::Rand());
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const FT66LootWheelRewardWeights RewardWeights = PlayerExperience
		? PlayerExperience->GetDifficultyLootWheelRewardWeights(Difficulty, Rarity)
		: FT66LootWheelRewardWeights{};
	const ET66LootWheelRewardType RewardType = T66RollLootWheelReward(RewardWeights, Rng);

	switch (RewardType)
	{
	case ET66LootWheelRewardType::Gold:
		GrantGoldReward(PC);
		break;
	case ET66LootWheelRewardType::Item:
		GrantItemReward(PC);
		break;
	case ET66LootWheelRewardType::Boost:
		GrantBoostReward(PC, Rng);
		break;
	default:
		GrantGoldReward(PC);
		break;
	}

	UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("LootWheel.Spin")), GetActorLocation(), this);
	if (IsShowcaseReusable())
	{
		bConsumed = false;
		RefreshInteractionPrompt();
		return true;
	}

	bConsumed = true;
	Destroy();
	return true;
}

void AT66LootWheelInteractable::ApplyRarityVisuals()
{
	if (!VisualMesh)
	{
		return;
	}

	if (TryApplyImportedMesh())
	{
		return;
	}

	VisualMesh->SetStaticMesh(nullptr);
}

FText AT66LootWheelInteractable::BuildInteractionPromptText() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66InteractionPromptSubsystem* PromptSubsystem = GI ? GI->GetSubsystem<UT66InteractionPromptSubsystem>() : nullptr;
	const FText Action = NSLOCTEXT("T66.LootWheel", "SpinLootWheelVerb", "spin loot wheel");
	return PromptSubsystem ? PromptSubsystem->BuildCustomPromptText(Action) : Action;
}

FText AT66LootWheelInteractable::BuildInteractionPromptTargetName() const
{
	return NSLOCTEXT("T66.LootWheel", "LootWheelTargetName", "Loot Wheel");
}

void AT66LootWheelInteractable::GrantGoldReward(APlayerController* PC)
{
	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	UT66RngSubsystem* RngSub = T66GI ? T66GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const FT66IntRange GoldRange = PlayerExperience
		? PlayerExperience->GetDifficultyLootWheelGoldRange(Difficulty, Rarity)
		: FT66IntRange{ 35, 80 };
	const int32 MinGold = FMath::Max(0, FMath::Min(GoldRange.Min, GoldRange.Max));
	const int32 MaxGold = FMath::Max(MinGold, FMath::Max(GoldRange.Min, GoldRange.Max));
	int32 Gold = MaxGold;
	int32 DrawIndex = INDEX_NONE;
	int32 PreDrawSeed = 0;
	if (RngSub)
	{
		FRandomStream& Rng = RngSub->GetRunStream();
		Gold = FMath::Max(0, RngSub->RollIntRangeBiased(GoldRange, Rng));
		DrawIndex = RngSub->GetLastRunDrawIndex();
		PreDrawSeed = RngSub->GetLastRunPreDrawSeed();
	}
	else
	{
		Gold = FMath::RandRange(MinGold, MaxGold);
	}

	RunState->AddGold(Gold);
	RunState->RecordLuckQuantityRoll(FName(TEXT("LootWheelGold")), Gold, MinGold, MaxGold, DrawIndex, PreDrawSeed);
	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC))
	{
		T66PC->StartChestRewardHUD(Rarity, Gold);
	}
}

void AT66LootWheelInteractable::GrantItemReward(APlayerController* PC)
{
	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66RngSubsystem* RngSub = T66GI ? T66GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	if (!RunState || !T66GI)
	{
		return;
	}

	FRandomStream LocalRng(FMath::Rand());
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;
	const FName ItemID = T66GI->GetRandomItemIDForLootRarityFromStream(Rarity, Rng);
	const ET66ItemRarity ItemRarity = T66LootRarityToItemRarity(Rarity);
	RunState->AddItemWithRarity(ItemID, ItemRarity);
	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC))
	{
		T66PC->ShowPickupItemCardHUD(ItemID, ItemRarity);
	}
}

void AT66LootWheelInteractable::GrantBoostReward(APlayerController* PC, FRandomStream& Rng)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TArray<ET66HeroStatType>& StatPool = T66GetLootWheelBoostStatPool();
	const ET66HeroStatType StatType = StatPool.IsValidIndex(0)
		? StatPool[Rng.RandRange(0, StatPool.Num() - 1)]
		: ET66HeroStatType::Damage;

	const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 80.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66BoostInteractable* Boost = World->SpawnActor<AT66BoostInteractable>(AT66BoostInteractable::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Boost->ConfigureBoost(StatType, 8, 10.f);
		Boost->Interact(PC);
	}
}
