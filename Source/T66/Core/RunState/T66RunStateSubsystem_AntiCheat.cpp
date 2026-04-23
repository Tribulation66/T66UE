// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

void UT66RunStateSubsystem::ResetLuckRatingTracking()
{
	LuckQuantityByCategory.Reset();
	LuckQualityByCategory.Reset();
}


void UT66RunStateSubsystem::RecordLuckQuantityRoll(FName Category, int32 RolledValue, int32 MinValue, int32 MaxValue, int32 RunDrawIndex, int32 PreDrawSeed)
{
	const int32 MinV = FMath::Min(MinValue, MaxValue);
	const int32 MaxV = FMath::Max(MinValue, MaxValue);
	float Score01 = 0.5f;
	if (MaxV <= MinV)
	{
		Score01 = 1.f;
	}
	else
	{
		Score01 = FMath::Clamp(static_cast<float>(RolledValue - MinV) / static_cast<float>(MaxV - MinV), 0.f, 1.f);
	}

	LuckQuantityByCategory.FindOrAdd(Category).Add01(Score01);
	RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType::QuantityRoll, Category, Score01, RolledValue, MinV, MaxV, RunDrawIndex, PreDrawSeed);
}


void UT66RunStateSubsystem::RecordLuckQuantityFloatRollRounded(FName Category, int32 RolledValue, int32 MinValue, int32 MaxValue, float ReplayMinValue, float ReplayMaxValue, int32 RunDrawIndex, int32 PreDrawSeed)
{
	const int32 MinV = FMath::Min(MinValue, MaxValue);
	const int32 MaxV = FMath::Max(MinValue, MaxValue);
	float Score01 = 0.5f;
	if (MaxV <= MinV)
	{
		Score01 = 1.f;
	}
	else
	{
		Score01 = FMath::Clamp(static_cast<float>(RolledValue - MinV) / static_cast<float>(MaxV - MinV), 0.f, 1.f);
	}

	LuckQuantityByCategory.FindOrAdd(Category).Add01(Score01);
	FT66FloatRange ReplayRange;
	ReplayRange.Min = ReplayMinValue;
	ReplayRange.Max = ReplayMaxValue;
	RecordAntiCheatLuckEvent(
		ET66AntiCheatLuckEventType::QuantityRoll,
		Category,
		Score01,
		RolledValue,
		MinV,
		MaxV,
		RunDrawIndex,
		PreDrawSeed,
		-1.f,
		nullptr,
		&ReplayRange);
}


void UT66RunStateSubsystem::RecordLuckQuantityBool(FName Category, bool bSucceeded, float ExpectedChance01, int32 RunDrawIndex, int32 PreDrawSeed)
{
	LuckQuantityByCategory.FindOrAdd(Category).Add01(bSucceeded ? 1.f : 0.f);
	RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType::QuantityBool, Category, bSucceeded ? 1.f : 0.f, bSucceeded ? 1 : 0, 0, 1, RunDrawIndex, PreDrawSeed, ExpectedChance01);
}


void UT66RunStateSubsystem::RecordLuckQualityRarity(FName Category, ET66Rarity Rarity, int32 RunDrawIndex, int32 PreDrawSeed, const FT66RarityWeights* ReplayWeights)
{
	const float Score01 = T66_RarityTo01(Rarity);
	LuckQualityByCategory.FindOrAdd(Category).Add01(Score01);
	RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType::QualityRarity, Category, Score01, static_cast<int32>(Rarity), 0, 3, RunDrawIndex, PreDrawSeed, -1.f, ReplayWeights);
}


float UT66RunStateSubsystem::ComputeLuckRatingQuantity01() const
{
	return T66_AverageCategories01(LuckQuantityByCategory);
}


float UT66RunStateSubsystem::ComputeLuckRatingQuality01() const
{
	return T66_AverageCategories01(LuckQualityByCategory);
}


int32 UT66RunStateSubsystem::GetLuckRatingQuantity0To100() const
{
	return FMath::Clamp(FMath::RoundToInt(ComputeLuckRatingQuantity01() * 100.f), 0, 100);
}


int32 UT66RunStateSubsystem::GetLuckRatingQuality0To100() const
{
	return FMath::Clamp(FMath::RoundToInt(ComputeLuckRatingQuality01() * 100.f), 0, 100);
}


int32 UT66RunStateSubsystem::GetLuckRating0To100() const
{
	const bool bHasQuantity = (LuckQuantityByCategory.Num() > 0);
	const bool bHasQuality = (LuckQualityByCategory.Num() > 0);

	float Rating01 = 0.5f;
	if (bHasQuantity && bHasQuality)
	{
		Rating01 = 0.5f * (ComputeLuckRatingQuantity01() + ComputeLuckRatingQuality01());
	}
	else if (bHasQuantity)
	{
		Rating01 = ComputeLuckRatingQuantity01();
	}
	else if (bHasQuality)
	{
		Rating01 = ComputeLuckRatingQuality01();
	}

	return FMath::Clamp(FMath::RoundToInt(Rating01 * 100.f), 0, 100);
}


int32 UT66RunStateSubsystem::GetLuckQuantitySampleCount() const
{
	int32 TotalCount = 0;
	for (const TPair<FName, FT66LuckAccumulator>& Pair : LuckQuantityByCategory)
	{
		TotalCount = FMath::Clamp(TotalCount + FMath::Max(0, Pair.Value.Count), 0, 1000000);
	}
	return TotalCount;
}


int32 UT66RunStateSubsystem::GetLuckQualitySampleCount() const
{
	int32 TotalCount = 0;
	for (const TPair<FName, FT66LuckAccumulator>& Pair : LuckQualityByCategory)
	{
		TotalCount = FMath::Clamp(TotalCount + FMath::Max(0, Pair.Value.Count), 0, 1000000);
	}
	return TotalCount;
}


void UT66RunStateSubsystem::GetLuckQuantityAccumulators(TArray<FT66SavedLuckAccumulator>& OutAccumulators) const
{
	OutAccumulators.Reset();

	TArray<FName> Categories;
	LuckQuantityByCategory.GenerateKeyArray(Categories);
	Categories.Sort([](const FName& A, const FName& B)
	{
		return A.ToString() < B.ToString();
	});

	for (const FName& Category : Categories)
	{
		if (const FT66LuckAccumulator* Accumulator = LuckQuantityByCategory.Find(Category))
		{
			FT66SavedLuckAccumulator& Saved = OutAccumulators.AddDefaulted_GetRef();
			Saved.Category = Category;
			Saved.Sum01 = static_cast<float>(Accumulator->Sum01);
			Saved.Count = FMath::Max(0, Accumulator->Count);
		}
	}
}


void UT66RunStateSubsystem::GetLuckQualityAccumulators(TArray<FT66SavedLuckAccumulator>& OutAccumulators) const
{
	OutAccumulators.Reset();

	TArray<FName> Categories;
	LuckQualityByCategory.GenerateKeyArray(Categories);
	Categories.Sort([](const FName& A, const FName& B)
	{
		return A.ToString() < B.ToString();
	});

	for (const FName& Category : Categories)
	{
		if (const FT66LuckAccumulator* Accumulator = LuckQualityByCategory.Find(Category))
		{
			FT66SavedLuckAccumulator& Saved = OutAccumulators.AddDefaulted_GetRef();
			Saved.Category = Category;
			Saved.Sum01 = static_cast<float>(Accumulator->Sum01);
			Saved.Count = FMath::Max(0, Accumulator->Count);
		}
	}
}


void UT66RunStateSubsystem::GetAntiCheatLuckEvents(TArray<FT66AntiCheatLuckEvent>& OutEvents) const
{
	OutEvents = AntiCheatLuckEvents;
}


void UT66RunStateSubsystem::GetAntiCheatHitCheckEvents(TArray<FT66AntiCheatHitCheckEvent>& OutEvents) const
{
	OutEvents = AntiCheatHitCheckEvents;
}


void UT66RunStateSubsystem::GetAntiCheatEvasionBuckets(TArray<FT66AntiCheatEvasionBucketSummary>& OutBuckets) const
{
	OutBuckets = AntiCheatEvasionBuckets;
}


void UT66RunStateSubsystem::GetAntiCheatPressureWindowSummary(FT66AntiCheatPressureWindowSummary& OutSummary) const
{
	OutSummary = BuildAntiCheatPressureWindowSummarySnapshot();
}


void UT66RunStateSubsystem::GetAntiCheatGamblerSummaries(TArray<FT66AntiCheatGamblerGameSummary>& OutSummaries) const
{
	OutSummaries = AntiCheatGamblerSummaries;
}


void UT66RunStateSubsystem::GetAntiCheatGamblerEvents(TArray<FT66AntiCheatGamblerEvent>& OutEvents) const
{
	OutEvents = AntiCheatGamblerEvents;
}


float UT66RunStateSubsystem::GetRunElapsedSecondsForAntiCheatEvent() const
{
	if (bRunEnded)
	{
		return FMath::Max(0.f, FinalRunElapsedSeconds);
	}

	if (bSpeedRunStarted && GetWorld())
	{
		return FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds()) - SpeedRunStartWorldSeconds);
	}

	return FMath::Max(0.f, SpeedRunElapsedSeconds);
}


void UT66RunStateSubsystem::InitializeAntiCheatEvasionBuckets()
{
	if (AntiCheatEvasionBuckets.Num() == AntiCheatEvasionBucketCount)
	{
		return;
	}

	AntiCheatEvasionBuckets.Reset();
	for (int32 BucketIndex = 0; BucketIndex < AntiCheatEvasionBucketCount; ++BucketIndex)
	{
		FT66AntiCheatEvasionBucketSummary& Bucket = AntiCheatEvasionBuckets.AddDefaulted_GetRef();
		Bucket.BucketIndex = BucketIndex;
		Bucket.MinChance01 = GetAntiCheatEvasionBucketMinChance01(BucketIndex);
		Bucket.MaxChance01 = GetAntiCheatEvasionBucketMaxChance01(BucketIndex);
	}
}


int32 UT66RunStateSubsystem::GetAntiCheatEvasionBucketIndex(float EvasionChance01)
{
	const float Clamped = FMath::Clamp(EvasionChance01, 0.f, 1.f);
	if (Clamped < 0.15f) return 0;
	if (Clamped < 0.30f) return 1;
	if (Clamped < 0.45f) return 2;
	if (Clamped < 0.60f) return 3;
	return 4;
}


float UT66RunStateSubsystem::GetAntiCheatEvasionBucketMinChance01(int32 BucketIndex)
{
	switch (BucketIndex)
	{
	case 1: return 0.15f;
	case 2: return 0.30f;
	case 3: return 0.45f;
	case 4: return 0.60f;
	default: return 0.f;
	}
}


float UT66RunStateSubsystem::GetAntiCheatEvasionBucketMaxChance01(int32 BucketIndex)
{
	switch (BucketIndex)
	{
	case 0: return 0.15f;
	case 1: return 0.30f;
	case 2: return 0.45f;
	case 3: return 0.60f;
	default: return 1.f;
	}
}


void UT66RunStateSubsystem::ResetAntiCheatPressureTracking()
{
	AntiCheatPressureWindowSummary = FT66AntiCheatPressureWindowSummary{};
	AntiCheatPressureWindowSummary.WindowSeconds = AntiCheatPressureWindowSeconds;
	AntiCheatCurrentPressureWindowIndex = INDEX_NONE;
	AntiCheatCurrentPressureHitChecks = 0;
	AntiCheatCurrentPressureDodges = 0;
	AntiCheatCurrentPressureDamageApplied = 0;
	AntiCheatCurrentPressureExpectedDodges = 0.f;
}


void UT66RunStateSubsystem::FinalizeCurrentAntiCheatPressureWindow()
{
	if (AntiCheatCurrentPressureHitChecks <= 0)
	{
		AntiCheatCurrentPressureWindowIndex = INDEX_NONE;
		AntiCheatCurrentPressureDodges = 0;
		AntiCheatCurrentPressureDamageApplied = 0;
		AntiCheatCurrentPressureExpectedDodges = 0.f;
		return;
	}

	T66_AccumulatePressureWindowSummary(
		AntiCheatPressureWindowSummary,
		AntiCheatCurrentPressureHitChecks,
		AntiCheatCurrentPressureDodges,
		AntiCheatCurrentPressureDamageApplied,
		AntiCheatCurrentPressureExpectedDodges);

	AntiCheatCurrentPressureWindowIndex = INDEX_NONE;
	AntiCheatCurrentPressureHitChecks = 0;
	AntiCheatCurrentPressureDodges = 0;
	AntiCheatCurrentPressureDamageApplied = 0;
	AntiCheatCurrentPressureExpectedDodges = 0.f;
}


void UT66RunStateSubsystem::RecordAntiCheatPressureHitCheck(float RunElapsedSeconds, float EvasionChance01, bool bDodged, bool bDamageApplied)
{
	const float ClampedTime = FMath::Max(0.f, RunElapsedSeconds);
	const int32 WindowIndex = FMath::Max(0, FMath::FloorToInt(ClampedTime / static_cast<float>(AntiCheatPressureWindowSeconds)));
	if (AntiCheatCurrentPressureWindowIndex != WindowIndex)
	{
		FinalizeCurrentAntiCheatPressureWindow();
		AntiCheatCurrentPressureWindowIndex = WindowIndex;
	}

	AntiCheatCurrentPressureHitChecks = FMath::Clamp(AntiCheatCurrentPressureHitChecks + 1, 0, 1000000);
	AntiCheatCurrentPressureExpectedDodges = FMath::Clamp(AntiCheatCurrentPressureExpectedDodges + FMath::Clamp(EvasionChance01, 0.f, 1.f), 0.f, 1000000.f);
	if (bDodged)
	{
		AntiCheatCurrentPressureDodges = FMath::Clamp(AntiCheatCurrentPressureDodges + 1, 0, 1000000);
	}
	if (bDamageApplied)
	{
		AntiCheatCurrentPressureDamageApplied = FMath::Clamp(AntiCheatCurrentPressureDamageApplied + 1, 0, 1000000);
	}
}


FT66AntiCheatPressureWindowSummary UT66RunStateSubsystem::BuildAntiCheatPressureWindowSummarySnapshot() const
{
	FT66AntiCheatPressureWindowSummary Summary = AntiCheatPressureWindowSummary;
	Summary.WindowSeconds = AntiCheatPressureWindowSeconds;
	if (AntiCheatCurrentPressureHitChecks > 0)
	{
		T66_AccumulatePressureWindowSummary(
			Summary,
			AntiCheatCurrentPressureHitChecks,
			AntiCheatCurrentPressureDodges,
			AntiCheatCurrentPressureDamageApplied,
			AntiCheatCurrentPressureExpectedDodges);
	}
	return Summary;
}


void UT66RunStateSubsystem::RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType EventType, FName Category, float Value01, int32 RawValue, int32 RawMin, int32 RawMax, int32 RunDrawIndex, int32 PreDrawSeed, float ExpectedChance01, const FT66RarityWeights* ReplayWeights, const FT66FloatRange* ReplayFloatRange)
{
	FT66AntiCheatLuckEvent& Event = AntiCheatLuckEvents.AddDefaulted_GetRef();
	Event.EventType = EventType;
	Event.Category = Category;
	Event.TimeSeconds = GetRunElapsedSecondsForAntiCheatEvent();
	Event.SeedLuck0To100 = GetSeedLuck0To100();
	Event.LuckModifierPercent = GetTotalLuckModifierPercent();
	Event.EffectiveLuck = GetEffectiveLuckValue();
	Event.Value01 = FMath::Clamp(Value01, 0.f, 1.f);
	Event.RawValue = RawValue;
	Event.RawMin = RawMin;
	Event.RawMax = RawMax;
	Event.RunDrawIndex = RunDrawIndex;
	Event.PreDrawSeed = PreDrawSeed;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66RngSubsystem* Rng = GI->GetSubsystem<UT66RngSubsystem>())
		{
			Event.LuckStatSnapshot = Rng->GetLuckStat();
			Event.Luck01Snapshot = Rng->GetLuck01();
		}
	}
	Event.ExpectedChance01 = ExpectedChance01;
	Event.bHasRarityWeights = (ReplayWeights != nullptr);
	if (ReplayWeights)
	{
		Event.RarityWeights = *ReplayWeights;
	}
	Event.bHasFloatReplayRange = (ReplayFloatRange != nullptr);
	if (ReplayFloatRange)
	{
		Event.FloatReplayMin = ReplayFloatRange->Min;
		Event.FloatReplayMax = ReplayFloatRange->Max;
	}

	if (AntiCheatLuckEvents.Num() > MaxAntiCheatLuckEvents)
	{
		const int32 RemoveCount = AntiCheatLuckEvents.Num() - MaxAntiCheatLuckEvents;
		AntiCheatLuckEvents.RemoveAt(0, RemoveCount, EAllowShrinking::No);
		bAntiCheatLuckEventsTruncated = true;
	}
}


void UT66RunStateSubsystem::RecordAntiCheatHitCheckEvent(float EvasionChance01, bool bDodged, bool bDamageApplied)
{
	FT66AntiCheatHitCheckEvent& Event = AntiCheatHitCheckEvents.AddDefaulted_GetRef();
	Event.TimeSeconds = GetRunElapsedSecondsForAntiCheatEvent();
	Event.EvasionChance01 = FMath::Clamp(EvasionChance01, 0.f, 1.f);
	Event.bDodged = bDodged;
	Event.bDamageApplied = bDamageApplied;
	InitializeAntiCheatEvasionBuckets();
	if (AntiCheatEvasionBuckets.Num() == AntiCheatEvasionBucketCount)
	{
		const int32 BucketIndex = GetAntiCheatEvasionBucketIndex(Event.EvasionChance01);
		if (AntiCheatEvasionBuckets.IsValidIndex(BucketIndex))
		{
			FT66AntiCheatEvasionBucketSummary& Bucket = AntiCheatEvasionBuckets[BucketIndex];
			Bucket.HitChecks = FMath::Clamp(Bucket.HitChecks + 1, 0, 1000000);
			Bucket.ExpectedDodges = FMath::Clamp(Bucket.ExpectedDodges + Event.EvasionChance01, 0.f, 1000000.f);
			if (Event.bDodged)
			{
				Bucket.Dodges = FMath::Clamp(Bucket.Dodges + 1, 0, 1000000);
			}
			if (Event.bDamageApplied)
			{
				Bucket.DamageApplied = FMath::Clamp(Bucket.DamageApplied + 1, 0, 1000000);
			}
		}
	}
	RecordAntiCheatPressureHitCheck(GetCurrentRunElapsedSeconds(), Event.EvasionChance01, Event.bDodged, Event.bDamageApplied);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SkillRatingSubsystem* Skill = GI->GetSubsystem<UT66SkillRatingSubsystem>())
		{
			Skill->NotifyHitCheck(Event.EvasionChance01, Event.bDodged, Event.bDamageApplied);
		}
	}

	if (AntiCheatHitCheckEvents.Num() > MaxAntiCheatHitCheckEvents)
	{
		const int32 RemoveCount = AntiCheatHitCheckEvents.Num() - MaxAntiCheatHitCheckEvents;
		AntiCheatHitCheckEvents.RemoveAt(0, RemoveCount, EAllowShrinking::No);
		bAntiCheatHitCheckEventsTruncated = true;
	}
}


FT66AntiCheatGamblerGameSummary& UT66RunStateSubsystem::FindOrAddAntiCheatGamblerSummary(ET66AntiCheatGamblerGameType GameType)
{
	for (FT66AntiCheatGamblerGameSummary& Summary : AntiCheatGamblerSummaries)
	{
		if (Summary.GameType == GameType)
		{
			return Summary;
		}
	}

	FT66AntiCheatGamblerGameSummary& Summary = AntiCheatGamblerSummaries.AddDefaulted_GetRef();
	Summary.GameType = GameType;
	return Summary;
}


void UT66RunStateSubsystem::RecordAntiCheatGamblerRound(
	ET66AntiCheatGamblerGameType GameType,
	int32 BetGold,
	int32 PayoutGold,
	bool bCheatAttempted,
	bool bCheatSucceeded,
	bool bWin,
	bool bDraw,
	int32 PlayerChoice,
	int32 OpponentChoice,
	int32 OutcomeValue,
	int32 OutcomeSecondaryValue,
	int32 SelectedMask,
	int32 ResolvedMask,
	int32 PathBits,
	int32 ShufflePreDrawSeed,
	int32 ShuffleStartDrawIndex,
	int32 OutcomePreDrawSeed,
	int32 OutcomeDrawIndex,
	float OutcomeExpectedChance01,
	const FString& ActionSequence)
{
	FT66AntiCheatGamblerGameSummary& Summary = FindOrAddAntiCheatGamblerSummary(GameType);
	Summary.Rounds = FMath::Clamp(Summary.Rounds + 1, 0, 1000000);
	Summary.TotalBetGold = FMath::Clamp(Summary.TotalBetGold + FMath::Max(0, BetGold), 0, INT32_MAX);
	Summary.TotalPayoutGold = FMath::Clamp(Summary.TotalPayoutGold + FMath::Max(0, PayoutGold), 0, INT32_MAX);
	if (bCheatAttempted)
	{
		Summary.CheatAttempts = FMath::Clamp(Summary.CheatAttempts + 1, 0, 1000000);
		if (bCheatSucceeded)
		{
			Summary.CheatSuccesses = FMath::Clamp(Summary.CheatSuccesses + 1, 0, 1000000);
		}
	}
	if (bDraw)
	{
		Summary.Draws = FMath::Clamp(Summary.Draws + 1, 0, 1000000);
	}
	else if (bWin)
	{
		Summary.Wins = FMath::Clamp(Summary.Wins + 1, 0, 1000000);
	}
	else
	{
		Summary.Losses = FMath::Clamp(Summary.Losses + 1, 0, 1000000);
	}

	FT66AntiCheatGamblerEvent& Event = AntiCheatGamblerEvents.AddDefaulted_GetRef();
	Event.GameType = GameType;
	Event.TimeSeconds = GetRunElapsedSecondsForAntiCheatEvent();
	Event.BetGold = FMath::Max(0, BetGold);
	Event.PayoutGold = FMath::Max(0, PayoutGold);
	Event.bCheatAttempted = bCheatAttempted;
	Event.bCheatSucceeded = bCheatSucceeded;
	Event.bWin = bWin;
	Event.bDraw = bDraw;
	Event.PlayerChoice = PlayerChoice;
	Event.OpponentChoice = OpponentChoice;
	Event.OutcomeValue = OutcomeValue;
	Event.OutcomeSecondaryValue = OutcomeSecondaryValue;
	Event.SelectedMask = SelectedMask;
	Event.ResolvedMask = ResolvedMask;
	Event.PathBits = PathBits;
	Event.ShufflePreDrawSeed = ShufflePreDrawSeed;
	Event.ShuffleStartDrawIndex = ShuffleStartDrawIndex;
	Event.OutcomePreDrawSeed = OutcomePreDrawSeed;
	Event.OutcomeDrawIndex = OutcomeDrawIndex;
	Event.OutcomeExpectedChance01 = OutcomeExpectedChance01;
	Event.ActionSequence = ActionSequence;

	if (AntiCheatGamblerEvents.Num() > MaxAntiCheatGamblerEvents)
	{
		const int32 RemoveCount = AntiCheatGamblerEvents.Num() - MaxAntiCheatGamblerEvents;
		AntiCheatGamblerEvents.RemoveAt(0, RemoveCount, EAllowShrinking::No);
		bAntiCheatGamblerEventsTruncated = true;
	}
}
