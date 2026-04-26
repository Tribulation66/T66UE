// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArcadeInteractableBase.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66ArcadeAmplifierPickup.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

namespace
{
	static FVector T66ResolveArcadeScale(const FVector& InScale)
	{
		return FVector(
			FMath::Max(KINDA_SMALL_NUMBER, InScale.X),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Y),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Z));
	}

	static FVector T66ResolveImportedArcadeLocation(const UStaticMeshComponent* MeshComponent, const FVector& Offset)
	{
		const float GroundedZ = MeshComponent ? MeshComponent->GetRelativeLocation().Z : 0.f;
		return FVector(Offset.X, Offset.Y, GroundedZ + Offset.Z);
	}

	static bool T66TryResolveArcadeRowData(const AActor* Actor, const FName ArcadeRowID, FT66ArcadeInteractableData& OutData)
	{
		if (ArcadeRowID.IsNone())
		{
			return false;
		}

		if (const UWorld* World = Actor ? Actor->GetWorld() : nullptr)
		{
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
			{
				if (T66GI->GetArcadeInteractableData(ArcadeRowID, OutData))
				{
					return true;
				}
			}
		}

		if (UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_ArcadeInteractables.DT_ArcadeInteractables")))
		{
			if (const FT66ArcadeInteractableRow* FoundRow = DataTable->FindRow<FT66ArcadeInteractableRow>(ArcadeRowID, TEXT("ArcadeInteractableBaseResolve")))
			{
				OutData = FoundRow->ArcadeData;
				if (OutData.ArcadeID.IsNone())
				{
					OutData.ArcadeID = ArcadeRowID;
				}
				return true;
			}
		}

		return false;
	}

	static FName T66GetArcadeRowIDForGameType(const ET66ArcadeGameType GameType)
	{
		switch (GameType)
		{
		case ET66ArcadeGameType::WhackAMole:
			return FName(TEXT("Arcade_WhackAMole"));
		case ET66ArcadeGameType::Topwar:
			return FName(TEXT("Arcade_Topwar"));
		default:
			return NAME_None;
		}
	}

	static FText T66GetArcadeDisplayNameForGameType(const ET66ArcadeGameType GameType)
	{
		switch (GameType)
		{
		case ET66ArcadeGameType::WhackAMole:
			return NSLOCTEXT("T66.Arcade", "WhackAMoleDisplayNameFallback", "Whack-a-Mole");
		case ET66ArcadeGameType::Topwar:
			return NSLOCTEXT("T66.Arcade", "TopwarDisplayNameFallback", "Topwar");
		default:
			return NSLOCTEXT("T66.Arcade", "ArcadeGameDisplayNameFallback", "Arcade");
		}
	}

	static bool T66IsPlayableArcadeGameType(const ET66ArcadeGameType GameType)
	{
		return GameType == ET66ArcadeGameType::WhackAMole
			|| GameType == ET66ArcadeGameType::Topwar;
	}

	static const TArray<ET66HeroStatType>& T66GetAmplifierStatPool()
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

	static float T66ResolveRewardChance(const float BaseChance, const float MaxChance, const float ScoreAlpha)
	{
		const float MinChance = FMath::Clamp(BaseChance, 0.f, 1.f);
		const float HighChance = FMath::Clamp(MaxChance, MinChance, 1.f);
		return FMath::Lerp(MinChance, HighChance, FMath::Clamp(ScoreAlpha, 0.f, 1.f));
	}
}

AT66ArcadeInteractableBase::AT66ArcadeInteractableBase()
{
	ArcadeData.DisplayName = NSLOCTEXT("T66.Arcade", "DefaultArcadeDisplayName", "Arcade");
	ArcadeData.InteractionVerb = NSLOCTEXT("T66.Arcade", "DefaultArcadeVerb", "play arcade");
	ArcadeData.ArcadeClass = ET66ArcadeInteractableClass::PopupArcade;
	ArcadeData.ArcadeGameType = ET66ArcadeGameType::None;
	ArcadeData.DisplayMeshScale = FVector(1.f, 1.f, 1.f);
	ArcadeData.Tint = FLinearColor(0.18f, 0.56f, 0.34f, 1.f);

	ResolvedArcadeData = ArcadeData;
	ApplyRarityVisuals();
}

void AT66ArcadeInteractableBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshResolvedArcadeData();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

bool AT66ArcadeInteractableBase::Interact(APlayerController* PC)
{
	const FT66ArcadeInteractableData& Data = GetArcadeData();
	if (bConsumed
		|| bArcadeSessionActive
		|| Data.ArcadeClass != ET66ArcadeInteractableClass::PopupArcade)
	{
		return false;
	}

	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC)
	{
		return false;
	}

	const FT66ArcadeInteractableData SessionData = BuildArcadeSessionData();
	if (!T66IsPlayableArcadeGameType(SessionData.ArcadeGameType)
		|| !T66PC->OpenArcadePopup(SessionData, this))
	{
		return false;
	}

	bArcadeSessionActive = true;
	RefreshInteractionPrompt();
	return true;
}

void AT66ArcadeInteractableBase::HandleArcadePopupClosed(const bool bSucceeded, const int32 FinalScore)
{
	bArcadeSessionActive = false;

	const FT66ArcadeInteractableData& Data = GetArcadeData();
	const bool bShouldConsume = bSucceeded ? Data.bConsumeOnSuccess : Data.bConsumeOnFailure;
	if (bShouldConsume)
	{
		if (FinalScore > 0)
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
				{
					RunState->AddScore(FinalScore);
				}
			}
		}
		PlayCompletionEffects(FinalScore);
		SpawnCompletionRewards(FinalScore);
		bConsumed = true;
		Destroy();
		return;
	}

	RefreshInteractionPrompt();
}

void AT66ArcadeInteractableBase::ApplyRarityVisuals()
{
	if (!VisualMesh)
	{
		return;
	}

	const FT66ArcadeInteractableData& Data = GetArcadeData();
	SingleMesh = Data.DisplayMesh;
	if (TryApplyImportedMesh())
	{
		VisualMesh->SetRelativeLocation(T66ResolveImportedArcadeLocation(VisualMesh, Data.DisplayMeshOffset));
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, Data.Tint);
		RefreshInteractionPrompt();
		return;
	}

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
	}

	VisualMesh->SetRelativeLocation(Data.DisplayMeshOffset);
	VisualMesh->SetRelativeScale3D(T66ResolveArcadeScale(Data.DisplayMeshScale));
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, Data.Tint);
	RefreshInteractionPrompt();
}

bool AT66ArcadeInteractableBase::ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const
{
	return !bArcadeSessionActive && AT66WorldInteractableBase::ShouldShowInteractionPrompt(LocalHero);
}

FText AT66ArcadeInteractableBase::BuildInteractionPromptText() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66InteractionPromptSubsystem* PromptSubsystem = GI ? GI->GetSubsystem<UT66InteractionPromptSubsystem>() : nullptr;
	return PromptSubsystem
		? PromptSubsystem->BuildCustomPromptText(ResolveInteractionVerb())
		: ResolveInteractionVerb();
}

FVector AT66ArcadeInteractableBase::GetImportedVisualScale() const
{
	return T66ResolveArcadeScale(GetArcadeData().DisplayMeshScale);
}

void AT66ArcadeInteractableBase::RefreshResolvedArcadeData()
{
	ResolvedArcadeData = ArcadeData;

	const FName EffectiveRowID = !ArcadeRowID.IsNone() ? ArcadeRowID : ArcadeData.ArcadeID;
	FT66ArcadeInteractableData TableData;
	if (T66TryResolveArcadeRowData(this, EffectiveRowID, TableData))
	{
		ResolvedArcadeData = TableData;
	}

	if (ResolvedArcadeData.ArcadeID.IsNone())
	{
		ResolvedArcadeData.ArcadeID = EffectiveRowID;
	}
	ResolvedArcadeData.ArcadeClass = ET66ArcadeInteractableClass::PopupArcade;
}

FT66ArcadeInteractableData AT66ArcadeInteractableBase::BuildArcadeSessionData() const
{
	const FT66ArcadeInteractableData& SourceData = GetArcadeData();
	FT66ArcadeInteractableData SessionData = SourceData;
	if (SessionData.ArcadeGameType != ET66ArcadeGameType::Random)
	{
		return SessionData;
	}

	const ET66ArcadeGameType ChosenGameType = ResolveRandomGameType(SourceData);
	FT66ArcadeInteractableData GameData;
	if (const FName GameRowID = T66GetArcadeRowIDForGameType(ChosenGameType); !GameRowID.IsNone()
		&& T66TryResolveArcadeRowData(this, GameRowID, GameData))
	{
		SessionData = GameData;
	}

	SessionData.ArcadeClass = ET66ArcadeInteractableClass::PopupArcade;
	SessionData.ArcadeGameType = ChosenGameType;
	if (SessionData.DisplayName.IsEmpty())
	{
		SessionData.DisplayName = T66GetArcadeDisplayNameForGameType(ChosenGameType);
	}

	return SessionData;
}

FText AT66ArcadeInteractableBase::ResolveInteractionVerb() const
{
	const FT66ArcadeInteractableData& Data = GetArcadeData();
	if (!Data.InteractionVerb.IsEmpty())
	{
		return Data.InteractionVerb;
	}

	return NSLOCTEXT("T66.Arcade", "FallbackArcadeVerb", "play arcade");
}

ET66ArcadeGameType AT66ArcadeInteractableBase::ResolveRandomGameType(const FT66ArcadeInteractableData& Data) const
{
	TArray<ET66ArcadeGameType> Candidates;
	for (const ET66ArcadeGameType GameType : Data.RandomGameTypes)
	{
		if (T66IsPlayableArcadeGameType(GameType))
		{
			Candidates.AddUnique(GameType);
		}
	}

	if (Candidates.Num() == 0)
	{
		Candidates.Add(ET66ArcadeGameType::WhackAMole);
		Candidates.Add(ET66ArcadeGameType::Topwar);
	}

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
		{
			return Candidates[RngSub->RunRandRange(0, Candidates.Num() - 1)];
		}
	}

	return Candidates[FMath::RandHelper(Candidates.Num())];
}

void AT66ArcadeInteractableBase::SpawnCompletionRewards(const int32 FinalScore)
{
	if (FinalScore <= 0 || !GetWorld())
	{
		return;
	}

	const FT66ArcadeRewardTuning& Tuning = GetArcadeData().RewardTuning;
	const float ScoreAlpha = Tuning.ScoreForFullRewardChance > 0
		? FMath::Clamp(static_cast<float>(FinalScore) / static_cast<float>(Tuning.ScoreForFullRewardChance), 0.f, 1.f)
		: 1.f;

	FRandomStream FallbackRewardRng(FMath::Rand());
	FRandomStream* RewardRng = &FallbackRewardRng;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
		{
			RewardRng = &RngSub->GetRunStream();
		}
	}

	int32 RewardIndex = 0;
	const int32 RewardCountBudget = FMath::Max(1, Tuning.MaxLootBags + 1 + Tuning.MaxWeaponCrates + Tuning.MaxAmplifiers);
	const float LootBagChance = T66ResolveRewardChance(Tuning.LootBagBaseChance, Tuning.LootBagMaxChance, ScoreAlpha);
	for (int32 Attempt = 0; Attempt < FMath::Max(0, Tuning.MaxLootBags); ++Attempt)
	{
		const float AttemptChance = FMath::Clamp(LootBagChance * FMath::Pow(0.78f, static_cast<float>(Attempt)), 0.f, 1.f);
		if (RewardRng->FRand() <= AttemptChance)
		{
			SpawnLootBagReward(BuildRewardSpawnLocation(RewardIndex++, RewardCountBudget), *RewardRng);
		}
	}

	const float ChestChance = T66ResolveRewardChance(Tuning.ChestBaseChance, Tuning.ChestMaxChance, ScoreAlpha);
	if (RewardRng->FRand() <= ChestChance)
	{
		SpawnChestReward(BuildRewardSpawnLocation(RewardIndex++, RewardCountBudget));
	}

	const float WeaponCrateChance = T66ResolveRewardChance(Tuning.WeaponCrateBaseChance, Tuning.WeaponCrateMaxChance, ScoreAlpha);
	for (int32 Attempt = 0; Attempt < FMath::Max(0, Tuning.MaxWeaponCrates); ++Attempt)
	{
		const float AttemptChance = FMath::Clamp(WeaponCrateChance * FMath::Pow(0.65f, static_cast<float>(Attempt)), 0.f, 1.f);
		if (RewardRng->FRand() <= AttemptChance)
		{
			SpawnWeaponCrateReward(BuildRewardSpawnLocation(RewardIndex++, RewardCountBudget), *RewardRng);
		}
	}

	const float AmplifierChance = T66ResolveRewardChance(Tuning.AmplifierBaseChance, Tuning.AmplifierMaxChance, ScoreAlpha);
	for (int32 Attempt = 0; Attempt < FMath::Max(0, Tuning.MaxAmplifiers); ++Attempt)
	{
		const float AttemptChance = FMath::Clamp(AmplifierChance * FMath::Pow(0.70f, static_cast<float>(Attempt)), 0.f, 1.f);
		if (RewardRng->FRand() <= AttemptChance)
		{
			SpawnAmplifierReward(
				BuildRewardSpawnLocation(RewardIndex++, RewardCountBudget),
				*RewardRng,
				FMath::Max(1, Tuning.AmplifierStatBonus),
				FMath::Max(1.f, Tuning.AmplifierDurationSeconds));
		}
	}
}

void AT66ArcadeInteractableBase::SpawnLootBagReward(const FVector& SpawnLocation, FRandomStream& Rng)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AT66LootBagPickup* LootBag = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (!LootBag)
	{
		return;
	}

	const ET66Rarity BagRarity = FT66RarityUtil::RollDefaultRarity(Rng);
	LootBag->SetLootRarity(BagRarity);
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
	{
		LootBag->SetItemID(T66GI->GetRandomItemIDForLootRarityFromStream(BagRarity, Rng));
	}
}

void AT66ArcadeInteractableBase::SpawnChestReward(const FVector& SpawnLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66ChestInteractable* Chest = World->SpawnActor<AT66ChestInteractable>(AT66ChestInteractable::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Chest->bIsMimic = false;
		Chest->SetRarity(ET66Rarity::Yellow);
	}
}

void AT66ArcadeInteractableBase::SpawnWeaponCrateReward(const FVector& SpawnLocation, FRandomStream& Rng)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66CrateInteractable* Crate = World->SpawnActor<AT66CrateInteractable>(AT66CrateInteractable::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Crate->SetRarity(FT66RarityUtil::RollDefaultRarity(Rng));
	}
}

void AT66ArcadeInteractableBase::SpawnAmplifierReward(
	const FVector& SpawnLocation,
	FRandomStream& Rng,
	const int32 BonusStatPoints,
	const float DurationSeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TArray<ET66HeroStatType>& StatPool = T66GetAmplifierStatPool();
	const ET66HeroStatType StatType = StatPool.IsValidIndex(0)
		? StatPool[Rng.RandRange(0, StatPool.Num() - 1)]
		: ET66HeroStatType::Damage;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66ArcadeAmplifierPickup* Amplifier = World->SpawnActor<AT66ArcadeAmplifierPickup>(AT66ArcadeAmplifierPickup::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Amplifier->ConfigureAmplifier(StatType, BonusStatPoints, DurationSeconds);
	}
}

FVector AT66ArcadeInteractableBase::BuildRewardSpawnLocation(const int32 RewardIndex, const int32 RewardCount) const
{
	const int32 SafeCount = FMath::Max(1, RewardCount);
	const float AngleRadians = (TWO_PI * static_cast<float>(RewardIndex)) / static_cast<float>(SafeCount);
	const float Radius = 240.f + 38.f * static_cast<float>(RewardIndex % 3);
	return GetActorLocation() + FVector(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 90.f);
}

void AT66ArcadeInteractableBase::PlayCompletionEffects(const int32 /*FinalScore*/)
{
	UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Arcade.Machine.Explode")), GetActorLocation(), this);

	if (UWorld* World = GetWorld())
	{
		if (UT66PixelVFXSubsystem* PixelVFX = World->GetSubsystem<UT66PixelVFXSubsystem>())
		{
			const FVector Origin = GetActorLocation() + FVector(0.f, 0.f, 120.f);
			for (int32 Index = 0; Index < 18; ++Index)
			{
				const float Angle = (TWO_PI * static_cast<float>(Index)) / 18.f;
				const float Radius = FMath::FRandRange(18.f, 90.f);
				const FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, FMath::FRandRange(-20.f, 80.f));
				const FLinearColor Tint = (Index % 3 == 0)
					? FLinearColor(1.f, 0.26f, 0.08f, 1.f)
					: ((Index % 3 == 1)
						? FLinearColor(1.f, 0.78f, 0.18f, 1.f)
						: FLinearColor(0.24f, 0.58f, 1.f, 1.f));
				PixelVFX->SpawnPixelAtLocation(
					Origin + Offset,
					Tint,
					FVector2D(FMath::FRandRange(10.f, 20.f), FMath::FRandRange(10.f, 20.f)),
					ET66PixelVFXPriority::High);
			}
		}
	}
}
