// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LocalLeaderboardSaveGame.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66SteamHelper.h"

#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Leaderboard, Log, All);

static TAutoConsoleVariable<int32> CVarT66AccountStatusForce(
	TEXT("t66.AccountStatus.Force"),
	0,
	TEXT("Debug: force Account Status visible. 0=normal, 1=Suspicion, 2=CheatingCertainty"),
	ECVF_Cheat
);

// Short debug aliases (requested): `sus1` / `sus2`
static FAutoConsoleCommand T66AccountStatusSus1Command(
	TEXT("sus1"),
	TEXT("Debug: force Account Status (Suspicion). Equivalent to: t66.AccountStatus.Force 1"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		CVarT66AccountStatusForce->Set(1, ECVF_SetByConsole);
	})
);

static FAutoConsoleCommand T66AccountStatusSus2Command(
	TEXT("sus2"),
	TEXT("Debug: force Account Status (Cheating Certainty). Equivalent to: t66.AccountStatus.Force 2"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		CVarT66AccountStatusForce->Set(2, ECVF_SetByConsole);
	})
);

static void T66_ClearLead(const TArray<FString>& Args, UWorld* World)
{
	(void)Args;
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (!LB) return;
	LB->DebugClearLocalLeaderboard();
}

// Requested debug console command: "clearlead" (plus a couple of handy aliases).
static FAutoConsoleCommandWithWorldAndArgs T66ClearLeadCommand(
	TEXT("clearlead"),
	TEXT("Debug: clear local leaderboard + local best run summary snapshots."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_ClearLead)
);

static FAutoConsoleCommandWithWorldAndArgs T66ClearLeadCommandAlias(
	TEXT("clear.lead"),
	TEXT("Debug alias for clearlead."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_ClearLead)
);

static ET66AccountRestrictionKind T66RestrictionKindFromBackend(const FString& Restriction)
{
	if (Restriction.Equals(TEXT("suspicion"), ESearchCase::IgnoreCase))
	{
		return ET66AccountRestrictionKind::Suspicion;
	}
	if (Restriction.Equals(TEXT("certainty"), ESearchCase::IgnoreCase)
		|| Restriction.Equals(TEXT("cheating"), ESearchCase::IgnoreCase))
	{
		return ET66AccountRestrictionKind::CheatingCertainty;
	}
	return ET66AccountRestrictionKind::None;
}

static ET66AppealReviewStatus T66AppealStatusFromBackend(const FString& Status)
{
	if (Status.Equals(TEXT("submitted"), ESearchCase::IgnoreCase)
		|| Status.Equals(TEXT("under_review"), ESearchCase::IgnoreCase)
		|| Status.Equals(TEXT("under review"), ESearchCase::IgnoreCase))
	{
		return ET66AppealReviewStatus::UnderReview;
	}
	if (Status.Equals(TEXT("approved"), ESearchCase::IgnoreCase))
	{
		return ET66AppealReviewStatus::Approved;
	}
	if (Status.Equals(TEXT("denied"), ESearchCase::IgnoreCase))
	{
		return ET66AppealReviewStatus::Denied;
	}
	return ET66AppealReviewStatus::NotSubmitted;
}

void UT66LeaderboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreateLocalSave();
	if (!LoadTargetsFromDataTablesIfPresent())
	{
		UE_LOG(
			LogT66RuntimeData,
			Warning,
			TEXT("RuntimeData: leaderboard targets failed to load from cooked DataTables ScorePath='%s' SpeedRunPath='%s'"),
			ScoreTargetsDTPath,
			SpeedRunTargetsDTPath);
	}

	UE_LOG(
		LogT66RuntimeData,
		Log,
		TEXT("RuntimeData: leaderboard targets initialized ScoreSource=%s ScoreTargets=%d SpeedRunSource=%s SpeedRunTargets=%d"),
		T66LegacyRuntimeDataAccess::ToString(ScoreTargetRuntimeSource),
		ScoreTarget10ByKey.Num(),
		T66LegacyRuntimeDataAccess::ToString(SpeedRunTargetRuntimeSource),
		SpeedRunTarget10ByKey_DiffStage.Num());

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnAccountStatusComplete.AddDynamic(this, &UT66LeaderboardSubsystem::HandleBackendAccountStatusComplete);
			Backend->OnAppealSubmitComplete.AddDynamic(this, &UT66LeaderboardSubsystem::HandleBackendAppealSubmitComplete);
			Backend->OnSubmitRunDataReady.AddUObject(this, &UT66LeaderboardSubsystem::HandleBackendSubmitRunDataReady);
			Backend->OnRunSummaryReady.AddUObject(this, &UT66LeaderboardSubsystem::HandleBackendRunSummaryReady);
			RefreshAccountStatusFromBackend();
		}
	}
}

bool UT66LeaderboardSubsystem::GetSettingsPracticeAndAnon(bool& bOutPractice, bool& bOutAnon) const
{
	bOutPractice = false;
	bOutAnon = false;
	UGameInstance* GI = GetGameInstance();
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	if (!PS) return false;
	bOutPractice = PS->GetPracticeMode();
	bOutAnon = PS->GetSubmitLeaderboardAnonymous();
	return true;
}

FString UT66LeaderboardSubsystem::MakePendingBestRankRequestKey(EBestRankRecordType Type, ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	const TCHAR* TypeLabel = TEXT("score");
	switch (Type)
	{
	case EBestRankRecordType::Score:
		TypeLabel = TEXT("score");
		break;
	case EBestRankRecordType::CompletedRunTime:
		TypeLabel = TEXT("completed");
		break;
	case EBestRankRecordType::DifficultyClear:
		TypeLabel = TEXT("difficulty_clear");
		break;
	}

	return FString::Printf(
		TEXT("best_rank_%s_%s_%s_%s"),
		TypeLabel,
		*DifficultyKey(Difficulty),
		*PartySizeKey(PartySize),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
}

void UT66LeaderboardSubsystem::HandleBackendSubmitRunDataReady(
	const FString& RequestKey,
	bool bSuccess,
	int32 ScoreRankAlltime,
	int32 ScoreRankWeekly,
	int32 SpeedRunRankAlltime,
	int32 SpeedRunRankWeekly,
	bool bNewScorePersonalBest,
	bool bNewSpeedRunPersonalBest)
{
	FPendingBestRankSubmission Pending;
	if (!PendingBestRankSubmissions.RemoveAndCopyValue(RequestKey, Pending))
	{
		return;
	}

	const bool bTracksScoreRequest =
		Pending.Type == EBestRankRecordType::Score
		|| Pending.Type == EBestRankRecordType::DifficultyClear;
	const bool bTracksCompletedRequest =
		Pending.Type == EBestRankRecordType::CompletedRunTime
		|| Pending.Type == EBestRankRecordType::DifficultyClear;
	const bool bMatchesLatestScoreRequest =
		bTracksScoreRequest
		&& LastSubmittedScoreRequestKey.Equals(RequestKey, ESearchCase::CaseSensitive);
	const bool bMatchesLatestCompletedRequest =
		bTracksCompletedRequest
		&& LastSubmittedCompletedRunTimeRequestKey.Equals(RequestKey, ESearchCase::CaseSensitive);

	if (bMatchesLatestScoreRequest)
	{
		LastSubmittedScoreRequestKey.Reset();
		bLastScoreWasNewBest = false;
	}
	if (bMatchesLatestCompletedRequest)
	{
		LastSubmittedCompletedRunTimeRequestKey.Reset();
		bLastCompletedRunTimeWasNewBest = false;
	}

	if (bMatchesLatestScoreRequest)
	{
		LastSubmittedScoreRankAllTime = bSuccess ? FMath::Max(0, ScoreRankAlltime) : 0;
		LastSubmittedScoreRankWeekly = bSuccess ? FMath::Max(0, ScoreRankWeekly) : 0;
	}
	if (bMatchesLatestCompletedRequest)
	{
		LastSubmittedCompletedRunRankAllTime = bSuccess ? FMath::Max(0, SpeedRunRankAlltime) : 0;
		LastSubmittedCompletedRunRankWeekly = bSuccess ? FMath::Max(0, SpeedRunRankWeekly) : 0;
	}

	if (!bSuccess)
	{
		return;
	}

	if (!Pending.RunSummarySlotName.IsEmpty())
	{
		UpdateSavedRunSummaryRanks(
			Pending.RunSummarySlotName,
			ScoreRankAlltime,
			ScoreRankWeekly,
			SpeedRunRankAlltime,
			SpeedRunRankWeekly);
	}

	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		return;
	}

	bool bDidChange = false;
	bool bCommittedCurrentScorePersonalBest = false;
	bool bCommittedCurrentCompletedRunPersonalBest = false;

	auto ApplyScoreSubmission = [&]() -> bool
	{
		FT66LocalScoreRecord* Record = FindLocalScoreRecordMutable(Pending.Difficulty, Pending.PartySize);
		if (!Record)
		{
			LocalSave->ScoreRecords.AddDefaulted();
			Record = &LocalSave->ScoreRecords.Last();
			Record->Difficulty = Pending.Difficulty;
			Record->PartySize = Pending.PartySize;
		}

		bool bCommittedCurrentPersonalBest = false;
		if (Record)
		{
			const bool bIsNewBest = Pending.Score > Record->BestScore;
			if (bIsNewBest)
			{
				Record->BestScore = Pending.Score;
				Record->bSubmittedAnonymous = Pending.bSubmittedAnonymous;
				Record->RunSummarySlotName = Pending.RunSummarySlotName;
				Record->AchievedAtUtc = Pending.AchievedAtUtc;
				Record->BestScoreRankAllTime = FMath::Max(0, ScoreRankAlltime);
				Record->BestScoreRankWeekly = FMath::Max(0, ScoreRankWeekly);
				bCommittedCurrentPersonalBest = true;
				bDidChange = true;
			}
			else if (Record->BestScore == Pending.Score)
			{
				if ((Record->RunSummarySlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(Record->RunSummarySlotName, 0))
					&& !Pending.RunSummarySlotName.IsEmpty())
				{
					Record->RunSummarySlotName = Pending.RunSummarySlotName;
					bDidChange = true;
				}
				if (Record->AchievedAtUtc == FDateTime::MinValue() && Pending.AchievedAtUtc != FDateTime::MinValue())
				{
					Record->AchievedAtUtc = Pending.AchievedAtUtc;
					bDidChange = true;
				}
			}

			if (Record->BestScore == Pending.Score
				&& ScoreRankAlltime > 0
				&& (Record->BestScoreRankAllTime <= 0 || ScoreRankAlltime < Record->BestScoreRankAllTime))
			{
				Record->BestScoreRankAllTime = ScoreRankAlltime;
				bDidChange = true;
			}
			if (Record->BestScore == Pending.Score
				&& ScoreRankWeekly > 0
				&& (Record->BestScoreRankWeekly <= 0 || ScoreRankWeekly < Record->BestScoreRankWeekly))
			{
				Record->BestScoreRankWeekly = ScoreRankWeekly;
				bDidChange = true;
			}

			const bool bShouldUpdateBestRank =
				ScoreRankAlltime > 0
				&& (
				Record->BestRankAllTime <= 0
				|| ScoreRankAlltime < Record->BestRankAllTime
				|| (ScoreRankAlltime == Record->BestRankAllTime && Pending.Score > Record->BestRankScore));
			if (bShouldUpdateBestRank)
			{
				const FString ResolvedBestRankSlot = !Pending.RunSummarySlotName.IsEmpty()
					? Pending.RunSummarySlotName
					: ((Record->BestScore == Pending.Score) ? Record->RunSummarySlotName : FString());
				Record->BestRankAllTime = ScoreRankAlltime;
				Record->BestRankScore = Pending.Score;
				Record->BestRankRunSummarySlotName = ResolvedBestRankSlot;
				Record->BestRankAchievedAtUtc = Pending.AchievedAtUtc;
				bDidChange = true;
			}
		}

		return bCommittedCurrentPersonalBest;
	};

	auto ApplyCompletedRunSubmission = [&]() -> bool
	{
		FT66LocalCompletedRunTimeRecord* Record = FindLocalCompletedRunTimeRecordMutable(Pending.Difficulty, Pending.PartySize);
		if (!Record)
		{
			LocalSave->CompletedRunTimeRecords.AddDefaulted();
			Record = &LocalSave->CompletedRunTimeRecords.Last();
			Record->Difficulty = Pending.Difficulty;
			Record->PartySize = Pending.PartySize;
		}

		bool bCommittedCurrentPersonalBest = false;
		if (Record)
		{
			const bool bUnset = (Record->BestCompletedSeconds <= 0.01f);
			const bool bIsNewBest = bUnset || Pending.Seconds < Record->BestCompletedSeconds;
			if (bIsNewBest)
			{
				Record->BestCompletedSeconds = Pending.Seconds;
				Record->bSubmittedAnonymous = Pending.bSubmittedAnonymous;
				Record->RunSummarySlotName = Pending.RunSummarySlotName;
				Record->AchievedAtUtc = Pending.AchievedAtUtc;
				Record->BestCompletedRankAllTime = FMath::Max(0, SpeedRunRankAlltime);
				Record->BestCompletedRankWeekly = FMath::Max(0, SpeedRunRankWeekly);
				bCommittedCurrentPersonalBest = true;
				bDidChange = true;
			}
			else if (FMath::IsNearlyEqual(Record->BestCompletedSeconds, Pending.Seconds))
			{
				if ((Record->RunSummarySlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(Record->RunSummarySlotName, 0))
					&& !Pending.RunSummarySlotName.IsEmpty())
				{
					Record->RunSummarySlotName = Pending.RunSummarySlotName;
					bDidChange = true;
				}
				if (Record->AchievedAtUtc == FDateTime::MinValue() && Pending.AchievedAtUtc != FDateTime::MinValue())
				{
					Record->AchievedAtUtc = Pending.AchievedAtUtc;
					bDidChange = true;
				}
			}

			if (FMath::IsNearlyEqual(Record->BestCompletedSeconds, Pending.Seconds)
				&& SpeedRunRankAlltime > 0
				&& (Record->BestCompletedRankAllTime <= 0 || SpeedRunRankAlltime < Record->BestCompletedRankAllTime))
			{
				Record->BestCompletedRankAllTime = SpeedRunRankAlltime;
				bDidChange = true;
			}
			if (FMath::IsNearlyEqual(Record->BestCompletedSeconds, Pending.Seconds)
				&& SpeedRunRankWeekly > 0
				&& (Record->BestCompletedRankWeekly <= 0 || SpeedRunRankWeekly < Record->BestCompletedRankWeekly))
			{
				Record->BestCompletedRankWeekly = SpeedRunRankWeekly;
				bDidChange = true;
			}

			const bool bShouldUpdateBestRank =
				SpeedRunRankAlltime > 0
				&& (
				Record->BestRankAllTime <= 0
				|| SpeedRunRankAlltime < Record->BestRankAllTime
				|| (SpeedRunRankAlltime == Record->BestRankAllTime
					&& (Record->BestRankCompletedSeconds <= 0.01f || Pending.Seconds < Record->BestRankCompletedSeconds)));
			if (bShouldUpdateBestRank)
			{
				const FString ResolvedBestRankSlot = !Pending.RunSummarySlotName.IsEmpty()
					? Pending.RunSummarySlotName
					: (FMath::IsNearlyEqual(Record->BestCompletedSeconds, Pending.Seconds) ? Record->RunSummarySlotName : FString());
				Record->BestRankAllTime = SpeedRunRankAlltime;
				Record->BestRankCompletedSeconds = Pending.Seconds;
				Record->BestRankRunSummarySlotName = ResolvedBestRankSlot;
				Record->BestRankAchievedAtUtc = Pending.AchievedAtUtc;
				bDidChange = true;
			}
		}
		return bCommittedCurrentPersonalBest;
	};

	switch (Pending.Type)
	{
	case EBestRankRecordType::Score:
		bCommittedCurrentScorePersonalBest = ApplyScoreSubmission();
		break;
	case EBestRankRecordType::CompletedRunTime:
		bCommittedCurrentCompletedRunPersonalBest = ApplyCompletedRunSubmission();
		break;
	case EBestRankRecordType::DifficultyClear:
		bCommittedCurrentScorePersonalBest = ApplyScoreSubmission();
		bCommittedCurrentCompletedRunPersonalBest = ApplyCompletedRunSubmission();
		break;
	}

	if (bMatchesLatestScoreRequest)
	{
		bLastScoreWasNewBest = bCommittedCurrentScorePersonalBest || bNewScorePersonalBest;
	}
	if (bMatchesLatestCompletedRequest)
	{
		bLastCompletedRunTimeWasNewBest =
			bCommittedCurrentCompletedRunPersonalBest || bNewSpeedRunPersonalBest;
	}

	if (bDidChange)
	{
		SaveLocalSave();
	}
}

void UT66LeaderboardSubsystem::LoadOrCreateLocalSave()
{
	const FString DesiredSlotName = MakeResolvedLocalSaveSlotName();
	if (LocalSave && ActiveLocalSaveSlotName.Equals(DesiredSlotName, ESearchCase::CaseSensitive))
	{
		return;
	}

	if (LocalSave && !ActiveLocalSaveSlotName.IsEmpty())
	{
		SaveLocalSave();
		LocalSave = nullptr;
		bLastScoreWasNewBest = false;
		bLastSpeedRunWasNewBest = false;
		bLastCompletedRunTimeWasNewBest = false;
		LastSpeedRunSubmittedStage = 0;
		LastSubmittedScoreRankAllTime = 0;
		LastSubmittedScoreRankWeekly = 0;
		LastSubmittedCompletedRunRankAllTime = 0;
		LastSubmittedCompletedRunRankWeekly = 0;
		LastSubmittedScoreRequestKey.Reset();
		LastSubmittedCompletedRunTimeRequestKey.Reset();
		PendingBestRankSubmissions.Reset();
	}

	ActiveLocalSaveSlotName = DesiredSlotName;

	if (UGameplayStatics::DoesSaveGameExist(ActiveLocalSaveSlotName, 0))
	{
		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(ActiveLocalSaveSlotName, 0);
		LocalSave = Cast<UT66LocalLeaderboardSaveGame>(Loaded);
	}

	bool bNeedsSave = false;
	if (!LocalSave)
	{
		LocalSave = Cast<UT66LocalLeaderboardSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66LocalLeaderboardSaveGame::StaticClass()));
		bNeedsSave = true;
	}
	else if (LocalSave->SchemaVersion < MinCompatibleLocalSaveSchemaVersion)
	{
		UE_LOG(
			LogT66Leaderboard,
			Log,
			TEXT("Leaderboard: wiping incompatible local save. Loaded schema=%d Required schema=%d"),
			LocalSave->SchemaVersion,
			MinCompatibleLocalSaveSchemaVersion);
		DebugClearLocalLeaderboard();
		return;
	}

	LocalSave->SchemaVersion = FMath::Max(LocalSave->SchemaVersion, CurrentLocalSaveSchemaVersion);

	const FString CurrentSteamId = GetCurrentLocalSteamId();
	if (!LocalSave->OwnerSteamId.Equals(CurrentSteamId, ESearchCase::CaseSensitive))
	{
		LocalSave->OwnerSteamId = CurrentSteamId;
		bNeedsSave = true;
	}

	const FString CurrentDisplayName = GetCurrentLocalDisplayName();
	if (!CurrentDisplayName.IsEmpty() && !LocalSave->OwnerDisplayName.Equals(CurrentDisplayName, ESearchCase::CaseSensitive))
	{
		LocalSave->OwnerDisplayName = CurrentDisplayName;
		bNeedsSave = true;
	}
	if (PruneInvalidRecentRunRecords())
	{
		bNeedsSave = true;
	}
	if (SyncRecentRunRecordsFromSnapshots())
	{
		bNeedsSave = true;
	}

	if (bNeedsSave)
	{
		SaveLocalSave();
	}
}

void UT66LeaderboardSubsystem::SaveLocalSave() const
{
	if (!LocalSave) return;
	const FString SlotName = ActiveLocalSaveSlotName.IsEmpty()
		? MakeResolvedLocalSaveSlotName()
		: ActiveLocalSaveSlotName;
	UGameplayStatics::SaveGameToSlot(LocalSave, SlotName, 0);
}

void UT66LeaderboardSubsystem::DebugClearLocalLeaderboard()
{
	LoadOrCreateLocalSave();

	TSet<FString> SlotsToDelete;
	if (LocalSave)
	{
		for (const FT66RecentRunRecord& Record : LocalSave->RecentRuns)
		{
			if (!Record.RunSummarySlotName.IsEmpty())
			{
				SlotsToDelete.Add(Record.RunSummarySlotName);
			}
		}

		for (const FT66LocalScoreRecord& Record : LocalSave->ScoreRecords)
		{
			if (!Record.RunSummarySlotName.IsEmpty())
			{
				SlotsToDelete.Add(Record.RunSummarySlotName);
			}
			if (!Record.BestRankRunSummarySlotName.IsEmpty())
			{
				SlotsToDelete.Add(Record.BestRankRunSummarySlotName);
			}
		}

		for (const FT66LocalCompletedRunTimeRecord& Record : LocalSave->CompletedRunTimeRecords)
		{
			if (!Record.RunSummarySlotName.IsEmpty())
			{
				SlotsToDelete.Add(Record.RunSummarySlotName);
			}
			if (!Record.BestRankRunSummarySlotName.IsEmpty())
			{
				SlotsToDelete.Add(Record.BestRankRunSummarySlotName);
			}
		}

		if (!LocalSave->AccountRestriction.RunSummarySlotName.IsEmpty())
		{
			SlotsToDelete.Add(LocalSave->AccountRestriction.RunSummarySlotName);
		}
	}

	// Delete the core local leaderboard save.
	const FString LocalSaveSlotName = ActiveLocalSaveSlotName.IsEmpty()
		? MakeResolvedLocalSaveSlotName()
		: ActiveLocalSaveSlotName;
	(void)UGameplayStatics::DeleteGameInSlot(LocalSaveSlotName, 0);

	// Delete all local best score run summary snapshots (all supported difficulty/party combinations).
	static const ET66Difficulty Diffs[] =
	{
		ET66Difficulty::Easy,
		ET66Difficulty::Medium,
		ET66Difficulty::Hard,
		ET66Difficulty::VeryHard,
		ET66Difficulty::Impossible,
	};

	static const ET66PartySize Parties[] =
	{
		ET66PartySize::Solo,
		ET66PartySize::Duo,
		ET66PartySize::Trio,
		ET66PartySize::Quad,
	};

	for (const ET66Difficulty Diff : Diffs)
	{
		for (const ET66PartySize Party : Parties)
		{
			const FString Slot = MakeLocalBestScoreRunSummarySlotName(Diff, Party);
			(void)UGameplayStatics::DeleteGameInSlot(Slot, 0);
		}
	}

	for (const FString& Slot : SlotsToDelete)
	{
		(void)UGameplayStatics::DeleteGameInSlot(Slot, 0);
	}

	// Reset transient state and recreate the save so UI can immediately read a valid object.
	LocalSave = nullptr;
	ActiveLocalSaveSlotName.Reset();
	PendingRunSummarySlotName.Reset();
	PendingReturnModalAfterViewerRunSummary = ET66ScreenType::None;
	bLastScoreWasNewBest = false;
	bLastSpeedRunWasNewBest = false;
	bLastCompletedRunTimeWasNewBest = false;
	LastSpeedRunSubmittedStage = 0;

	LoadOrCreateLocalSave();

	UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: cleared local saves (LocalLeaderboard + LocalBestScoreRunSummary_*_*)"));
}

FString UT66LeaderboardSubsystem::DifficultyKey(ET66Difficulty Difficulty)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return TEXT("Easy");
	case ET66Difficulty::Medium: return TEXT("Medium");
	case ET66Difficulty::Hard: return TEXT("Hard");
	case ET66Difficulty::VeryHard: return TEXT("VeryHard");
	case ET66Difficulty::Impossible: return TEXT("Impossible");
	default: return TEXT("Unknown");
	}
}

FString UT66LeaderboardSubsystem::PartySizeKey(ET66PartySize PartySize)
{
	switch (PartySize)
	{
	case ET66PartySize::Solo: return TEXT("Solo");
	case ET66PartySize::Duo: return TEXT("Duo");
	case ET66PartySize::Trio: return TEXT("Trio");
	case ET66PartySize::Quad: return TEXT("Quad");
	default: return TEXT("Unknown");
	}
}

FString UT66LeaderboardSubsystem::GetCurrentLocalSteamId() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	return SteamHelper ? SteamHelper->GetLocalSteamId() : FString();
}

FString UT66LeaderboardSubsystem::GetCurrentLocalDisplayName() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	return SteamHelper ? SteamHelper->GetLocalDisplayName() : FString();
}

FString UT66LeaderboardSubsystem::BuildAccountSlotSuffix() const
{
	FString SteamId = GetCurrentLocalSteamId();
	SteamId.TrimStartAndEndInline();
	return SteamId.IsEmpty() ? TEXT("NoSteam") : SteamId;
}

FString UT66LeaderboardSubsystem::MakeResolvedLocalSaveSlotName() const
{
	return FString::Printf(TEXT("%s_%s"), LocalSaveSlotBaseName, *BuildAccountSlotSuffix());
}

FString UT66LeaderboardSubsystem::MakeResolvedAccountRestrictionRunSummarySlotName() const
{
	return FString::Printf(TEXT("%s_%s"), AccountRestrictionRunSummarySlotBaseName, *BuildAccountSlotSuffix());
}

FString UT66LeaderboardSubsystem::MakeLocalBestScoreRunSummarySlotName(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	return FString::Printf(
		TEXT("%s_%s_%s_%s"),
		LocalBestScoreRunSummarySlotBaseName,
		*DifficultyKey(Difficulty),
		*PartySizeKey(PartySize),
		*BuildAccountSlotSuffix());
}

FString UT66LeaderboardSubsystem::MakeRecentRunSummarySlotName(const FGuid& RunId) const
{
	return FString::Printf(
		TEXT("%s_%s_%s"),
		RecentRunSummarySlotBaseName,
		*BuildAccountSlotSuffix(),
		*RunId.ToString(EGuidFormats::DigitsWithHyphens));
}

bool UT66LeaderboardSubsystem::PruneInvalidRecentRunRecords()
{
	if (!LocalSave)
	{
		return false;
	}

	bool bChanged = false;
	for (int32 Index = LocalSave->RecentRuns.Num() - 1; Index >= 0; --Index)
	{
		const FT66RecentRunRecord& Record = LocalSave->RecentRuns[Index];
		if (Record.RunSummarySlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(Record.RunSummarySlotName, 0))
		{
			continue;
		}

		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(Record.RunSummarySlotName, 0);
		const UT66LeaderboardRunSummarySaveGame* Snapshot = Cast<UT66LeaderboardRunSummarySaveGame>(Loaded);
		if (!Snapshot)
		{
			continue;
		}

		const bool bLooksLikeFrontendPlaceholder =
			Snapshot->EntryId.IsEmpty()
			&& Snapshot->Score <= 0
			&& Snapshot->StageReached <= 1
			&& Snapshot->RunDurationSeconds <= KINDA_SMALL_NUMBER
			&& !Snapshot->bWasFullClear
			&& Snapshot->EventLog.Num() == 0
			&& Snapshot->DamageBySource.Num() == 0;
		if (!bLooksLikeFrontendPlaceholder)
		{
			continue;
		}

		UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: pruning invalid recent-run placeholder snapshot %s."), *Record.RunSummarySlotName);
		UGameplayStatics::DeleteGameInSlot(Record.RunSummarySlotName, 0);
		LocalSave->RecentRuns.RemoveAt(Index);
		bChanged = true;
	}

	return bChanged;
}

bool UT66LeaderboardSubsystem::SyncRecentRunRecordsFromSnapshots()
{
	if (!LocalSave)
	{
		return false;
	}

	bool bChanged = false;
	for (FT66RecentRunRecord& Record : LocalSave->RecentRuns)
	{
		if (Record.RunSummarySlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(Record.RunSummarySlotName, 0))
		{
			continue;
		}

		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(Record.RunSummarySlotName, 0);
		const UT66LeaderboardRunSummarySaveGame* Snapshot = Cast<UT66LeaderboardRunSummarySaveGame>(Loaded);
		if (!Snapshot)
		{
			continue;
		}

		bool bRecordChanged = false;
		auto SyncField = [&bRecordChanged](auto& Target, const auto& Source)
		{
			if (!(Target == Source))
			{
				Target = Source;
				bRecordChanged = true;
			}
		};

		if (Snapshot->RunEndedAtUtc > FDateTime::MinValue())
		{
			SyncField(Record.EndedAtUtc, Snapshot->RunEndedAtUtc);
		}

		SyncField(Record.Difficulty, Snapshot->Difficulty);
		SyncField(Record.PartySize, Snapshot->PartySize);
		SyncField(Record.HeroID, Snapshot->HeroID);
		SyncField(Record.CompanionID, Snapshot->CompanionID);
		SyncField(Record.Score, Snapshot->Score);
		SyncField(Record.StageReached, Snapshot->StageReached);
		SyncField(Record.DurationSeconds, Snapshot->RunDurationSeconds);
		SyncField(Record.bWasFullClear, Snapshot->bWasFullClear);
		SyncField(Record.bWasSpeedRunMode, Snapshot->bWasSpeedRunMode);

		bChanged = bChanged || bRecordChanged;
	}

	return bChanged;
}

bool UT66LeaderboardSubsystem::HasLocalBestScoreRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	if (const FT66LocalScoreRecord* Record = FindLocalScoreRecord(Difficulty, PartySize))
	{
		if (!Record->RunSummarySlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(Record->RunSummarySlotName, 0))
		{
			return true;
		}
	}

	const FString LegacySlotName = MakeLocalBestScoreRunSummarySlotName(Difficulty, PartySize);
	return UGameplayStatics::DoesSaveGameExist(LegacySlotName, 0);
}

UT66LeaderboardRunSummarySaveGame* UT66LeaderboardSubsystem::CreateCurrentRunSummarySnapshot(
	ET66LeaderboardType LeaderboardType,
	ET66Difficulty Difficulty,
	ET66PartySize PartySize,
	int32 Score) const
{
	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66RngSubsystem* Rng = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66SkillRatingSubsystem* Skill = GI ? GI->GetSubsystem<UT66SkillRatingSubsystem>() : nullptr;
	UT66RunIntegritySubsystem* Integrity = GI ? GI->GetSubsystem<UT66RunIntegritySubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	if (!T66GI || !RunState)
	{
		return nullptr;
	}

	UT66LeaderboardRunSummarySaveGame* Snapshot =
		Cast<UT66LeaderboardRunSummarySaveGame>(UGameplayStatics::CreateSaveGameObject(UT66LeaderboardRunSummarySaveGame::StaticClass()));
	if (!Snapshot)
	{
		return nullptr;
	}

	Snapshot->SchemaVersion = 19;
	Snapshot->LeaderboardType = LeaderboardType;
	Snapshot->Difficulty = Difficulty;
	Snapshot->PartySize = PartySize;
	Snapshot->SavedAtUtc = FDateTime::UtcNow();
	Snapshot->RunEndedAtUtc = FDateTime::UtcNow();
	Snapshot->RunDurationSeconds = RunState->GetFinalRunElapsedSeconds();
	Snapshot->bWasFullClear = RunState->DidRunEndInVictory() || RunState->HasPendingDifficultyClearSummary();
	Snapshot->bWasSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;

	Snapshot->StageReached = FMath::Clamp(RunState->GetCurrentStage(), 1, 23);
	Snapshot->Score = FMath::Max(0, Score);

	Snapshot->SecondaryStatValues.Reset();
	for (int32 i = 1; i <= static_cast<int32>(ET66SecondaryStatType::Accuracy); ++i)
	{
		const ET66SecondaryStatType SecType = static_cast<ET66SecondaryStatType>(i);
		if (!T66IsLiveSecondaryStatType(SecType))
		{
			continue;
		}
		Snapshot->SecondaryStatValues.Add(SecType, RunState->GetSecondaryStatValue(SecType));
	}

	Snapshot->HeroID = T66GI->SelectedHeroID;
	Snapshot->HeroBodyType = T66GI->SelectedHeroBodyType;
	Snapshot->CompanionID = T66GI->SelectedCompanionID;
	Snapshot->CompanionBodyType = T66GI->SelectedCompanionBodyType;

	Snapshot->HeroLevel = FMath::Max(1, RunState->GetHeroLevel());
	Snapshot->DamageStat = FMath::Max(1, RunState->GetDamageStat());
	Snapshot->AttackSpeedStat = FMath::Max(1, RunState->GetAttackSpeedStat());
	Snapshot->AttackScaleStat = FMath::Max(1, RunState->GetScaleStat());
	Snapshot->AccuracyStat = FMath::Max(1, RunState->GetAccuracyStat());
	Snapshot->ArmorStat = FMath::Max(1, RunState->GetArmorStat());
	Snapshot->EvasionStat = FMath::Max(1, RunState->GetEvasionStat());
	Snapshot->LuckStat = FMath::Max(1, RunState->GetLuckStat());
	Snapshot->SpeedStat = FMath::Max(1, RunState->GetSpeedStat());

	Snapshot->LuckRating0To100 = RunState->GetLuckRating0To100();
	Snapshot->SeedLuck0To100 = RunState->GetSeedLuck0To100();
	Snapshot->LuckModifierPercent = RunState->GetTotalLuckModifierPercent();
	Snapshot->EffectiveLuck = RunState->GetEffectiveLuckValue();
	Snapshot->LuckRatingQuantity0To100 = RunState->GetLuckRatingQuantity0To100();
	Snapshot->LuckRatingQuality0To100 = RunState->GetLuckRatingQuality0To100();
	Snapshot->SkillRating0To100 = Skill ? Skill->GetSkillRating0To100() : -1;
	Snapshot->RunSeed = Rng ? Rng->GetRunSeed() : 0;
	Snapshot->LuckQuantitySampleCount = RunState->GetLuckQuantitySampleCount();
	Snapshot->LuckQualitySampleCount = RunState->GetLuckQualitySampleCount();
	RunState->GetLuckQuantityAccumulators(Snapshot->LuckQuantityAccumulators);
	RunState->GetLuckQualityAccumulators(Snapshot->LuckQualityAccumulators);
	RunState->GetAntiCheatLuckEvents(Snapshot->AntiCheatLuckEvents);
	Snapshot->bAntiCheatLuckEventsTruncated = RunState->AreAntiCheatLuckEventsTruncated();
	RunState->GetAntiCheatHitCheckEvents(Snapshot->AntiCheatHitCheckEvents);
	Snapshot->bAntiCheatHitCheckEventsTruncated = RunState->AreAntiCheatHitCheckEventsTruncated();
	Snapshot->IncomingHitChecks = RunState->GetIncomingHitCheckCount();
	Snapshot->DamageTakenHitCount = RunState->GetDamageTakenHitCount();
	Snapshot->DodgeCount = RunState->GetDodgeCount();
	Snapshot->MaxConsecutiveDodges = RunState->GetMaxConsecutiveDodges();
	Snapshot->TotalEvasionChance = RunState->GetTotalEvasionChanceAccumulated();
	RunState->GetAntiCheatEvasionBuckets(Snapshot->AntiCheatEvasionBuckets);
	RunState->GetAntiCheatPressureWindowSummary(Snapshot->AntiCheatPressureWindowSummary);
	RunState->GetAntiCheatGamblerSummaries(Snapshot->AntiCheatGamblerSummaries);
	RunState->GetAntiCheatGamblerEvents(Snapshot->AntiCheatGamblerEvents);
	Snapshot->bAntiCheatGamblerEventsTruncated = RunState->AreAntiCheatGamblerEventsTruncated();
	if (Integrity)
	{
		Integrity->FinalizeCurrentRun();
		Integrity->CopyCurrentContextTo(Snapshot->IntegrityContext);
	}

	// Proof-of-run link is user-provided post-run; default empty for new snapshots.
	Snapshot->ProofOfRunUrl = FString();
	Snapshot->bProofOfRunLocked = false;

	if (UT66IdolManagerSubsystem* IdolManager = GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr)
	{
		Snapshot->EquippedIdols = IdolManager->GetEquippedIdols();
		Snapshot->EquippedIdolTiers = IdolManager->GetEquippedIdolTierValues();
	}
	else
	{
		Snapshot->EquippedIdols = RunState->GetEquippedIdols();
		Snapshot->EquippedIdolTiers = RunState->GetEquippedIdolTierValues();
	}
	Snapshot->Inventory = RunState->GetInventory();
	if (UT66BuffSubsystem* Buffs = GI ? GI->GetSubsystem<UT66BuffSubsystem>() : nullptr)
	{
		Snapshot->TemporaryBuffSlots = Buffs->GetSelectedSingleUseBuffSlots();
	}
	Snapshot->EventLog = RunState->GetEventLog();
	Snapshot->StagePacingPoints = RunState->GetStagePacingPoints();

	if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
	{
		const TArray<FDamageLogEntry> Sorted = DamageLog->GetDamageBySourceSorted();
		for (const FDamageLogEntry& Entry : Sorted)
		{
			Snapshot->DamageBySource.Add(Entry.SourceID, Entry.TotalDamage);
		}
	}

	PopulateSnapshotLeaderboardRanks(*Snapshot, Difficulty, PartySize);
	return Snapshot;
}

bool UT66LeaderboardSubsystem::SaveRunSummarySnapshotToSlot(UT66LeaderboardRunSummarySaveGame* Snapshot, const FString& SlotName) const
{
	return Snapshot && !SlotName.IsEmpty() && UGameplayStatics::SaveGameToSlot(Snapshot, SlotName, 0);
}

bool UT66LeaderboardSubsystem::UpdateSavedRunSummaryRanks(
	const FString& SlotName,
	const int32 ScoreRankAlltime,
	const int32 ScoreRankWeekly,
	const int32 SpeedRunRankAlltime,
	const int32 SpeedRunRankWeekly) const
{
	if (SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return false;
	}

	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	UT66LeaderboardRunSummarySaveGame* Snapshot = Cast<UT66LeaderboardRunSummarySaveGame>(LoadedGame);
	if (!Snapshot)
	{
		return false;
	}

	Snapshot->SchemaVersion = FMath::Max(Snapshot->SchemaVersion, 18);
	Snapshot->ScoreRankAllTime = FMath::Max(0, ScoreRankAlltime);
	Snapshot->ScoreRankWeekly = FMath::Max(0, ScoreRankWeekly);
	Snapshot->SpeedRunRankAllTime = FMath::Max(0, SpeedRunRankAlltime);
	Snapshot->SpeedRunRankWeekly = FMath::Max(0, SpeedRunRankWeekly);
	return UGameplayStatics::SaveGameToSlot(Snapshot, SlotName, 0);
}

void UT66LeaderboardSubsystem::PopulateSnapshotLeaderboardRanks(
	UT66LeaderboardRunSummarySaveGame& Snapshot,
	const ET66Difficulty Difficulty,
	const ET66PartySize PartySize) const
{
	Snapshot.ScoreRankAllTime = GetLocalScoreRankAllTime(Difficulty, PartySize);
	Snapshot.ScoreRankWeekly = GetLocalScoreRankWeekly(Difficulty, PartySize);
	Snapshot.SpeedRunRankAllTime = GetLocalSpeedRunRankAllTime(Difficulty, PartySize);
	Snapshot.SpeedRunRankWeekly = GetLocalSpeedRunRankWeekly(Difficulty, PartySize);
}

bool UT66LeaderboardSubsystem::SaveLocalBestScoreRunSummarySnapshot(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Score, const FString& ExistingRunSummarySlotName) const
{
	if (!ExistingRunSummarySlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(ExistingRunSummarySlotName, 0))
	{
		return true;
	}

	UT66LeaderboardRunSummarySaveGame* Snapshot = CreateCurrentRunSummarySnapshot(ET66LeaderboardType::Score, Difficulty, PartySize, Score);
	if (!Snapshot)
	{
		return false;
	}

	const FString SlotName = MakeLocalBestScoreRunSummarySlotName(Difficulty, PartySize);
	return SaveRunSummarySnapshotToSlot(Snapshot, SlotName);
}

uint64 UT66LeaderboardSubsystem::MakeScoreKey(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
	return (static_cast<uint64>(Difficulty) << 8) | static_cast<uint64>(PartySize);
}

uint64 UT66LeaderboardSubsystem::MakeSpeedRunKey_DiffStage(ET66Difficulty Difficulty, int32 Stage)
{
	return (static_cast<uint64>(Difficulty) << 8) | static_cast<uint64>(FMath::Clamp(Stage, 0, 255));
}

const FT66LocalScoreRecord* UT66LeaderboardSubsystem::FindLocalScoreRecord(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	if (!LocalSave)
	{
		const_cast<UT66LeaderboardSubsystem*>(this)->LoadOrCreateLocalSave();
	}

	if (!LocalSave)
	{
		return nullptr;
	}

	for (const FT66LocalScoreRecord& Record : LocalSave->ScoreRecords)
	{
		if (Record.Difficulty == Difficulty && Record.PartySize == PartySize)
		{
			return &Record;
		}
	}

	return nullptr;
}

FT66LocalScoreRecord* UT66LeaderboardSubsystem::FindLocalScoreRecordMutable(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		return nullptr;
	}

	for (FT66LocalScoreRecord& Record : LocalSave->ScoreRecords)
	{
		if (Record.Difficulty == Difficulty && Record.PartySize == PartySize)
		{
			return &Record;
		}
	}

	return nullptr;
}

const FT66LocalCompletedRunTimeRecord* UT66LeaderboardSubsystem::FindLocalCompletedRunTimeRecord(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	if (!LocalSave)
	{
		const_cast<UT66LeaderboardSubsystem*>(this)->LoadOrCreateLocalSave();
	}

	if (!LocalSave)
	{
		return nullptr;
	}

	for (const FT66LocalCompletedRunTimeRecord& Record : LocalSave->CompletedRunTimeRecords)
	{
		if (Record.Difficulty == Difficulty && Record.PartySize == PartySize)
		{
			return &Record;
		}
	}

	return nullptr;
}

FT66LocalCompletedRunTimeRecord* UT66LeaderboardSubsystem::FindLocalCompletedRunTimeRecordMutable(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		return nullptr;
	}

	for (FT66LocalCompletedRunTimeRecord& Record : LocalSave->CompletedRunTimeRecords)
	{
		if (Record.Difficulty == Difficulty && Record.PartySize == PartySize)
		{
			return &Record;
		}
	}

	return nullptr;
}

bool UT66LeaderboardSubsystem::LoadTargetsFromDataTablesIfPresent()
{
	const bool bNeedScoreTargets = ScoreTarget10ByKey.Num() <= 0;
	const bool bNeedSpeedRunTargets = SpeedRunTarget10ByKey_DiffStage.Num() <= 0;

	if (!bNeedScoreTargets && !bNeedSpeedRunTargets)
	{
		return true;
	}

	TSoftObjectPtr<UDataTable> ScoreDT = bNeedScoreTargets
		? TSoftObjectPtr<UDataTable>(FSoftObjectPath(ScoreTargetsDTPath))
		: TSoftObjectPtr<UDataTable>();
	TSoftObjectPtr<UDataTable> SpeedDT = bNeedSpeedRunTargets
		? TSoftObjectPtr<UDataTable>(FSoftObjectPath(SpeedRunTargetsDTPath))
		: TSoftObjectPtr<UDataTable>();

	UDataTable* SrcDT = bNeedScoreTargets ? ScoreDT.LoadSynchronous() : nullptr;
	UDataTable* SDT = bNeedSpeedRunTargets ? SpeedDT.LoadSynchronous() : nullptr;

	// If neither exists, caller should fall back to CSV.
	if (!SrcDT && !SDT)
	{
		return false;
	}

	if (SrcDT)
	{
		if (SrcDT->GetRowStruct() != FT66LeaderboardScoreTargetRow::StaticStruct())
		{
			UE_LOG(LogT66Leaderboard, Warning, TEXT("DT_Leaderboard_ScoreTargets exists but has wrong RowStruct. Expected FT66LeaderboardScoreTargetRow."));
		}
		else
		{
			static const FString Ctx(TEXT("ScoreTargetsDT"));
			const int32 InitialCount = ScoreTarget10ByKey.Num();
			TArray<FT66LeaderboardScoreTargetRow*> Rows;
			SrcDT->GetAllRows<FT66LeaderboardScoreTargetRow>(Ctx, Rows);
			for (const FT66LeaderboardScoreTargetRow* R : Rows)
			{
				if (!R) continue;
				if (R->TargetScore10 <= 0) continue;
				ScoreTarget10ByKey.Add(MakeScoreKey(R->Difficulty, R->PartySize), R->TargetScore10);
			}

			if (ScoreTarget10ByKey.Num() > InitialCount)
			{
				ScoreTargetRuntimeSource = ET66RuntimeDataSource::CookedDataTable;
				UE_LOG(
					LogT66RuntimeData,
					Log,
					TEXT("RuntimeData: loaded leaderboard score targets from cooked DataTable '%s' Added=%d Total=%d"),
					ScoreTargetsDTPath,
					ScoreTarget10ByKey.Num() - InitialCount,
					ScoreTarget10ByKey.Num());
			}
		}
	}
	else if (bNeedScoreTargets)
	{
		UE_LOG(LogT66RuntimeData, Warning, TEXT("RuntimeData: cooked leaderboard score target DataTable missing at '%s'"), ScoreTargetsDTPath);
	}

	if (SDT)
	{
		if (SDT->GetRowStruct() != FT66LeaderboardSpeedRunTargetRow::StaticStruct())
		{
			UE_LOG(LogT66Leaderboard, Warning, TEXT("DT_Leaderboard_SpeedrunTargets exists but has wrong RowStruct. Expected FT66LeaderboardSpeedRunTargetRow."));
		}
		else
		{
			static const FString Ctx(TEXT("SpeedRunTargetsDT"));
			const int32 InitialCount = SpeedRunTarget10ByKey_DiffStage.Num();
			TArray<FT66LeaderboardSpeedRunTargetRow*> Rows;
			SDT->GetAllRows<FT66LeaderboardSpeedRunTargetRow>(Ctx, Rows);
			for (const FT66LeaderboardSpeedRunTargetRow* R : Rows)
			{
				if (!R) continue;
				if (R->Stage <= 0 || R->TargetTime10Seconds <= 0.f) continue;
				SpeedRunTarget10ByKey_DiffStage.Add(MakeSpeedRunKey_DiffStage(R->Difficulty, R->Stage), R->TargetTime10Seconds);
			}

			if (SpeedRunTarget10ByKey_DiffStage.Num() > InitialCount)
			{
				SpeedRunTargetRuntimeSource = ET66RuntimeDataSource::CookedDataTable;
				UE_LOG(
					LogT66RuntimeData,
					Log,
					TEXT("RuntimeData: loaded leaderboard speed run targets from cooked DataTable '%s' Added=%d Total=%d"),
					SpeedRunTargetsDTPath,
					SpeedRunTarget10ByKey_DiffStage.Num() - InitialCount,
					SpeedRunTarget10ByKey_DiffStage.Num());
			}
		}
	}
	else if (bNeedSpeedRunTargets)
	{
		UE_LOG(LogT66RuntimeData, Warning, TEXT("RuntimeData: cooked leaderboard speed run target DataTable missing at '%s'"), SpeedRunTargetsDTPath);
	}

	// Success if we loaded the missing target set(s) from DTs.
	return (!bNeedScoreTargets || ScoreTarget10ByKey.Num() > 0)
		&& (!bNeedSpeedRunTargets || SpeedRunTarget10ByKey_DiffStage.Num() > 0);
}
int64 UT66LeaderboardSubsystem::GetScoreTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	const uint64 Key = MakeScoreKey(Difficulty, PartySize);
	if (const int64* Found = ScoreTarget10ByKey.Find(Key))
	{
		return *Found;
	}

	// Fallback tuning (lower numbers so the player can realistically break into Top 10).
	const int32 DiffIndex = FMath::Clamp(static_cast<int32>(Difficulty), 0, 999);
	const int32 PartyIndex = FMath::Clamp(static_cast<int32>(PartySize), 0, 3);
	return static_cast<int64>(600 + (DiffIndex * 400) + (PartyIndex * 150));
}

float UT66LeaderboardSubsystem::GetSpeedRunTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const
{
	const uint64 Key = MakeSpeedRunKey_DiffStage(Difficulty, Stage);
	float Base = 0.f;
	if (const float* Found = SpeedRunTarget10ByKey_DiffStage.Find(Key))
	{
		Base = *Found;
	}
	else
	{
		// Fallback tuning: stage-based, difficulty-based, and scaled slightly by party size.
		const int32 DiffIndex = FMath::Clamp(static_cast<int32>(Difficulty), 0, 999);
		const int32 S = FMath::Clamp(Stage, 1, 23);
		Base = 35.f + (static_cast<float>(S) * 12.f) + (static_cast<float>(DiffIndex) * 6.f);
	}

	const float PartyMult =
		(PartySize == ET66PartySize::Solo) ? 1.0f :
		(PartySize == ET66PartySize::Duo) ? 1.06f :
		(PartySize == ET66PartySize::Trio) ? 1.12f : 1.18f;

	return FMath::Max(1.f, Base * PartyMult);
}

bool UT66LeaderboardSubsystem::GetSpeedRunTarget10Seconds(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage, float& OutSeconds) const
{
	if (Stage <= 0) return false;
	OutSeconds = GetSpeedRunTarget10(Difficulty, PartySize, Stage);
	return OutSeconds > 0.f;
}

bool UT66LeaderboardSubsystem::SubmitRunScore(int32 Score, const FString& ExistingRunSummarySlotName)
{
	UGameInstance* GICheck = GetGameInstance();
	UT66GameInstance* T66GICheck = GICheck ? Cast<UT66GameInstance>(GICheck) : nullptr;
	if (T66GICheck && T66GICheck->bRunIneligibleForLeaderboard)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: score submit blocked because the run is marked ineligible. Score=%d"), Score);
		return false;
	}

	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: score submit blocked because local leaderboard save was unavailable. Score=%d"), Score);
		return false;
	}

	Score = FMath::Max(0, Score);
	bLastScoreWasNewBest = false;
	LastSubmittedScoreRankAllTime = 0;
	LastSubmittedScoreRankWeekly = 0;

	if (!CanAttemptBackendRankedSubmission(TEXT("score")))
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: blocked (account status). Score=%d"), Score);
		return false;
	}

	bool bPractice = false;
	bool bAnon = false;
	(void)GetSettingsPracticeAndAnon(bPractice, bAnon);
	if (bPractice)
	{
		UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: blocked (Practice Mode). Score=%d"), Score);
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	const ET66Difficulty Diff = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66PartySize Party = T66GI ? T66GI->SelectedPartySize : ET66PartySize::Solo;
	const FString ResolvedRunSummarySlotName =
		(!ExistingRunSummarySlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(ExistingRunSummarySlotName, 0))
		? ExistingRunSummarySlotName
		: FString();

	// Ranked state only updates after backend acceptance.
	if (UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		if (Backend->IsBackendConfigured() && Backend->HasSteamTicket())
		{
			UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			const int32 StageReached = RunState ? RunState->GetCurrentStage() : 1;
			const int32 TimeMs = 0;
			UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
			const FString DisplayName = bAnon
				? TEXT("ANONYMOUS")
				: ((PartySubsystem && !PartySubsystem->GetLocalDisplayName().IsEmpty()) ? PartySubsystem->GetLocalDisplayName() : TEXT("Player"));
			const FString HeroId = T66GI ? T66GI->SelectedHeroID.ToString() : TEXT("unknown");
			const FString CompanionId = T66GI ? T66GI->SelectedCompanionID.ToString() : TEXT("unknown");

			UT66LeaderboardRunSummarySaveGame* BackendSnapshot = CreateCurrentRunSummarySnapshot(ET66LeaderboardType::Score, Diff, Party, Score);
			const FString SubmitRequestKey = MakePendingBestRankRequestKey(EBestRankRecordType::Score, Diff, Party);
			FPendingBestRankSubmission& Pending = PendingBestRankSubmissions.Add(SubmitRequestKey);
			Pending.Type = EBestRankRecordType::Score;
			Pending.Difficulty = Diff;
			Pending.PartySize = Party;
			Pending.Score = static_cast<int64>(Score);
			Pending.Seconds = 0.f;
			Pending.bSubmittedAnonymous = bAnon;
			Pending.RunSummarySlotName = ResolvedRunSummarySlotName;
			Pending.AchievedAtUtc = FDateTime::UtcNow();
			LastSubmittedScoreRequestKey = SubmitRequestKey;

			Backend->SubmitRunToBackend(DisplayName, Score, TimeMs, Diff, Party, StageReached, HeroId, CompanionId, BackendSnapshot, SubmitRequestKey);
			UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: queued score submit for backend acceptance. Score=%d Anonymous=%s"), Score, bAnon ? TEXT("true") : TEXT("false"));
			return true;
		}
	}

	UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: score submit skipped because backend auth is unavailable. Score=%d"), Score);
	return false;
}

bool UT66LeaderboardSubsystem::SubmitCompletedRunTime(float Seconds, const FString& ExistingRunSummarySlotName)
{
	UGameInstance* GICheck = GetGameInstance();
	UT66GameInstance* T66GICheck = GICheck ? Cast<UT66GameInstance>(GICheck) : nullptr;
	if (T66GICheck && T66GICheck->bRunIneligibleForLeaderboard)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: completed-run submit blocked because the run is marked ineligible. Seconds=%.3f"), Seconds);
		return false;
	}

	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: completed-run submit blocked because local leaderboard save was unavailable. Seconds=%.3f"), Seconds);
		return false;
	}

	Seconds = FMath::Max(0.f, Seconds);
	bLastCompletedRunTimeWasNewBest = false;
	LastSubmittedCompletedRunRankAllTime = 0;
	LastSubmittedCompletedRunRankWeekly = 0;
	if (Seconds <= 0.01f)
	{
		return false;
	}

	if (!CanAttemptBackendRankedSubmission(TEXT("completed-run")))
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: completed run time blocked (account status). Seconds=%.3f"), Seconds);
		return false;
	}

	bool bPractice = false;
	bool bAnon = false;
	(void)GetSettingsPracticeAndAnon(bPractice, bAnon);
	if (bPractice)
	{
		UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: completed run time blocked (Practice Mode). Seconds=%.3f"), Seconds);
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	const ET66Difficulty Diff = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66PartySize Party = T66GI ? T66GI->SelectedPartySize : ET66PartySize::Solo;
	const FString ResolvedRunSummarySlotName =
		(!ExistingRunSummarySlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(ExistingRunSummarySlotName, 0))
		? ExistingRunSummarySlotName
		: FString();
	if (UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		if (Backend->IsBackendConfigured() && Backend->HasSteamTicket())
		{
			UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			const int32 Score = RunState ? RunState->GetCurrentScore() : 0;
			const int32 StageReached = RunState ? RunState->GetCurrentStage() : 1;
			const int32 TimeMs = FMath::RoundToInt(Seconds * 1000.0f);
			UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
			const FString DisplayName = bAnon
				? TEXT("ANONYMOUS")
				: ((PartySubsystem && !PartySubsystem->GetLocalDisplayName().IsEmpty()) ? PartySubsystem->GetLocalDisplayName() : TEXT("Player"));
			const FString HeroId = T66GI ? T66GI->SelectedHeroID.ToString() : TEXT("unknown");
			const FString CompanionId = T66GI ? T66GI->SelectedCompanionID.ToString() : TEXT("unknown");

			UT66LeaderboardRunSummarySaveGame* BackendSnapshot = CreateCurrentRunSummarySnapshot(ET66LeaderboardType::SpeedRun, Diff, Party, Score);
			const FString SubmitRequestKey = MakePendingBestRankRequestKey(EBestRankRecordType::CompletedRunTime, Diff, Party);
			FPendingBestRankSubmission& Pending = PendingBestRankSubmissions.Add(SubmitRequestKey);
			Pending.Type = EBestRankRecordType::CompletedRunTime;
			Pending.Difficulty = Diff;
			Pending.PartySize = Party;
			Pending.Score = static_cast<int64>(Score);
			Pending.Seconds = Seconds;
			Pending.bSubmittedAnonymous = bAnon;
			Pending.RunSummarySlotName = ResolvedRunSummarySlotName;
			Pending.AchievedAtUtc = FDateTime::UtcNow();
			LastSubmittedCompletedRunTimeRequestKey = SubmitRequestKey;

			Backend->SubmitRunToBackend(DisplayName, Score, TimeMs, Diff, Party, StageReached, HeroId, CompanionId, BackendSnapshot, SubmitRequestKey);
			UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: queued completed-run submit for backend acceptance. Seconds=%.3f Anonymous=%s"), Seconds, bAnon ? TEXT("true") : TEXT("false"));
			return true;
		}
	}
	UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: completed-run submit skipped because backend auth is unavailable. Seconds=%.3f"), Seconds);
	return false;
}

bool UT66LeaderboardSubsystem::SubmitDifficultyClearRun(float Seconds, const FString& ExistingRunSummarySlotName)
{
	UGameInstance* GICheck = GetGameInstance();
	UT66GameInstance* T66GICheck = GICheck ? Cast<UT66GameInstance>(GICheck) : nullptr;
	if (T66GICheck && T66GICheck->bRunIneligibleForLeaderboard)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: difficulty-clear submit blocked because the run is marked ineligible. Seconds=%.3f"), Seconds);
		return false;
	}

	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: difficulty-clear submit blocked because local leaderboard save was unavailable. Seconds=%.3f"), Seconds);
		return false;
	}

	Seconds = FMath::Max(0.f, Seconds);
	bLastScoreWasNewBest = false;
	bLastCompletedRunTimeWasNewBest = false;
	LastSubmittedScoreRankAllTime = 0;
	LastSubmittedScoreRankWeekly = 0;
	LastSubmittedCompletedRunRankAllTime = 0;
	LastSubmittedCompletedRunRankWeekly = 0;
	if (Seconds <= 0.01f)
	{
		return false;
	}

	if (!CanAttemptBackendRankedSubmission(TEXT("difficulty-clear")))
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: difficulty-clear submit blocked (account status). Seconds=%.3f"), Seconds);
		return false;
	}

	bool bPractice = false;
	bool bAnon = false;
	(void)GetSettingsPracticeAndAnon(bPractice, bAnon);
	if (bPractice)
	{
		UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: difficulty-clear submit blocked (Practice Mode). Seconds=%.3f"), Seconds);
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	const ET66Difficulty Diff = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66PartySize Party = T66GI ? T66GI->SelectedPartySize : ET66PartySize::Solo;
	const FString ResolvedRunSummarySlotName =
		(!ExistingRunSummarySlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(ExistingRunSummarySlotName, 0))
		? ExistingRunSummarySlotName
		: FString();
	if (UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		if (Backend->IsBackendConfigured() && Backend->HasSteamTicket())
		{
			UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			const int32 Score = RunState ? RunState->GetCurrentScore() : 0;
			const int32 StageReached = RunState ? RunState->GetCurrentStage() : 1;
			const int32 TimeMs = FMath::RoundToInt(Seconds * 1000.0f);
			UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
			const FString DisplayName = bAnon
				? TEXT("ANONYMOUS")
				: ((PartySubsystem && !PartySubsystem->GetLocalDisplayName().IsEmpty()) ? PartySubsystem->GetLocalDisplayName() : TEXT("Player"));
			const FString HeroId = T66GI ? T66GI->SelectedHeroID.ToString() : TEXT("unknown");
			const FString CompanionId = T66GI ? T66GI->SelectedCompanionID.ToString() : TEXT("unknown");

			UT66LeaderboardRunSummarySaveGame* BackendSnapshot = CreateCurrentRunSummarySnapshot(ET66LeaderboardType::SpeedRun, Diff, Party, Score);
			const FString SubmitRequestKey = MakePendingBestRankRequestKey(EBestRankRecordType::DifficultyClear, Diff, Party);
			FPendingBestRankSubmission& Pending = PendingBestRankSubmissions.Add(SubmitRequestKey);
			Pending.Type = EBestRankRecordType::DifficultyClear;
			Pending.Difficulty = Diff;
			Pending.PartySize = Party;
			Pending.Score = static_cast<int64>(Score);
			Pending.Seconds = Seconds;
			Pending.bSubmittedAnonymous = bAnon;
			Pending.RunSummarySlotName = ResolvedRunSummarySlotName;
			Pending.AchievedAtUtc = FDateTime::UtcNow();
			LastSubmittedScoreRequestKey = SubmitRequestKey;
			LastSubmittedCompletedRunTimeRequestKey = SubmitRequestKey;

			Backend->SubmitRunToBackend(DisplayName, Score, TimeMs, Diff, Party, StageReached, HeroId, CompanionId, BackendSnapshot, SubmitRequestKey);
			UE_LOG(
				LogT66Leaderboard,
				Log,
				TEXT("Leaderboard: queued single-request difficulty-clear submit for backend acceptance. Score=%d Seconds=%.3f Anonymous=%s"),
				Score,
				Seconds,
				bAnon ? TEXT("true") : TEXT("false"));
			return true;
		}
	}

	UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: difficulty-clear submit skipped because backend auth is unavailable. Seconds=%.3f"), Seconds);
	return false;
}

bool UT66LeaderboardSubsystem::SubmitStageSpeedRunTime(int32 Stage, float Seconds)
{
	UGameInstance* GICheck = GetGameInstance();
	UT66GameInstance* T66GICheck = GICheck ? Cast<UT66GameInstance>(GICheck) : nullptr;
	if (T66GICheck && T66GICheck->bRunIneligibleForLeaderboard)
	{
		return false;
	}

	Stage = FMath::Clamp(Stage, 1, 23);
	Seconds = FMath::Max(0.f, Seconds);
	if (Seconds <= 0.01f) return false;
	bLastSpeedRunWasNewBest = false;
	LastSpeedRunSubmittedStage = Stage;

	if (!CanAttemptBackendRankedSubmission(TEXT("stage-speedrun")))
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: blocked (account status). Stage=%d Seconds=%.3f"), Stage, Seconds);
		return false;
	}

	bool bPractice = false;
	bool bAnon = false;
	(void)GetSettingsPracticeAndAnon(bPractice, bAnon);
	if (bPractice)
	{
		UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: blocked (Practice Mode). Stage=%d Seconds=%.3f"), Stage, Seconds);
		return false;
	}

	UE_LOG(
		LogT66Leaderboard,
		Log,
		TEXT("Leaderboard: stage speedrun timing is summary-only. Stage=%d Seconds=%.3f Anonymous=%s"),
		Stage,
		Seconds,
		bAnon ? TEXT("true") : TEXT("false"));
	return false;
}

void UT66LeaderboardSubsystem::RequestOpenLocalBestScoreRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
	PendingFakeSnapshot = nullptr;
	if (const FT66LocalScoreRecord* Record = FindLocalScoreRecord(Difficulty, PartySize))
	{
		if (!Record->RunSummarySlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(Record->RunSummarySlotName, 0))
		{
			PendingRunSummarySlotName = Record->RunSummarySlotName;
			return;
		}
	}

	PendingRunSummarySlotName = MakeLocalBestScoreRunSummarySlotName(Difficulty, PartySize);
}

bool UT66LeaderboardSubsystem::RequestOpenRunSummarySlot(const FString& SlotName, ET66ScreenType ReturnModalAfterClose)
{
	if (SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return false;
	}

	PendingFakeSnapshot = nullptr;
	PendingRunSummarySlotName = SlotName;
	PendingReturnModalAfterViewerRunSummary = ReturnModalAfterClose;
	return true;
}

bool UT66LeaderboardSubsystem::ConsumePendingRunSummaryRequest(FString& OutSaveSlotName)
{
	if (PendingRunSummarySlotName.IsEmpty())
	{
		return false;
	}
	OutSaveSlotName = PendingRunSummarySlotName;
	PendingRunSummarySlotName.Reset();
	return true;
}

void UT66LeaderboardSubsystem::SetPendingFakeRunSummarySnapshot(UT66LeaderboardRunSummarySaveGame* Snapshot)
{
	PendingRunSummarySlotName.Reset();
	PendingFakeSnapshot = Snapshot;
}

UT66LeaderboardRunSummarySaveGame* UT66LeaderboardSubsystem::ConsumePendingFakeRunSummarySnapshot()
{
	UT66LeaderboardRunSummarySaveGame* Out = PendingFakeSnapshot;
	PendingFakeSnapshot = nullptr;
	return Out;
}

TArray<FT66RecentRunRecord> UT66LeaderboardSubsystem::GetRecentRuns() const
{
	if (!LocalSave)
	{
		const_cast<UT66LeaderboardSubsystem*>(this)->LoadOrCreateLocalSave();
	}
	else if (const_cast<UT66LeaderboardSubsystem*>(this)->SyncRecentRunRecordsFromSnapshots())
	{
		const_cast<UT66LeaderboardSubsystem*>(this)->SaveLocalSave();
	}

	return LocalSave ? LocalSave->RecentRuns : TArray<FT66RecentRunRecord>();
}

bool UT66LeaderboardSubsystem::GetLocalBestScoreRecord(ET66Difficulty Difficulty, ET66PartySize PartySize, FT66LocalScoreRecord& OutRecord) const
{
	if (const FT66LocalScoreRecord* Record = FindLocalScoreRecord(Difficulty, PartySize))
	{
		OutRecord = *Record;
		return true;
	}

	return false;
}

bool UT66LeaderboardSubsystem::GetLocalBestCompletedRunTimeRecord(ET66Difficulty Difficulty, ET66PartySize PartySize, FT66LocalCompletedRunTimeRecord& OutRecord) const
{
	if (const FT66LocalCompletedRunTimeRecord* Record = FindLocalCompletedRunTimeRecord(Difficulty, PartySize))
	{
		OutRecord = *Record;
		return true;
	}

	return false;
}

void UT66LeaderboardSubsystem::SetPendingPickerSnapshots(TArray<UT66LeaderboardRunSummarySaveGame*> Snapshots)
{
	PendingPickerSnapshots.Reset();
	for (UT66LeaderboardRunSummarySaveGame* P : Snapshots)
	{
		if (P) PendingPickerSnapshots.Add(P);
	}
}

const TArray<TObjectPtr<UT66LeaderboardRunSummarySaveGame>>& UT66LeaderboardSubsystem::GetPendingPickerSnapshots() const
{
	return PendingPickerSnapshots;
}

void UT66LeaderboardSubsystem::ClearPendingPickerSnapshots()
{
	PendingPickerSnapshots.Reset();
}

bool UT66LeaderboardSubsystem::SaveFinishedRunSummarySnapshot(FString& OutSlotName)
{
	OutSlotName.Reset();
	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: could not save run summary snapshot because local leaderboard save was unavailable."));
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	if (!T66GI || !RunState)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: could not save run summary snapshot because runtime state was unavailable."));
		return false;
	}
	if (!RunState->HasPendingDifficultyClearSummary() && !RunState->HasRunEnded())
	{
		UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: refusing to save run summary snapshot because the run has not ended and no difficulty-clear summary is pending."));
		return false;
	}

	const FGuid RunId = FGuid::NewGuid();
	const FString SlotName = MakeRecentRunSummarySlotName(RunId);
	const ET66LeaderboardType LeaderboardType = (PS && PS->GetSpeedRunMode()) ? ET66LeaderboardType::SpeedRun : ET66LeaderboardType::Score;
	UT66LeaderboardRunSummarySaveGame* Snapshot = CreateCurrentRunSummarySnapshot(
		LeaderboardType,
		T66GI->SelectedDifficulty,
		T66GI->SelectedPartySize,
		RunState->GetCurrentScore());
	if (!Snapshot || !SaveRunSummarySnapshotToSlot(Snapshot, SlotName))
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("Leaderboard: failed to persist run summary snapshot to slot %s."), *SlotName);
		return false;
	}

	OutSlotName = SlotName;
	AppendRecentRunRecord(SlotName);
	SaveLocalSave();
	return true;
}

void UT66LeaderboardSubsystem::AppendRecentRunRecord(const FString& RunSummarySlotName)
{
	if (!LocalSave || RunSummarySlotName.IsEmpty())
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	if (!T66GI || !RunState)
	{
		return;
	}

	FT66RecentRunRecord Record;
	Record.RunId = FGuid::NewGuid();
	Record.RunSummarySlotName = RunSummarySlotName;
	Record.EndedAtUtc = FDateTime::UtcNow();
	Record.Difficulty = T66GI->SelectedDifficulty;
	Record.PartySize = T66GI->SelectedPartySize;
	Record.HeroID = T66GI->SelectedHeroID;
	Record.CompanionID = T66GI->SelectedCompanionID;
	Record.Score = RunState->GetCurrentScore();
	Record.StageReached = RunState->GetCurrentStage();
	Record.DurationSeconds = RunState->GetFinalRunElapsedSeconds();
	Record.bWasFullClear = RunState->DidRunEndInVictory() || RunState->HasPendingDifficultyClearSummary();
	Record.bWasSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;

	if (UGameplayStatics::DoesSaveGameExist(RunSummarySlotName, 0))
	{
		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(RunSummarySlotName, 0);
		if (const UT66LeaderboardRunSummarySaveGame* Snapshot = Cast<UT66LeaderboardRunSummarySaveGame>(Loaded))
		{
			if (Snapshot->RunEndedAtUtc > FDateTime::MinValue())
			{
				Record.EndedAtUtc = Snapshot->RunEndedAtUtc;
			}
			Record.Difficulty = Snapshot->Difficulty;
			Record.PartySize = Snapshot->PartySize;
			Record.HeroID = Snapshot->HeroID;
			Record.CompanionID = Snapshot->CompanionID;
			Record.Score = Snapshot->Score;
			Record.StageReached = Snapshot->StageReached;
			Record.DurationSeconds = Snapshot->RunDurationSeconds;
			Record.bWasFullClear = Snapshot->bWasFullClear;
			Record.bWasSpeedRunMode = Snapshot->bWasSpeedRunMode;
		}
	}

	LocalSave->RecentRuns.Insert(Record, 0);
}

bool UT66LeaderboardSubsystem::IsRunSummarySlotStillReferenced(const FString& SlotName) const
{
	if (!LocalSave || SlotName.IsEmpty())
	{
		return false;
	}

	for (const FT66RecentRunRecord& Record : LocalSave->RecentRuns)
	{
		if (Record.RunSummarySlotName == SlotName)
		{
			return true;
		}
	}

	for (const FT66LocalScoreRecord& Record : LocalSave->ScoreRecords)
	{
		if (Record.RunSummarySlotName == SlotName || Record.BestRankRunSummarySlotName == SlotName)
		{
			return true;
		}
	}

	for (const FT66LocalCompletedRunTimeRecord& Record : LocalSave->CompletedRunTimeRecords)
	{
		if (Record.RunSummarySlotName == SlotName || Record.BestRankRunSummarySlotName == SlotName)
		{
			return true;
		}
	}

	return LocalSave->AccountRestriction.RunSummarySlotName == SlotName;
}

bool UT66LeaderboardSubsystem::DeleteRunSummarySlotIfUnreferenced(const FString& SlotName) const
{
	if (SlotName.IsEmpty() || IsRunSummarySlotStillReferenced(SlotName))
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

namespace
{
	constexpr int32 GVisibleLeaderboardEntryCount = 15;
	constexpr int32 GVisibleLeaderboardEntryCountWithLocal = GVisibleLeaderboardEntryCount + 1;
}

bool UT66LeaderboardSubsystem::ShouldShowAccountStatusButton() const
{
	return true;
}

void UT66LeaderboardSubsystem::RefreshAccountStatusFromBackend()
{
	if (bAccountStatusRefreshRequested)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend || !Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
	{
		return;
	}

	bAccountStatusRefreshRequested = true;
	Backend->FetchAccountStatus();
}

FT66AccountRestrictionRecord UT66LeaderboardSubsystem::GetAccountRestrictionRecord() const
{
	const int32 Force = CVarT66AccountStatusForce.GetValueOnGameThread();
	if (Force > 0)
	{
		FT66AccountRestrictionRecord DebugRec;
		DebugRec.Restriction = (Force >= 2) ? ET66AccountRestrictionKind::CheatingCertainty : ET66AccountRestrictionKind::Suspicion;
		DebugRec.RestrictionReason =
			(Force >= 2)
			? FString(TEXT("Forbidden Event: Impossible outcome detected"))
			: FString(TEXT("Too Lucky"));
		DebugRec.RunSummarySlotName = FString(); // optional; leave unset for now
		DebugRec.AppealStatus = ET66AppealReviewStatus::NotSubmitted;
		return DebugRec;
	}
	if (!LocalSave)
	{
		const_cast<UT66LeaderboardSubsystem*>(this)->LoadOrCreateLocalSave();
	}
	const_cast<UT66LeaderboardSubsystem*>(this)->RefreshAccountStatusFromBackend();
	return LocalSave ? LocalSave->AccountRestriction : FT66AccountRestrictionRecord();
}

bool UT66LeaderboardSubsystem::IsAccountEligibleForLeaderboard() const
{
	return GetAccountRestrictionRecord().Restriction == ET66AccountRestrictionKind::None;
}

bool UT66LeaderboardSubsystem::CanAttemptBackendRankedSubmission(const TCHAR* SubmissionLabel) const
{
	if (IsAccountEligibleForLeaderboard())
	{
		return true;
	}

	const UGameInstance* GI = GetGameInstance();
	const UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	const bool bBackendCanAuthoritativelyEvaluate =
		Backend
		&& Backend->IsBackendConfigured()
		&& Backend->HasSteamTicket();
	if (!bBackendCanAuthoritativelyEvaluate)
	{
		return false;
	}

	const_cast<UT66LeaderboardSubsystem*>(this)->RefreshAccountStatusFromBackend();
	UE_LOG(
		LogT66Leaderboard,
		Warning,
		TEXT("Leaderboard: cached account restriction present during %s submit; deferring final ranked eligibility verdict to backend."),
		SubmissionLabel ? SubmissionLabel : TEXT("unknown"));
	return true;
}

bool UT66LeaderboardSubsystem::HasAccountRestrictionRunSummary() const
{
	const FT66AccountRestrictionRecord Rec = GetAccountRestrictionRecord();
	if (!Rec.RunSummarySlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(Rec.RunSummarySlotName, 0))
	{
		return true;
	}

	if (Rec.Restriction == ET66AccountRestrictionKind::None)
	{
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	return Backend && !Backend->GetLastAccountRunSummaryId().IsEmpty();
}

bool UT66LeaderboardSubsystem::CanSubmitAccountAppeal() const
{
	if (bAccountAppealSubmitInFlight)
	{
		return false;
	}

	const FT66AccountRestrictionRecord Rec = GetAccountRestrictionRecord();
	if (Rec.Restriction != ET66AccountRestrictionKind::Suspicion)
	{
		return false;
	}
	return Rec.AppealStatus == ET66AppealReviewStatus::NotSubmitted;
}

void UT66LeaderboardSubsystem::SubmitAccountAppeal(const FString& Message, const FString& EvidenceUrl)
{
	if (!LocalSave)
	{
		LoadOrCreateLocalSave();
	}
	if (!LocalSave || !CanSubmitAccountAppeal())
	{
		return;
	}

	FString Msg = Message;
	FString Url = EvidenceUrl;
	Msg.TrimStartAndEndInline();
	Url.TrimStartAndEndInline();

	// Minimal guard: empty submissions do nothing.
	if (Msg.IsEmpty())
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("AccountStatus: appeal not submitted (empty message)."));
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (Backend->IsBackendConfigured() && Backend->HasSteamTicket())
			{
				bAccountAppealSubmitInFlight = true;
				PendingAppealMessage = Msg;
				PendingAppealEvidenceUrl = Url;
				Backend->SubmitAppeal(Msg, Url);
				return;
			}
		}
	}

	LocalSave->AccountRestriction.LastAppealMessage = Msg;
	LocalSave->AccountRestriction.LastEvidenceUrl = Url;
	LocalSave->AccountRestriction.AppealStatus = ET66AppealReviewStatus::UnderReview;
	SaveLocalSave();

	UE_LOG(LogT66Leaderboard, Log, TEXT("AccountStatus: appeal submitted. EvidenceUrl=%s"), *Url);
}

void UT66LeaderboardSubsystem::HandleBackendAppealSubmitComplete(bool bSuccess, const FString& Message)
{
	if (!bAccountAppealSubmitInFlight)
	{
		return;
	}

	bAccountAppealSubmitInFlight = false;

	if (!bSuccess)
	{
		UE_LOG(LogT66Leaderboard, Warning, TEXT("AccountStatus: backend appeal submission failed. %s"), *Message);
		PendingAppealMessage.Reset();
		PendingAppealEvidenceUrl.Reset();
		return;
	}

	LoadOrCreateLocalSave();
	if (LocalSave)
	{
		LocalSave->AccountRestriction.LastAppealMessage = PendingAppealMessage;
		LocalSave->AccountRestriction.LastEvidenceUrl = PendingAppealEvidenceUrl;
		LocalSave->AccountRestriction.AppealStatus = ET66AppealReviewStatus::UnderReview;
		SaveLocalSave();
	}

	PendingAppealMessage.Reset();
	PendingAppealEvidenceUrl.Reset();
	RefreshAccountStatusFromBackend();

	UE_LOG(LogT66Leaderboard, Log, TEXT("AccountStatus: backend appeal submission succeeded."));
}

void UT66LeaderboardSubsystem::HandleBackendRunSummaryReady(const FString& EntryId)
{
	if (EntryId.IsEmpty())
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		return;
	}

	const bool bMatchesPending = !PendingAccountRestrictionRunSummaryId.IsEmpty()
		&& PendingAccountRestrictionRunSummaryId.Equals(EntryId, ESearchCase::CaseSensitive);
	const bool bMatchesCurrentAccountStatus = Backend->GetLastAccountRunSummaryId().Equals(EntryId, ESearchCase::CaseSensitive);
	if (!bMatchesPending && !bMatchesCurrentAccountStatus)
	{
		return;
	}

	UT66LeaderboardRunSummarySaveGame* Snapshot = Backend->GetCachedRunSummary(EntryId);
	if (!Snapshot)
	{
		return;
	}

	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		return;
	}

	const FString RestrictionRunSummarySlotName = MakeResolvedAccountRestrictionRunSummarySlotName();

	if (SaveRunSummarySnapshotToSlot(Snapshot, RestrictionRunSummarySlotName))
	{
		LocalSave->AccountRestriction.RunSummarySlotName = RestrictionRunSummarySlotName;
		SaveLocalSave();
	}

	if (bMatchesPending)
	{
		PendingAccountRestrictionRunSummaryId.Reset();
	}
}

void UT66LeaderboardSubsystem::HandleBackendAccountStatusComplete(bool bSuccess, const FString& Restriction)
{
	bAccountStatusRefreshRequested = false;

	if (!bSuccess)
	{
		return;
	}

	LoadOrCreateLocalSave();
	if (!LocalSave)
	{
		return;
	}

	FT66AccountRestrictionRecord NewRecord;
	NewRecord.Restriction = T66RestrictionKindFromBackend(Restriction);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			NewRecord.RestrictionReason = Backend->GetLastAccountStatusReason();
			NewRecord.AppealStatus = T66AppealStatusFromBackend(Backend->GetLastAccountAppealStatus());
			const FString AccountRunSummaryId = Backend->GetLastAccountRunSummaryId();
			PendingAccountRestrictionRunSummaryId.Reset();
			const FString RestrictionRunSummarySlotName = MakeResolvedAccountRestrictionRunSummarySlotName();

			if (NewRecord.Restriction != ET66AccountRestrictionKind::None && !AccountRunSummaryId.IsEmpty())
			{
				NewRecord.RunSummarySlotName = RestrictionRunSummarySlotName;
				if (Backend->HasCachedRunSummary(AccountRunSummaryId))
				{
					if (UT66LeaderboardRunSummarySaveGame* Snapshot = Backend->GetCachedRunSummary(AccountRunSummaryId))
					{
						SaveRunSummarySnapshotToSlot(Snapshot, RestrictionRunSummarySlotName);
					}
				}
				else
				{
					PendingAccountRestrictionRunSummaryId = AccountRunSummaryId;
					Backend->FetchRunSummary(AccountRunSummaryId);
				}
			}
			else if (UGameplayStatics::DoesSaveGameExist(RestrictionRunSummarySlotName, 0))
			{
				UGameplayStatics::DeleteGameInSlot(RestrictionRunSummarySlotName, 0);
			}
		}
	}

	LocalSave->AccountRestriction = NewRecord;
	SaveLocalSave();
}

bool UT66LeaderboardSubsystem::RequestOpenAccountRestrictionRunSummary()
{
	const FT66AccountRestrictionRecord Rec = GetAccountRestrictionRecord();
	if (RequestOpenRunSummarySlot(Rec.RunSummarySlotName, ET66ScreenType::AccountStatus))
	{
		return true;
	}

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		return false;
	}

	const FString EntryId = Backend->GetLastAccountRunSummaryId();
	if (EntryId.IsEmpty())
	{
		return false;
	}

	if (Backend->HasCachedRunSummary(EntryId))
	{
		if (UT66LeaderboardRunSummarySaveGame* Snapshot = Backend->GetCachedRunSummary(EntryId))
		{
			SetPendingFakeRunSummarySnapshot(Snapshot);
			SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::AccountStatus);
			return true;
		}
	}

	PendingAccountRestrictionRunSummaryId = EntryId;
	Backend->FetchRunSummary(EntryId);
	return false;
}

void UT66LeaderboardSubsystem::SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType ModalType)
{
	PendingReturnModalAfterViewerRunSummary = ModalType;
}

ET66ScreenType UT66LeaderboardSubsystem::ConsumePendingReturnModalAfterViewerRunSummary()
{
	const ET66ScreenType Out = PendingReturnModalAfterViewerRunSummary;
	PendingReturnModalAfterViewerRunSummary = ET66ScreenType::None;
	return Out;
}

int32 UT66LeaderboardSubsystem::GetLocalScoreRank(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	return GetLocalScoreRankWeekly(Difficulty, PartySize);
}

int32 UT66LeaderboardSubsystem::GetLocalScoreRankWeekly(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	if (LastSubmittedScoreRankWeekly > 0)
	{
		return LastSubmittedScoreRankWeekly;
	}

	FT66LocalScoreRecord Record;
	if (GetLocalBestScoreRecord(Difficulty, PartySize, Record))
	{
		return FMath::Max(0, Record.BestScoreRankWeekly);
	}
	return 0;
}

int32 UT66LeaderboardSubsystem::GetLocalScoreRankAllTime(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	if (LastSubmittedScoreRankAllTime > 0)
	{
		return LastSubmittedScoreRankAllTime;
	}

	FT66LocalScoreRecord Record;
	if (GetLocalBestScoreRecord(Difficulty, PartySize, Record))
	{
		return FMath::Max(0, Record.BestScoreRankAllTime);
	}
	return 0;
}

int32 UT66LeaderboardSubsystem::GetLocalSpeedRunRank(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const
{
	static_cast<void>(Stage);
	return GetLocalSpeedRunRankWeekly(Difficulty, PartySize);
}

int32 UT66LeaderboardSubsystem::GetLocalSpeedRunRankWeekly(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	if (LastSubmittedCompletedRunRankWeekly > 0)
	{
		return LastSubmittedCompletedRunRankWeekly;
	}

	FT66LocalCompletedRunTimeRecord Record;
	if (GetLocalBestCompletedRunTimeRecord(Difficulty, PartySize, Record))
	{
		return FMath::Max(0, Record.BestCompletedRankWeekly);
	}

	return 0;
}

int32 UT66LeaderboardSubsystem::GetLocalSpeedRunRankAllTime(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	if (LastSubmittedCompletedRunRankAllTime > 0)
	{
		return LastSubmittedCompletedRunRankAllTime;
	}

	FT66LocalCompletedRunTimeRecord Record;
	if (GetLocalBestCompletedRunTimeRecord(Difficulty, PartySize, Record))
	{
		return FMath::Max(0, Record.BestCompletedRankAllTime);
	}

	return 0;
}

