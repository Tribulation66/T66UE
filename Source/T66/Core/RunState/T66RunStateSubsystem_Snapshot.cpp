// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

void UT66RunStateSubsystem::ExportSavedRunSnapshot(FT66SavedRunSnapshot& OutSnapshot) const
{
	OutSnapshot = FT66SavedRunSnapshot{};
	OutSnapshot.bValid = true;
	OutSnapshot.CurrentStage = CurrentStage;
	OutSnapshot.CurrentHP = CurrentHP;
	OutSnapshot.MaxHP = MaxHP;
	OutSnapshot.HeartSlotTiers.Reset();
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		OutSnapshot.HeartSlotTiers.Add(static_cast<uint8>(GetHeartSlotTier(SlotIndex)));
	}
	OutSnapshot.CurrentGold = CurrentGold;
	OutSnapshot.CurrentDebt = CurrentDebt;
	OutSnapshot.DifficultyTier = DifficultyTier;
	OutSnapshot.DifficultySkulls = DifficultySkulls;
	OutSnapshot.TotemsActivatedCount = TotemsActivatedCount;
	OutSnapshot.GamblerAnger01 = GamblerAnger01;
	OutSnapshot.OwedBossIDs = OwedBossIDs;
	OutSnapshot.CowardiceGatesTakenCount = CowardiceGatesTakenCount;
	OutSnapshot.InventorySlots = InventorySlots;
	OutSnapshot.ActiveGamblersTokenLevel = ActiveGamblersTokenLevel;
	OutSnapshot.EventLog = EventLog;
	OutSnapshot.StructuredEventLog = StructuredEventLog;
	OutSnapshot.StagePacingPoints = StagePacingPoints;
	OutSnapshot.bStageTimerActive = bStageTimerActive;
	OutSnapshot.StageTimerSecondsRemaining = StageTimerSecondsRemaining;
	OutSnapshot.SpeedRunElapsedSeconds = bSpeedRunStarted && GetWorld()
		? FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunStartWorldSeconds)
		: SpeedRunElapsedSeconds;
	OutSnapshot.bSpeedRunStarted = bSpeedRunStarted;
	OutSnapshot.CompletedStageActiveSeconds = CompletedStageActiveSeconds;
	OutSnapshot.FinalRunElapsedSeconds = FinalRunElapsedSeconds;
	OutSnapshot.bRunEnded = bRunEnded;
	OutSnapshot.bRunEndedAsVictory = bRunEndedAsVictory;
	OutSnapshot.CurrentScore = CurrentScore;
	OutSnapshot.ScoreBudgetContext = ScoreBudgetContext;
	OutSnapshot.HeroLevel = HeroLevel;
	OutSnapshot.HeroXP = HeroXP;
	OutSnapshot.XPToNextLevel = XPToNextLevel;
	OutSnapshot.HeroPreciseStats = HeroPreciseStats;
	OutSnapshot.HeroStatRngCurrentSeed = HeroStatRng.GetCurrentSeed();
	OutSnapshot.PersistentSecondaryStatBonusEntries.Reset();
	for (const TPair<ET66SecondaryStatType, int32>& Pair : PersistentSecondaryStatBonusTenths)
	{
		if (Pair.Key == ET66SecondaryStatType::None || Pair.Value <= 0)
		{
			continue;
		}

		FT66SavedSecondaryStatBonusEntry& SavedBonus = OutSnapshot.PersistentSecondaryStatBonusEntries.AddDefaulted_GetRef();
		SavedBonus.StatType = Pair.Key;
		SavedBonus.BonusTenths = Pair.Value;
	}
	OutSnapshot.HeroStats = HeroPreciseStats.ToDisplayStatBlock();
	OutSnapshot.PowerCrystalsEarnedThisRun = PowerCrystalsEarnedThisRun;
	OutSnapshot.PowerCrystalsGrantedToWalletThisRun = PowerCrystalsGrantedToWalletThisRun;
	OutSnapshot.SeedLuck0To100 = GetSeedLuck0To100();
	OutSnapshot.LuckModifierPercent = GetTotalLuckModifierPercent();
	OutSnapshot.EffectiveLuck = GetEffectiveLuckValue();
	OutSnapshot.CompanionHealingDoneThisRun = CompanionHealingDoneThisRun;
	OutSnapshot.bBossActive = bBossActive;
	OutSnapshot.ActiveBossID = ActiveBossID;
	OutSnapshot.BossMaxHP = BossMaxHP;
	OutSnapshot.BossCurrentHP = BossCurrentHP;
	OutSnapshot.BossParts = BossPartSnapshots;
	OutSnapshot.bPendingDifficultyClearSummary = bPendingDifficultyClearSummary;
	OutSnapshot.bSaintBlessingActive = bSaintBlessingActive;
	OutSnapshot.FinalSurvivalEnemyScalar = FinalSurvivalEnemyScalar;
	OutSnapshot.AntiCheatIncomingHitChecks = AntiCheatIncomingHitChecks;
	OutSnapshot.AntiCheatDamageTakenHitCount = AntiCheatDamageTakenHitCount;
	OutSnapshot.AntiCheatDodgeCount = AntiCheatDodgeCount;
	OutSnapshot.AntiCheatCurrentConsecutiveDodges = AntiCheatCurrentConsecutiveDodges;
	OutSnapshot.AntiCheatMaxConsecutiveDodges = AntiCheatMaxConsecutiveDodges;
	OutSnapshot.AntiCheatTotalEvasionChance = AntiCheatTotalEvasionChance;
	OutSnapshot.AntiCheatEvasionBuckets = AntiCheatEvasionBuckets;
	OutSnapshot.AntiCheatPressureWindowSummary = AntiCheatPressureWindowSummary;
	OutSnapshot.AntiCheatGamblerSummaries = AntiCheatGamblerSummaries;
	OutSnapshot.AntiCheatGamblerEvents = AntiCheatGamblerEvents;
	OutSnapshot.bAntiCheatGamblerEventsTruncated = bAntiCheatGamblerEventsTruncated;
	OutSnapshot.AntiCheatCurrentPressureWindowIndex = AntiCheatCurrentPressureWindowIndex;
	OutSnapshot.AntiCheatCurrentPressureHitChecks = AntiCheatCurrentPressureHitChecks;
	OutSnapshot.AntiCheatCurrentPressureDodges = AntiCheatCurrentPressureDodges;
	OutSnapshot.AntiCheatCurrentPressureDamageApplied = AntiCheatCurrentPressureDamageApplied;
	OutSnapshot.AntiCheatCurrentPressureExpectedDodges = AntiCheatCurrentPressureExpectedDodges;

	if (const UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		OutSnapshot.EquippedIdols = IdolManager->GetEquippedIdols();
		OutSnapshot.EquippedIdolTiers = IdolManager->GetEquippedIdolTierValues();
		OutSnapshot.RemainingCatchUpIdolPicks = IdolManager->GetRemainingCatchUpIdolPicks();
	}
	else
	{
		OutSnapshot.EquippedIdols = GetEquippedIdols();
		OutSnapshot.EquippedIdolTiers = GetEquippedIdolTierValues();
	}

	for (const TPair<FName, FT66LuckAccumulator>& Pair : LuckQuantityByCategory)
	{
		FT66SavedLuckAccumulator& Saved = OutSnapshot.LuckQuantityAccumulators.AddDefaulted_GetRef();
		Saved.Category = Pair.Key;
		Saved.Sum01 = static_cast<float>(Pair.Value.Sum01);
		Saved.Count = Pair.Value.Count;
	}

	for (const TPair<FName, FT66LuckAccumulator>& Pair : LuckQualityByCategory)
	{
		FT66SavedLuckAccumulator& Saved = OutSnapshot.LuckQualityAccumulators.AddDefaulted_GetRef();
		Saved.Category = Pair.Key;
		Saved.Sum01 = static_cast<float>(Pair.Value.Sum01);
		Saved.Count = Pair.Value.Count;
	}

	OutSnapshot.AntiCheatLuckEvents = AntiCheatLuckEvents;
	OutSnapshot.bAntiCheatLuckEventsTruncated = bAntiCheatLuckEventsTruncated;
	OutSnapshot.AntiCheatHitCheckEvents = AntiCheatHitCheckEvents;
	OutSnapshot.bAntiCheatHitCheckEventsTruncated = bAntiCheatHitCheckEventsTruncated;
}


void UT66RunStateSubsystem::ImportSavedRunSnapshot(const FT66SavedRunSnapshot& Snapshot)
{
	if (!Snapshot.bValid)
	{
		return;
	}

	CurrentStage = FMath::Clamp(Snapshot.CurrentStage, 1, 23);
	CurrentHP = FMath::Max(0.f, Snapshot.CurrentHP);
	MaxHP = FMath::Max(1.f, Snapshot.MaxHP);
	if (Snapshot.HeartSlotTiers.Num() >= DefaultMaxHearts)
	{
		HeartSlotTiers = Snapshot.HeartSlotTiers;
		HeartSlotTiers.SetNum(DefaultMaxHearts);
		for (uint8& TierValue : HeartSlotTiers)
		{
			TierValue = static_cast<uint8>(FMath::Clamp(static_cast<int32>(TierValue), 0, MaxHeartTier));
		}
		SyncMaxHPToHeartTiers();
		CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);
	}
	else
	{
		RebuildHeartSlotTiersFromMaxHP();
	}
	CurrentGold = FMath::Max(0, Snapshot.CurrentGold);
	CurrentDebt = FMath::Max(0, Snapshot.CurrentDebt);
	DifficultyTier = FMath::Max(0, Snapshot.DifficultyTier);
	DifficultySkulls = FMath::Max(0, Snapshot.DifficultySkulls);
	TotemsActivatedCount = FMath::Max(0, Snapshot.TotemsActivatedCount);
	GamblerAnger01 = FMath::Clamp(Snapshot.GamblerAnger01, 0.f, 1.f);
	OwedBossIDs = Snapshot.OwedBossIDs;
	CowardiceGatesTakenCount = FMath::Max(0, Snapshot.CowardiceGatesTakenCount);
	InventorySlots = Snapshot.InventorySlots;
	ActiveGamblersTokenLevel = T66_ClampGamblersTokenLevel(Snapshot.ActiveGamblersTokenLevel);
	EventLog = Snapshot.EventLog;
	StructuredEventLog = Snapshot.StructuredEventLog;
	StagePacingPoints = Snapshot.StagePacingPoints;
	bStageTimerActive = Snapshot.bStageTimerActive;
	StageTimerSecondsRemaining = FMath::Clamp(Snapshot.StageTimerSecondsRemaining, 0.f, StageTimerDurationSeconds);
	LastBroadcastStageTimerSecond = FMath::FloorToInt(StageTimerSecondsRemaining);
	SpeedRunElapsedSeconds = FMath::Max(0.f, Snapshot.SpeedRunElapsedSeconds);
	bSpeedRunStarted = Snapshot.bSpeedRunStarted;
	if (bSpeedRunStarted && GetWorld())
	{
		SpeedRunStartWorldSeconds = static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunElapsedSeconds;
	}
	else
	{
		SpeedRunStartWorldSeconds = 0.f;
	}
	LastBroadcastSpeedRunSecond = FMath::FloorToInt(SpeedRunElapsedSeconds);
	CompletedStageActiveSeconds = FMath::Max(0.f, Snapshot.CompletedStageActiveSeconds);
	FinalRunElapsedSeconds = FMath::Max(0.f, Snapshot.FinalRunElapsedSeconds);
	bRunEnded = Snapshot.bRunEnded;
	bRunEndedAsVictory = Snapshot.bRunEndedAsVictory;
	bPendingDifficultyClearSummary = Snapshot.bPendingDifficultyClearSummary;
	bSaintBlessingActive = Snapshot.bSaintBlessingActive;
	FinalSurvivalEnemyScalar = FMath::Clamp(Snapshot.FinalSurvivalEnemyScalar, 1.f, 99.f);
	AntiCheatIncomingHitChecks = FMath::Max(0, Snapshot.AntiCheatIncomingHitChecks);
	AntiCheatDamageTakenHitCount = FMath::Max(0, Snapshot.AntiCheatDamageTakenHitCount);
	AntiCheatDodgeCount = FMath::Max(0, Snapshot.AntiCheatDodgeCount);
	AntiCheatCurrentConsecutiveDodges = FMath::Max(0, Snapshot.AntiCheatCurrentConsecutiveDodges);
	AntiCheatMaxConsecutiveDodges = FMath::Max(AntiCheatCurrentConsecutiveDodges, Snapshot.AntiCheatMaxConsecutiveDodges);
	AntiCheatTotalEvasionChance = FMath::Clamp(Snapshot.AntiCheatTotalEvasionChance, 0.f, 1000000.f);
	AntiCheatEvasionBuckets = Snapshot.AntiCheatEvasionBuckets;
	InitializeAntiCheatEvasionBuckets();
	AntiCheatPressureWindowSummary = Snapshot.AntiCheatPressureWindowSummary;
	AntiCheatPressureWindowSummary.WindowSeconds = AntiCheatPressureWindowSeconds;
	AntiCheatGamblerSummaries = Snapshot.AntiCheatGamblerSummaries;
	AntiCheatGamblerEvents = Snapshot.AntiCheatGamblerEvents;
	bAntiCheatGamblerEventsTruncated = Snapshot.bAntiCheatGamblerEventsTruncated;
	AntiCheatCurrentPressureWindowIndex = Snapshot.AntiCheatCurrentPressureWindowIndex;
	AntiCheatCurrentPressureHitChecks = FMath::Max(0, Snapshot.AntiCheatCurrentPressureHitChecks);
	AntiCheatCurrentPressureDodges = FMath::Max(0, Snapshot.AntiCheatCurrentPressureDodges);
	AntiCheatCurrentPressureDamageApplied = FMath::Max(0, Snapshot.AntiCheatCurrentPressureDamageApplied);
	AntiCheatCurrentPressureExpectedDodges = FMath::Clamp(Snapshot.AntiCheatCurrentPressureExpectedDodges, 0.f, 1000000.f);
	CurrentScore = FMath::Max(0, Snapshot.CurrentScore);
	ScoreBudgetContext = Snapshot.ScoreBudgetContext;
	HeroLevel = FMath::Clamp(Snapshot.HeroLevel, DefaultHeroLevel, MaxHeroLevel);
	HeroXP = FMath::Max(0, Snapshot.HeroXP);
	XPToNextLevel = FMath::Max(1, Snapshot.XPToNextLevel);
	HeroStats = Snapshot.HeroStats;
	if (Snapshot.HeroPreciseStats.HasAnyPositiveValue())
	{
		HeroPreciseStats = Snapshot.HeroPreciseStats;
	}
	else
	{
		HeroPreciseStats.SetFromWholeStatBlock(HeroStats);
	}
	HeroPreciseStats.DamageTenths = ClampHeroStatTenths(HeroPreciseStats.DamageTenths);
	HeroPreciseStats.AttackSpeedTenths = ClampHeroStatTenths(HeroPreciseStats.AttackSpeedTenths);
	HeroPreciseStats.AttackScaleTenths = ClampHeroStatTenths(HeroPreciseStats.AttackScaleTenths);
	HeroPreciseStats.AccuracyTenths = ClampHeroStatTenths(HeroPreciseStats.AccuracyTenths);
	HeroPreciseStats.ArmorTenths = ClampHeroStatTenths(HeroPreciseStats.ArmorTenths);
	HeroPreciseStats.EvasionTenths = ClampHeroStatTenths(HeroPreciseStats.EvasionTenths);
	HeroPreciseStats.LuckTenths = ClampHeroStatTenths(HeroPreciseStats.LuckTenths);
	HeroPreciseStats.SpeedTenths = ClampHeroStatTenths(HeroPreciseStats.SpeedTenths);
	HeroStatRng.Initialize(Snapshot.HeroStatRngCurrentSeed != 0 ? Snapshot.HeroStatRngCurrentSeed : static_cast<int32>(FPlatformTime::Cycles()));
	ClearPersistentSecondaryStatBonuses();
	for (const FT66SavedSecondaryStatBonusEntry& SavedBonus : Snapshot.PersistentSecondaryStatBonusEntries)
	{
		AddPersistentSecondaryStatBonusTenths(SavedBonus.StatType, SavedBonus.BonusTenths);
	}
	SyncLegacyHeroStatsFromPrecise();
	PowerCrystalsEarnedThisRun = FMath::Max(0, Snapshot.PowerCrystalsEarnedThisRun);
	PowerCrystalsGrantedToWalletThisRun = FMath::Clamp(Snapshot.PowerCrystalsGrantedToWalletThisRun, 0, PowerCrystalsEarnedThisRun);
	SeedLuck0To100 = (Snapshot.SeedLuck0To100 >= 0) ? FMath::Clamp(Snapshot.SeedLuck0To100, 0, 100) : -1;
	CompanionHealingDoneThisRun = FMath::Max(0.f, Snapshot.CompanionHealingDoneThisRun);
	bBossActive = Snapshot.bBossActive;
	ActiveBossID = Snapshot.ActiveBossID;
	BossMaxHP = FMath::Max(1, Snapshot.BossMaxHP);
	BossCurrentHP = FMath::Clamp(Snapshot.BossCurrentHP, 0, BossMaxHP);
	BossPartSnapshots = Snapshot.BossParts;
	if (BossPartSnapshots.Num() > 0)
	{
		T66_RecomputeBossAggregate(BossPartSnapshots, BossMaxHP, BossCurrentHP);
	}

	LuckQuantityByCategory.Reset();
	for (const FT66SavedLuckAccumulator& Saved : Snapshot.LuckQuantityAccumulators)
	{
		FT66LuckAccumulator Accumulator;
		Accumulator.Sum01 = static_cast<double>(Saved.Sum01);
		Accumulator.Count = FMath::Max(0, Saved.Count);
		LuckQuantityByCategory.Add(Saved.Category, Accumulator);
	}

	LuckQualityByCategory.Reset();
	for (const FT66SavedLuckAccumulator& Saved : Snapshot.LuckQualityAccumulators)
	{
		FT66LuckAccumulator Accumulator;
		Accumulator.Sum01 = static_cast<double>(Saved.Sum01);
		Accumulator.Count = FMath::Max(0, Saved.Count);
		LuckQualityByCategory.Add(Saved.Category, Accumulator);
	}

	AntiCheatLuckEvents = Snapshot.AntiCheatLuckEvents;
	bAntiCheatLuckEventsTruncated = Snapshot.bAntiCheatLuckEventsTruncated;
	AntiCheatHitCheckEvents = Snapshot.AntiCheatHitCheckEvents;
	bAntiCheatHitCheckEventsTruncated = Snapshot.bAntiCheatHitCheckEventsTruncated;
	if (AntiCheatGamblerEvents.Num() > MaxAntiCheatGamblerEvents)
	{
		const int32 RemoveCount = AntiCheatGamblerEvents.Num() - MaxAntiCheatGamblerEvents;
		AntiCheatGamblerEvents.RemoveAt(0, RemoveCount, EAllowShrinking::No);
		bAntiCheatGamblerEventsTruncated = true;
	}

	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		ET66Difficulty Difficulty = ET66Difficulty::Easy;
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
		{
			Difficulty = T66GI->SelectedDifficulty;
		}
		IdolManager->RestoreState(Snapshot.EquippedIdols, Snapshot.EquippedIdolTiers, Difficulty, Snapshot.RemainingCatchUpIdolPicks);
	}

	RecomputeItemDerivedStats();
	TrimLogsIfNeeded();

	HeartsChanged.Broadcast();
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	DifficultyChanged.Broadcast();
	GamblerAngerChanged.Broadcast();
	InventoryChanged.Broadcast();
	IdolsChanged.Broadcast();
	ScoreChanged.Broadcast();
	StageChanged.Broadcast();
	StageTimerChanged.Broadcast();
	SpeedRunTimerChanged.Broadcast();
	BossChanged.Broadcast();
	HeroProgressChanged.Broadcast();
	SurvivalChanged.Broadcast();
	VendorChanged.Broadcast();
	StatusEffectsChanged.Broadcast();
}
