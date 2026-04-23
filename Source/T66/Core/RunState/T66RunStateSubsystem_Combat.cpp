// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

int32 UT66RunStateSubsystem::GetHeartDisplayTier() const
{
	int32 HighestTier = 0;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		HighestTier = FMath::Max(HighestTier, GetHeartSlotTier(SlotIndex));
	}
	return HighestTier;
}


int32 UT66RunStateSubsystem::GetHeartSlotTier(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0;
	}

	const int32 StoredTier = HeartSlotTiers.IsValidIndex(SlotIndex)
		? static_cast<int32>(HeartSlotTiers[SlotIndex])
		: 0;
	return FMath::Clamp(StoredTier, 0, MaxHeartTier);
}


float UT66RunStateSubsystem::GetHPForHeartTier(const int32 Tier)
{
	return HPPerRedHeart * FMath::Pow(HeartTierScale, static_cast<float>(FMath::Clamp(Tier, 0, MaxHeartTier)));
}


float UT66RunStateSubsystem::GetHeartSlotCapacity(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0.f;
	}

	return GetHPForHeartTier(GetHeartSlotTier(SlotIndex));
}


float UT66RunStateSubsystem::GetTotalHeartCapacity() const
{
	float TotalCapacity = 0.f;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		TotalCapacity += GetHeartSlotCapacity(SlotIndex);
	}
	return TotalCapacity;
}


void UT66RunStateSubsystem::ResetHeartSlotTiers()
{
	HeartSlotTiers.Init(0, DefaultMaxHearts);
}


void UT66RunStateSubsystem::SyncMaxHPToHeartTiers()
{
	MaxHP = FMath::Max(DefaultMaxHP, GetTotalHeartCapacity());
}


int32 UT66RunStateSubsystem::FindUpgradeableHeartSlot() const
{
	int32 BestSlot = INDEX_NONE;
	int32 LowestTier = MaxHeartTier + 1;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		const int32 Tier = GetHeartSlotTier(SlotIndex);
		if (Tier >= MaxHeartTier)
		{
			continue;
		}

		if (Tier < LowestTier)
		{
			LowestTier = Tier;
			BestSlot = SlotIndex;
		}
	}

	return BestSlot;
}


void UT66RunStateSubsystem::RebuildHeartSlotTiersFromMaxHP()
{
	ResetHeartSlotTiers();

	MaxHP = FMath::Max(DefaultMaxHP, MaxHP);
	float RemainingExtraHP = MaxHP - DefaultMaxHP;
	while (RemainingExtraHP > KINDA_SMALL_NUMBER)
	{
		const int32 SlotIndex = FindUpgradeableHeartSlot();
		if (SlotIndex == INDEX_NONE)
		{
			break;
		}

		const int32 CurrentTier = GetHeartSlotTier(SlotIndex);
		const float UpgradeDelta = GetHPForHeartTier(CurrentTier + 1) - GetHPForHeartTier(CurrentTier);
		if (RemainingExtraHP + 0.01f < UpgradeDelta)
		{
			break;
		}

		HeartSlotTiers[SlotIndex] = static_cast<uint8>(CurrentTier + 1);
		RemainingExtraHP -= UpgradeDelta;
	}

	SyncMaxHPToHeartTiers();
	CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);
}


float UT66RunStateSubsystem::GetHeartSlotFill(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0.f;
	}

	float SlotStart = 0.f;
	for (int32 Index = 0; Index < SlotIndex; ++Index)
	{
		SlotStart += GetHeartSlotCapacity(Index);
	}

	const float SlotCapacity = GetHeartSlotCapacity(SlotIndex);
	if (SlotCapacity <= 0.f)
	{
		return 0.f;
	}

	const float SlotEnd = SlotStart + SlotCapacity;
	if (CurrentHP <= SlotStart) return 0.f;
	if (CurrentHP >= SlotEnd) return 1.f;
	return (CurrentHP - SlotStart) / SlotCapacity;
}


void UT66RunStateSubsystem::AddMaxHearts(int32 DeltaHearts)
{
	if (DeltaHearts <= 0)
	{
		return;
	}

	if (HeartSlotTiers.Num() != DefaultMaxHearts)
	{
		RebuildHeartSlotTiersFromMaxHP();
	}

	const float PreviousMaxHP = MaxHP;
	bool bUpgraded = false;
	for (int32 UpgradeIndex = 0; UpgradeIndex < DeltaHearts; ++UpgradeIndex)
	{
		const int32 SlotIndex = FindUpgradeableHeartSlot();
		if (SlotIndex == INDEX_NONE)
		{
			break;
		}

		HeartSlotTiers[SlotIndex] = static_cast<uint8>(GetHeartSlotTier(SlotIndex) + 1);
		bUpgraded = true;
	}

	if (!bUpgraded)
	{
		return;
	}

	SyncMaxHPToHeartTiers();
	CurrentHP = FMath::Clamp(CurrentHP + (MaxHP - PreviousMaxHP), 0.f, MaxHP);
	HeartsChanged.Broadcast();
}


void UT66RunStateSubsystem::NotifyEnemyKilledByHero()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;
	const double Now = World->GetTimeSeconds();

	if (PassiveType == ET66PassiveType::RallyingBlow)
	{
		RallyStacks = FMath::Min(3, RallyStacks + 1);
		RallyTimerEndWorldTime = Now + 3.0;
	}

	if (PassiveType == ET66PassiveType::ChaosTheory)
	{
		ChaosTheoryBounceStacks = FMath::Min(3, ChaosTheoryBounceStacks + 1);
		ChaosTheoryTimerEndWorldTime = Now + 5.0;
	}
}


float UT66RunStateSubsystem::GetRallyAttackSpeedMultiplier() const
{
	if (PassiveType != ET66PassiveType::RallyingBlow || RallyStacks <= 0) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World || World->GetTimeSeconds() >= RallyTimerEndWorldTime) return 1.f;
	return 1.f + 0.15f * static_cast<float>(RallyStacks);
}


float UT66RunStateSubsystem::GetQuickDrawDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::QuickDraw) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return 1.f;
	const double Now = World->GetTimeSeconds();
	return (Now - LastAttackFireWorldTime >= 2.0) ? 2.f : 1.f;
}


void UT66RunStateSubsystem::NotifyAttackFired()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;
	LastAttackFireWorldTime = World->GetTimeSeconds();
	if (PassiveType == ET66PassiveType::Overclock)
		OverclockAttackCounter++;
}


bool UT66RunStateSubsystem::ShouldOverclockDouble() const
{
	if (PassiveType != ET66PassiveType::Overclock) return false;
	return (OverclockAttackCounter % 8) == 0 && OverclockAttackCounter > 0;
}


int32 UT66RunStateSubsystem::GetChaosTheoryBonusBounceCount() const
{
	if (PassiveType != ET66PassiveType::ChaosTheory || ChaosTheoryBounceStacks <= 0) return 0;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World || World->GetTimeSeconds() >= ChaosTheoryTimerEndWorldTime) return 0;
	return ChaosTheoryBounceStacks;
}


float UT66RunStateSubsystem::GetEnduranceAttackSpeedMultiplier() const
{
	if (PassiveType != ET66PassiveType::Endurance) return 1.f;
	return (CurrentHP > 0.f && CurrentHP <= MaxHP * 0.3f) ? 2.f : 1.f;
}


float UT66RunStateSubsystem::GetEnduranceDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::Endurance) return 1.f;
	return (CurrentHP > 0.f && CurrentHP <= MaxHP * 0.3f) ? 1.25f : 1.f;
}


float UT66RunStateSubsystem::GetBrawlersFuryDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::BrawlersFury) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return 1.f;
	return (World->GetTimeSeconds() < BrawlersFuryEndWorldTime) ? 1.3f : 1.f;
}


float UT66RunStateSubsystem::GetTreasureHunterGoldMultiplier() const
{
	return (PassiveType == ET66PassiveType::TreasureHunter) ? 1.25f : 1.f;
}


void UT66RunStateSubsystem::NotifyEvasionProc()
{
	if (PassiveType == ET66PassiveType::Evasive)
		bEvasiveNextAttackBonusDOT = true;
}


bool UT66RunStateSubsystem::ConsumeEvasiveBonusDOT()
{
	if (PassiveType != ET66PassiveType::Evasive || !bEvasiveNextAttackBonusDOT) return false;
	bEvasiveNextAttackBonusDOT = false;
	return true;
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


bool UT66RunStateSubsystem::ApplyTrueDamage(int32 /*DamageHP*/)
{
	return false;
}


void UT66RunStateSubsystem::ApplyStatusBurn(float /*DurationSeconds*/, float /*DamagePerSecond*/) {}


void UT66RunStateSubsystem::ApplyStatusChill(float /*DurationSeconds*/, float /*MoveSpeedMultiplier*/) {}


void UT66RunStateSubsystem::ApplyStatusCurse(float /*DurationSeconds*/) {}


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
	// HP regen (numerical)
	ApplyHpRegen(DeltaTime);

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

	if (bInQuickReviveDowned && QuickReviveDownedSecondsRemaining > 0.f)
	{
		QuickReviveDownedSecondsRemaining = FMath::Max(0.f, QuickReviveDownedSecondsRemaining - DeltaTime);
		const int32 Cur = FMath::CeilToInt(QuickReviveDownedSecondsRemaining);
		if (Cur != LastBroadcastQuickReviveSecond)
		{
			LastBroadcastQuickReviveSecond = Cur;
			QuickReviveChanged.Broadcast();
		}
		if (QuickReviveDownedSecondsRemaining <= 0.f)
		{
			EndQuickReviveDownedAndRevive();
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

	// Status effects removed — enemies no longer apply Burn/Chill/Curse.
}


bool UT66RunStateSubsystem::GrantQuickReviveCharge()
{
	if (bQuickReviveChargeReady || bInQuickReviveDowned)
	{
		return false;
	}

	bQuickReviveChargeReady = true;
	QuickReviveChanged.Broadcast();
	return true;
}


void UT66RunStateSubsystem::ClearQuickReviveCharge()
{
	if (!bQuickReviveChargeReady)
	{
		return;
	}

	bQuickReviveChargeReady = false;
	QuickReviveChanged.Broadcast();
}


bool UT66RunStateSubsystem::IsBossDamageSource(const AActor* Attacker)
{
	return Cast<AT66BossBase>(Attacker) != nullptr;
}


void UT66RunStateSubsystem::HandleLethalDamage(AActor* Attacker)
{
	if (bInQuickReviveDowned)
	{
		EndQuickReviveDownedAndDie();
		return;
	}

	if (bInLastStand)
	{
		EndLastStandAndDie();
		return;
	}

	// Dev Immortality: never end the run.
	if (bDevImmortality)
	{
		CurrentHP = 0.f;
		LastDamageTime = -9999.f;
		HeartsChanged.Broadcast();
		return;
	}

	if (bQuickReviveChargeReady)
	{
		EnterQuickReviveDowned();
		return;
	}

	// If charged, trigger last-stand instead of dying.
	if (SurvivalCharge01 >= 1.f)
	{
		EnterLastStand();
		return;
	}

	CurrentHP = 0.f;
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}


void UT66RunStateSubsystem::EnterLastStand()
{
	bInLastStand = true;
	LastStandSecondsRemaining = LastStandDurationSeconds;
	LastBroadcastLastStandSecond = FMath::CeilToInt(LastStandSecondsRemaining);

	// Consume the charge.
	SurvivalCharge01 = 0.f;

	// HP stays at 0, but the run continues.
	HeartsChanged.Broadcast();
	SurvivalChanged.Broadcast();
}


void UT66RunStateSubsystem::EnterQuickReviveDowned()
{
	bQuickReviveChargeReady = false;
	bInQuickReviveDowned = true;
	QuickReviveDownedSecondsRemaining = QuickReviveDownedDurationSeconds;
	LastBroadcastQuickReviveSecond = FMath::CeilToInt(QuickReviveDownedSecondsRemaining);
	CurrentHP = 0.f;
	LastDamageTime = -9999.f;

	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
			{
				if (AT66EnemyBase* Enemy = WeakEnemy.Get())
				{
					Enemy->ApplyForcedRunAway(QuickReviveDownedDurationSeconds);
				}
			}
		}
	}

	HeartsChanged.Broadcast();
	QuickReviveChanged.Broadcast();
}


void UT66RunStateSubsystem::EndQuickReviveDownedAndRevive()
{
	if (!bInQuickReviveDowned)
	{
		return;
	}

	bInQuickReviveDowned = false;
	QuickReviveDownedSecondsRemaining = 0.f;
	LastBroadcastQuickReviveSecond = 0;

	if (HeartSlotTiers.Num() != DefaultMaxHearts)
	{
		RebuildHeartSlotTiersFromMaxHP();
	}

	CurrentHP = FMath::Clamp(GetHeartSlotCapacity(0), 1.f, MaxHP);

	if (UWorld* World = GetWorld())
	{
		LastDamageTime = static_cast<float>(World->GetTimeSeconds());
	}
	else
	{
		LastDamageTime = 0.f;
	}

	HeartsChanged.Broadcast();
	QuickReviveChanged.Broadcast();
}


void UT66RunStateSubsystem::EndQuickReviveDownedAndDie()
{
	if (!bInQuickReviveDowned)
	{
		return;
	}

	bInQuickReviveDowned = false;
	QuickReviveDownedSecondsRemaining = 0.f;
	LastBroadcastQuickReviveSecond = 0;
	QuickReviveChanged.Broadcast();

	if (bDevImmortality)
	{
		CurrentHP = 0.f;
		HeartsChanged.Broadcast();
		return;
	}

	CurrentHP = 0.f;
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}


void UT66RunStateSubsystem::EndLastStandAndDie()
{
	bInLastStand = false;
	LastStandSecondsRemaining = 0.f;
	LastBroadcastLastStandSecond = 0;
	SurvivalChanged.Broadcast();

	// Dev Immortality: never end the run.
	if (bDevImmortality)
	{
		CurrentHP = 0.f;
		HeartsChanged.Broadcast();
		return;
	}

	// Die as normal now.
	CurrentHP = 0.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}


bool UT66RunStateSubsystem::ApplyDamage(int32 DamageHP, AActor* Attacker)
{
	if (DamageHP <= 0) return false;

	if (bSaintBlessingActive)
	{
		return false;
	}

	// If we're in last-stand, we ignore damage (invincible).
	if (bInLastStand) return false;

	// Quick Revive downed state only ends if a boss lands the finishing hit.
	if (bInQuickReviveDowned)
	{
		if (IsBossDamageSource(Attacker))
		{
			EndQuickReviveDownedAndDie();
			return true;
		}
		return false;
	}

	// Iron Will: flat damage reduction before armor.
	if (PassiveType == ET66PassiveType::IronWill)
	{
		const int32 FlatReduction = GetArmorStat() * 2;
		DamageHP = FMath::Max(1, DamageHP - FlatReduction);
	}

	// Unflinching: permanent 15% damage reduction.
	if (PassiveType == ET66PassiveType::Unflinching)
	{
		DamageHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageHP) * 0.85f));
	}

	// Evasion: dodge the entire hit. On dodge: Assassinate (OHKO) and CounterAttack (deal fraction of would-be damage to attacker).
	UT66RngSubsystem* RngSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
	const float Evade = GetEvasionChance01();
	AntiCheatIncomingHitChecks = FMath::Clamp(AntiCheatIncomingHitChecks + 1, 0, 1000000);
	AntiCheatTotalEvasionChance = FMath::Clamp(AntiCheatTotalEvasionChance + FMath::Clamp(Evade, 0.f, 1.f), 0.f, 1000000.f);
	const bool bDodged = Evade > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < Evade) : (FMath::FRand() < Evade));
	if (bDodged)
	{
		RecordAntiCheatHitCheckEvent(Evade, true, false);
		AntiCheatDodgeCount = FMath::Clamp(AntiCheatDodgeCount + 1, 0, 1000000);
		AntiCheatCurrentConsecutiveDodges = FMath::Clamp(AntiCheatCurrentConsecutiveDodges + 1, 0, 1000000);
		AntiCheatMaxConsecutiveDodges = FMath::Max(AntiCheatMaxConsecutiveDodges, AntiCheatCurrentConsecutiveDodges);
		UWorld* DodgeWorld = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
		if (DodgeWorld)
		{
			if (APawn* HeroPawn = DodgeWorld->GetFirstPlayerController() ? DodgeWorld->GetFirstPlayerController()->GetPawn() : nullptr)
			{
				if (UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
				{
					FloatingText->ShowStatusEvent(HeroPawn, UT66FloatingCombatTextSubsystem::EventType_Dodge);
				}
			}
		}
		if (Attacker)
		{
			const float AssassinateChance = GetAssassinateChance01();
			if (AssassinateChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < AssassinateChance) : (FMath::FRand() < AssassinateChance)))
			{
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker)) { E->TakeDamageFromHero(99999, FName(TEXT("Assassinate")), NAME_None); }
				else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker)) { B->TakeDamageFromHeroHit(99999, FName(TEXT("Assassinate")), NAME_None); }
			}
			const float CounterChance = FMath::Clamp(GetCounterAttackFraction(), 0.f, 1.f);
			if (CounterChance > 0.f && DamageHP > 0 && (RngSub ? (RngSub->GetRunStream().GetFraction() < CounterChance) : (FMath::FRand() < CounterChance)))
			{
				const int32 CounterDmg = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageHP) * 0.5f));
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker)) { if (E->CurrentHP > 0) E->TakeDamageFromHero(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				else if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(Attacker)) { if (GB->CurrentHP > 0) GB->TakeDamageFromHeroHit(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				else if (AT66VendorBoss* VB = Cast<AT66VendorBoss>(Attacker)) { if (VB->CurrentHP > 0) VB->TakeDamageFromHeroHit(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker)) { if (B->IsAwakened() && B->IsAlive()) B->TakeDamageFromHeroHit(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				if (UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
				{
					FloatingText->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_CounterAttack);
				}
			}
		}
		NotifyEvasionProc();
		return false;
	}
	// Armor: reduce the hit (still at least 1 HP if hit > 0).
	const float Armor = GetArmorReduction01();
	const float Reduced = static_cast<float>(FMath::Max(1, FMath::CeilToInt(static_cast<float>(DamageHP) * (1.f - Armor))));

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	if (Now - LastDamageTime < InvulnDurationSeconds)
	{
		// Any resolved non-dodge hit check breaks the consecutive dodge streak,
		// even when invulnerability prevents damage from being applied.
		AntiCheatCurrentConsecutiveDodges = 0;
		RecordAntiCheatHitCheckEvent(Evade, false, false);
		return false;
	}

	// Reflect: % chance to reflect; when it procs, 50% of reduced damage back to the attacker. Crush: chance to OHKO when reflect fires.
	if (Attacker && Reduced > 0.f)
	{
		const float ReflectChance = FMath::Clamp(GetReflectDamageFraction(), 0.f, 1.f);
		if (ReflectChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < ReflectChance) : (FMath::FRand() < ReflectChance)))
		{
			const int32 ReflectedAmount = FMath::Max(1, FMath::RoundToInt(Reduced * 0.5f));
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker))
			{
				if (E->CurrentHP > 0) E->TakeDamageFromHero(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			else if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(Attacker))
			{
				if (GB->CurrentHP > 0) GB->TakeDamageFromHeroHit(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			else if (AT66VendorBoss* VB = Cast<AT66VendorBoss>(Attacker))
			{
				if (VB->CurrentHP > 0) VB->TakeDamageFromHeroHit(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker))
			{
				if (B->IsAwakened() && B->IsAlive()) B->TakeDamageFromHeroHit(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			if (UGameInstance* ReflectGI = GetGameInstance())
			{
				if (UT66FloatingCombatTextSubsystem* FloatingText = ReflectGI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
				{
					FloatingText->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_Reflect);
				}
			}

			const float CrushChance = GetCrushChance01();
			if (CrushChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < CrushChance) : (FMath::FRand() < CrushChance)))
			{
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker)) { E->TakeDamageFromHero(99999, FName(TEXT("Crush")), NAME_None); }
				else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker)) { B->TakeDamageFromHeroHit(99999, FName(TEXT("Crush")), NAME_None); }
				if (UGameInstance* CrushGI = GetGameInstance())
				{
						if (UT66FloatingCombatTextSubsystem* CrushFloating = CrushGI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
					{
						CrushFloating->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_Crush);
					}
				}
			}
		}
	}

	LastDamageTime = Now;
	CurrentHP = FMath::Max(0.f, CurrentHP - Reduced);
	AntiCheatDamageTakenHitCount = FMath::Clamp(AntiCheatDamageTakenHitCount + 1, 0, 1000000);
	AntiCheatCurrentConsecutiveDodges = 0;
	RecordAntiCheatHitCheckEvent(Evade, false, true);

	// BrawlersFury: taking damage triggers +30% damage dealt for 3s.
	if (PassiveType == ET66PassiveType::BrawlersFury && World)
	{
		BrawlersFuryEndWorldTime = World->GetTimeSeconds() + 3.0;
	}

	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* HeroPawn = PC->GetPawn())
			{
				if (UT66FloatingCombatTextSubsystem* FCT = GI ? GI->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
				{
					FCT->ShowDamageTaken(HeroPawn, FMath::RoundToInt(Reduced));
				}
			}
		}
	}

	// Survival charge: 100 HP damage = full (5 hearts * 20 HP). So 1 HP = 0.01 charge.
	SurvivalCharge01 = FMath::Clamp(SurvivalCharge01 + (Reduced / DefaultMaxHP), 0.f, 1.f);
	SurvivalChanged.Broadcast();
	HeartsChanged.Broadcast();

	if (CurrentHP <= 0.f)
	{
		HandleLethalDamage(Attacker);
	}
	return true;
}


void UT66RunStateSubsystem::HealToFull()
{
	if (bInLastStand || bInQuickReviveDowned) return;
	CurrentHP = MaxHP;
	HeartsChanged.Broadcast();
}


void UT66RunStateSubsystem::HealHP(float Amount)
{
	if (bInLastStand || bInQuickReviveDowned) return;
	if (Amount <= 0.f) return;

	const float NewHP = FMath::Clamp(CurrentHP + Amount, 0.f, MaxHP);
	if (FMath::IsNearlyEqual(NewHP, CurrentHP)) return;
	CurrentHP = NewHP;
	HeartsChanged.Broadcast();
}


void UT66RunStateSubsystem::HealHPFromCompanion(float Amount)
{
	if (bInLastStand || bInQuickReviveDowned) return;
	if (Amount <= 0.f) return;

	const float PreviousHP = CurrentHP;
	HealHP(Amount);
	const float AppliedHealing = FMath::Max(0.f, CurrentHP - PreviousHP);
	if (AppliedHealing > 0.f)
	{
		CompanionHealingDoneThisRun += AppliedHealing;
	}
}


void UT66RunStateSubsystem::HealHearts(int32 Hearts)
{
	if (Hearts <= 0) return;
	HealHP(static_cast<float>(Hearts) * HPPerRedHeart);
}


void UT66RunStateSubsystem::ApplyHpRegen(float DeltaTime)
{
	if (bInLastStand || bInQuickReviveDowned) return;
	const float Rate = GetHpRegenPerSecond();
	if (Rate <= 0.f || DeltaTime <= 0.f) return;

	const float Healed = Rate * DeltaTime;
	HealHP(Healed);
}


void UT66RunStateSubsystem::KillPlayer()
{
	CurrentHP = 0.f;
	HandleLethalDamage(nullptr);
}
