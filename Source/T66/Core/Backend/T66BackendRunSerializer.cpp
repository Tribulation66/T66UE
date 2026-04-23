// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Backend/T66BackendRunSerializer.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	int32 T66GetDifficultyStageCount(ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy:
		case ET66Difficulty::Medium:
		case ET66Difficulty::Hard:
		case ET66Difficulty::VeryHard:
			return 4;
		case ET66Difficulty::Impossible:
			return 3;
		default:
			return 4;
		}
	}

	int32 T66GetDifficultyStartStage(ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy: return 1;
		case ET66Difficulty::Medium: return 6;
		case ET66Difficulty::Hard: return 11;
		case ET66Difficulty::VeryHard: return 16;
		case ET66Difficulty::Impossible: return 21;
		default: return 1;
		}
	}

	int32 T66NormalizeLocalStageReached(ET66Difficulty Difficulty, int32 StageReached)
	{
		const int32 StageCount = T66GetDifficultyStageCount(Difficulty);
		if (StageReached >= 1 && StageReached <= StageCount)
		{
			return StageReached;
		}

		const int32 StageStart = T66GetDifficultyStartStage(Difficulty);
		return FMath::Clamp(StageReached - StageStart + 1, 1, StageCount);
	}

	int32 T66MakeBackendCompatibleStageReached(ET66Difficulty Difficulty, int32 StageReached)
	{
		const int32 LocalStageReached = T66NormalizeLocalStageReached(Difficulty, StageReached);
		const int32 StageStart = T66GetDifficultyStartStage(Difficulty);
		const int32 CanonicalAbsoluteStage = StageStart + LocalStageReached - 1;

		if (Difficulty != ET66Difficulty::Impossible && LocalStageReached == T66GetDifficultyStageCount(Difficulty))
		{
			return StageStart + 4;
		}

		if (StageReached < StageStart || StageReached > StageStart + 4)
		{
			return CanonicalAbsoluteStage;
		}

		return StageReached;
	}

	FString T66DifficultyToApiString(ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy: return TEXT("easy");
		case ET66Difficulty::Medium: return TEXT("medium");
		case ET66Difficulty::Hard: return TEXT("hard");
		case ET66Difficulty::VeryHard: return TEXT("veryhard");
		case ET66Difficulty::Impossible: return TEXT("impossible");
		default: return TEXT("easy");
		}
	}

	FString T66PartySizeToApiString(ET66PartySize PartySize)
	{
		switch (PartySize)
		{
		case ET66PartySize::Solo: return TEXT("solo");
		case ET66PartySize::Duo: return TEXT("duo");
		case ET66PartySize::Trio: return TEXT("trio");
		case ET66PartySize::Quad: return TEXT("quad");
		default: return TEXT("solo");
		}
	}
}

TSharedPtr<FJsonObject> T66BackendRunSerializer::BuildRunJsonObject(
	const FString& HeroId,
	const FString& CompanionId,
	ET66Difficulty Difficulty,
	ET66PartySize PartySize,
	int32 StageReached,
	int32 Score,
	int32 TimeMs,
	UT66LeaderboardRunSummarySaveGame* Snapshot)
{
	TSharedPtr<FJsonObject> RunObj = MakeShared<FJsonObject>();
	RunObj->SetStringField(TEXT("hero_id"), HeroId);
	RunObj->SetStringField(TEXT("companion_id"), CompanionId);
	RunObj->SetStringField(TEXT("difficulty"), T66DifficultyToApiString(Difficulty));
	RunObj->SetStringField(TEXT("party_size"), T66PartySizeToApiString(PartySize));
	RunObj->SetNumberField(TEXT("stage_reached"), T66MakeBackendCompatibleStageReached(Difficulty, StageReached));
	RunObj->SetNumberField(TEXT("local_stage_reached"), T66NormalizeLocalStageReached(Difficulty, StageReached));
	RunObj->SetNumberField(TEXT("score"), Score);
	RunObj->SetNumberField(TEXT("time_ms"), TimeMs);

	if (!Snapshot)
	{
		return RunObj;
	}

	RunObj->SetNumberField(TEXT("hero_level"), Snapshot->HeroLevel);

	TSharedPtr<FJsonObject> StatsObj = MakeShared<FJsonObject>();
	StatsObj->SetNumberField(TEXT("damage"), Snapshot->DamageStat);
	StatsObj->SetNumberField(TEXT("attack_speed"), Snapshot->AttackSpeedStat);
	StatsObj->SetNumberField(TEXT("attack_scale"), Snapshot->AttackScaleStat);
	StatsObj->SetNumberField(TEXT("accuracy"), Snapshot->AccuracyStat);
	StatsObj->SetNumberField(TEXT("armor"), Snapshot->ArmorStat);
	StatsObj->SetNumberField(TEXT("evasion"), Snapshot->EvasionStat);
	StatsObj->SetNumberField(TEXT("luck"), Snapshot->LuckStat);
	StatsObj->SetNumberField(TEXT("speed"), Snapshot->SpeedStat);
	RunObj->SetObjectField(TEXT("stats"), StatsObj);

	TSharedPtr<FJsonObject> SecObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Snapshot->SecondaryStatValues)
	{
		FString KeyName;
		switch (Pair.Key)
		{
		case ET66SecondaryStatType::AoeDamage: KeyName = TEXT("AoeDamage"); break;
		case ET66SecondaryStatType::BounceDamage: KeyName = TEXT("BounceDamage"); break;
		case ET66SecondaryStatType::PierceDamage: KeyName = TEXT("PierceDamage"); break;
		case ET66SecondaryStatType::DotDamage: KeyName = TEXT("DotDamage"); break;
		case ET66SecondaryStatType::AoeSpeed: KeyName = TEXT("AoeSpeed"); break;
		case ET66SecondaryStatType::BounceSpeed: KeyName = TEXT("BounceSpeed"); break;
		case ET66SecondaryStatType::PierceSpeed: KeyName = TEXT("PierceSpeed"); break;
		case ET66SecondaryStatType::DotSpeed: KeyName = TEXT("DotSpeed"); break;
		case ET66SecondaryStatType::AoeScale: KeyName = TEXT("AoeScale"); break;
		case ET66SecondaryStatType::BounceScale: KeyName = TEXT("BounceScale"); break;
		case ET66SecondaryStatType::PierceScale: KeyName = TEXT("PierceScale"); break;
		case ET66SecondaryStatType::DotScale: KeyName = TEXT("DotScale"); break;
		case ET66SecondaryStatType::CritDamage: KeyName = TEXT("CritDamage"); break;
		case ET66SecondaryStatType::CritChance: KeyName = TEXT("CritChance"); break;
		case ET66SecondaryStatType::CloseRangeDamage: KeyName = TEXT("CloseRangeDamage"); break;
		case ET66SecondaryStatType::LongRangeDamage: KeyName = TEXT("LongRangeDamage"); break;
		case ET66SecondaryStatType::AttackRange: KeyName = TEXT("AttackRange"); break;
		case ET66SecondaryStatType::Taunt: KeyName = TEXT("Taunt"); break;
		case ET66SecondaryStatType::ReflectDamage: KeyName = TEXT("ReflectDamage"); break;
		case ET66SecondaryStatType::HpRegen: KeyName = TEXT("HpRegen"); break;
		case ET66SecondaryStatType::Crush: KeyName = TEXT("Crush"); break;
		case ET66SecondaryStatType::Invisibility: KeyName = TEXT("Invisibility"); break;
		case ET66SecondaryStatType::CounterAttack: KeyName = TEXT("CounterAttack"); break;
		case ET66SecondaryStatType::LifeSteal: KeyName = TEXT("LifeSteal"); break;
		case ET66SecondaryStatType::Assassinate: KeyName = TEXT("Assassinate"); break;
		case ET66SecondaryStatType::SpinWheel: KeyName = TEXT("SpinWheel"); break;
		case ET66SecondaryStatType::Goblin: KeyName = TEXT("Goblin"); break;
		case ET66SecondaryStatType::Leprechaun: KeyName = TEXT("Leprechaun"); break;
		case ET66SecondaryStatType::TreasureChest: KeyName = TEXT("TreasureChest"); break;
		case ET66SecondaryStatType::Fountain: KeyName = TEXT("Fountain"); break;
		case ET66SecondaryStatType::Cheating: KeyName = TEXT("Cheating"); break;
		case ET66SecondaryStatType::Stealing: KeyName = TEXT("Stealing"); break;
		case ET66SecondaryStatType::MovementSpeed: KeyName = TEXT("MovementSpeed"); break;
		case ET66SecondaryStatType::LootCrate: KeyName = TEXT("LootCrate"); break;
		case ET66SecondaryStatType::DamageReduction: KeyName = TEXT("DamageReduction"); break;
		case ET66SecondaryStatType::EvasionChance: KeyName = TEXT("EvasionChance"); break;
		case ET66SecondaryStatType::Alchemy: KeyName = TEXT("Alchemy"); break;
		case ET66SecondaryStatType::Accuracy: KeyName = TEXT("Accuracy"); break;
		default: continue;
		}
		SecObj->SetNumberField(KeyName, Pair.Value);
	}
	RunObj->SetObjectField(TEXT("secondary_stats"), SecObj);

	if (Snapshot->LuckRating0To100 >= 0) RunObj->SetNumberField(TEXT("luck_rating"), Snapshot->LuckRating0To100);
	if (Snapshot->SeedLuck0To100 >= 0) RunObj->SetNumberField(TEXT("seed_luck"), Snapshot->SeedLuck0To100);
	if (Snapshot->LuckModifierPercent > 0.f) RunObj->SetNumberField(TEXT("luck_modifier_percent"), Snapshot->LuckModifierPercent);
	if (Snapshot->EffectiveLuck > 0.f) RunObj->SetNumberField(TEXT("effective_luck"), Snapshot->EffectiveLuck);
	if (Snapshot->LuckRatingQuantity0To100 >= 0) RunObj->SetNumberField(TEXT("luck_quantity"), Snapshot->LuckRatingQuantity0To100);
	if (Snapshot->LuckRatingQuality0To100 >= 0) RunObj->SetNumberField(TEXT("luck_quality"), Snapshot->LuckRatingQuality0To100);
	if (Snapshot->SkillRating0To100 >= 0) RunObj->SetNumberField(TEXT("skill_rating"), Snapshot->SkillRating0To100);

	TSharedPtr<FJsonObject> AntiCheatObj = MakeShared<FJsonObject>();
	AntiCheatObj->SetNumberField(TEXT("run_seed"), Snapshot->RunSeed);
	AntiCheatObj->SetNumberField(TEXT("luck_quantity_sample_count"), Snapshot->LuckQuantitySampleCount);
	AntiCheatObj->SetNumberField(TEXT("luck_quality_sample_count"), Snapshot->LuckQualitySampleCount);
	if (Snapshot->LuckQuantityAccumulators.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> QuantityCategoryArr;
		for (const FT66SavedLuckAccumulator& Accumulator : Snapshot->LuckQuantityAccumulators)
		{
			TSharedPtr<FJsonObject> CategoryObj = MakeShared<FJsonObject>();
			CategoryObj->SetStringField(TEXT("category"), Accumulator.Category.ToString());
			CategoryObj->SetNumberField(TEXT("sum_01"), Accumulator.Sum01);
			CategoryObj->SetNumberField(TEXT("count"), Accumulator.Count);
			CategoryObj->SetNumberField(TEXT("avg_01"), Accumulator.Count > 0 ? (Accumulator.Sum01 / static_cast<float>(Accumulator.Count)) : 0.5f);
			QuantityCategoryArr.Add(MakeShared<FJsonValueObject>(CategoryObj));
		}
		AntiCheatObj->SetArrayField(TEXT("luck_quantity_categories"), QuantityCategoryArr);
	}
	if (Snapshot->LuckQualityAccumulators.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> QualityCategoryArr;
		for (const FT66SavedLuckAccumulator& Accumulator : Snapshot->LuckQualityAccumulators)
		{
			TSharedPtr<FJsonObject> CategoryObj = MakeShared<FJsonObject>();
			CategoryObj->SetStringField(TEXT("category"), Accumulator.Category.ToString());
			CategoryObj->SetNumberField(TEXT("sum_01"), Accumulator.Sum01);
			CategoryObj->SetNumberField(TEXT("count"), Accumulator.Count);
			CategoryObj->SetNumberField(TEXT("avg_01"), Accumulator.Count > 0 ? (Accumulator.Sum01 / static_cast<float>(Accumulator.Count)) : 0.5f);
			QualityCategoryArr.Add(MakeShared<FJsonValueObject>(CategoryObj));
		}
		AntiCheatObj->SetArrayField(TEXT("luck_quality_categories"), QualityCategoryArr);
	}
	if (Snapshot->AntiCheatLuckEvents.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> LuckEventArr;
		for (const FT66AntiCheatLuckEvent& Event : Snapshot->AntiCheatLuckEvents)
		{
			TSharedPtr<FJsonObject> EventObj = MakeShared<FJsonObject>();
			const TCHAR* EventTypeString = TEXT("quantity_roll");
			switch (Event.EventType)
			{
			case ET66AntiCheatLuckEventType::QuantityBool:
				EventTypeString = TEXT("quantity_bool");
				break;
			case ET66AntiCheatLuckEventType::QualityRarity:
				EventTypeString = TEXT("quality_rarity");
				break;
			default:
				break;
			}

			EventObj->SetStringField(TEXT("event_type"), EventTypeString);
			EventObj->SetStringField(TEXT("category"), Event.Category.ToString());
			EventObj->SetNumberField(TEXT("time_seconds"), Event.TimeSeconds);
			EventObj->SetNumberField(TEXT("value_01"), Event.Value01);
			EventObj->SetNumberField(TEXT("raw_value"), Event.RawValue);
			EventObj->SetNumberField(TEXT("raw_min"), Event.RawMin);
			EventObj->SetNumberField(TEXT("raw_max"), Event.RawMax);
			if (Event.RunDrawIndex != INDEX_NONE)
			{
				EventObj->SetNumberField(TEXT("draw_index"), Event.RunDrawIndex);
				EventObj->SetNumberField(TEXT("pre_draw_seed"), Event.PreDrawSeed);
			}
			if (Event.LuckStatSnapshot >= 0)
			{
				EventObj->SetNumberField(TEXT("luck_stat_snapshot"), Event.LuckStatSnapshot);
			}
			if (Event.Luck01Snapshot >= 0.f)
			{
				EventObj->SetNumberField(TEXT("luck_01_snapshot"), Event.Luck01Snapshot);
			}
			if (Event.ExpectedChance01 >= 0.f)
			{
				EventObj->SetNumberField(TEXT("expected_chance_01"), Event.ExpectedChance01);
			}
			if (Event.bHasRarityWeights)
			{
				EventObj->SetNumberField(TEXT("weight_black"), Event.RarityWeights.Black);
				EventObj->SetNumberField(TEXT("weight_red"), Event.RarityWeights.Red);
				EventObj->SetNumberField(TEXT("weight_yellow"), Event.RarityWeights.Yellow);
				EventObj->SetNumberField(TEXT("weight_white"), Event.RarityWeights.White);
			}
			if (Event.bHasFloatReplayRange)
			{
				EventObj->SetNumberField(TEXT("float_min"), Event.FloatReplayMin);
				EventObj->SetNumberField(TEXT("float_max"), Event.FloatReplayMax);
			}
			LuckEventArr.Add(MakeShared<FJsonValueObject>(EventObj));
		}
		AntiCheatObj->SetArrayField(TEXT("luck_events"), LuckEventArr);
	}
	AntiCheatObj->SetBoolField(TEXT("luck_events_truncated"), Snapshot->bAntiCheatLuckEventsTruncated);
	if (Snapshot->AntiCheatHitCheckEvents.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> HitCheckEventArr;
		for (const FT66AntiCheatHitCheckEvent& Event : Snapshot->AntiCheatHitCheckEvents)
		{
			TSharedPtr<FJsonObject> EventObj = MakeShared<FJsonObject>();
			EventObj->SetNumberField(TEXT("time_seconds"), Event.TimeSeconds);
			EventObj->SetNumberField(TEXT("evasion_chance_01"), Event.EvasionChance01);
			EventObj->SetBoolField(TEXT("dodged"), Event.bDodged);
			EventObj->SetBoolField(TEXT("damage_applied"), Event.bDamageApplied);
			HitCheckEventArr.Add(MakeShared<FJsonValueObject>(EventObj));
		}
		AntiCheatObj->SetArrayField(TEXT("hit_check_events"), HitCheckEventArr);
	}
	AntiCheatObj->SetBoolField(TEXT("hit_check_events_truncated"), Snapshot->bAntiCheatHitCheckEventsTruncated);
	AntiCheatObj->SetNumberField(TEXT("incoming_hit_checks"), Snapshot->IncomingHitChecks);
	AntiCheatObj->SetNumberField(TEXT("damage_taken_hit_count"), Snapshot->DamageTakenHitCount);
	AntiCheatObj->SetNumberField(TEXT("dodge_count"), Snapshot->DodgeCount);
	AntiCheatObj->SetNumberField(TEXT("max_consecutive_dodges"), Snapshot->MaxConsecutiveDodges);
	AntiCheatObj->SetNumberField(TEXT("total_evasion_chance"), Snapshot->TotalEvasionChance);
	if (Snapshot->AntiCheatEvasionBuckets.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> EvasionBucketArr;
		for (const FT66AntiCheatEvasionBucketSummary& Bucket : Snapshot->AntiCheatEvasionBuckets)
		{
			TSharedPtr<FJsonObject> BucketObj = MakeShared<FJsonObject>();
			BucketObj->SetNumberField(TEXT("bucket_index"), Bucket.BucketIndex);
			BucketObj->SetNumberField(TEXT("min_chance_01"), Bucket.MinChance01);
			BucketObj->SetNumberField(TEXT("max_chance_01"), Bucket.MaxChance01);
			BucketObj->SetNumberField(TEXT("hit_checks"), Bucket.HitChecks);
			BucketObj->SetNumberField(TEXT("dodges"), Bucket.Dodges);
			BucketObj->SetNumberField(TEXT("damage_applied"), Bucket.DamageApplied);
			BucketObj->SetNumberField(TEXT("expected_dodges"), Bucket.ExpectedDodges);
			EvasionBucketArr.Add(MakeShared<FJsonValueObject>(BucketObj));
		}
		AntiCheatObj->SetArrayField(TEXT("evasion_buckets"), EvasionBucketArr);
	}
	{
		TSharedPtr<FJsonObject> PressureObj = MakeShared<FJsonObject>();
		PressureObj->SetNumberField(TEXT("window_seconds"), Snapshot->AntiCheatPressureWindowSummary.WindowSeconds);
		PressureObj->SetNumberField(TEXT("active_windows"), Snapshot->AntiCheatPressureWindowSummary.ActiveWindows);
		PressureObj->SetNumberField(TEXT("pressured_windows_4_plus"), Snapshot->AntiCheatPressureWindowSummary.PressuredWindows4Plus);
		PressureObj->SetNumberField(TEXT("pressured_windows_8_plus"), Snapshot->AntiCheatPressureWindowSummary.PressuredWindows8Plus);
		PressureObj->SetNumberField(TEXT("zero_damage_windows_4_plus"), Snapshot->AntiCheatPressureWindowSummary.ZeroDamageWindows4Plus);
		PressureObj->SetNumberField(TEXT("zero_damage_windows_8_plus"), Snapshot->AntiCheatPressureWindowSummary.ZeroDamageWindows8Plus);
		PressureObj->SetNumberField(TEXT("near_perfect_windows_8_plus"), Snapshot->AntiCheatPressureWindowSummary.NearPerfectWindows8Plus);
		PressureObj->SetNumberField(TEXT("max_hit_checks_in_window"), Snapshot->AntiCheatPressureWindowSummary.MaxHitChecksInWindow);
		PressureObj->SetNumberField(TEXT("max_dodges_in_window"), Snapshot->AntiCheatPressureWindowSummary.MaxDodgesInWindow);
		PressureObj->SetNumberField(TEXT("max_damage_applied_in_window"), Snapshot->AntiCheatPressureWindowSummary.MaxDamageAppliedInWindow);
		PressureObj->SetNumberField(TEXT("max_expected_dodges_in_window"), Snapshot->AntiCheatPressureWindowSummary.MaxExpectedDodgesInWindow);
		PressureObj->SetNumberField(TEXT("total_hit_checks"), Snapshot->AntiCheatPressureWindowSummary.TotalHitChecks);
		PressureObj->SetNumberField(TEXT("total_dodges"), Snapshot->AntiCheatPressureWindowSummary.TotalDodges);
		PressureObj->SetNumberField(TEXT("total_damage_applied"), Snapshot->AntiCheatPressureWindowSummary.TotalDamageApplied);
		PressureObj->SetNumberField(TEXT("total_expected_dodges"), Snapshot->AntiCheatPressureWindowSummary.TotalExpectedDodges);
		AntiCheatObj->SetObjectField(TEXT("pressure_window_summary"), PressureObj);
	}
	if (Snapshot->AntiCheatGamblerSummaries.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> GamblerSummaryArr;
		for (const FT66AntiCheatGamblerGameSummary& Summary : Snapshot->AntiCheatGamblerSummaries)
		{
			const TCHAR* GameTypeString = TEXT("coin_flip");
			switch (Summary.GameType)
			{
			case ET66AntiCheatGamblerGameType::RockPaperScissors: GameTypeString = TEXT("rps"); break;
			case ET66AntiCheatGamblerGameType::BlackJack: GameTypeString = TEXT("blackjack"); break;
			case ET66AntiCheatGamblerGameType::Lottery: GameTypeString = TEXT("lottery"); break;
			case ET66AntiCheatGamblerGameType::Plinko: GameTypeString = TEXT("plinko"); break;
			case ET66AntiCheatGamblerGameType::BoxOpening: GameTypeString = TEXT("box_opening"); break;
			default: break;
			}

			TSharedPtr<FJsonObject> SummaryObj = MakeShared<FJsonObject>();
			SummaryObj->SetStringField(TEXT("game_type"), GameTypeString);
			SummaryObj->SetNumberField(TEXT("rounds"), Summary.Rounds);
			SummaryObj->SetNumberField(TEXT("wins"), Summary.Wins);
			SummaryObj->SetNumberField(TEXT("losses"), Summary.Losses);
			SummaryObj->SetNumberField(TEXT("draws"), Summary.Draws);
			SummaryObj->SetNumberField(TEXT("cheat_attempts"), Summary.CheatAttempts);
			SummaryObj->SetNumberField(TEXT("cheat_successes"), Summary.CheatSuccesses);
			SummaryObj->SetNumberField(TEXT("total_bet_gold"), Summary.TotalBetGold);
			SummaryObj->SetNumberField(TEXT("total_payout_gold"), Summary.TotalPayoutGold);
			GamblerSummaryArr.Add(MakeShared<FJsonValueObject>(SummaryObj));
		}
		AntiCheatObj->SetArrayField(TEXT("gambler_summary"), GamblerSummaryArr);
	}
	if (Snapshot->AntiCheatGamblerEvents.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> GamblerEventArr;
		for (const FT66AntiCheatGamblerEvent& Event : Snapshot->AntiCheatGamblerEvents)
		{
			const TCHAR* GameTypeString = TEXT("coin_flip");
			switch (Event.GameType)
			{
			case ET66AntiCheatGamblerGameType::RockPaperScissors: GameTypeString = TEXT("rps"); break;
			case ET66AntiCheatGamblerGameType::BlackJack: GameTypeString = TEXT("blackjack"); break;
			case ET66AntiCheatGamblerGameType::Lottery: GameTypeString = TEXT("lottery"); break;
			case ET66AntiCheatGamblerGameType::Plinko: GameTypeString = TEXT("plinko"); break;
			case ET66AntiCheatGamblerGameType::BoxOpening: GameTypeString = TEXT("box_opening"); break;
			default: break;
			}

			TSharedPtr<FJsonObject> EventObj = MakeShared<FJsonObject>();
			EventObj->SetStringField(TEXT("game_type"), GameTypeString);
			EventObj->SetNumberField(TEXT("time_seconds"), Event.TimeSeconds);
			EventObj->SetNumberField(TEXT("bet_gold"), Event.BetGold);
			EventObj->SetNumberField(TEXT("payout_gold"), Event.PayoutGold);
			EventObj->SetBoolField(TEXT("cheat_attempted"), Event.bCheatAttempted);
			EventObj->SetBoolField(TEXT("cheat_succeeded"), Event.bCheatSucceeded);
			EventObj->SetBoolField(TEXT("win"), Event.bWin);
			EventObj->SetBoolField(TEXT("draw"), Event.bDraw);
			if (Event.PlayerChoice != INDEX_NONE) EventObj->SetNumberField(TEXT("player_choice"), Event.PlayerChoice);
			if (Event.OpponentChoice != INDEX_NONE) EventObj->SetNumberField(TEXT("opponent_choice"), Event.OpponentChoice);
			EventObj->SetNumberField(TEXT("outcome_value"), Event.OutcomeValue);
			EventObj->SetNumberField(TEXT("outcome_value_secondary"), Event.OutcomeSecondaryValue);
			if (Event.SelectedMask != 0) EventObj->SetNumberField(TEXT("selected_mask"), Event.SelectedMask);
			if (Event.ResolvedMask != 0) EventObj->SetNumberField(TEXT("resolved_mask"), Event.ResolvedMask);
			EventObj->SetNumberField(TEXT("path_bits"), Event.PathBits);
			if (Event.ShuffleStartDrawIndex != INDEX_NONE)
			{
				EventObj->SetNumberField(TEXT("shuffle_pre_draw_seed"), Event.ShufflePreDrawSeed);
				EventObj->SetNumberField(TEXT("shuffle_start_draw_index"), Event.ShuffleStartDrawIndex);
			}
			if (Event.OutcomeDrawIndex != INDEX_NONE)
			{
				EventObj->SetNumberField(TEXT("outcome_pre_draw_seed"), Event.OutcomePreDrawSeed);
				EventObj->SetNumberField(TEXT("outcome_draw_index"), Event.OutcomeDrawIndex);
			}
			if (Event.OutcomeExpectedChance01 >= 0.f)
			{
				EventObj->SetNumberField(TEXT("outcome_expected_chance_01"), Event.OutcomeExpectedChance01);
			}
			if (!Event.ActionSequence.IsEmpty())
			{
				EventObj->SetStringField(TEXT("action_sequence"), Event.ActionSequence);
			}
			GamblerEventArr.Add(MakeShared<FJsonValueObject>(EventObj));
		}
		AntiCheatObj->SetArrayField(TEXT("gambler_events"), GamblerEventArr);
	}
	AntiCheatObj->SetBoolField(TEXT("gambler_events_truncated"), Snapshot->bAntiCheatGamblerEventsTruncated);
	RunObj->SetObjectField(TEXT("anti_cheat_context"), AntiCheatObj);

	TSharedPtr<FJsonObject> IntegrityObj = MakeShared<FJsonObject>();
	IntegrityObj->SetStringField(TEXT("verdict"), Snapshot->IntegrityContext.Verdict);
	IntegrityObj->SetStringField(TEXT("steam_app_id"), Snapshot->IntegrityContext.SteamAppId);
	IntegrityObj->SetNumberField(TEXT("steam_build_id"), Snapshot->IntegrityContext.SteamBuildId);
	if (!Snapshot->IntegrityContext.SteamBetaName.IsEmpty())
	{
		IntegrityObj->SetStringField(TEXT("steam_beta_name"), Snapshot->IntegrityContext.SteamBetaName);
	}
	if (!Snapshot->IntegrityContext.ManifestId.IsEmpty())
	{
		IntegrityObj->SetStringField(TEXT("manifest_id"), Snapshot->IntegrityContext.ManifestId);
	}
	if (!Snapshot->IntegrityContext.ManifestRootHash.IsEmpty())
	{
		IntegrityObj->SetStringField(TEXT("manifest_root_hash"), Snapshot->IntegrityContext.ManifestRootHash);
	}
	if (!Snapshot->IntegrityContext.ModuleListHash.IsEmpty())
	{
		IntegrityObj->SetStringField(TEXT("module_list_hash"), Snapshot->IntegrityContext.ModuleListHash);
	}
	if (!Snapshot->IntegrityContext.MountedContentHash.IsEmpty())
	{
		IntegrityObj->SetStringField(TEXT("mounted_content_hash"), Snapshot->IntegrityContext.MountedContentHash);
	}
	if (!Snapshot->IntegrityContext.BaselineHash.IsEmpty())
	{
		IntegrityObj->SetStringField(TEXT("baseline_hash"), Snapshot->IntegrityContext.BaselineHash);
	}
	if (!Snapshot->IntegrityContext.FinalHash.IsEmpty())
	{
		IntegrityObj->SetStringField(TEXT("final_hash"), Snapshot->IntegrityContext.FinalHash);
	}
	if (Snapshot->IntegrityContext.Reasons.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> ReasonArr;
		for (const FString& Reason : Snapshot->IntegrityContext.Reasons)
		{
			ReasonArr.Add(MakeShared<FJsonValueString>(Reason));
		}
		IntegrityObj->SetArrayField(TEXT("reasons"), ReasonArr);
	}
	RunObj->SetObjectField(TEXT("integrity_context"), IntegrityObj);

	TSharedPtr<FJsonObject> ScoreBudgetObj = MakeShared<FJsonObject>();
	ScoreBudgetObj->SetNumberField(TEXT("enemy_score_budget"), Snapshot->ScoreBudgetContext.EnemyScoreBudget);
	ScoreBudgetObj->SetNumberField(TEXT("boss_score_budget"), Snapshot->ScoreBudgetContext.BossScoreBudget);
	ScoreBudgetObj->SetNumberField(TEXT("enemy_score_awarded"), Snapshot->ScoreBudgetContext.EnemyScoreAwarded);
	ScoreBudgetObj->SetNumberField(TEXT("boss_score_awarded"), Snapshot->ScoreBudgetContext.BossScoreAwarded);
	ScoreBudgetObj->SetNumberField(TEXT("registered_enemy_spawns"), Snapshot->ScoreBudgetContext.RegisteredEnemySpawns);
	ScoreBudgetObj->SetNumberField(TEXT("registered_boss_spawns"), Snapshot->ScoreBudgetContext.RegisteredBossSpawns);
	ScoreBudgetObj->SetNumberField(TEXT("peak_score_per_minute"), Snapshot->ScoreBudgetContext.GetPeakAwardedScorePerMinute());
	ScoreBudgetObj->SetBoolField(TEXT("exceeded_legal_score"), Snapshot->ScoreBudgetContext.bExceededLegalScore);
	ScoreBudgetObj->SetNumberField(TEXT("first_exceeded_at_stage"), Snapshot->ScoreBudgetContext.FirstExceededAtStage);
	ScoreBudgetObj->SetNumberField(TEXT("first_exceeded_at_run_seconds"), Snapshot->ScoreBudgetContext.FirstExceededAtRunSeconds);
	if (Snapshot->ScoreBudgetContext.AwardedScorePerMinute.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> ScorePerMinuteArr;
		for (const int32 ScoreValue : Snapshot->ScoreBudgetContext.AwardedScorePerMinute)
		{
			ScorePerMinuteArr.Add(MakeShared<FJsonValueNumber>(ScoreValue));
		}
		ScoreBudgetObj->SetArrayField(TEXT("awarded_score_per_minute"), ScorePerMinuteArr);
	}
	if (Snapshot->ScoreBudgetContext.AwardedScorePerStage.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> ScorePerStageArr;
		for (const int32 ScoreValue : Snapshot->ScoreBudgetContext.AwardedScorePerStage)
		{
			ScorePerStageArr.Add(MakeShared<FJsonValueNumber>(ScoreValue));
		}
		ScoreBudgetObj->SetArrayField(TEXT("awarded_score_per_stage"), ScorePerStageArr);
	}
	if (!Snapshot->ScoreBudgetContext.FailureReason.IsEmpty())
	{
		ScoreBudgetObj->SetStringField(TEXT("failure_reason"), Snapshot->ScoreBudgetContext.FailureReason);
	}
	RunObj->SetObjectField(TEXT("score_budget_context"), ScoreBudgetObj);

	TArray<TSharedPtr<FJsonValue>> IdolArr;
	for (const FName& Idol : Snapshot->EquippedIdols)
	{
		IdolArr.Add(MakeShared<FJsonValueString>(Idol.ToString()));
	}
	RunObj->SetArrayField(TEXT("equipped_idols"), IdolArr);

	TArray<TSharedPtr<FJsonValue>> InvArr;
	for (const FName& Item : Snapshot->Inventory)
	{
		InvArr.Add(MakeShared<FJsonValueString>(Item.ToString()));
	}
	RunObj->SetArrayField(TEXT("inventory"), InvArr);

	TArray<TSharedPtr<FJsonValue>> LogArr;
	for (const FString& Msg : Snapshot->EventLog)
	{
		LogArr.Add(MakeShared<FJsonValueString>(Msg));
	}
	RunObj->SetArrayField(TEXT("event_log"), LogArr);

	if (Snapshot->StagePacingPoints.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> StageSplitArr;
		for (const FT66StagePacingPoint& Point : Snapshot->StagePacingPoints)
		{
			const int32 SplitMs = FMath::RoundToInt(FMath::Max(0.f, Point.ElapsedSeconds) * 1000.f);
			StageSplitArr.Add(MakeShared<FJsonValueNumber>(SplitMs));
		}
		RunObj->SetArrayField(TEXT("stage_splits_ms"), StageSplitArr);
	}

	TSharedPtr<FJsonObject> DmgObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Snapshot->DamageBySource)
	{
		DmgObj->SetNumberField(Pair.Key.ToString(), Pair.Value);
	}
	RunObj->SetObjectField(TEXT("damage_by_source"), DmgObj);

	return RunObj;
}

bool T66BackendRunSerializer::SerializeJsonObjectToString(const TSharedPtr<FJsonObject>& Json, FString& OutJsonString)
{
	if (!Json.IsValid())
	{
		return false;
	}

	OutJsonString.Reset();
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonString);
	return FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);
}

bool T66BackendRunSerializer::DeserializeJsonObjectString(const FString& JsonString, TSharedPtr<FJsonObject>& OutJson)
{
	OutJson.Reset();
	if (JsonString.IsEmpty())
	{
		return false;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	return FJsonSerializer::Deserialize(Reader, OutJson) && OutJson.IsValid();
}
