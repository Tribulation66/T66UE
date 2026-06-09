// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BoostInteractable.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	static FSoftObjectPath T66ResolveBoostMeshPath(const ET66HeroStatType StatType)
	{
		switch (StatType)
		{
		case ET66HeroStatType::Damage:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_DamageBoost_Pixal3D.SM_DamageBoost_Pixal3D"));
		case ET66HeroStatType::AttackSpeed:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_AttackSpeedBoost_Pixal3D.SM_AttackSpeedBoost_Pixal3D"));
		case ET66HeroStatType::AttackScale:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_AttackScaleBoost_Pixal3D.SM_AttackScaleBoost_Pixal3D"));
		case ET66HeroStatType::Armor:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_ArmorBoost_Pixal3D.SM_ArmorBoost_Pixal3D"));
		case ET66HeroStatType::Evasion:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_EvasionBoost_Pixal3D.SM_EvasionBoost_Pixal3D"));
		case ET66HeroStatType::Luck:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_LuckBoost_Pixal3D.SM_LuckBoost_Pixal3D"));
		case ET66HeroStatType::Speed:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_SpeedBoost_Pixal3D.SM_SpeedBoost_Pixal3D"));
		case ET66HeroStatType::Accuracy:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_AccuracyBoost_Pixal3D.SM_AccuracyBoost_Pixal3D"));
		default:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_DamageBoost_Pixal3D.SM_DamageBoost_Pixal3D"));
		}
	}

	static FSoftObjectPath T66ResolveStatBoostMeshPath(const ET66StatType StatType)
	{
		switch (StatType)
		{
		case ET66StatType::FirePower:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_DamageBoost_Pixal3D.SM_DamageBoost_Pixal3D"));
		case ET66StatType::IcePower:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_ArmorBoost_Pixal3D.SM_ArmorBoost_Pixal3D"));
		case ET66StatType::ElectricityPower:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_AttackSpeedBoost_Pixal3D.SM_AttackSpeedBoost_Pixal3D"));
		case ET66StatType::NaturePower:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_LuckBoost_Pixal3D.SM_LuckBoost_Pixal3D"));
		case ET66StatType::WindPower:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_SpeedBoost_Pixal3D.SM_SpeedBoost_Pixal3D"));
		default:
			return FSoftObjectPath(TEXT("/Game/World/Interactables/Boosts/SM_DamageBoost_Pixal3D.SM_DamageBoost_Pixal3D"));
		}
	}
}

AT66BoostInteractable::AT66BoostInteractable()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 45.f;
	RefreshMeshForStatType();

	if (VisualMesh)
	{
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	ApplyRarityVisuals();
}

void AT66BoostInteractable::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (VisualMesh)
	{
		const FRotator Spin(0.f, 125.f * DeltaSeconds, 0.f);
		VisualMesh->AddRelativeRotation(Spin);
	}
}

void AT66BoostInteractable::ConfigureBoost(
	const ET66HeroStatType InStatType,
	const int32 InBonusStatPoints,
	const float InDurationSeconds)
{
	bUseStat = false;
	BaseStatType = InStatType;
	StatType = ET66StatType::None;
	BonusStatPoints = FMath::Max(1, InBonusStatPoints);
	DurationSeconds = FMath::Max(1.f, InDurationSeconds);
	RefreshMeshForStatType();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

void AT66BoostInteractable::ConfigureStatBoost(
	const ET66StatType InStatType,
	const int32 InBonusStatPoints,
	const float InDurationSeconds)
{
	bUseStat = true;
	StatType = InStatType;
	BonusStatPoints = FMath::Max(1, InBonusStatPoints);
	DurationSeconds = FMath::Max(1.f, InDurationSeconds);
	RefreshMeshForStatType();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

bool AT66BoostInteractable::Interact(APlayerController* PC)
{
	if (!PC || bConsumed)
	{
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return false;
	}

	if (bUseStat)
	{
		RunState->ApplyTemporaryStatAmplifier(StatType, BonusStatPoints, DurationSeconds);
	}
	else
	{
		RunState->ApplyTemporaryBaseStatAmplifier(BaseStatType, BonusStatPoints, DurationSeconds);
	}
	UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Boost.Interact")), GetActorLocation(), this);

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

void AT66BoostInteractable::ApplyRarityVisuals()
{
	if (!VisualMesh)
	{
		return;
	}

	RefreshMeshForStatType();
	if (TryApplyImportedMesh())
	{
		return;
	}

	VisualMesh->SetStaticMesh(nullptr);
}

void AT66BoostInteractable::RefreshMeshForStatType()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(bUseStat
		? T66ResolveStatBoostMeshPath(StatType)
		: T66ResolveBoostMeshPath(BaseStatType));
}

FText AT66BoostInteractable::BuildInteractionPromptText() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66InteractionPromptSubsystem* PromptSubsystem = GI ? GI->GetSubsystem<UT66InteractionPromptSubsystem>() : nullptr;
	const FText Action = NSLOCTEXT("T66.BoostInteractable", "ClaimBoostVerb", "claim boost");
	return PromptSubsystem ? PromptSubsystem->BuildCustomPromptText(Action) : Action;
}

FText AT66BoostInteractable::BuildInteractionPromptTargetName() const
{
	return FText::Format(
		NSLOCTEXT("T66.BoostInteractable", "BoostTargetName", "{0} Boost"),
		ResolveStatDisplayName());
}

FText AT66BoostInteractable::ResolveStatDisplayName() const
{
	if (bUseStat)
	{
		switch (StatType)
		{
		case ET66StatType::FirePower:
			return NSLOCTEXT("T66.BoostInteractable", "FirePowerStat", "Fire Power");
		case ET66StatType::IcePower:
			return NSLOCTEXT("T66.BoostInteractable", "IcePowerStat", "Ice Power");
		case ET66StatType::ElectricityPower:
			return NSLOCTEXT("T66.BoostInteractable", "ElectricityPowerStat", "Electricity Power");
		case ET66StatType::NaturePower:
			return NSLOCTEXT("T66.BoostInteractable", "NaturePowerStat", "Nature Power");
		case ET66StatType::WindPower:
			return NSLOCTEXT("T66.BoostInteractable", "WindPowerStat", "Wind Power");
		default:
			return NSLOCTEXT("T66.BoostInteractable", "Stat", "Stat");
		}
	}

	switch (BaseStatType)
	{
	case ET66HeroStatType::Damage:
		return NSLOCTEXT("T66.BoostInteractable", "DamageStat", "Damage");
	case ET66HeroStatType::AttackSpeed:
		return NSLOCTEXT("T66.BoostInteractable", "AttackSpeedStat", "Attack Speed");
	case ET66HeroStatType::AttackScale:
		return NSLOCTEXT("T66.BoostInteractable", "AttackScaleStat", "Attack Scale");
	case ET66HeroStatType::Armor:
		return NSLOCTEXT("T66.BoostInteractable", "ArmorStat", "Armor");
	case ET66HeroStatType::Evasion:
		return NSLOCTEXT("T66.BoostInteractable", "EvasionStat", "Evasion");
	case ET66HeroStatType::Luck:
		return NSLOCTEXT("T66.BoostInteractable", "LuckStat", "Luck");
	case ET66HeroStatType::Speed:
		return NSLOCTEXT("T66.BoostInteractable", "SpeedStat", "Speed");
	case ET66HeroStatType::Accuracy:
		return NSLOCTEXT("T66.BoostInteractable", "AccuracyStat", "Accuracy");
	default:
		return NSLOCTEXT("T66.BoostInteractable", "GenericStat", "Stat");
	}
}
