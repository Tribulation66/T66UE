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
	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
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

	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const FT66IntRange GoldRange = PlayerExperience
		? PlayerExperience->GetDifficultyChestGoldRange(Difficulty, Rarity)
		: FT66IntRange{ 35, 75 };
	const int32 MinGold = FMath::Max(0, FMath::Min(GoldRange.Min, GoldRange.Max));
	const int32 MaxGold = FMath::Max(MinGold, FMath::Max(GoldRange.Min, GoldRange.Max));

	int32 Gold = MaxGold;
	if (UT66RngSubsystem* RngSub = T66GI ? T66GI->GetSubsystem<UT66RngSubsystem>() : nullptr)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
		FRandomStream& Stream = RngSub->GetRunStream();
		Gold = FMath::Max(0, RngSub->RollIntRangeBiased(GoldRange, Stream));
	}
	else
	{
		Gold = FMath::RandRange(MinGold, MaxGold);
	}

	RunState->AddGold(Gold);
	RunState->RecordLuckQuantityRoll(FName(TEXT("ChestGold")), Gold, MinGold, MaxGold);
	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC))
	{
		T66PC->StartChestRewardHUD(Rarity, Gold);
	}
	bConsumed = true;
	Destroy();
	return true;
}
