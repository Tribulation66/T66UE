// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ChestInteractable.h"

#include "Gameplay/T66ChestMimicEnemy.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

AT66ChestInteractable::AT66ChestInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Chests/ChestModel/SM_Chest_Pixal3D.SM_Chest_Pixal3D")));
	ApplyRarityVisuals();
}

void AT66ChestInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CommitLockedChestRewardIfNeeded();
	Super::EndPlay(EndPlayReason);
}

void AT66ChestInteractable::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh()) return;

	const FLinearColor BodyC = bIsMimic
		? FLinearColor(0.35f, 0.10f, 0.55f, 1.f)
		: FT66RarityUtil::GetRarityColor(ET66Rarity::Yellow);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, BodyC);
}

bool AT66ChestInteractable::Interact(APlayerController* PC)
{
	if (bConsumed) return false;

	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;

	if (bIsMimic)
	{
		if (IsShowcaseReusable())
		{
			bIsMimic = false;
			ApplyRarityVisuals();
			RefreshInteractionPrompt();
			return true;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66ChestMimicEnemy>(AT66ChestMimicEnemy::StaticClass(), GetActorLocation(), GetActorRotation(), SpawnParams);
		bConsumed = true;
		Destroy();
		return true;
	}

	if (!LockChestReward(PC))
	{
		return false;
	}

	bConsumed = true;
	RefreshInteractionPrompt();
	PresentLockedChestReward();
	return true;
}

bool AT66ChestInteractable::LockChestReward(APlayerController* PC)
{
	if (LockedReward.bLocked)
	{
		return true;
	}

	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	UT66RngSubsystem* RngSub = T66GI ? T66GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	if (!RunState)
	{
		return false;
	}

	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66Rarity RewardRarity = Rarity;
	RunState->RecordLuckQualityRarity(
		FName(TEXT("ChestRewardRarity")),
		RewardRarity,
		INDEX_NONE,
		0,
		nullptr);

	const FT66IntRange GoldRange = PlayerExperience
		? PlayerExperience->GetDifficultyChestGoldRange(Difficulty, RewardRarity)
		: FT66IntRange{ 35, 75 };
	const int32 MinGold = FMath::Max(0, FMath::Min(GoldRange.Min, GoldRange.Max));
	const int32 MaxGold = FMath::Max(MinGold, FMath::Max(GoldRange.Min, GoldRange.Max));

	int32 Gold = MaxGold;
	int32 DrawIndex = INDEX_NONE;
	int32 PreDrawSeed = 0;
	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
		FRandomStream& Stream = RngSub->GetRunStream();
		Gold = FMath::Max(0, RngSub->RollIntRangeBiased(GoldRange, Stream));
		DrawIndex = RngSub->GetLastRunDrawIndex();
		PreDrawSeed = RngSub->GetLastRunPreDrawSeed();
	}
	else
	{
		Gold = FMath::RandRange(MinGold, MaxGold);
	}
	Gold = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Gold) * RunState->GetLootChestRewardMultiplier()));

	LockedReward.bLocked = true;
	LockedReward.bCommitAttempted = false;
	LockedReward.RewardRarity = RewardRarity;
	LockedReward.Gold = Gold;
	LockedReward.MinGold = MinGold;
	LockedReward.MaxGold = MaxGold;
	LockedReward.DrawIndex = DrawIndex;
	LockedReward.PreDrawSeed = PreDrawSeed;
	LockedReward.PlayerController = PC;
	return true;
}

void AT66ChestInteractable::PresentLockedChestReward()
{
	if (!LockedReward.bLocked || bChestRewardPresentationStarted)
	{
		return;
	}

	bChestRewardPresentationStarted = true;
	TWeakObjectPtr<AT66ChestInteractable> WeakThis(this);
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(LockedReward.PlayerController.Get());
	const bool bPresentationStarted = T66PC
		&& T66PC->StartChestRewardHUD(
			LockedReward.RewardRarity,
			LockedReward.Gold,
			[WeakThis]()
			{
				if (AT66ChestInteractable* Self = WeakThis.Get())
				{
					Self->CommitLockedChestRewardIfNeeded();
				}
			},
			[WeakThis]()
			{
				if (AT66ChestInteractable* Self = WeakThis.Get())
				{
					Self->FinishChestInteraction();
				}
			});

	if (!bPresentationStarted)
	{
		CommitLockedChestRewardIfNeeded();
		FinishChestInteraction();
	}
}

void AT66ChestInteractable::CommitLockedChestRewardIfNeeded()
{
	if (!LockedReward.bLocked || LockedReward.bCommitAttempted)
	{
		return;
	}

	LockedReward.bCommitAttempted = true;
	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	RunState->AddGold(LockedReward.Gold);
	RunState->RecordLuckQuantityRoll(
		FName(TEXT("ChestGold")),
		LockedReward.Gold,
		LockedReward.MinGold,
		LockedReward.MaxGold,
		LockedReward.DrawIndex,
		LockedReward.PreDrawSeed);
}

void AT66ChestInteractable::FinishChestInteraction()
{
	if (IsShowcaseReusable())
	{
		LockedReward = FLockedChestReward{};
		bConsumed = false;
		bChestRewardPresentationStarted = false;
		RefreshInteractionPrompt();
		return;
	}

	Destroy();
}
