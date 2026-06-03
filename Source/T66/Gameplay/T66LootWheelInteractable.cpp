// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LootWheelInteractable.h"

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

	static ET66ItemRarity T66UpgradeItemRarityByRewardMultiplier(const ET66ItemRarity BaseRarity, const float Multiplier)
	{
		const int32 TierAdvance = FMath::Clamp(FMath::FloorToInt(FMath::Clamp(Multiplier, 1.f, 3.f) - 1.f), 0, 2);
		const int32 BaseTier = static_cast<int32>(BaseRarity);
		const int32 UpgradedTier = FMath::Clamp(BaseTier + TierAdvance, static_cast<int32>(ET66ItemRarity::Black), static_cast<int32>(ET66ItemRarity::White));
		return static_cast<ET66ItemRarity>(UpgradedTier);
	}

	static AT66LootWheelInteractable::ELockedLootWheelRewardType T66RollLootWheelReward(const FT66LootWheelRewardWeights& Weights, FRandomStream& Rng)
	{
		const float GoldWeight = FMath::Max(0.f, Weights.Gold);
		const float ItemWeight = FMath::Max(0.f, Weights.Item);
		const float BoostWeight = FMath::Max(0.f, Weights.Boost);
		const float Total = GoldWeight + ItemWeight + BoostWeight;
		if (Total <= KINDA_SMALL_NUMBER)
		{
			return AT66LootWheelInteractable::ELockedLootWheelRewardType::Gold;
		}

		const float Roll = Rng.FRandRange(0.f, Total);
		if (Roll < GoldWeight)
		{
			return AT66LootWheelInteractable::ELockedLootWheelRewardType::Gold;
		}
		if (Roll < GoldWeight + ItemWeight)
		{
			return AT66LootWheelInteractable::ELockedLootWheelRewardType::Item;
		}
		return AT66LootWheelInteractable::ELockedLootWheelRewardType::Boost;
	}

	struct FT66LootWheelBoostTarget
	{
		bool bUsesSecondaryStat = false;
		ET66HeroStatType PrimaryStatType = ET66HeroStatType::Damage;
		ET66SecondaryStatType SecondaryStatType = ET66SecondaryStatType::None;
	};

	static const TArray<FT66LootWheelBoostTarget>& T66GetLootWheelBoostStatPool()
	{
		static const TArray<FT66LootWheelBoostTarget> StatPool = {
			{ false, ET66HeroStatType::Damage, ET66SecondaryStatType::None },
			{ false, ET66HeroStatType::AttackSpeed, ET66SecondaryStatType::None },
			{ false, ET66HeroStatType::AttackScale, ET66SecondaryStatType::None },
			{ false, ET66HeroStatType::Armor, ET66SecondaryStatType::None },
			{ false, ET66HeroStatType::Evasion, ET66SecondaryStatType::None },
			{ false, ET66HeroStatType::Luck, ET66SecondaryStatType::None },
			{ false, ET66HeroStatType::Speed, ET66SecondaryStatType::None },
			{ false, ET66HeroStatType::Accuracy, ET66SecondaryStatType::None },
			{ true, ET66HeroStatType::Special, ET66SecondaryStatType::FirePower },
			{ true, ET66HeroStatType::Special, ET66SecondaryStatType::IcePower },
			{ true, ET66HeroStatType::Special, ET66SecondaryStatType::ElectricityPower },
			{ true, ET66HeroStatType::Special, ET66SecondaryStatType::NaturePower },
		};
		return StatPool;
	}

	static ET66LootWheelRewardVisualType T66LootWheelRewardTypeToVisualType(const AT66LootWheelInteractable::ELockedLootWheelRewardType RewardType)
	{
		switch (RewardType)
		{
		case AT66LootWheelInteractable::ELockedLootWheelRewardType::Gold:
			return ET66LootWheelRewardVisualType::Gold;
		case AT66LootWheelInteractable::ELockedLootWheelRewardType::Item:
			return ET66LootWheelRewardVisualType::Item;
		case AT66LootWheelInteractable::ELockedLootWheelRewardType::Boost:
			return ET66LootWheelRewardVisualType::Boost;
		default:
			return ET66LootWheelRewardVisualType::Gold;
		}
	}
}

AT66LootWheelInteractable::AT66LootWheelInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/LootWheel/SM_LootWheel_Pixal3D.SM_LootWheel_Pixal3D")));

	if (VisualMesh)
	{
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	ApplyRarityVisuals();
}

bool AT66LootWheelInteractable::Interact(APlayerController* PC)
{
	if (!PC || bConsumed)
	{
		return false;
	}

	if (!LockLootWheelReward(PC))
	{
		return false;
	}

	bConsumed = true;
	RefreshInteractionPrompt();
	PresentLockedLootWheelReward();
	return true;
}

void AT66LootWheelInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CommitLockedLootWheelRewardIfNeeded();
	Super::EndPlay(EndPlayReason);
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

bool AT66LootWheelInteractable::LockLootWheelReward(APlayerController* PC)
{
	if (LockedReward.bLocked)
	{
		return true;
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

	LockedReward = FLockedLootWheelReward{};
	LockedReward.bLocked = true;
	LockedReward.WheelRarity = Rarity;
	LockedReward.PlayerController = PC;
	LockedReward.RewardType = T66RollLootWheelReward(RewardWeights, Rng);
	const float LootWheelMultiplier = RunState->GetLootWheelRewardMultiplier();

	switch (LockedReward.RewardType)
	{
	case ELockedLootWheelRewardType::Gold:
	{
		FT66IntRange GoldRange = PlayerExperience
			? PlayerExperience->GetDifficultyLootWheelGoldRange(Difficulty, Rarity)
			: FT66IntRange{ 35, 80 };
		if (FMath::Max(GoldRange.Min, GoldRange.Max) <= 0)
		{
			GoldRange = FT66IntRange{ 35, 80 };
		}
		LockedReward.MinGold = FMath::Max(0, FMath::Min(GoldRange.Min, GoldRange.Max));
		LockedReward.MaxGold = FMath::Max(LockedReward.MinGold, FMath::Max(GoldRange.Min, GoldRange.Max));
		if (RngSub)
		{
			LockedReward.Gold = FMath::Max(0, RngSub->RollIntRangeBiased(GoldRange, Rng));
			LockedReward.DrawIndex = RngSub->GetLastRunDrawIndex();
			LockedReward.PreDrawSeed = RngSub->GetLastRunPreDrawSeed();
		}
		else
		{
			LockedReward.Gold = FMath::RandRange(LockedReward.MinGold, LockedReward.MaxGold);
		}
		LockedReward.Gold = FMath::Max(0, FMath::RoundToInt(static_cast<float>(LockedReward.Gold) * LootWheelMultiplier));
		break;
	}
	case ELockedLootWheelRewardType::Item:
		LockedReward.ItemID = T66GI ? T66GI->GetRandomItemIDForLootRarityFromStream(Rarity, Rng) : NAME_None;
		LockedReward.ItemRarity = T66UpgradeItemRarityByRewardMultiplier(T66LootRarityToItemRarity(Rarity), LootWheelMultiplier);
		break;
	case ELockedLootWheelRewardType::Boost:
	{
		const TArray<FT66LootWheelBoostTarget>& StatPool = T66GetLootWheelBoostStatPool();
		const FT66LootWheelBoostTarget Target = StatPool.IsValidIndex(0)
			? StatPool[Rng.RandRange(0, StatPool.Num() - 1)]
			: FT66LootWheelBoostTarget{};
		LockedReward.bBoostUsesSecondaryStat = Target.bUsesSecondaryStat;
		LockedReward.BoostStatType = Target.PrimaryStatType;
		LockedReward.BoostSecondaryStatType = Target.SecondaryStatType;
		LockedReward.BoostBonusStatPoints = FMath::Max(1, FMath::RoundToInt(8.f * LootWheelMultiplier));
		LockedReward.BoostDurationSeconds = 10.f;
		break;
	}
	default:
		break;
	}

	return true;
}

void AT66LootWheelInteractable::CommitLockedLootWheelRewardIfNeeded()
{
	if (!LockedReward.bLocked || LockedReward.bCommitAttempted)
	{
		return;
	}

	LockedReward.bCommitAttempted = true;
	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	APlayerController* PC = LockedReward.PlayerController.Get();
	if (!RunState && LockedReward.RewardType != ELockedLootWheelRewardType::Boost)
	{
		return;
	}

	switch (LockedReward.RewardType)
	{
	case ELockedLootWheelRewardType::Gold:
		RunState->AddGold(LockedReward.Gold);
		RunState->RecordLuckQuantityRoll(
			FName(TEXT("LootWheelGold")),
			LockedReward.Gold,
			LockedReward.MinGold,
			LockedReward.MaxGold,
			LockedReward.DrawIndex,
			LockedReward.PreDrawSeed);
		UE_LOG(LogTemp, Display, TEXT("[LootWheelUI] committed gold amount=%d drawIndex=%d preDrawSeed=%d"),
			LockedReward.Gold,
			LockedReward.DrawIndex,
			LockedReward.PreDrawSeed);
		break;
	case ELockedLootWheelRewardType::Item:
		if (!LockedReward.ItemID.IsNone())
		{
			RunState->AddItemWithRarity(LockedReward.ItemID, LockedReward.ItemRarity);
			UE_LOG(LogTemp, Display, TEXT("[LootWheelUI] committed item item=%s rarity=%d"),
				*LockedReward.ItemID.ToString(),
				static_cast<int32>(LockedReward.ItemRarity));
		}
		break;
	case ELockedLootWheelRewardType::Boost:
		if (World)
		{
			const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 80.f);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66BoostInteractable* Boost = World->SpawnActor<AT66BoostInteractable>(AT66BoostInteractable::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
			{
				if (LockedReward.bBoostUsesSecondaryStat)
				{
					Boost->ConfigureSecondaryBoost(LockedReward.BoostSecondaryStatType, LockedReward.BoostBonusStatPoints, LockedReward.BoostDurationSeconds);
				}
				else
				{
					Boost->ConfigureBoost(LockedReward.BoostStatType, LockedReward.BoostBonusStatPoints, LockedReward.BoostDurationSeconds);
				}
				Boost->Interact(PC);
				UE_LOG(LogTemp, Display, TEXT("[LootWheelUI] committed boost primary=%d secondary=%d usesSecondary=%d points=%d duration=%.1f"),
					static_cast<int32>(LockedReward.BoostStatType),
					static_cast<int32>(LockedReward.BoostSecondaryStatType),
					LockedReward.bBoostUsesSecondaryStat ? 1 : 0,
					LockedReward.BoostBonusStatPoints,
					LockedReward.BoostDurationSeconds);
			}
		}
		break;
	default:
		break;
	}
}

void AT66LootWheelInteractable::PresentLockedLootWheelReward()
{
	if (!LockedReward.bLocked || bWheelResultPresented)
	{
		return;
	}

	bWheelResultPresented = true;
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(LockedReward.PlayerController.Get());
	if (T66PC)
	{
		FT66LootWheelPresentationParams PresentationParams;
		PresentationParams.WheelRarity = LockedReward.WheelRarity;
		PresentationParams.RewardType = T66LootWheelRewardTypeToVisualType(LockedReward.RewardType);
		PresentationParams.Gold = LockedReward.Gold;
		PresentationParams.ItemID = LockedReward.ItemID;
		PresentationParams.ItemRarity = LockedReward.ItemRarity;
		PresentationParams.BoostStatType = LockedReward.BoostStatType;
		PresentationParams.BoostSecondaryStatType = LockedReward.BoostSecondaryStatType;
		PresentationParams.bBoostUsesSecondaryStat = LockedReward.bBoostUsesSecondaryStat;
		PresentationParams.BoostBonusStatPoints = LockedReward.BoostBonusStatPoints;
		PresentationParams.BoostDurationSeconds = LockedReward.BoostDurationSeconds;

		TWeakObjectPtr<AT66LootWheelInteractable> WeakThis(this);
		PresentationParams.OnLandingCommit = [WeakThis]()
		{
			if (AT66LootWheelInteractable* Self = WeakThis.Get())
			{
				Self->HandleLootWheelSpinCommit();
			}
		};
		PresentationParams.OnFinished = [WeakThis]()
		{
			if (AT66LootWheelInteractable* Self = WeakThis.Get())
			{
				Self->HandleLootWheelSpinFinished();
			}
		};

		if (T66PC->StartLootWheelSpinHUD(MoveTemp(PresentationParams)))
		{
			UE_LOG(LogTemp, Display, TEXT("[LootWheelUI] started radial spin rewardType=%d rarity=%d"),
				static_cast<int32>(LockedReward.RewardType),
				static_cast<int32>(LockedReward.WheelRarity));
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[LootWheelUI] radial spin unavailable; committing and finishing fallback rewardType=%d"),
		static_cast<int32>(LockedReward.RewardType));
	CommitLockedLootWheelRewardIfNeeded();
	PresentLockedLootWheelRewardAfterSpin();
}

void AT66LootWheelInteractable::HandleLootWheelSpinCommit()
{
	UE_LOG(LogTemp, Display, TEXT("[LootWheelUI] landing commit rewardType=%d committedBefore=%d"),
		static_cast<int32>(LockedReward.RewardType),
		LockedReward.bCommitAttempted ? 1 : 0);
	CommitLockedLootWheelRewardIfNeeded();
}

void AT66LootWheelInteractable::HandleLootWheelSpinFinished()
{
	PresentLockedLootWheelRewardAfterSpin();
}

void AT66LootWheelInteractable::PresentLockedLootWheelRewardAfterSpin()
{
	if (!LockedReward.bLocked)
	{
		FinishLootWheelInteraction();
		return;
	}

	AT66PlayerController* T66PC = Cast<AT66PlayerController>(LockedReward.PlayerController.Get());
	if (T66PC)
	{
		switch (LockedReward.RewardType)
		{
		case ELockedLootWheelRewardType::Gold:
		{
			TWeakObjectPtr<AT66LootWheelInteractable> WeakThis(this);
			if (T66PC->StartChestRewardHUD(
				LockedReward.WheelRarity,
				LockedReward.Gold,
				nullptr,
				[WeakThis]()
				{
					if (AT66LootWheelInteractable* Self = WeakThis.Get())
					{
						Self->FinishLootWheelInteraction();
					}
				}))
			{
				return;
			}
			break;
		}
		case ELockedLootWheelRewardType::Item:
			T66PC->ShowPickupItemCardHUD(LockedReward.ItemID, LockedReward.ItemRarity);
			break;
		case ELockedLootWheelRewardType::Boost:
		default:
			break;
		}
	}

	FinishLootWheelInteraction();
}

void AT66LootWheelInteractable::FinishLootWheelInteraction()
{
	if (IsShowcaseReusable())
	{
		LockedReward = FLockedLootWheelReward{};
		bConsumed = false;
		bWheelResultPresented = false;
		RefreshInteractionPrompt();
		return;
	}

	Destroy();
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
	FT66IntRange GoldRange = PlayerExperience
		? PlayerExperience->GetDifficultyLootWheelGoldRange(Difficulty, Rarity)
		: FT66IntRange{ 35, 80 };
	if (FMath::Max(GoldRange.Min, GoldRange.Max) <= 0)
	{
		GoldRange = FT66IntRange{ 35, 80 };
	}
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

	const TArray<FT66LootWheelBoostTarget>& StatPool = T66GetLootWheelBoostStatPool();
	const FT66LootWheelBoostTarget Target = StatPool.IsValidIndex(0)
		? StatPool[Rng.RandRange(0, StatPool.Num() - 1)]
		: FT66LootWheelBoostTarget{};

	const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 80.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66BoostInteractable* Boost = World->SpawnActor<AT66BoostInteractable>(AT66BoostInteractable::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		if (Target.bUsesSecondaryStat)
		{
			Boost->ConfigureSecondaryBoost(Target.SecondaryStatType, 8, 10.f);
		}
		else
		{
			Boost->ConfigureBoost(Target.PrimaryStatType, 8, 10.f);
		}
		Boost->Interact(PC);
	}
}
