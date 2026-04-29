// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66WeaponManagerSubsystem.h"
#include "Core/T66GameInstance.h"

void UT66WeaponManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	WeaponOfferIDs.Reserve(4);
}

void UT66WeaponManagerSubsystem::BroadcastWeaponStateChanged()
{
	OnWeaponStateChanged.Broadcast();
}

void UT66WeaponManagerSubsystem::ResetForNewRun(FName HeroID)
{
	EquippedWeaponID = MakeStarterWeaponID(HeroID);
	WeaponOfferIDs.Reset();
	CurrentOfferRarity = ET66WeaponRarity::Black;

	if (HeroID.IsNone())
	{
		EquippedWeaponID = NAME_None;
	}

	BroadcastWeaponStateChanged();
}

void UT66WeaponManagerSubsystem::RestoreState(FName InEquippedWeaponID)
{
	EquippedWeaponID = InEquippedWeaponID;
	WeaponOfferIDs.Reset();
	CurrentOfferRarity = ET66WeaponRarity::Black;
	BroadcastWeaponStateChanged();
}

bool UT66WeaponManagerSubsystem::GetEquippedWeaponData(FWeaponData& OutWeaponData) const
{
	if (EquippedWeaponID.IsNone())
	{
		return false;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	return GI ? GI->GetWeaponData(EquippedWeaponID, OutWeaponData) : false;
}

void UT66WeaponManagerSubsystem::BuildWeaponOffers(FName HeroID, ET66WeaponRarity Rarity)
{
	WeaponOfferIDs.Reset();
	CurrentOfferRarity = Rarity;

	if (HeroID.IsNone())
	{
		BroadcastWeaponStateChanged();
		return;
	}

	static const ET66AttackCategory Branches[] =
	{
		ET66AttackCategory::Pierce,
		ET66AttackCategory::Bounce,
		ET66AttackCategory::AOE,
		ET66AttackCategory::DOT,
	};

	for (ET66AttackCategory Branch : Branches)
	{
		WeaponOfferIDs.Add(MakeWeaponID(HeroID, Rarity, Branch));
	}

	BroadcastWeaponStateChanged();
}

bool UT66WeaponManagerSubsystem::SelectWeapon(FName WeaponID)
{
	if (WeaponID.IsNone())
	{
		return false;
	}

	FWeaponData WeaponData;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI || !GI->GetWeaponData(WeaponID, WeaponData))
	{
		return false;
	}

	if (WeaponOfferIDs.Num() > 0 && !WeaponOfferIDs.Contains(WeaponID))
	{
		return false;
	}

	EquippedWeaponID = WeaponID;
	BroadcastWeaponStateChanged();
	return true;
}

FName UT66WeaponManagerSubsystem::MakeStarterWeaponID(FName HeroID)
{
	return HeroID.IsNone()
		? NAME_None
		: FName(FString::Printf(TEXT("%s_Grey_Base"), *HeroID.ToString()));
}

FName UT66WeaponManagerSubsystem::MakeWeaponID(FName HeroID, ET66WeaponRarity Rarity, ET66AttackCategory Branch)
{
	if (HeroID.IsNone())
	{
		return NAME_None;
	}

	return FName(FString::Printf(
		TEXT("%s_%s_%s"),
		*HeroID.ToString(),
		*WeaponRarityToString(Rarity),
		*AttackBranchToString(Branch)));
}

ET66WeaponRarity UT66WeaponManagerSubsystem::GetUpgradeRarityForClearedDifficulty(ET66Difficulty Difficulty)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:
		return ET66WeaponRarity::Black;
	case ET66Difficulty::Medium:
		return ET66WeaponRarity::Red;
	case ET66Difficulty::Hard:
		return ET66WeaponRarity::Yellow;
	case ET66Difficulty::VeryHard:
	case ET66Difficulty::Impossible:
	default:
		return ET66WeaponRarity::White;
	}
}

ET66WeaponRarity UT66WeaponManagerSubsystem::GetUpgradeRarityForStartingDifficulty(ET66Difficulty Difficulty)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Medium:
		return ET66WeaponRarity::Black;
	case ET66Difficulty::Hard:
		return ET66WeaponRarity::Red;
	case ET66Difficulty::VeryHard:
		return ET66WeaponRarity::Yellow;
	case ET66Difficulty::Impossible:
		return ET66WeaponRarity::White;
	case ET66Difficulty::Easy:
	default:
		return ET66WeaponRarity::Grey;
	}
}

FString UT66WeaponManagerSubsystem::WeaponRarityToString(ET66WeaponRarity Rarity)
{
	switch (Rarity)
	{
	case ET66WeaponRarity::Grey:
		return TEXT("Grey");
	case ET66WeaponRarity::Black:
		return TEXT("Black");
	case ET66WeaponRarity::Red:
		return TEXT("Red");
	case ET66WeaponRarity::Yellow:
		return TEXT("Yellow");
	case ET66WeaponRarity::White:
	default:
		return TEXT("White");
	}
}

FString UT66WeaponManagerSubsystem::AttackBranchToString(ET66AttackCategory Branch)
{
	switch (Branch)
	{
	case ET66AttackCategory::Pierce:
		return TEXT("Pierce");
	case ET66AttackCategory::Bounce:
		return TEXT("Bounce");
	case ET66AttackCategory::AOE:
		return TEXT("AOE");
	case ET66AttackCategory::DOT:
	default:
		return TEXT("DOT");
	}
}

FLinearColor UT66WeaponManagerSubsystem::GetWeaponRarityColor(ET66WeaponRarity Rarity)
{
	switch (Rarity)
	{
	case ET66WeaponRarity::Grey:
		return FLinearColor(0.40f, 0.42f, 0.45f, 1.0f);
	case ET66WeaponRarity::Black:
		return FLinearColor(0.05f, 0.05f, 0.06f, 1.0f);
	case ET66WeaponRarity::Red:
		return FLinearColor(0.86f, 0.10f, 0.10f, 1.0f);
	case ET66WeaponRarity::Yellow:
		return FLinearColor(1.0f, 0.78f, 0.18f, 1.0f);
	case ET66WeaponRarity::White:
	default:
		return FLinearColor(0.92f, 0.95f, 1.0f, 1.0f);
	}
}
