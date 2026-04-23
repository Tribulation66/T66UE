// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Backend/T66BackendRunSummaryParser.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Dom/JsonObject.h"

UT66LeaderboardRunSummarySaveGame* T66BackendRunSummaryParser::Parse(const TSharedPtr<FJsonObject>& Json, UObject* Outer)
{
	if (!Json.IsValid())
	{
		return nullptr;
	}

	UT66LeaderboardRunSummarySaveGame* S = NewObject<UT66LeaderboardRunSummarySaveGame>(Outer);

	S->SchemaVersion = Json->HasField(TEXT("schema_version")) ? static_cast<int32>(Json->GetNumberField(TEXT("schema_version"))) : 6;
	S->EntryId = Json->HasField(TEXT("entry_id")) ? Json->GetStringField(TEXT("entry_id")) : FString();
	S->OwnerSteamId = Json->HasField(TEXT("steam_id")) ? Json->GetStringField(TEXT("steam_id")) : FString();
	S->StageReached = Json->HasField(TEXT("stage_reached")) ? static_cast<int32>(Json->GetNumberField(TEXT("stage_reached"))) : 1;
	S->Score = Json->HasField(TEXT("score")) ? static_cast<int32>(Json->GetNumberField(TEXT("score"))) : 0;
	S->ScoreRankAllTime = Json->HasField(TEXT("score_rank_alltime")) ? static_cast<int32>(Json->GetNumberField(TEXT("score_rank_alltime"))) : 0;
	S->ScoreRankWeekly = Json->HasField(TEXT("score_rank_weekly")) ? static_cast<int32>(Json->GetNumberField(TEXT("score_rank_weekly"))) : 0;
	S->SpeedRunRankAllTime = Json->HasField(TEXT("speedrun_rank_alltime")) ? static_cast<int32>(Json->GetNumberField(TEXT("speedrun_rank_alltime"))) : 0;
	S->SpeedRunRankWeekly = Json->HasField(TEXT("speedrun_rank_weekly")) ? static_cast<int32>(Json->GetNumberField(TEXT("speedrun_rank_weekly"))) : 0;
	S->RunDurationSeconds = Json->HasField(TEXT("time_seconds")) ? static_cast<float>(Json->GetNumberField(TEXT("time_seconds"))) : 0.f;

	const FString HeroIdStr = Json->GetStringField(TEXT("hero_id"));
	S->HeroID = HeroIdStr.IsEmpty() ? NAME_None : FName(*HeroIdStr);

	const FString CompanionIdStr = Json->GetStringField(TEXT("companion_id"));
	S->CompanionID = CompanionIdStr.IsEmpty() ? NAME_None : FName(*CompanionIdStr);

	S->HeroLevel = Json->HasField(TEXT("hero_level")) ? static_cast<int32>(Json->GetNumberField(TEXT("hero_level"))) : 1;
	S->DisplayName = Json->GetStringField(TEXT("display_name"));

	const TSharedPtr<FJsonObject>* IntegrityObj = nullptr;
	if (Json->TryGetObjectField(TEXT("integrity_context"), IntegrityObj) && IntegrityObj && (*IntegrityObj).IsValid())
	{
		const TSharedPtr<FJsonObject>& Integrity = *IntegrityObj;
		S->IntegrityContext.Verdict = Integrity->HasField(TEXT("verdict")) ? Integrity->GetStringField(TEXT("verdict")) : TEXT("unknown");
		S->IntegrityContext.SteamAppId = Integrity->HasField(TEXT("steam_app_id")) ? Integrity->GetStringField(TEXT("steam_app_id")) : FString();
		S->IntegrityContext.SteamBuildId = Integrity->HasField(TEXT("steam_build_id")) ? static_cast<int32>(Integrity->GetNumberField(TEXT("steam_build_id"))) : 0;
		S->IntegrityContext.SteamBetaName = Integrity->HasField(TEXT("steam_beta_name")) ? Integrity->GetStringField(TEXT("steam_beta_name")) : FString();
		S->IntegrityContext.ManifestId = Integrity->HasField(TEXT("manifest_id")) ? Integrity->GetStringField(TEXT("manifest_id")) : FString();
		S->IntegrityContext.ManifestRootHash = Integrity->HasField(TEXT("manifest_root_hash")) ? Integrity->GetStringField(TEXT("manifest_root_hash")) : FString();
		S->IntegrityContext.ModuleListHash = Integrity->HasField(TEXT("module_list_hash")) ? Integrity->GetStringField(TEXT("module_list_hash")) : FString();
		S->IntegrityContext.MountedContentHash = Integrity->HasField(TEXT("mounted_content_hash")) ? Integrity->GetStringField(TEXT("mounted_content_hash")) : FString();
		S->IntegrityContext.BaselineHash = Integrity->HasField(TEXT("baseline_hash")) ? Integrity->GetStringField(TEXT("baseline_hash")) : FString();
		S->IntegrityContext.FinalHash = Integrity->HasField(TEXT("final_hash")) ? Integrity->GetStringField(TEXT("final_hash")) : FString();

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
		S->DamageStat = St->HasField(TEXT("damage")) ? static_cast<int32>(St->GetNumberField(TEXT("damage"))) : 1;
		S->AttackSpeedStat = St->HasField(TEXT("attack_speed")) ? static_cast<int32>(St->GetNumberField(TEXT("attack_speed"))) : 1;
		S->AttackScaleStat = St->HasField(TEXT("attack_scale")) ? static_cast<int32>(St->GetNumberField(TEXT("attack_scale"))) : 1;
		S->AccuracyStat = St->HasField(TEXT("accuracy")) ? static_cast<int32>(St->GetNumberField(TEXT("accuracy"))) : 1;
		S->ArmorStat = St->HasField(TEXT("armor")) ? static_cast<int32>(St->GetNumberField(TEXT("armor"))) : 1;
		S->EvasionStat = St->HasField(TEXT("evasion")) ? static_cast<int32>(St->GetNumberField(TEXT("evasion"))) : 1;
		S->LuckStat = St->HasField(TEXT("luck")) ? static_cast<int32>(St->GetNumberField(TEXT("luck"))) : 1;
		S->SpeedStat = St->HasField(TEXT("speed")) ? static_cast<int32>(St->GetNumberField(TEXT("speed"))) : 1;
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
				ET66SecondaryStatType StatType;
				bool bFound = true;

				if (Key == TEXT("CritChance")) StatType = ET66SecondaryStatType::CritChance;
				else if (Key == TEXT("CritDamage")) StatType = ET66SecondaryStatType::CritDamage;
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
				else { bFound = false; }

				if (bFound)
				{
					S->SecondaryStatValues.Add(StatType, static_cast<float>(Val));
				}
			}
		}
	}

	S->LuckRating0To100 = Json->HasField(TEXT("luck_rating")) ? static_cast<int32>(Json->GetNumberField(TEXT("luck_rating"))) : -1;
	S->LuckRatingQuantity0To100 = Json->HasField(TEXT("luck_quantity")) ? static_cast<int32>(Json->GetNumberField(TEXT("luck_quantity"))) : -1;
	S->LuckRatingQuality0To100 = Json->HasField(TEXT("luck_quality")) ? static_cast<int32>(Json->GetNumberField(TEXT("luck_quality"))) : -1;
	S->SkillRating0To100 = Json->HasField(TEXT("skill_rating")) ? static_cast<int32>(Json->GetNumberField(TEXT("skill_rating"))) : -1;

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

	if (Json->HasField(TEXT("proof_of_run_url")))
	{
		S->ProofOfRunUrl = Json->GetStringField(TEXT("proof_of_run_url"));
	}
	S->bProofOfRunLocked =
		Json->HasField(TEXT("proof_locked"))
		? Json->GetBoolField(TEXT("proof_locked"))
		: !S->ProofOfRunUrl.IsEmpty();

	return S;
}

UT66LeaderboardRunSummarySaveGame* UT66BackendSubsystem::ParseRunSummaryFromJson(const TSharedPtr<FJsonObject>& Json, UObject* Outer)
{
	return T66BackendRunSummaryParser::Parse(Json, Outer);
}
