// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Backend/T66BackendRunSummaryParser.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66SaveMigration.h"
#include "Dom/JsonObject.h"

namespace
{
	int32 T66RunSummaryGetJsonIntOrDefault(const TSharedPtr<FJsonObject>& Json, const FString& FieldName, int32 DefaultValue)
	{
		double Value = 0.0;
		return Json.IsValid() && Json->TryGetNumberField(FieldName, Value) ? static_cast<int32>(Value) : DefaultValue;
	}

	float T66RunSummaryGetJsonFloatOrDefault(const TSharedPtr<FJsonObject>& Json, const FString& FieldName, float DefaultValue)
	{
		double Value = 0.0;
		return Json.IsValid() && Json->TryGetNumberField(FieldName, Value) ? static_cast<float>(Value) : DefaultValue;
	}

	FString T66RunSummaryGetJsonStringOrDefault(const TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FString& DefaultValue = FString())
	{
		FString Value;
		return Json.IsValid() && Json->TryGetStringField(FieldName, Value) ? Value : DefaultValue;
	}

	bool T66RunSummaryGetJsonBoolOrDefault(const TSharedPtr<FJsonObject>& Json, const FString& FieldName, bool bDefaultValue)
	{
		bool bValue = false;
		return Json.IsValid() && Json->TryGetBoolField(FieldName, bValue) ? bValue : bDefaultValue;
	}

	ET66AttackCategory T66RunSummaryParseAttackCategory(const FString& Value, ET66AttackCategory DefaultValue = ET66AttackCategory::Pierce)
	{
		if (Value.Equals(TEXT("aoe"), ESearchCase::IgnoreCase)) return ET66AttackCategory::AOE;
		if (Value.Equals(TEXT("bounce"), ESearchCase::IgnoreCase)) return ET66AttackCategory::Bounce;
		if (Value.Equals(TEXT("dot"), ESearchCase::IgnoreCase)) return ET66AttackCategory::DOT;
		if (Value.Equals(TEXT("pierce"), ESearchCase::IgnoreCase)) return ET66AttackCategory::Pierce;
		return DefaultValue;
	}

	ET66IdolElement T66RunSummaryParseIdolElement(const FString& Value, ET66IdolElement DefaultValue = ET66IdolElement::Fire)
	{
		if (Value.Equals(TEXT("ice"), ESearchCase::IgnoreCase)) return ET66IdolElement::Ice;
		if (Value.Equals(TEXT("electricity"), ESearchCase::IgnoreCase)) return ET66IdolElement::Electricity;
		if (Value.Equals(TEXT("nature"), ESearchCase::IgnoreCase)) return ET66IdolElement::Nature;
		if (Value.Equals(TEXT("fire"), ESearchCase::IgnoreCase)) return ET66IdolElement::Fire;
		return DefaultValue;
	}

	ET66ItemRarity T66RunSummaryParseItemRarity(const FString& Value, ET66ItemRarity DefaultValue = ET66ItemRarity::Black)
	{
		if (Value.Equals(TEXT("red"), ESearchCase::IgnoreCase)) return ET66ItemRarity::Red;
		if (Value.Equals(TEXT("yellow"), ESearchCase::IgnoreCase)) return ET66ItemRarity::Yellow;
		if (Value.Equals(TEXT("white"), ESearchCase::IgnoreCase)) return ET66ItemRarity::White;
		if (Value.Equals(TEXT("black"), ESearchCase::IgnoreCase)) return ET66ItemRarity::Black;
		return DefaultValue;
	}

	ET66WeaponRarity T66RunSummaryParseWeaponRarity(const FString& Value, ET66WeaponRarity DefaultValue = ET66WeaponRarity::Black)
	{
		if (Value.Equals(TEXT("red"), ESearchCase::IgnoreCase)) return ET66WeaponRarity::Red;
		if (Value.Equals(TEXT("yellow"), ESearchCase::IgnoreCase)) return ET66WeaponRarity::Yellow;
		if (Value.Equals(TEXT("white"), ESearchCase::IgnoreCase)) return ET66WeaponRarity::White;
		if (Value.Equals(TEXT("black"), ESearchCase::IgnoreCase)) return ET66WeaponRarity::Black;
		return DefaultValue;
	}

	ET66AntiCheatGamblerGameType T66RunSummaryParseGamblerGameType(const FString& Value)
	{
		if (Value.Equals(TEXT("guess_the_cup"), ESearchCase::IgnoreCase))
		{
			return ET66AntiCheatGamblerGameType::GuessTheCup;
		}
		if (Value.Equals(TEXT("pick_longest_shortest_stick"), ESearchCase::IgnoreCase))
		{
			return ET66AntiCheatGamblerGameType::PickLongestShortestStick;
		}
		if (Value.Equals(TEXT("find_joker"), ESearchCase::IgnoreCase)) return ET66AntiCheatGamblerGameType::FindJoker;
		return ET66AntiCheatGamblerGameType::CoinFlip;
	}

	ET66HitZoneType T66RunSummaryParseHitZone(const FString& Value, ET66HitZoneType DefaultValue = ET66HitZoneType::Body)
	{
		if (Value.Equals(TEXT("none"), ESearchCase::IgnoreCase)) return ET66HitZoneType::None;
		if (Value.Equals(TEXT("head"), ESearchCase::IgnoreCase)) return ET66HitZoneType::Head;
		if (Value.Equals(TEXT("core"), ESearchCase::IgnoreCase)) return ET66HitZoneType::Core;
		if (Value.Equals(TEXT("weak_point"), ESearchCase::IgnoreCase)) return ET66HitZoneType::WeakPoint;
		if (Value.Equals(TEXT("left_arm"), ESearchCase::IgnoreCase)) return ET66HitZoneType::LeftArm;
		if (Value.Equals(TEXT("right_arm"), ESearchCase::IgnoreCase)) return ET66HitZoneType::RightArm;
		if (Value.Equals(TEXT("left_leg"), ESearchCase::IgnoreCase)) return ET66HitZoneType::LeftLeg;
		if (Value.Equals(TEXT("right_leg"), ESearchCase::IgnoreCase)) return ET66HitZoneType::RightLeg;
		if (Value.Equals(TEXT("body"), ESearchCase::IgnoreCase)) return ET66HitZoneType::Body;
		return DefaultValue;
	}
}

UT66LeaderboardRunSummarySaveGame* T66BackendRunSummaryParser::Parse(const TSharedPtr<FJsonObject>& Json, UObject* Outer)
{
	if (!Json.IsValid())
	{
		return nullptr;
	}

	UT66LeaderboardRunSummarySaveGame* S = NewObject<UT66LeaderboardRunSummarySaveGame>(Outer);

	S->SchemaVersion = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("schema_version"), 6);
	S->EntryId = T66RunSummaryGetJsonStringOrDefault(Json, TEXT("entry_id"));
	S->OwnerSteamId = T66RunSummaryGetJsonStringOrDefault(Json, TEXT("steam_id"));
	S->OwnerDisplayName = T66RunSummaryGetJsonStringOrDefault(Json, TEXT("display_name"));
	S->DisplayName = S->OwnerDisplayName;
	S->StageReached = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("stage_reached"), 1);
	S->Score = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("score"), 0);
	S->ScoreRankAllTime = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("score_rank_alltime"), 0);
	S->ScoreRankWeekly = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("score_rank_weekly"), 0);
	S->SpeedRunRankAllTime = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("speedrun_rank_alltime"), 0);
	S->SpeedRunRankWeekly = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("speedrun_rank_weekly"), 0);
	S->RunDurationSeconds = T66RunSummaryGetJsonFloatOrDefault(Json, TEXT("time_seconds"), 0.f);

	const FString HeroIdStr = T66RunSummaryGetJsonStringOrDefault(Json, TEXT("hero_id"));
	S->HeroID = HeroIdStr.IsEmpty() ? NAME_None : FName(*HeroIdStr);

	const FString CompanionIdStr = T66RunSummaryGetJsonStringOrDefault(Json, TEXT("companion_id"));
	S->CompanionID = CompanionIdStr.IsEmpty() ? NAME_None : FName(*CompanionIdStr);

	S->HeroLevel = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("hero_level"), 1);
	S->HeroMasteryLevel = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("hero_mastery_level"), 1);
	S->HeroMasteryXP = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("hero_mastery_xp"), 0);

	const TSharedPtr<FJsonObject>* IntegrityObj = nullptr;
	if (Json->TryGetObjectField(TEXT("integrity_context"), IntegrityObj) && IntegrityObj && (*IntegrityObj).IsValid())
	{
		const TSharedPtr<FJsonObject>& Integrity = *IntegrityObj;
		S->IntegrityContext.Verdict = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("verdict"), TEXT("unknown"));
		S->IntegrityContext.SteamAppId = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("steam_app_id"));
		S->IntegrityContext.SteamBuildId = T66RunSummaryGetJsonIntOrDefault(Integrity, TEXT("steam_build_id"), 0);
		S->IntegrityContext.SteamBetaName = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("steam_beta_name"));
		S->IntegrityContext.ManifestId = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("manifest_id"));
		S->IntegrityContext.ManifestRootHash = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("manifest_root_hash"));
		S->IntegrityContext.ModuleListHash = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("module_list_hash"));
		S->IntegrityContext.MountedContentHash = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("mounted_content_hash"));
		S->IntegrityContext.BaselineHash = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("baseline_hash"));
		S->IntegrityContext.FinalHash = T66RunSummaryGetJsonStringOrDefault(Integrity, TEXT("final_hash"));

		const TArray<TSharedPtr<FJsonValue>>* ReasonArr = nullptr;
		if (Integrity->TryGetArrayField(TEXT("reasons"), ReasonArr) && ReasonArr)
		{
			for (const TSharedPtr<FJsonValue>& ReasonValue : *ReasonArr)
			{
				FString ReasonStr;
				if (ReasonValue.IsValid() && ReasonValue->TryGetString(ReasonStr))
				{
					S->IntegrityContext.Reasons.Add(ReasonStr);
				}
			}
		}
	}

	const TSharedPtr<FJsonObject>* StatsObj = nullptr;
	if (Json->TryGetObjectField(TEXT("stats"), StatsObj) && StatsObj && (*StatsObj).IsValid())
	{
		const TSharedPtr<FJsonObject>& St = *StatsObj;
		S->DamageStat = T66RunSummaryGetJsonIntOrDefault(St, TEXT("damage"), 1);
		S->AttackSpeedStat = T66RunSummaryGetJsonIntOrDefault(St, TEXT("attack_speed"), 1);
		S->AttackScaleStat = T66RunSummaryGetJsonIntOrDefault(St, TEXT("attack_scale"), 1);
		S->AccuracyStat = T66RunSummaryGetJsonIntOrDefault(St, TEXT("accuracy"), 1);
		S->ArmorStat = T66RunSummaryGetJsonIntOrDefault(St, TEXT("armor"), 1);
		S->EvasionStat = T66RunSummaryGetJsonIntOrDefault(St, TEXT("evasion"), 1);
		S->LuckStat = T66RunSummaryGetJsonIntOrDefault(St, TEXT("luck"), 1);
		S->SpeedStat = T66RunSummaryGetJsonIntOrDefault(St, TEXT("speed"), 1);
	}

	const TSharedPtr<FJsonObject>* SecObj = nullptr;
	if (Json->TryGetObjectField(TEXT("secondary_stats"), SecObj) && SecObj && (*SecObj).IsValid())
	{
		for (const auto& Pair : (*SecObj)->Values)
		{
			double Val = 0.0;
			if (Pair.Value.IsValid() && Pair.Value->TryGetNumber(Val))
			{
				const FString& Key = Pair.Key;
				ET66SecondaryStatType StatType = ET66SecondaryStatType::None;
				bool bFound = true;
				bool bLegacyCritDamageKey = false;

				if (Key == TEXT("CritChance")) StatType = ET66SecondaryStatType::CritChance;
				else if (Key == TEXT("HeadshotChance")) StatType = ET66SecondaryStatType::HeadshotChance;
				else if (Key == TEXT("CritDamage"))
				{
					StatType = ET66SecondaryStatType::HeadshotChance;
					bLegacyCritDamageKey = true;
				}
				else if (Key == TEXT("Crush")) StatType = ET66SecondaryStatType::Crush;
				else if (Key == TEXT("Invisibility")) StatType = ET66SecondaryStatType::Invisibility;
				else if (Key == TEXT("LifeSteal")) StatType = ET66SecondaryStatType::LifeSteal;
				else if (Key == TEXT("Assassinate")) StatType = ET66SecondaryStatType::Assassinate;
				else if (Key == TEXT("Cheating")) StatType = ET66SecondaryStatType::Cheating;
				else if (Key == TEXT("Stealing")) StatType = ET66SecondaryStatType::Stealing;
				else if (Key == TEXT("AoeDamage")) StatType = ET66SecondaryStatType::AoeDamage;
				else if (Key == TEXT("BounceDamage")) StatType = ET66SecondaryStatType::BounceDamage;
				else if (Key == TEXT("PierceDamage")) StatType = ET66SecondaryStatType::PierceDamage;
				else if (Key == TEXT("DotDamage")) StatType = ET66SecondaryStatType::DotDamage;
				else if (Key == TEXT("CloseRangeDamage")) StatType = ET66SecondaryStatType::CloseRangeDamage;
				else if (Key == TEXT("LongRangeDamage")) StatType = ET66SecondaryStatType::LongRangeDamage;
				else if (Key == TEXT("AoeSpeed")) StatType = ET66SecondaryStatType::AoeSpeed;
				else if (Key == TEXT("BounceSpeed")) StatType = ET66SecondaryStatType::BounceSpeed;
				else if (Key == TEXT("PierceSpeed")) StatType = ET66SecondaryStatType::PierceSpeed;
				else if (Key == TEXT("DotSpeed")) StatType = ET66SecondaryStatType::DotSpeed;
				else if (Key == TEXT("AoeScale")) StatType = ET66SecondaryStatType::AoeScale;
				else if (Key == TEXT("BounceScale")) StatType = ET66SecondaryStatType::BounceScale;
				else if (Key == TEXT("PierceScale")) StatType = ET66SecondaryStatType::PierceScale;
				else if (Key == TEXT("DotScale")) StatType = ET66SecondaryStatType::DotScale;
				else if (Key == TEXT("AttackRange")) StatType = ET66SecondaryStatType::AttackRange;
				else if (Key == TEXT("Taunt")) StatType = ET66SecondaryStatType::Taunt;
				else if (Key == TEXT("ReflectDamage")) StatType = ET66SecondaryStatType::ReflectDamage;
				else if (Key == TEXT("CounterAttack")) StatType = ET66SecondaryStatType::CounterAttack;
				else if (Key == TEXT("HpRegen")) StatType = ET66SecondaryStatType::HpRegen;
				else if (Key == TEXT("SpinWheel")) StatType = ET66SecondaryStatType::SpinWheel;
				else if (Key == TEXT("Goblin")) StatType = ET66SecondaryStatType::Goblin;
				else if (Key == TEXT("Leprechaun")) StatType = ET66SecondaryStatType::Leprechaun;
				else if (Key == TEXT("TreasureChest")) StatType = ET66SecondaryStatType::TreasureChest;
				else if (Key == TEXT("Fountain")) StatType = ET66SecondaryStatType::Fountain;
				else if (Key == TEXT("MovementSpeed")) StatType = ET66SecondaryStatType::MovementSpeed;
				else if (Key == TEXT("LootCrate")) StatType = ET66SecondaryStatType::LootCrate;
				else if (Key == TEXT("DamageReduction")) StatType = ET66SecondaryStatType::DamageReduction;
				else if (Key == TEXT("EvasionChance")) StatType = ET66SecondaryStatType::EvasionChance;
				else if (Key == TEXT("Alchemy")) StatType = ET66SecondaryStatType::Alchemy;
				else if (Key == TEXT("Accuracy")) StatType = ET66SecondaryStatType::Accuracy;
				else if (Key == TEXT("Execute")) StatType = ET66SecondaryStatType::Execute;
				else if (Key == TEXT("LootBag")) StatType = ET66SecondaryStatType::LootBag;
				else if (Key == TEXT("LootWheel")) StatType = ET66SecondaryStatType::LootWheel;
				else if (Key == TEXT("VendorToken")) StatType = ET66SecondaryStatType::VendorToken;
				else if (Key == TEXT("FirePower")) StatType = ET66SecondaryStatType::FirePower;
				else if (Key == TEXT("IcePower")) StatType = ET66SecondaryStatType::IcePower;
				else if (Key == TEXT("ElectricityPower")) StatType = ET66SecondaryStatType::ElectricityPower;
				else if (Key == TEXT("NaturePower")) StatType = ET66SecondaryStatType::NaturePower;
				else if (Key == TEXT("InteractableLuck")) StatType = ET66SecondaryStatType::InteractableLuck;
				else if (Key == TEXT("StealingLuck")) StatType = ET66SecondaryStatType::StealingLuck;
				else if (Key == TEXT("GamblingLuck")) StatType = ET66SecondaryStatType::GamblingLuck;
				else if (Key == TEXT("ProcLuck")) StatType = ET66SecondaryStatType::ProcLuck;
				else { bFound = false; }

				if (bFound)
				{
					float ParsedValue = static_cast<float>(Val);
					if (StatType == ET66SecondaryStatType::HeadshotChance)
					{
						if (bLegacyCritDamageKey && (ParsedValue < 0.f || ParsedValue > 1.f))
						{
							continue;
						}
						ParsedValue = FMath::Clamp(ParsedValue, 0.f, 1.f);
					}
					S->SecondaryStatValues.Add(StatType, ParsedValue);
				}
			}
		}
	}

	S->LuckRating0To100 = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("luck_rating"), -1);
	S->LuckRatingQuantity0To100 = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("luck_quantity"), -1);
	S->LuckRatingQuality0To100 = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("luck_quality"), -1);
	S->SkillRating0To100 = T66RunSummaryGetJsonIntOrDefault(Json, TEXT("skill_rating"), -1);

	const TSharedPtr<FJsonObject>* AntiCheatObj = nullptr;
	if (Json->TryGetObjectField(TEXT("anti_cheat_context"), AntiCheatObj) && AntiCheatObj && (*AntiCheatObj).IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* GamblerSummaryArr = nullptr;
		if ((*AntiCheatObj)->TryGetArrayField(TEXT("gambler_summary"), GamblerSummaryArr) && GamblerSummaryArr)
		{
			for (const TSharedPtr<FJsonValue>& SummaryValue : *GamblerSummaryArr)
			{
				const TSharedPtr<FJsonObject>* SummaryObj = nullptr;
				if (!SummaryValue.IsValid() || !SummaryValue->TryGetObject(SummaryObj) || !SummaryObj || !(*SummaryObj).IsValid())
				{
					continue;
				}

				FT66AntiCheatGamblerGameSummary& Summary = S->AntiCheatGamblerSummaries.AddDefaulted_GetRef();
				Summary.GameType = T66RunSummaryParseGamblerGameType(T66RunSummaryGetJsonStringOrDefault(*SummaryObj, TEXT("game_type"), TEXT("coin_flip")));
				Summary.Rounds = T66RunSummaryGetJsonIntOrDefault(*SummaryObj, TEXT("rounds"), 0);
				Summary.Wins = T66RunSummaryGetJsonIntOrDefault(*SummaryObj, TEXT("wins"), 0);
				Summary.Losses = T66RunSummaryGetJsonIntOrDefault(*SummaryObj, TEXT("losses"), 0);
				Summary.Draws = T66RunSummaryGetJsonIntOrDefault(*SummaryObj, TEXT("draws"), 0);
				Summary.CheatAttempts = T66RunSummaryGetJsonIntOrDefault(*SummaryObj, TEXT("cheat_attempts"), 0);
				Summary.CheatSuccesses = T66RunSummaryGetJsonIntOrDefault(*SummaryObj, TEXT("cheat_successes"), 0);
				Summary.TotalBetGold = T66RunSummaryGetJsonIntOrDefault(*SummaryObj, TEXT("total_bet_gold"), 0);
				Summary.TotalPayoutGold = T66RunSummaryGetJsonIntOrDefault(*SummaryObj, TEXT("total_payout_gold"), 0);
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* GamblerEventArr = nullptr;
		if ((*AntiCheatObj)->TryGetArrayField(TEXT("gambler_events"), GamblerEventArr) && GamblerEventArr)
		{
			for (const TSharedPtr<FJsonValue>& EventValue : *GamblerEventArr)
			{
				const TSharedPtr<FJsonObject>* EventObj = nullptr;
				if (!EventValue.IsValid() || !EventValue->TryGetObject(EventObj) || !EventObj || !(*EventObj).IsValid())
				{
					continue;
				}

				FT66AntiCheatGamblerEvent& Event = S->AntiCheatGamblerEvents.AddDefaulted_GetRef();
				Event.GameType = T66RunSummaryParseGamblerGameType(T66RunSummaryGetJsonStringOrDefault(*EventObj, TEXT("game_type"), TEXT("coin_flip")));
				Event.TimeSeconds = T66RunSummaryGetJsonFloatOrDefault(*EventObj, TEXT("time_seconds"), 0.f);
				Event.BetGold = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("bet_gold"), 0);
				Event.PayoutGold = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("payout_gold"), 0);
				Event.bCheatAttempted = T66RunSummaryGetJsonBoolOrDefault(*EventObj, TEXT("cheat_attempted"), false);
				Event.bCheatSucceeded = T66RunSummaryGetJsonBoolOrDefault(*EventObj, TEXT("cheat_succeeded"), false);
				Event.bWin = T66RunSummaryGetJsonBoolOrDefault(*EventObj, TEXT("win"), false);
				Event.bDraw = T66RunSummaryGetJsonBoolOrDefault(*EventObj, TEXT("draw"), false);
				Event.PlayerChoice = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("player_choice"), INDEX_NONE);
				Event.OpponentChoice = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("opponent_choice"), INDEX_NONE);
				Event.OutcomeValue = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("outcome_value"), 0);
				Event.OutcomeSecondaryValue = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("outcome_value_secondary"), 0);
				Event.SelectedMask = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("selected_mask"), 0);
				Event.ResolvedMask = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("resolved_mask"), 0);
				Event.PathBits = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("path_bits"), 0);
				Event.ShufflePreDrawSeed = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("shuffle_pre_draw_seed"), 0);
				Event.ShuffleStartDrawIndex = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("shuffle_start_draw_index"), INDEX_NONE);
				Event.OutcomePreDrawSeed = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("outcome_pre_draw_seed"), 0);
				Event.OutcomeDrawIndex = T66RunSummaryGetJsonIntOrDefault(*EventObj, TEXT("outcome_draw_index"), INDEX_NONE);
				Event.OutcomeExpectedChance01 = T66RunSummaryGetJsonFloatOrDefault(*EventObj, TEXT("outcome_expected_chance_01"), -1.f);
				Event.ActionSequence = T66RunSummaryGetJsonStringOrDefault(*EventObj, TEXT("action_sequence"));
			}
		}
		S->bAntiCheatGamblerEventsTruncated = T66RunSummaryGetJsonBoolOrDefault(*AntiCheatObj, TEXT("gambler_events_truncated"), false);
		S->GamblerOutcomeSummaries = S->AntiCheatGamblerSummaries;
		S->GamblerOutcomeEvents = S->AntiCheatGamblerEvents;
		S->bGamblerOutcomeEventsTruncated = S->bAntiCheatGamblerEventsTruncated;
	}

	const TArray<TSharedPtr<FJsonValue>>* IdolsArr = nullptr;
	if (Json->TryGetArrayField(TEXT("equipped_idols"), IdolsArr) && IdolsArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *IdolsArr)
		{
			FString IdolStr;
			if (V.IsValid() && V->TryGetString(IdolStr))
			{
				S->EquippedIdols.Add(FName(*IdolStr));
			}
		}
	}
	const TArray<TSharedPtr<FJsonValue>>* IdolTiersArr = nullptr;
	if (Json->TryGetArrayField(TEXT("equipped_idol_tiers"), IdolTiersArr) && IdolTiersArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *IdolTiersArr)
		{
			double TierValue = 0.0;
			if (V.IsValid() && V->TryGetNumber(TierValue))
			{
				S->EquippedIdolTiers.Add(static_cast<uint8>(FMath::Clamp<int32>(FMath::RoundToInt(TierValue), 0, 4)));
			}
		}
	}
	T66NormalizeEquippedIdolSaveArrays(S->EquippedIdols, S->EquippedIdolTiers);

	const TArray<TSharedPtr<FJsonValue>>* IdolElementsArr = nullptr;
	if (Json->TryGetArrayField(TEXT("equipped_idol_elements"), IdolElementsArr) && IdolElementsArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *IdolElementsArr)
		{
			FString ElementStr;
			S->EquippedIdolElements.Add(V.IsValid() && V->TryGetString(ElementStr)
				? T66RunSummaryParseIdolElement(ElementStr)
				: ET66IdolElement::Fire);
		}
	}
	const TArray<TSharedPtr<FJsonValue>>* IdolTypesArr = nullptr;
	if (Json->TryGetArrayField(TEXT("equipped_idol_types"), IdolTypesArr) && IdolTypesArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *IdolTypesArr)
		{
			FString TypeStr;
			S->EquippedIdolCategories.Add(V.IsValid() && V->TryGetString(TypeStr)
				? T66RunSummaryParseAttackCategory(TypeStr)
				: ET66AttackCategory::Pierce);
		}
	}
	while (S->EquippedIdolElements.Num() < S->EquippedIdols.Num())
	{
		S->EquippedIdolElements.Add(ET66IdolElement::Fire);
	}
	while (S->EquippedIdolCategories.Num() < S->EquippedIdols.Num())
	{
		S->EquippedIdolCategories.Add(ET66AttackCategory::Pierce);
	}

	const TArray<TSharedPtr<FJsonValue>>* InvArr = nullptr;
	if (Json->TryGetArrayField(TEXT("inventory"), InvArr) && InvArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *InvArr)
		{
			FString ItemStr;
			if (V.IsValid() && V->TryGetString(ItemStr))
			{
				S->Inventory.Add(FName(*ItemStr));
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* InventorySlotsArr = nullptr;
	if (Json->TryGetArrayField(TEXT("inventory_slots"), InventorySlotsArr) && InventorySlotsArr)
	{
		for (const TSharedPtr<FJsonValue>& SlotValue : *InventorySlotsArr)
		{
			const TSharedPtr<FJsonObject>* SlotObj = nullptr;
			if (!SlotValue.IsValid() || !SlotValue->TryGetObject(SlotObj) || !SlotObj || !(*SlotObj).IsValid())
			{
				continue;
			}

			FT66InventorySlot& Slot = S->InventorySlots.AddDefaulted_GetRef();
			const FString ItemIdStr = T66RunSummaryGetJsonStringOrDefault(*SlotObj, TEXT("item_id"));
			Slot.ItemTemplateID = ItemIdStr.IsEmpty() ? NAME_None : FName(*ItemIdStr);
			Slot.Rarity = T66RunSummaryParseItemRarity(T66RunSummaryGetJsonStringOrDefault(*SlotObj, TEXT("rarity"), TEXT("black")));
			Slot.Line1RolledValue = T66RunSummaryGetJsonIntOrDefault(*SlotObj, TEXT("line1_rolled_value"), 1);
			Slot.SecondaryStatBonusOverride = T66RunSummaryGetJsonIntOrDefault(*SlotObj, TEXT("secondary_stat_bonus_override"), 0);
			Slot.Line2MultiplierOverride = T66RunSummaryGetJsonFloatOrDefault(*SlotObj, TEXT("line2_multiplier_override"), 0.f);
			Slot.RollSeed = T66RunSummaryGetJsonIntOrDefault(*SlotObj, TEXT("roll_seed"), 0);
		}
	}
	if (S->Inventory.IsEmpty() && S->InventorySlots.Num() > 0)
	{
		for (const FT66InventorySlot& Slot : S->InventorySlots)
		{
			if (Slot.IsValid())
			{
				S->Inventory.Add(Slot.ItemTemplateID);
			}
		}
	}

	const TSharedPtr<FJsonObject>* NoIdolObj = nullptr;
	if (Json->TryGetObjectField(TEXT("no_idol"), NoIdolObj) && NoIdolObj && (*NoIdolObj).IsValid())
	{
		S->NoIdolSelectionStacks = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("stacks"), 0);
		S->NoIdolPrimaryStatBonuses.DamageTenths = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("damage_tenths"), 0);
		S->NoIdolPrimaryStatBonuses.AttackSpeedTenths = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("attack_speed_tenths"), 0);
		S->NoIdolPrimaryStatBonuses.AttackScaleTenths = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("attack_scale_tenths"), 0);
		S->NoIdolPrimaryStatBonuses.AccuracyTenths = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("accuracy_tenths"), 0);
		S->NoIdolPrimaryStatBonuses.ArmorTenths = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("armor_tenths"), 0);
		S->NoIdolPrimaryStatBonuses.EvasionTenths = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("evasion_tenths"), 0);
		S->NoIdolPrimaryStatBonuses.LuckTenths = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("luck_tenths"), 0);
		S->NoIdolPrimaryStatBonuses.SpeedTenths = T66RunSummaryGetJsonIntOrDefault(*NoIdolObj, TEXT("speed_tenths"), 0);
	}

	const TSharedPtr<FJsonObject>* MobLootObj = nullptr;
	if (Json->TryGetObjectField(TEXT("mob_loot"), MobLootObj) && MobLootObj && (*MobLootObj).IsValid())
	{
		S->MobLootDropsCollectedThisRun = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("drops_collected"), 0);
		S->MobLootQuantityCollectedThisRun = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("quantity_collected"), 0);
		S->MobLootGoldValueCollectedThisRun = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("gold_value_collected"), 0);
		S->MobLootQuantityCollectedByPlayerThisRun = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("quantity_collected_by_player"), 0);
		S->MobLootQuantityCollectedByPetThisRun = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("quantity_collected_by_pet"), 0);
		S->MobLootDropsCollectedByPetThisRun = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("drops_collected_by_pet"), 0);
		S->MobLootQuantitySoldThisRun = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("quantity_sold"), 0);
		S->MobLootSaleGoldThisRun = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("sale_gold"), 0);
		S->MobLootRemainingStack = T66RunSummaryGetJsonIntOrDefault(*MobLootObj, TEXT("remaining_stack"), 0);
	}

	const TArray<TSharedPtr<FJsonValue>>* GamblerResultsArr = nullptr;
	if (S->GamblerOutcomeSummaries.IsEmpty() && Json->TryGetArrayField(TEXT("gambler_results"), GamblerResultsArr) && GamblerResultsArr)
	{
		for (const TSharedPtr<FJsonValue>& ResultValue : *GamblerResultsArr)
		{
			const TSharedPtr<FJsonObject>* ResultObj = nullptr;
			if (!ResultValue.IsValid() || !ResultValue->TryGetObject(ResultObj) || !ResultObj || !(*ResultObj).IsValid())
			{
				continue;
			}
			FT66AntiCheatGamblerGameSummary& Summary = S->GamblerOutcomeSummaries.AddDefaulted_GetRef();
			Summary.GameType = T66RunSummaryParseGamblerGameType(T66RunSummaryGetJsonStringOrDefault(*ResultObj, TEXT("game_type"), TEXT("coin_flip")));
			Summary.Rounds = T66RunSummaryGetJsonIntOrDefault(*ResultObj, TEXT("rounds"), 0);
			Summary.Wins = T66RunSummaryGetJsonIntOrDefault(*ResultObj, TEXT("wins"), 0);
			Summary.Losses = T66RunSummaryGetJsonIntOrDefault(*ResultObj, TEXT("losses"), 0);
			Summary.Draws = T66RunSummaryGetJsonIntOrDefault(*ResultObj, TEXT("draws"), 0);
			Summary.CheatAttempts = T66RunSummaryGetJsonIntOrDefault(*ResultObj, TEXT("cheat_attempts"), 0);
			Summary.CheatSuccesses = T66RunSummaryGetJsonIntOrDefault(*ResultObj, TEXT("cheat_successes"), 0);
			Summary.TotalBetGold = T66RunSummaryGetJsonIntOrDefault(*ResultObj, TEXT("total_bet_gold"), 0);
			Summary.TotalPayoutGold = T66RunSummaryGetJsonIntOrDefault(*ResultObj, TEXT("total_payout_gold"), 0);
		}
		S->AntiCheatGamblerSummaries = S->GamblerOutcomeSummaries;
	}
	S->bGamblerOutcomeEventsTruncated = T66RunSummaryGetJsonBoolOrDefault(Json, TEXT("gambler_results_truncated"), S->bGamblerOutcomeEventsTruncated);

	const TSharedPtr<FJsonObject>* VendorObj = nullptr;
	if (Json->TryGetObjectField(TEXT("vendor"), VendorObj) && VendorObj && (*VendorObj).IsValid())
	{
		S->CurrentGold = T66RunSummaryGetJsonIntOrDefault(*VendorObj, TEXT("current_gold"), 0);
		S->CurrentDebt = T66RunSummaryGetJsonIntOrDefault(*VendorObj, TEXT("current_debt"), 0);
		S->InventorySellValueTotal = T66RunSummaryGetJsonIntOrDefault(*VendorObj, TEXT("inventory_sell_value_total"), 0);
		S->NetWorth = T66RunSummaryGetJsonIntOrDefault(*VendorObj, TEXT("net_worth"), 0);
		S->ActiveVendorTokenStacks = FMath::Clamp(
			T66RunSummaryGetJsonIntOrDefault(
				*VendorObj,
				TEXT("active_vendor_token_stacks"),
				T66RunSummaryGetJsonIntOrDefault(*VendorObj, TEXT("active_vendor_token_level"), 0)),
			0,
			16);
		S->CurrentSellFraction = T66RunSummaryGetJsonFloatOrDefault(*VendorObj, TEXT("current_sell_fraction"), 0.f);
		S->ShopStockCount = T66RunSummaryGetJsonIntOrDefault(*VendorObj, TEXT("shop_stock_count"), 0);
		S->BuybackPoolSize = T66RunSummaryGetJsonIntOrDefault(*VendorObj, TEXT("buyback_pool_size"), 0);
	}

	const TSharedPtr<FJsonObject>* WeaponObj = nullptr;
	if (Json->TryGetObjectField(TEXT("weapon"), WeaponObj) && WeaponObj && (*WeaponObj).IsValid())
	{
		const FString WeaponIdStr = T66RunSummaryGetJsonStringOrDefault(*WeaponObj, TEXT("weapon_id"));
		S->EquippedWeaponID = WeaponIdStr.IsEmpty() ? NAME_None : FName(*WeaponIdStr);
		S->EquippedWeaponBranch = T66RunSummaryParseAttackCategory(T66RunSummaryGetJsonStringOrDefault(*WeaponObj, TEXT("branch"), TEXT("pierce")));
		S->EquippedWeaponRarity = T66RunSummaryParseWeaponRarity(T66RunSummaryGetJsonStringOrDefault(*WeaponObj, TEXT("rarity"), TEXT("black")));
		const FString PatternStr = T66RunSummaryGetJsonStringOrDefault(*WeaponObj, TEXT("attack_pattern_id"));
		S->EquippedWeaponAttackPatternID = PatternStr.IsEmpty() ? NAME_None : FName(*PatternStr);
		S->EquippedWeaponProjectileCount = T66RunSummaryGetJsonIntOrDefault(*WeaponObj, TEXT("projectile_count"), 0);
		S->EquippedWeaponSpreadAngleDegrees = T66RunSummaryGetJsonFloatOrDefault(*WeaponObj, TEXT("spread_angle_degrees"), 0.f);
	}

	const TSharedPtr<FJsonObject>* PetObj = nullptr;
	if (Json->TryGetObjectField(TEXT("pet"), PetObj) && PetObj && (*PetObj).IsValid())
	{
		const FString PetIdStr = T66RunSummaryGetJsonStringOrDefault(*PetObj, TEXT("active_pet_id"));
		S->ActivePetID = PetIdStr.IsEmpty() ? NAME_None : FName(*PetIdStr);
		const FString SkinIdStr = T66RunSummaryGetJsonStringOrDefault(*PetObj, TEXT("active_pet_skin_id"));
		S->ActivePetSkinID = SkinIdStr.IsEmpty() ? NAME_None : FName(*SkinIdStr);
		S->ActivePetBondStagesCleared = T66RunSummaryGetJsonIntOrDefault(*PetObj, TEXT("bond_stages_cleared"), 0);
		S->ActivePetBondMovementSpeedMultiplier = T66RunSummaryGetJsonFloatOrDefault(*PetObj, TEXT("bond_movement_speed_multiplier"), 1.f);
		S->PetMobLootQuantityCollectedThisRun = T66RunSummaryGetJsonIntOrDefault(*PetObj, TEXT("mob_loot_quantity_collected"), 0);
		S->PetMobLootDropsCollectedThisRun = T66RunSummaryGetJsonIntOrDefault(*PetObj, TEXT("mob_loot_drops_collected"), 0);
	}

	const TSharedPtr<FJsonObject>* BossObj = nullptr;
	if (Json->TryGetObjectField(TEXT("boss"), BossObj) && BossObj && (*BossObj).IsValid())
	{
		S->bBossActiveAtSummary = T66RunSummaryGetJsonBoolOrDefault(*BossObj, TEXT("active_at_summary"), false);
		const FString BossIdStr = T66RunSummaryGetJsonStringOrDefault(*BossObj, TEXT("active_boss_id"));
		S->ActiveBossID = BossIdStr.IsEmpty() ? NAME_None : FName(*BossIdStr);
		S->BossMaxHP = T66RunSummaryGetJsonIntOrDefault(*BossObj, TEXT("boss_max_hp"), 0);
		S->BossCurrentHP = T66RunSummaryGetJsonIntOrDefault(*BossObj, TEXT("boss_current_hp"), 0);
		S->CowardiceGatesTakenCount = T66RunSummaryGetJsonIntOrDefault(*BossObj, TEXT("cowardice_gates_taken"), 0);

		const TArray<TSharedPtr<FJsonValue>>* OwedBossArr = nullptr;
		if ((*BossObj)->TryGetArrayField(TEXT("owed_boss_ids"), OwedBossArr) && OwedBossArr)
		{
			for (const TSharedPtr<FJsonValue>& BossValue : *OwedBossArr)
			{
				FString OwedBossIdStr;
				if (BossValue.IsValid() && BossValue->TryGetString(OwedBossIdStr) && !OwedBossIdStr.IsEmpty())
				{
					S->OwedBossIDs.Add(FName(*OwedBossIdStr));
				}
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* BossPartArr = nullptr;
		if ((*BossObj)->TryGetArrayField(TEXT("parts"), BossPartArr) && BossPartArr)
		{
			for (const TSharedPtr<FJsonValue>& PartValue : *BossPartArr)
			{
				const TSharedPtr<FJsonObject>* PartObj = nullptr;
				if (!PartValue.IsValid() || !PartValue->TryGetObject(PartObj) || !PartObj || !(*PartObj).IsValid())
				{
					continue;
				}
				FT66BossPartSnapshot& Part = S->BossParts.AddDefaulted_GetRef();
				const FString PartIdStr = T66RunSummaryGetJsonStringOrDefault(*PartObj, TEXT("part_id"));
				Part.PartID = PartIdStr.IsEmpty() ? NAME_None : FName(*PartIdStr);
				Part.HitZoneType = T66RunSummaryParseHitZone(T66RunSummaryGetJsonStringOrDefault(*PartObj, TEXT("hit_zone_type"), TEXT("body")));
				Part.MaxHP = T66RunSummaryGetJsonIntOrDefault(*PartObj, TEXT("max_hp"), 0);
				Part.CurrentHP = T66RunSummaryGetJsonIntOrDefault(*PartObj, TEXT("current_hp"), 0);
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* LogArr = nullptr;
	if (Json->TryGetArrayField(TEXT("event_log"), LogArr) && LogArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *LogArr)
		{
			FString LogStr;
			if (V.IsValid() && V->TryGetString(LogStr))
			{
				S->EventLog.Add(LogStr);
			}
		}
	}

	T66LeaderboardPacing::ExtractStageMarkers(S->EventLog, S->StagePacingPoints);

	const TArray<TSharedPtr<FJsonValue>>* StageSplitsArr = nullptr;
	if (Json->TryGetArrayField(TEXT("stage_splits_ms"), StageSplitsArr) && StageSplitsArr)
	{
		const int32 StageCount = StageSplitsArr->Num();
		const int32 StageStart = FMath::Max(1, S->StageReached - StageCount + 1);
		for (int32 SplitIndex = 0; SplitIndex < StageCount; ++SplitIndex)
		{
			double SplitMsValue = 0.0;
			if (!(*StageSplitsArr)[SplitIndex].IsValid() || !(*StageSplitsArr)[SplitIndex]->TryGetNumber(SplitMsValue))
			{
				continue;
			}

			FT66StagePacingPoint Point;
			Point.Stage = StageStart + SplitIndex;
			Point.Score = 0;
			Point.ElapsedSeconds = FMath::Max(0.0, SplitMsValue) / 1000.0;

			const int32 ExistingIndex = S->StagePacingPoints.IndexOfByPredicate([Point](const FT66StagePacingPoint& ExistingPoint)
			{
				return ExistingPoint.Stage == Point.Stage;
			});

			if (ExistingIndex != INDEX_NONE)
			{
				S->StagePacingPoints[ExistingIndex].ElapsedSeconds = Point.ElapsedSeconds;
			}
			else
			{
				S->StagePacingPoints.Add(Point);
			}
		}

		S->StagePacingPoints.Sort([](const FT66StagePacingPoint& A, const FT66StagePacingPoint& B)
		{
			return A.Stage < B.Stage;
		});
	}

	const TSharedPtr<FJsonObject>* DmgObj = nullptr;
	if (Json->TryGetObjectField(TEXT("damage_by_source"), DmgObj) && DmgObj && (*DmgObj).IsValid())
	{
		for (const auto& Pair : (*DmgObj)->Values)
		{
			double Val = 0.0;
			if (Pair.Value.IsValid() && Pair.Value->TryGetNumber(Val))
			{
				S->DamageBySource.Add(FName(*Pair.Key), static_cast<int32>(Val));
			}
		}
	}

	const TSharedPtr<FJsonObject>* DmgReceivedObj = nullptr;
	if (Json->TryGetObjectField(TEXT("damage_received_by_source"), DmgReceivedObj) && DmgReceivedObj && (*DmgReceivedObj).IsValid())
	{
		for (const auto& Pair : (*DmgReceivedObj)->Values)
		{
			double Val = 0.0;
			if (Pair.Value.IsValid() && Pair.Value->TryGetNumber(Val))
			{
				S->DamageReceivedBySource.Add(FName(*Pair.Key), static_cast<int32>(Val));
			}
		}
	}

	Json->TryGetStringField(TEXT("proof_of_run_url"), S->ProofOfRunUrl);
	S->bProofOfRunLocked = T66RunSummaryGetJsonBoolOrDefault(Json, TEXT("proof_locked"), !S->ProofOfRunUrl.IsEmpty());

	return S;
}

UT66LeaderboardRunSummarySaveGame* UT66BackendSubsystem::ParseRunSummaryFromJson(const TSharedPtr<FJsonObject>& Json, UObject* Outer)
{
	return T66BackendRunSummaryParser::Parse(Json, Outer);
}
