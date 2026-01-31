// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"

const TArray<FName>& UT66RunStateSubsystem::GetAllIdolIDs()
{
	// Stable IDs (contract): keep these names stable forever.
	static const TArray<FName> Idols = {
		FName(TEXT("Idol_Frost")),
		FName(TEXT("Idol_Shock")),
		FName(TEXT("Idol_Glue")),
		FName(TEXT("Idol_Silence")),
		FName(TEXT("Idol_Mark")),
		FName(TEXT("Idol_Pierce")),
		FName(TEXT("Idol_Split")),
		FName(TEXT("Idol_Knockback")),
		FName(TEXT("Idol_Ricochet")),
		FName(TEXT("Idol_Hex")),
		FName(TEXT("Idol_Fire")),
		FName(TEXT("Idol_Lifesteal")),
	};
	return Idols;
}

FLinearColor UT66RunStateSubsystem::GetIdolColor(FName IdolID)
{
	if (IdolID == FName(TEXT("Idol_Frost"))) return FLinearColor(0.35f, 0.75f, 1.0f, 1.f);
	if (IdolID == FName(TEXT("Idol_Shock"))) return FLinearColor(0.70f, 0.25f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Glue"))) return FLinearColor(0.20f, 0.85f, 0.35f, 1.f);
	if (IdolID == FName(TEXT("Idol_Silence"))) return FLinearColor(0.55f, 0.55f, 0.75f, 1.f);
	if (IdolID == FName(TEXT("Idol_Mark"))) return FLinearColor(0.95f, 0.80f, 0.20f, 1.f);
	if (IdolID == FName(TEXT("Idol_Pierce"))) return FLinearColor(0.92f, 0.92f, 0.98f, 1.f);
	if (IdolID == FName(TEXT("Idol_Split"))) return FLinearColor(0.95f, 0.20f, 0.20f, 1.f);
	if (IdolID == FName(TEXT("Idol_Knockback"))) return FLinearColor(0.95f, 0.55f, 0.15f, 1.f);
	if (IdolID == FName(TEXT("Idol_Ricochet"))) return FLinearColor(0.20f, 0.95f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Hex"))) return FLinearColor(0.10f, 0.10f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Fire"))) return FLinearColor(0.95f, 0.25f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Lifesteal"))) return FLinearColor(0.75f, 0.10f, 0.25f, 1.f);
	return FLinearColor(0.25f, 0.25f, 0.28f, 1.f);
}

bool UT66RunStateSubsystem::EquipIdolInSlot(int32 SlotIndex, FName IdolID)
{
	if (SlotIndex < 0 || SlotIndex >= MaxEquippedIdolSlots) return false;
	if (IdolID.IsNone()) return false;

	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	}

	if (EquippedIdolIDs[SlotIndex] == IdolID) return false;
	EquippedIdolIDs[SlotIndex] = IdolID;
	IdolsChanged.Broadcast();
	return true;
}

bool UT66RunStateSubsystem::EquipIdolFirstEmpty(FName IdolID)
{
	if (IdolID.IsNone()) return false;
	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	}

	for (int32 i = 0; i < EquippedIdolIDs.Num(); ++i)
	{
		if (EquippedIdolIDs[i].IsNone())
		{
			EquippedIdolIDs[i] = IdolID;
			IdolsChanged.Broadcast();
			return true;
		}
	}
	return false;
}

void UT66RunStateSubsystem::ClearEquippedIdols()
{
	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
		IdolsChanged.Broadcast();
		return;
	}

	bool bAny = false;
	for (FName& N : EquippedIdolIDs)
	{
		if (!N.IsNone())
		{
			N = NAME_None;
			bAny = true;
		}
	}
	if (bAny)
	{
		IdolsChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::TrimLogsIfNeeded()
{
	if (EventLog.Num() > MaxEventLogEntries)
	{
		const int32 RemoveCount = EventLog.Num() - MaxEventLogEntries;
		EventLog.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
	if (StructuredEventLog.Num() > MaxStructuredEventLogEntries)
	{
		const int32 RemoveCount = StructuredEventLog.Num() - MaxStructuredEventLogEntries;
		StructuredEventLog.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
}

float UT66RunStateSubsystem::GetDifficultyMultiplier() const
{
	// Linear scaling: Tier 0 => 1x, Tier 1 => 2x, Tier 2 => 3x, ...
	return 1.f + static_cast<float>(FMath::Max(0, DifficultyTier));
}

void UT66RunStateSubsystem::IncreaseDifficultyTier()
{
	AddDifficultySkulls(1.0f);
}

void UT66RunStateSubsystem::AddDifficultySkulls(float DeltaSkulls)
{
	if (DeltaSkulls == 0.f) return;
	DifficultySkulls = FMath::Clamp(DifficultySkulls + DeltaSkulls, 0.f, 9999.f);

	// Cache integer tier for existing enemy scaling logic (tier 0 -> 1x, tier 1 -> 2x, ...).
	DifficultyTier = FMath::Clamp(FMath::FloorToInt(DifficultySkulls), 0, 999);
	DifficultyChanged.Broadcast();
}

void UT66RunStateSubsystem::AddMaxHearts(int32 DeltaHearts)
{
	if (DeltaHearts <= 0) return;
	MaxHearts = FMath::Clamp(MaxHearts + DeltaHearts, 1, 9999);
	CurrentHearts = FMath::Clamp(CurrentHearts + DeltaHearts, 0, MaxHearts);
	HeartsChanged.Broadcast();
}

int32 UT66RunStateSubsystem::RegisterTotemActivated()
{
	TotemsActivatedCount = FMath::Clamp(TotemsActivatedCount + 1, 0, 999);
	return TotemsActivatedCount;
}

float UT66RunStateSubsystem::AddGamblerAngerFromBet(int32 BetGold)
{
	const float Delta = FMath::Clamp(static_cast<float>(FMath::Max(0, BetGold)) / 100.f, 0.f, 1.f);
	GamblerAnger01 = FMath::Clamp(GamblerAnger01 + Delta, 0.f, 1.f);
	GamblerAngerChanged.Broadcast();
	return GamblerAnger01;
}

void UT66RunStateSubsystem::ResetGamblerAnger()
{
	if (FMath::IsNearlyZero(GamblerAnger01)) return;
	GamblerAnger01 = 0.f;
	GamblerAngerChanged.Broadcast();
}

float UT66RunStateSubsystem::GetHeroMoveSpeedMultiplier() const
{
	// v0 mapping: +3% per level
	const int32 L = FMath::Max(1, HeroLevel);
	return 1.f + (static_cast<float>(L - 1) * 0.03f);
}

float UT66RunStateSubsystem::GetHeroDamageMultiplier() const
{
	// v0 mapping: +5% per level
	const int32 L = FMath::Max(1, HeroLevel);
	return 1.f + (static_cast<float>(L - 1) * 0.05f);
}

float UT66RunStateSubsystem::GetHeroAttackSpeedMultiplier() const
{
	// v0 mapping: +3% per level
	const int32 L = FMath::Max(1, HeroLevel);
	return 1.f + (static_cast<float>(L - 1) * 0.03f);
}

float UT66RunStateSubsystem::GetHeroScaleMultiplier() const
{
	// v0 mapping: +2% per level
	const int32 L = FMath::Max(1, HeroLevel);
	return 1.f + (static_cast<float>(L - 1) * 0.02f);
}

float UT66RunStateSubsystem::GetArmorReduction01() const
{
	// v0 mapping: +1% per level, clamped.
	const int32 L = FMath::Max(1, HeroLevel);
	const float Base = static_cast<float>(L - 1) * 0.01f;
	return FMath::Clamp(Base + ItemArmorBonus01, 0.f, 0.80f);
}

float UT66RunStateSubsystem::GetEvasionChance01() const
{
	// v0 mapping: +1% per level, clamped.
	const int32 L = FMath::Max(1, HeroLevel);
	const float Base = static_cast<float>(L - 1) * 0.01f;
	return FMath::Clamp(Base + ItemEvasionBonus01, 0.f, 0.60f);
}

void UT66RunStateSubsystem::AddHeroXP(int32 Amount)
{
	if (Amount <= 0) return;
	if (HeroLevel <= 0) HeroLevel = DefaultHeroLevel;
	if (XPToNextLevel <= 0) XPToNextLevel = DefaultXPToLevel;

	HeroXP = FMath::Clamp(HeroXP + Amount, 0, 1000000000);
	bool bLeveled = false;
	while (HeroXP >= XPToNextLevel && XPToNextLevel > 0)
	{
		HeroXP -= XPToNextLevel;
		HeroLevel = FMath::Clamp(HeroLevel + 1, 1, 9999);
		bLeveled = true;
	}
	HeroProgressChanged.Broadcast();
	if (bLeveled)
	{
		LogAdded.Broadcast();
	}
}

void UT66RunStateSubsystem::ResetVendorForStage()
{
	VendorAngerGold = 0;
	VendorStockStage = 0;
	VendorStockItemIDs.Reset();
	VendorStockSold.Reset();
	bVendorBoughtSomethingThisStage = false;
	VendorChanged.Broadcast();
}

void UT66RunStateSubsystem::ResetVendorAnger()
{
	if (VendorAngerGold == 0) return;
	VendorAngerGold = 0;
	VendorChanged.Broadcast();
}

void UT66RunStateSubsystem::SetInStageBoost(bool bInBoost)
{
	if (bInStageBoost == bInBoost) return;
	bInStageBoost = bInBoost;
	StageChanged.Broadcast();
}

void UT66RunStateSubsystem::ApplyStageSpeedBoost(float MoveSpeedMultiplier, float DurationSeconds)
{
	const float Mult = FMath::Clamp(MoveSpeedMultiplier, 0.25f, 5.f);
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;

	StageMoveSpeedMultiplier = FMath::Max(StageMoveSpeedMultiplier, Mult);
	StageMoveSpeedSecondsRemaining = FMath::Max(StageMoveSpeedSecondsRemaining, Dur);
	HeroProgressChanged.Broadcast(); // movement uses derived stat refresh
}

bool UT66RunStateSubsystem::ApplyTrueDamage(int32 Hearts)
{
	if (Hearts <= 0) return false;
	if (bInLastStand) return false;

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	if (Now - LastDamageTime < InvulnDurationSeconds)
	{
		return false;
	}

	LastDamageTime = Now;
	const int32 Reduced = FMath::Max(1, Hearts);
	CurrentHearts = FMath::Max(0, CurrentHearts - Reduced);

	// Survival charge fills on real damage taken.
	SurvivalCharge01 = FMath::Clamp(SurvivalCharge01 + (static_cast<float>(Reduced) * SurvivalChargePerHeart), 0.f, 1.f);
	SurvivalChanged.Broadcast();
	HeartsChanged.Broadcast();

	if (CurrentHearts <= 0)
	{
		// If charged, trigger last-stand instead of dying.
		if (SurvivalCharge01 >= 1.f)
		{
			EnterLastStand();
		}
		else
		{
			OnPlayerDied.Broadcast();
		}
	}
	return true;
}

void UT66RunStateSubsystem::ApplyStatusBurn(float DurationSeconds, float DamagePerSecond)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;
	StatusBurnSecondsRemaining = FMath::Max(StatusBurnSecondsRemaining, Dur);
	StatusBurnDamagePerSecond = FMath::Max(0.f, DamagePerSecond);
	StatusEffectsChanged.Broadcast();
}

void UT66RunStateSubsystem::ApplyStatusChill(float DurationSeconds, float MoveSpeedMultiplier)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;
	StatusChillSecondsRemaining = FMath::Max(StatusChillSecondsRemaining, Dur);
	StatusChillMoveSpeedMultiplier = FMath::Clamp(MoveSpeedMultiplier, 0.1f, 1.f);
	StatusEffectsChanged.Broadcast();
	HeroProgressChanged.Broadcast(); // movement updates
}

void UT66RunStateSubsystem::ApplyStatusCurse(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;
	StatusCurseSecondsRemaining = FMath::Max(StatusCurseSecondsRemaining, Dur);
	StatusEffectsChanged.Broadcast();
}

void UT66RunStateSubsystem::EnsureVendorStockForCurrentStage()
{
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 66);
	if (VendorStockStage == Stage && VendorStockItemIDs.Num() > 0 && VendorStockSold.Num() == VendorStockItemIDs.Num())
	{
		return;
	}

	VendorStockStage = Stage;
	VendorStockItemIDs.Reset();
	VendorStockSold.Reset();

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI)
	{
		// Fallback: keep deterministic placeholder behavior.
		VendorStockItemIDs = { FName(TEXT("Item_01")), FName(TEXT("Item_02")), FName(TEXT("Item_03")) };
		VendorStockSold.Init(false, VendorStockItemIDs.Num());
		VendorChanged.Broadcast();
		return;
	}

	UDataTable* ItemsDT = GI->GetItemsDataTable();
	TArray<FName> PoolBlack;
	TArray<FName> PoolRed;
	TArray<FName> PoolYellow;

		if (ItemsDT)
	{
		const TArray<FName> RowNames = ItemsDT->GetRowNames();
		for (const FName& ItemID : RowNames)
		{
			if (ItemID.IsNone()) continue;
			FItemData D;
			if (!GI->GetItemData(ItemID, D)) continue;
			if (D.BuyValueGold <= 0) continue;

				// Canonical pool: items can appear in shop and in loot bags.
			switch (D.ItemRarity)
			{
				case ET66ItemRarity::Black:  PoolBlack.Add(ItemID); break;
				case ET66ItemRarity::Red:    PoolRed.Add(ItemID); break;
				case ET66ItemRarity::Yellow: PoolYellow.Add(ItemID); break;
					case ET66ItemRarity::White:  PoolYellow.Add(ItemID); break; // v0: allow whites in the shop pool by treating them as "rare slot" candidates
				default: break;
			}
		}
	}

	// Fallback if DT missing or pools empty.
	if (PoolBlack.Num() == 0) PoolBlack = { FName(TEXT("Item_01")) };
	if (PoolRed.Num() == 0) PoolRed = { FName(TEXT("Item_02")) };
	if (PoolYellow.Num() == 0) PoolYellow = { FName(TEXT("Item_03")) };

	FRandomStream Rng(Stage * 777 + 13);
	auto PickUnique = [&](const TArray<FName>& Pool) -> FName
	{
		if (Pool.Num() == 0) return NAME_None;
		for (int32 Try = 0; Try < 40; ++Try)
		{
			const FName C = Pool[Rng.RandRange(0, Pool.Num() - 1)];
			if (!C.IsNone() && !VendorStockItemIDs.Contains(C))
			{
				return C;
			}
		}
		return Pool[0];
	};

	// v0 stock: 4 items (2 black, 1 red, 1 yellow)
	VendorStockItemIDs.Add(PickUnique(PoolBlack));
	VendorStockItemIDs.Add(PickUnique(PoolBlack));
	VendorStockItemIDs.Add(PickUnique(PoolRed));
	VendorStockItemIDs.Add(PickUnique(PoolYellow));

	for (int32 i = VendorStockItemIDs.Num() - 1; i >= 0; --i)
	{
		if (VendorStockItemIDs[i].IsNone())
		{
			VendorStockItemIDs.RemoveAt(i);
		}
	}
	VendorStockSold.Init(false, VendorStockItemIDs.Num());
	VendorChanged.Broadcast();
}

bool UT66RunStateSubsystem::IsVendorStockSlotSold(int32 Index) const
{
	if (Index < 0 || Index >= VendorStockSold.Num()) return true;
	return VendorStockSold[Index];
}

bool UT66RunStateSubsystem::TryBuyVendorStockSlot(int32 Index)
{
	EnsureVendorStockForCurrentStage();
	if (Index < 0 || Index >= VendorStockItemIDs.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;
	if (!HasInventorySpace()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	if (!GI || !GI->GetItemData(VendorStockItemIDs[Index], D)) return false;
	if (D.BuyValueGold <= 0) return false;
	if (!TrySpendGold(D.BuyValueGold)) return false;

	AddItem(VendorStockItemIDs[Index]);
	VendorStockSold[Index] = true;
	bVendorBoughtSomethingThisStage = true;
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("VendorPurchase=%s"), *VendorStockItemIDs[Index].ToString()));
	VendorChanged.Broadcast();
	return true;
}

bool UT66RunStateSubsystem::ResolveVendorStealAttempt(int32 Index, bool bTimingHit, bool bRngSuccess)
{
	EnsureVendorStockForCurrentStage();
	if (Index < 0 || Index >= VendorStockItemIDs.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	if (!GI || !GI->GetItemData(VendorStockItemIDs[Index], D)) return false;
	if (D.BuyValueGold <= 0) return false;

	// v0 anger model tuned to your request: 100g total triggers boss.
	const float AngerMult = bTimingHit ? (bRngSuccess ? 1.0f : 0.5f) : 1.0f;
	VendorAngerGold = FMath::Clamp(VendorAngerGold + FMath::RoundToInt(static_cast<float>(D.BuyValueGold) * AngerMult), 0, 9999999);

	bool bGranted = false;
	if (bTimingHit && bRngSuccess && HasInventorySpace())
	{
		AddItem(VendorStockItemIDs[Index]);
		VendorStockSold[Index] = true;
		AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("VendorSteal=%s"), *VendorStockItemIDs[Index].ToString()));
		bGranted = true;
	}

	VendorChanged.Broadcast();
	return bGranted;
}

bool UT66RunStateSubsystem::TryActivateUltimate()
{
	if (UltimateCooldownRemainingSeconds > 0.f) return false;
	UltimateCooldownRemainingSeconds = UltimateCooldownSeconds;
	LastBroadcastUltimateSecond = FMath::CeilToInt(UltimateCooldownRemainingSeconds);
	UltimateChanged.Broadcast();
	return true;
}

void UT66RunStateSubsystem::TickHeroTimers(float DeltaTime)
{
	// Ultimate cooldown
	if (UltimateCooldownRemainingSeconds > 0.f)
	{
		UltimateCooldownRemainingSeconds = FMath::Max(0.f, UltimateCooldownRemainingSeconds - DeltaTime);
		const int32 Cur = FMath::CeilToInt(UltimateCooldownRemainingSeconds);
		if (Cur != LastBroadcastUltimateSecond)
		{
			LastBroadcastUltimateSecond = Cur;
			UltimateChanged.Broadcast();
		}
	}

	// Last stand timer
	if (bInLastStand && LastStandSecondsRemaining > 0.f)
	{
		LastStandSecondsRemaining = FMath::Max(0.f, LastStandSecondsRemaining - DeltaTime);
		const int32 Cur = FMath::CeilToInt(LastStandSecondsRemaining);
		if (Cur != LastBroadcastLastStandSecond)
		{
			LastBroadcastLastStandSecond = Cur;
			SurvivalChanged.Broadcast();
		}
		if (LastStandSecondsRemaining <= 0.f)
		{
			EndLastStandAndDie();
		}
	}

	// Stage speed boost timer
	if (StageMoveSpeedSecondsRemaining > 0.f)
	{
		StageMoveSpeedSecondsRemaining = FMath::Max(0.f, StageMoveSpeedSecondsRemaining - DeltaTime);
		if (StageMoveSpeedSecondsRemaining <= 0.f)
		{
			StageMoveSpeedMultiplier = 1.f;
			HeroProgressChanged.Broadcast();
		}
	}

	// Status effects timers
	bool bStatusChanged = false;

	if (StatusBurnSecondsRemaining > 0.f)
	{
		StatusBurnSecondsRemaining = FMath::Max(0.f, StatusBurnSecondsRemaining - DeltaTime);
		// DoT tick
		if (StatusBurnDamagePerSecond > 0.f)
		{
			StatusBurnAccumDamage += StatusBurnDamagePerSecond * DeltaTime;
			while (StatusBurnAccumDamage >= 1.f)
			{
				StatusBurnAccumDamage -= 1.f;
				ApplyTrueDamage(1);
			}
		}
		if (StatusBurnSecondsRemaining <= 0.f)
		{
			StatusBurnDamagePerSecond = 0.f;
			StatusBurnAccumDamage = 0.f;
			bStatusChanged = true;
		}
	}

	if (StatusChillSecondsRemaining > 0.f)
	{
		StatusChillSecondsRemaining = FMath::Max(0.f, StatusChillSecondsRemaining - DeltaTime);
		if (StatusChillSecondsRemaining <= 0.f)
		{
			StatusChillMoveSpeedMultiplier = 1.f;
			HeroProgressChanged.Broadcast();
			bStatusChanged = true;
		}
	}

	if (StatusCurseSecondsRemaining > 0.f)
	{
		StatusCurseSecondsRemaining = FMath::Max(0.f, StatusCurseSecondsRemaining - DeltaTime);
		if (StatusCurseSecondsRemaining <= 0.f)
		{
			bStatusChanged = true;
		}
	}

	if (bStatusChanged)
	{
		StatusEffectsChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::EnterLastStand()
{
	bInLastStand = true;
	LastStandSecondsRemaining = LastStandDurationSeconds;
	LastBroadcastLastStandSecond = FMath::CeilToInt(LastStandSecondsRemaining);

	// Consume the charge.
	SurvivalCharge01 = 0.f;

	// Hearts stay at 0, but the run continues.
	HeartsChanged.Broadcast();
	SurvivalChanged.Broadcast();
}

void UT66RunStateSubsystem::EndLastStandAndDie()
{
	bInLastStand = false;
	LastStandSecondsRemaining = 0.f;
	LastBroadcastLastStandSecond = 0;
	SurvivalChanged.Broadcast();

	// Die as normal now.
	CurrentHearts = 0;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}

bool UT66RunStateSubsystem::ApplyDamage(int32 Hearts)
{
	if (Hearts <= 0) return false;

	// If we're in last-stand, we ignore damage (invincible).
	if (bInLastStand) return false;

	// Evasion: dodge the entire hit.
	const float Evade = GetEvasionChance01();
	if (Evade > 0.f && FMath::FRand() < Evade)
	{
		return false;
	}

	// Armor: reduce the hit size (still at least 1 if hit > 0).
	const float Armor = GetArmorReduction01();
	const int32 Reduced = FMath::Max(1, FMath::CeilToInt(static_cast<float>(Hearts) * (1.f - Armor)));

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	if (Now - LastDamageTime < InvulnDurationSeconds)
	{
		return false;
	}

	LastDamageTime = Now;
	CurrentHearts = FMath::Max(0, CurrentHearts - Reduced);

	// Survival charge fills on real damage taken.
	SurvivalCharge01 = FMath::Clamp(SurvivalCharge01 + (static_cast<float>(Reduced) * SurvivalChargePerHeart), 0.f, 1.f);
	SurvivalChanged.Broadcast();
	HeartsChanged.Broadcast();

	if (CurrentHearts <= 0)
	{
		// If charged, trigger last-stand instead of dying.
		if (SurvivalCharge01 >= 1.f)
		{
			EnterLastStand();
		}
		else
		{
			OnPlayerDied.Broadcast();
		}
	}
	return true;
}

void UT66RunStateSubsystem::AddGold(int32 Amount)
{
	if (Amount == 0) return;
	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Gambler"), Amount));
	GoldChanged.Broadcast();
	LogAdded.Broadcast();
}

bool UT66RunStateSubsystem::TrySpendGold(int32 Amount)
{
	if (Amount <= 0) return true;
	if (CurrentGold < Amount) return false;

	CurrentGold = FMath::Max(0, CurrentGold - Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=-%d,Source=Gambler"), Amount));
	GoldChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

void UT66RunStateSubsystem::AddOwedBoss(FName BossID)
{
	if (BossID.IsNone()) return;
	OwedBossIDs.Add(BossID);
	AddStructuredEvent(ET66RunEventType::StageExited, FString::Printf(TEXT("OwedBoss=%s"), *BossID.ToString()));
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::RemoveFirstOwedBoss()
{
	if (OwedBossIDs.Num() <= 0) return;
	OwedBossIDs.RemoveAt(0);
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::BorrowGold(int32 Amount)
{
	if (Amount <= 0) return;
	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	CurrentDebt = FMath::Max(0, CurrentDebt + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Borrow"), Amount));
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	LogAdded.Broadcast();
}

int32 UT66RunStateSubsystem::PayDebt(int32 Amount)
{
	if (Amount <= 0 || CurrentDebt <= 0 || CurrentGold <= 0) return 0;
	const int32 Pay = FMath::Clamp(Amount, 0, FMath::Min(CurrentDebt, CurrentGold));
	if (Pay <= 0) return 0;

	CurrentGold = FMath::Max(0, CurrentGold - Pay);
	CurrentDebt = FMath::Max(0, CurrentDebt - Pay);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=-%d,Source=PayDebt"), Pay));
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	LogAdded.Broadcast();

	// If debt is cleared, make sure a pending loan shark won't spawn.
	if (CurrentDebt <= 0)
	{
		bLoanSharkPending = false;
	}
	return Pay;
}

void UT66RunStateSubsystem::AddItem(FName ItemID)
{
	if (ItemID.IsNone()) return;
	if (Inventory.Num() >= MaxInventorySlots)
	{
		AddLogEntry(TEXT("Inventory full."));
		LogAdded.Broadcast();
		return;
	}
	Inventory.Add(ItemID);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=LootBag"), *ItemID.ToString()));
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::HealToFull()
{
	if (bInLastStand) return;
	CurrentHearts = MaxHearts;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::HealHearts(int32 Hearts)
{
	if (bInLastStand) return;
	if (Hearts <= 0) return;
	const int32 NewHearts = FMath::Clamp(CurrentHearts + Hearts, 0, MaxHearts);
	if (NewHearts == CurrentHearts) return;
	CurrentHearts = NewHearts;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::KillPlayer()
{
	// If charged, trigger last-stand instead of dying.
	if (!bInLastStand && SurvivalCharge01 >= 1.f)
	{
		EnterLastStand();
		return;
	}

	CurrentHearts = 0;
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}

bool UT66RunStateSubsystem::SellFirstItem()
{
	if (Inventory.Num() == 0) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI) return false;

	FName ItemID = Inventory[0];
	FItemData ItemData;
	if (!GI->GetItemData(ItemID, ItemData))
	{
		Inventory.RemoveAt(0);
		RecomputeItemDerivedStats();
		InventoryChanged.Broadcast();
		AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=unknown,Source=Vendor,ItemID=%s"), *ItemID.ToString()));
		LogAdded.Broadcast();
		return true;
	}

	CurrentGold += ItemData.SellValueGold;
	Inventory.RemoveAt(0);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Vendor,ItemID=%s"), ItemData.SellValueGold, *ItemID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

void UT66RunStateSubsystem::RecomputeItemDerivedStats()
{
	ItemPowerGivenPercent = 0.f;
	BonusDamagePercent = 0.f;
	BonusAttackSpeedPercent = 0.f;
	DashCooldownReductionPercent = 0.f;
	float BonusMoveSpeedPercent = 0.f;
	float BonusArmorPctPoints = 0.f;
	float BonusEvasionPctPoints = 0.f;
	int32 BonusLuckFlat = 0;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	for (const FName& ItemID : Inventory)
	{
		if (ItemID.IsNone()) continue;

		FItemData D;
		if (!GI || !GI->GetItemData(ItemID, D))
		{
			// Safe fallback: keep the old v0 feel even if the DT row is missing.
			if (ItemID == TEXT("Item_01")) { ItemPowerGivenPercent += 5.f; BonusDamagePercent += 10.f; }
			else if (ItemID == TEXT("Item_02")) { ItemPowerGivenPercent += 7.5f; BonusAttackSpeedPercent += 15.f; }
			else if (ItemID == TEXT("Item_03")) { ItemPowerGivenPercent += 10.f; DashCooldownReductionPercent += 25.f; }
			continue;
		}

		ItemPowerGivenPercent += D.PowerGivenPercent;
		switch (D.EffectType)
		{
			case ET66ItemEffectType::BonusDamagePct: BonusDamagePercent += D.EffectMagnitude; break;
			case ET66ItemEffectType::BonusAttackSpeedPct: BonusAttackSpeedPercent += D.EffectMagnitude; break;
			case ET66ItemEffectType::DashCooldownReductionPct: DashCooldownReductionPercent += D.EffectMagnitude; break;
			case ET66ItemEffectType::BonusMoveSpeedPct: BonusMoveSpeedPercent += D.EffectMagnitude; break;
			case ET66ItemEffectType::BonusArmorPctPoints: BonusArmorPctPoints += D.EffectMagnitude; break;
			case ET66ItemEffectType::BonusEvasionPctPoints: BonusEvasionPctPoints += D.EffectMagnitude; break;
			case ET66ItemEffectType::BonusLuckFlat: BonusLuckFlat += FMath::RoundToInt(D.EffectMagnitude); break;
			default: break;
		}
	}

	const float PowerMult = 1.f + (ItemPowerGivenPercent / 100.f);
	const float DamageMult = 1.f + (BonusDamagePercent / 100.f);
	const float AttackSpeedMult = 1.f + (BonusAttackSpeedPercent / 100.f);
	const float DashCdMult = 1.f - (DashCooldownReductionPercent / 100.f);

	ItemDamageMultiplier = FMath::Clamp(PowerMult * DamageMult, 0.1f, 100.f);
	ItemAttackSpeedMultiplier = FMath::Clamp(PowerMult * AttackSpeedMult, 0.1f, 100.f);
	DashCooldownMultiplier = FMath::Clamp(DashCdMult, 0.05f, 1.f);
	ItemScaleMultiplier = FMath::Clamp(PowerMult, 0.1f, 10.f);

	ItemMoveSpeedMultiplier = FMath::Clamp(1.f + (BonusMoveSpeedPercent / 100.f), 0.1f, 10.f);
	ItemArmorBonus01 = FMath::Clamp(BonusArmorPctPoints / 100.f, 0.f, 0.80f);
	ItemEvasionBonus01 = FMath::Clamp(BonusEvasionPctPoints / 100.f, 0.f, 0.60f);
	ItemBonusLuckFlat = FMath::Clamp(BonusLuckFlat, 0, 9999);
}

void UT66RunStateSubsystem::ResetForNewRun()
{
	MaxHearts = DefaultMaxHearts;
	CurrentHearts = MaxHearts;
	CurrentGold = DefaultStartGold;
	CurrentDebt = 0;
	bLoanSharkPending = false;
	DifficultyTier = 0;
	DifficultySkulls = 0.f;
	TotemsActivatedCount = 0;
	GamblerAnger01 = 0.f;
	ResetVendorForStage();
	OwedBossIDs.Empty();
	Inventory.Empty();
	RecomputeItemDerivedStats();
	EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	EventLog.Empty();
	StructuredEventLog.Empty();
	CurrentStage = 1;
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	SpeedRunElapsedSeconds = 0.f;
	SpeedRunStartWorldSeconds = 0.f;
	bSpeedRunStarted = false;
	LastBroadcastSpeedRunSecond = -1;
	CurrentScore = 0;
	LastDamageTime = -9999.f;
	bHUDPanelsVisible = true;

	// Clear transient stage/status effects at run start.
	StageMoveSpeedMultiplier = 1.f;
	StageMoveSpeedSecondsRemaining = 0.f;
	StatusBurnSecondsRemaining = 0.f;
	StatusBurnDamagePerSecond = 0.f;
	StatusBurnAccumDamage = 0.f;
	StatusChillSecondsRemaining = 0.f;
	StatusChillMoveSpeedMultiplier = 1.f;
	StatusCurseSecondsRemaining = 0.f;
	HeroLevel = DefaultHeroLevel;
	// Difficulty start boosts: each difficulty step grants +10 levels.
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		const int32 DiffIndex = static_cast<int32>(T66GI->SelectedDifficulty);
		const int32 Steps = FMath::Clamp(DiffIndex, 0, 999);
		HeroLevel = FMath::Clamp(DefaultHeroLevel + (Steps * 10), 1, 9999);
		bInStageBoost = T66GI->bStageBoostPending;
	}
	HeroXP = 0;
	XPToNextLevel = DefaultXPToLevel;
	UltimateCooldownRemainingSeconds = 0.f;
	LastBroadcastUltimateSecond = 0;
	SurvivalCharge01 = 0.f;
	bInLastStand = false;
	LastStandSecondsRemaining = 0.f;
	LastBroadcastLastStandSecond = 0;
	ResetBossState();
	HeartsChanged.Broadcast();
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	DifficultyChanged.Broadcast();
	GamblerAngerChanged.Broadcast();
	VendorChanged.Broadcast();
	InventoryChanged.Broadcast();
	IdolsChanged.Broadcast();
	PanelVisibilityChanged.Broadcast();
	ScoreChanged.Broadcast();
	StageTimerChanged.Broadcast();
	SpeedRunTimerChanged.Broadcast();
	BossChanged.Broadcast();
	HeroProgressChanged.Broadcast();
	UltimateChanged.Broadcast();
	SurvivalChanged.Broadcast();
	StatusEffectsChanged.Broadcast();
}

void UT66RunStateSubsystem::SetCurrentStage(int32 Stage)
{
	const int32 NewStage = FMath::Clamp(Stage, 1, 66);
	if (CurrentStage == NewStage) return;

	// If Speed Run Mode is enabled, record the stage completion time for the stage we're leaving.
	// This is used for the main menu Speed Run leaderboard (stage 1..10).
	{
		UGameInstance* GI = GetGameInstance();
		UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
		const bool bSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;
		if (bSpeedRunMode)
		{
			const int32 CompletedStage = CurrentStage;
			if (CompletedStage >= 1 && CompletedStage <= 10)
			{
				UWorld* World = GetWorld();
				const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
				const float Elapsed = (bSpeedRunStarted && SpeedRunStartWorldSeconds > 0.f) ? FMath::Max(0.f, Now - SpeedRunStartWorldSeconds) : SpeedRunElapsedSeconds;
				if (UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr)
				{
					LB->SubmitStageSpeedRunTime(CompletedStage, Elapsed);
				}
			}
		}
	}

	CurrentStage = NewStage;

	// New stage: clear transient movement/status effects so the Start Area is clean.
	StageMoveSpeedMultiplier = 1.f;
	StageMoveSpeedSecondsRemaining = 0.f;
	StatusBurnSecondsRemaining = 0.f;
	StatusBurnDamagePerSecond = 0.f;
	StatusBurnAccumDamage = 0.f;
	StatusChillSecondsRemaining = 0.f;
	StatusChillMoveSpeedMultiplier = 1.f;
	StatusCurseSecondsRemaining = 0.f;
	StatusEffectsChanged.Broadcast();
	HeroProgressChanged.Broadcast();
	// Bible: gambler anger resets at end of every stage.
	ResetGamblerAnger();
	ResetVendorForStage();
	StageChanged.Broadcast();
}

void UT66RunStateSubsystem::SetStageTimerActive(bool bActive)
{
	if (bStageTimerActive == bActive) return;
	bStageTimerActive = bActive;

	// Start (or stop) the speedrun timer alongside the stage timer.
	// User-requested behavior: speedrun time starts after leaving the start area (start gate), not at spawn.
	if (bStageTimerActive)
	{
		if (UWorld* World = GetWorld())
		{
			SpeedRunStartWorldSeconds = static_cast<float>(World->GetTimeSeconds());
			bSpeedRunStarted = true;
			SpeedRunElapsedSeconds = 0.f;
			LastBroadcastSpeedRunSecond = -1;
			SpeedRunTimerChanged.Broadcast();
		}
	}
	else
	{
		bSpeedRunStarted = false;
	}

	StageTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::TickStageTimer(float DeltaTime)
{
	if (!bStageTimerActive || StageTimerSecondsRemaining <= 0.f) return;
	StageTimerSecondsRemaining = FMath::Max(0.f, StageTimerSecondsRemaining - DeltaTime);
	const int32 CurrentSecond = FMath::FloorToInt(StageTimerSecondsRemaining);
	if (CurrentSecond != LastBroadcastStageTimerSecond)
	{
		LastBroadcastStageTimerSecond = CurrentSecond;
		StageTimerChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::TickSpeedRunTimer(float DeltaTime)
{
	// Speedrun time begins when the stage timer becomes active (start gate) and resets each stage.
	if (!bStageTimerActive || !bSpeedRunStarted) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	SpeedRunElapsedSeconds = FMath::Max(0.f, Now - SpeedRunStartWorldSeconds);

	const int32 CurrentSecond = FMath::FloorToInt(SpeedRunElapsedSeconds);
	if (CurrentSecond != LastBroadcastSpeedRunSecond)
	{
		LastBroadcastSpeedRunSecond = CurrentSecond;
		SpeedRunTimerChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::ResetStageTimerToFull()
{
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	StageTimerChanged.Broadcast();

	// New stage start area: reset speedrun timer to 0 until the next start gate triggers it again.
	SpeedRunElapsedSeconds = 0.f;
	SpeedRunStartWorldSeconds = 0.f;
	bSpeedRunStarted = false;
	LastBroadcastSpeedRunSecond = -1;
	SpeedRunTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::SetBossActive(int32 InMaxHP)
{
	bBossActive = true;
	BossMaxHP = FMath::Max(1, InMaxHP);
	BossCurrentHP = BossMaxHP;
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::SetBossInactive()
{
	bBossActive = false;
	BossCurrentHP = 0;
	BossChanged.Broadcast();
}

bool UT66RunStateSubsystem::ApplyBossDamage(int32 Damage)
{
	if (!bBossActive || Damage <= 0 || BossCurrentHP <= 0) return false;
	BossCurrentHP = FMath::Max(0, BossCurrentHP - Damage);
	BossChanged.Broadcast();
	return BossCurrentHP <= 0;
}

void UT66RunStateSubsystem::ResetBossState()
{
	bBossActive = false;
	BossMaxHP = 100;
	BossCurrentHP = 0;
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::AddScore(int32 Points)
{
	if (Points <= 0) return;
	CurrentScore += Points;
	ScoreChanged.Broadcast();
}

void UT66RunStateSubsystem::AddStructuredEvent(ET66RunEventType EventType, const FString& Payload)
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Timestamp = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	StructuredEventLog.Add(FRunEvent(EventType, Timestamp, Payload));
	// Append human-readable line to EventLog for Run Summary scroll
	FString Short = Payload;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	switch (EventType)
	{
		case ET66RunEventType::StageEntered:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_StageFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Stage %s"), *Payload);
			break;
		case ET66RunEventType::ItemAcquired:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_PickedUpFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Picked up %s"), *Payload);
			break;
		case ET66RunEventType::GoldGained:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_GoldFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Gold: %s"), *Payload);
			break;
		case ET66RunEventType::EnemyKilled:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_KillFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Kill +%s"), *Payload);
			break;
		default:
			break;
	}
	EventLog.Add(Short);
	TrimLogsIfNeeded();
}

void UT66RunStateSubsystem::ToggleHUDPanels()
{
	bHUDPanelsVisible = !bHUDPanelsVisible;
	PanelVisibilityChanged.Broadcast();
}

void UT66RunStateSubsystem::AddLogEntry(const FString& Entry)
{
	EventLog.Add(Entry);
	TrimLogsIfNeeded();
}
