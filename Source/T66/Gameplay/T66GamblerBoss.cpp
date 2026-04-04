// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GamblerBoss.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AT66GamblerBoss::AT66GamblerBoss()
{
	BossID = FName(TEXT("GamblerBoss"));
	MaxHP = 1000;
	FireIntervalSeconds = 0.6f;
	ProjectileSpeed = 1100.f;
	ProjectileDamageHearts = 1;

	GroundAOEIntervalSeconds = 4.f;
	GroundAOERadius = 250.f;
	GroundAOEWarningSeconds = 1.0f;
	GroundAOEBaseDamageHP = 20;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 520.f;
	}
}

void AT66GamblerBoss::BeginPlay()
{
	Super::BeginPlay();

	// Placeholder mesh: smaller sphere, casino yellow (overridden by ApplyCharacterVisual if mapped).
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(4.f, 4.f, 4.f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 112.f));
	}
	if (UMaterialInterface* ColorMat = FT66VisualUtil::GetFlatColorMaterial())
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this))
		{
			FT66VisualUtil::ConfigureFlatColorMaterial(Mat, FLinearColor(0.85f, 0.75f, 0.15f, 1.f));
			VisualMesh->SetMaterial(0, Mat);
		}
	}

	// Gambler boss must use the same mesh as Gambler NPC (per rule).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		float DifficultyScalar = 1.f;
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			DifficultyScalar = RunState->GetDifficultyScalar();
		}

		float MetaScalar = 1.f;
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			const int32 UnlockedTokenLevel = Achievements->GetGamblersTokenUnlockedLevel();
			if (UnlockedTokenLevel > 0)
			{
				MetaScalar = 1.f + 0.20f * static_cast<float>(UnlockedTokenLevel);
				GroundAOEBaseDamageHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(GroundAOEBaseDamageHP) * MetaScalar));
				FireIntervalSeconds = FMath::Max(0.25f, FireIntervalSeconds * (1.f - 0.05f * static_cast<float>(UnlockedTokenLevel)));
				GroundAOEIntervalSeconds = FMath::Max(1.5f, GroundAOEIntervalSeconds * (1.f - 0.04f * static_cast<float>(UnlockedTokenLevel)));
				if (UCharacterMovementComponent* Move = GetCharacterMovement())
				{
					Move->MaxWalkSpeed *= 1.f + 0.05f * static_cast<float>(UnlockedTokenLevel);
				}
			}
		}
		ApplyDifficultyScalar(DifficultyScalar * MetaScalar);

		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const bool bApplied = Visuals->ApplyCharacterVisual(FName(TEXT("GamblerBoss")), GetMesh(), VisualMesh, true);
			if (!bApplied && GetMesh())
			{
				GetMesh()->SetVisibility(false, true);
			}
		}
	}

	ForceAwaken();
}

void AT66GamblerBoss::Die()
{
	UWorld* World = GetWorld();
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		ET66Difficulty Difficulty = ET66Difficulty::Easy;
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			Difficulty = T66GI->SelectedDifficulty;
		}

		int32 TokenLevelToDrop = FMath::Clamp(static_cast<int32>(Difficulty) + 1, 1, 5);
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			TokenLevelToDrop = FMath::Max(1, Achievements->UpgradeGamblersTokenForDifficulty(Difficulty));
		}

		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66LootBagPickup* TokenBag = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), GetActorLocation() + FVector(0.f, 0.f, 80.f), FRotator::ZeroRotator, SpawnParams))
			{
				TokenBag->SetItemID(FName(TEXT("Item_GamblersToken")));
				TokenBag->SetLootRarity(ET66Rarity::White);
				TokenBag->SetExplicitLine1RolledValue(TokenLevelToDrop);
			}
		}

		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ResetGamblerAnger();
		}
	}
	Super::Die();
}
