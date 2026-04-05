// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LocalLeaderboardSaveGame.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"

#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
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

static bool ParseDifficulty(const FString& In, ET66Difficulty& Out)
{
	const FString S = In.TrimStartAndEnd();
	if (S.Equals(TEXT("Easy"), ESearchCase::IgnoreCase)) { Out = ET66Difficulty::Easy; return true; }
	if (S.Equals(TEXT("Medium"), ESearchCase::IgnoreCase)) { Out = ET66Difficulty::Medium; return true; }
	if (S.Equals(TEXT("Hard"), ESearchCase::IgnoreCase)) { Out = ET66Difficulty::Hard; return true; }
	if (S.Equals(TEXT("VeryHard"), ESearchCase::IgnoreCase) || S.Equals(TEXT("Very Hard"), ESearchCase::IgnoreCase)) { Out = ET66Difficulty::VeryHard; return true; }
	if (S.Equals(TEXT("Impossible"), ESearchCase::IgnoreCase)) { Out = ET66Difficulty::Impossible; return true; }
	return false;
}

static bool ParsePartySize(const FString& In, ET66PartySize& Out)
{
	const FString S = In.TrimStartAndEnd();
	if (S.Equals(TEXT("Solo"), ESearchCase::IgnoreCase)) { Out = ET66PartySize::Solo; return true; }
	if (S.Equals(TEXT("Duo"), ESearchCase::IgnoreCase)) { Out = ET66PartySize::Duo; return true; }
	if (S.Equals(TEXT("Trio"), ESearchCase::IgnoreCase)) { Out = ET66PartySize::Trio; return true; }
	if (S.Equals(TEXT("Quad"), ESearchCase::IgnoreCase)) { Out = ET66PartySize::Quad; return true; }
	return false;
}

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
	LoadTargetsFromCsv();
	// CSVs are the live source of truth for targets after the difficulty rewrite.
	// Only fall back to imported DT assets if the CSVs failed to populate anything.
	if (ScoreTarget10ByKey.Num() <= 0 || SpeedRunTarget10ByKey_DiffStage.Num() <= 0)
	{
		LoadTargetsFromDataTablesIfPresent();
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnAccountStatusComplete.AddDynamic(this, &UT66LeaderboardSubsystem::HandleBackendAccountStatusComplete);
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

void UT66LeaderboardSubsystem::LoadOrCreateLocalSave()
{
	if (LocalSave) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		(void)GI;
	}

	if (UGameplayStatics::DoesSaveGameExist(LocalSaveSlotName, 0))
	{
		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(LocalSaveSlotName, 0);
		LocalSave = Cast<UT66LocalLeaderboardSaveGame>(Loaded);
	}

	if (!LocalSave)
	{
		LocalSave = Cast<UT66LocalLeaderboardSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66LocalLeaderboardSaveGame::StaticClass()));
		SaveLocalSave();
		return;
	}

	LocalSave->SchemaVersion = FMath::Max(LocalSave->SchemaVersion, 2);
}

void UT66LeaderboardSubsystem::SaveLocalSave() const
{
	if (!LocalSave) return;
	UGameplayStatics::SaveGameToSlot(LocalSave, LocalSaveSlotName, 0);
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
		}

		for (const FT66LocalCompletedRunTimeRecord& Record : LocalSave->CompletedRunTimeRecords)
		{
			if (!Record.RunSummarySlotName.IsEmpty())
			{
				SlotsToDelete.Add(Record.RunSummarySlotName);
			}
		}

		if (!LocalSave->AccountRestriction.RunSummarySlotName.IsEmpty())
		{
			SlotsToDelete.Add(LocalSave->AccountRestriction.RunSummarySlotName);
		}
	}

	// Delete the core local leaderboard save.
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

FString UT66LeaderboardSubsystem::MakeLocalBestScoreRunSummarySlotName(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
	return FString::Printf(TEXT("T66_LocalBestScoreRunSummary_%s_%s"), *DifficultyKey(Difficulty), *PartySizeKey(PartySize));
}

FString UT66LeaderboardSubsystem::MakeRecentRunSummarySlotName(const FGuid& RunId)
{
	return FString::Printf(TEXT("T66_RunSummary_%s"), *RunId.ToString(EGuidFormats::DigitsWithHyphens));
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
	UT66SkillRatingSubsystem* Skill = GI ? GI->GetSubsystem<UT66SkillRatingSubsystem>() : nullptr;
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

	Snapshot->SchemaVersion = 7;
	Snapshot->LeaderboardType = LeaderboardType;
	Snapshot->Difficulty = Difficulty;
	Snapshot->PartySize = PartySize;
	Snapshot->SavedAtUtc = FDateTime::UtcNow();
	Snapshot->RunEndedAtUtc = FDateTime::UtcNow();
	Snapshot->RunDurationSeconds = RunState->GetFinalRunElapsedSeconds();
	Snapshot->bWasFullClear = RunState->DidRunEndInVictory();
	Snapshot->bWasSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;

	Snapshot->StageReached = FMath::Clamp(RunState->GetCurrentStage(), 1, 23);
	Snapshot->Score = FMath::Max(0, Score);

	Snapshot->SecondaryStatValues.Reset();
	for (int32 i = 1; i <= static_cast<int32>(ET66SecondaryStatType::EvasionChance); ++i)
	{
		const ET66SecondaryStatType SecType = static_cast<ET66SecondaryStatType>(i);
		if (!T66IsLiveSecondaryStatType(SecType) || SecType == ET66SecondaryStatType::GamblerToken)
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
	Snapshot->ArmorStat = FMath::Max(1, RunState->GetArmorStat());
	Snapshot->EvasionStat = FMath::Max(1, RunState->GetEvasionStat());
	Snapshot->LuckStat = FMath::Max(1, RunState->GetLuckStat());
	Snapshot->SpeedStat = FMath::Max(1, RunState->GetSpeedStat());

	Snapshot->LuckRating0To100 = RunState->GetLuckRating0To100();
	Snapshot->LuckRatingQuantity0To100 = RunState->GetLuckRatingQuantity0To100();
	Snapshot->LuckRatingQuality0To100 = RunState->GetLuckRatingQuality0To100();
	Snapshot->SkillRating0To100 = Skill ? Skill->GetSkillRating0To100() : -1;

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
	Snapshot->EventLog = RunState->GetEventLog();

	if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
	{
		const TArray<FDamageLogEntry> Sorted = DamageLog->GetDamageBySourceSorted();
		for (const FDamageLogEntry& Entry : Sorted)
		{
			Snapshot->DamageBySource.Add(Entry.SourceID, Entry.TotalDamage);
		}
	}

	return Snapshot;
}

bool UT66LeaderboardSubsystem::SaveRunSummarySnapshotToSlot(UT66LeaderboardRunSummarySaveGame* Snapshot, const FString& SlotName) const
{
	return Snapshot && !SlotName.IsEmpty() && UGameplayStatics::SaveGameToSlot(Snapshot, SlotName, 0);
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

void UT66LeaderboardSubsystem::LoadTargetsFromCsv()
{
	// CSVs live alongside other Data CSVs so they can be imported as DataTables later.
	const FString ContentDir = FPaths::ProjectContentDir();
	LoadScoreTargetsFromCsv(FPaths::Combine(ContentDir, TEXT("Data/Leaderboard_ScoreTargets.csv")));
	LoadSpeedRunTargetsFromCsv(FPaths::Combine(ContentDir, TEXT("Data/Leaderboard_SpeedrunTargets.csv")));
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
			TArray<FT66LeaderboardScoreTargetRow*> Rows;
			SrcDT->GetAllRows<FT66LeaderboardScoreTargetRow>(Ctx, Rows);
			for (const FT66LeaderboardScoreTargetRow* R : Rows)
			{
				if (!R) continue;
				if (R->TargetScore10 <= 0) continue;
				ScoreTarget10ByKey.Add(MakeScoreKey(R->Difficulty, R->PartySize), R->TargetScore10);
			}
		}
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
			TArray<FT66LeaderboardSpeedRunTargetRow*> Rows;
			SDT->GetAllRows<FT66LeaderboardSpeedRunTargetRow>(Ctx, Rows);
			for (const FT66LeaderboardSpeedRunTargetRow* R : Rows)
			{
				if (!R) continue;
				if (R->Stage <= 0 || R->TargetTime10Seconds <= 0.f) continue;
				SpeedRunTarget10ByKey_DiffStage.Add(MakeSpeedRunKey_DiffStage(R->Difficulty, R->Stage), R->TargetTime10Seconds);
			}
		}
	}

	// Success if we loaded the missing target set(s) from DTs.
	return (!bNeedScoreTargets || ScoreTarget10ByKey.Num() > 0)
		&& (!bNeedSpeedRunTargets || SpeedRunTarget10ByKey_DiffStage.Num() > 0);
}

void UT66LeaderboardSubsystem::LoadScoreTargetsFromCsv(const FString& AbsPath)
{
	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *AbsPath))
	{
		return;
	}

	// Header supported:
	// - Difficulty,PartySize,TargetScore10 (legacy)
	// - Name,Difficulty,PartySize,TargetScore10 (UE DataTable-friendly)
	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		const FString Line = Lines[i].TrimStartAndEnd();
		if (Line.IsEmpty()) continue;
		if (Line.StartsWith(TEXT("Difficulty"), ESearchCase::IgnoreCase)) continue;
		if (Line.StartsWith(TEXT("Name"), ESearchCase::IgnoreCase)) continue;
		if (Line.StartsWith(TEXT("#"))) continue;

		TArray<FString> Cells;
		Line.ParseIntoArray(Cells, TEXT(","), true);
		if (Cells.Num() < 3) continue;

		int32 Base = 0;
		ET66Difficulty ProbeDiff;
		if (!ParseDifficulty(Cells[0], ProbeDiff) && Cells.Num() >= 4 && ParseDifficulty(Cells[1], ProbeDiff))
		{
			// First column is row name.
			Base = 1;
		}
		if ((Cells.Num() - Base) < 3) continue;

		ET66Difficulty Diff;
		ET66PartySize Party;
		if (!ParseDifficulty(Cells[Base + 0], Diff)) continue;
		if (!ParsePartySize(Cells[Base + 1], Party)) continue;
		const int64 Target10 = FCString::Atoi64(*Cells[Base + 2].TrimStartAndEnd());
		if (Target10 <= 0) continue;

		const uint64 Key = MakeScoreKey(Diff, Party);
		if (!ScoreTarget10ByKey.Contains(Key))
		{
			ScoreTarget10ByKey.Add(Key, Target10);
		}
	}
}

void UT66LeaderboardSubsystem::LoadSpeedRunTargetsFromCsv(const FString& AbsPath)
{
	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *AbsPath))
	{
		return;
	}

	// Header supported:
	// - Difficulty,Stage,TargetTime10Seconds (legacy)
	// - Name,Difficulty,Stage,TargetTime10Seconds (UE DataTable-friendly)
	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		const FString Line = Lines[i].TrimStartAndEnd();
		if (Line.IsEmpty()) continue;
		if (Line.StartsWith(TEXT("Difficulty"), ESearchCase::IgnoreCase)) continue;
		if (Line.StartsWith(TEXT("Name"), ESearchCase::IgnoreCase)) continue;
		if (Line.StartsWith(TEXT("#"))) continue;

		TArray<FString> Cells;
		Line.ParseIntoArray(Cells, TEXT(","), true);
		if (Cells.Num() < 3) continue;

		int32 Base = 0;
		ET66Difficulty ProbeDiff;
		if (!ParseDifficulty(Cells[0], ProbeDiff) && Cells.Num() >= 4 && ParseDifficulty(Cells[1], ProbeDiff))
		{
			Base = 1;
		}
		if ((Cells.Num() - Base) < 3) continue;

		ET66Difficulty Diff;
		if (!ParseDifficulty(Cells[Base + 0], Diff)) continue;
		const int32 Stage = FCString::Atoi(*Cells[Base + 1].TrimStartAndEnd());
		const float Target10 = static_cast<float>(FCString::Atof(*Cells[Base + 2].TrimStartAndEnd()));
		if (Stage <= 0 || Target10 <= 0.f) continue;

		const uint64 Key = MakeSpeedRunKey_DiffStage(Diff, Stage);
		if (!SpeedRunTarget10ByKey_DiffStage.Contains(Key))
		{
			SpeedRunTarget10ByKey_DiffStage.Add(Key, Target10);
		}
	}
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
		return false;
	}

	LoadOrCreateLocalSave();
	if (!LocalSave) return false;

	Score = FMath::Max(0, Score);
	bLastScoreWasNewBest = false;

	if (!IsAccountEligibleForLeaderboard())
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

	FT66LocalScoreRecord* Record = FindLocalScoreRecordMutable(Diff, Party);
	if (!Record)
	{
		LocalSave->ScoreRecords.AddDefaulted();
		Record = &LocalSave->ScoreRecords.Last();
		Record->Difficulty = Diff;
		Record->PartySize = Party;
		Record->BestScore = 0;
		Record->bSubmittedAnonymous = false;
	}

	const bool bIsNewBest = (static_cast<int64>(Score) > Record->BestScore);
	if (bIsNewBest)
	{
		bLastScoreWasNewBest = true;
		Record->BestScore = static_cast<int64>(Score);
		Record->bSubmittedAnonymous = bAnon;
		Record->AchievedAtUtc = FDateTime::UtcNow();
		if (!ResolvedRunSummarySlotName.IsEmpty())
		{
			Record->RunSummarySlotName = ResolvedRunSummarySlotName;
		}
		else
		{
			(void)SaveLocalBestScoreRunSummarySnapshot(Diff, Party, Score, FString());
			Record->RunSummarySlotName = MakeLocalBestScoreRunSummarySlotName(Diff, Party);
		}
		SaveLocalSave();
	}
	else
	{
		const bool bHasBest = (Record->BestScore > 0);
		const bool bRecordSlotMissing = Record->RunSummarySlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(Record->RunSummarySlotName, 0);
		if (bHasBest && bRecordSlotMissing && static_cast<int64>(Score) == Record->BestScore)
		{
			if (!ResolvedRunSummarySlotName.IsEmpty())
			{
				Record->RunSummarySlotName = ResolvedRunSummarySlotName;
				if (Record->AchievedAtUtc == FDateTime::MinValue())
				{
					Record->AchievedAtUtc = FDateTime::UtcNow();
				}
				SaveLocalSave();
			}
			else
			{
				(void)SaveLocalBestScoreRunSummarySnapshot(Diff, Party, Score, FString());
				Record->RunSummarySlotName = MakeLocalBestScoreRunSummarySlotName(Diff, Party);
				if (Record->AchievedAtUtc == FDateTime::MinValue())
				{
					Record->AchievedAtUtc = FDateTime::UtcNow();
				}
				SaveLocalSave();
			}
		}
	}

	UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: submit local score. Score=%d NewBest=%s Anonymous=%s"), Score, bIsNewBest ? TEXT("true") : TEXT("false"), bAnon ? TEXT("true") : TEXT("false"));

	// Fire-and-forget backend submission (online complement to local save).
	if (UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		if (Party != ET66PartySize::Solo)
		{
			UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: skipping backend score submit for co-op party size until full co-op payload support is implemented. Party=%s"), *PartySizeKey(Party));
		}
		else if (Backend->IsBackendConfigured() && Backend->HasSteamTicket())
		{
			UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			const int32 StageReached = RunState ? RunState->GetCurrentStage() : 1;
			const int32 TimeMs = 0;
			const FString DisplayName = bAnon ? TEXT("ANONYMOUS") : TEXT("Player");
			const FString HeroId = T66GI ? T66GI->SelectedHeroID.ToString() : TEXT("unknown");
			const FString CompanionId = T66GI ? T66GI->SelectedCompanionID.ToString() : TEXT("unknown");

			UT66LeaderboardRunSummarySaveGame* BackendSnapshot = CreateCurrentRunSummarySnapshot(ET66LeaderboardType::Score, Diff, Party, Score);

			Backend->SubmitRunToBackend(DisplayName, Score, TimeMs, Diff, Party, StageReached, HeroId, CompanionId, BackendSnapshot);
		}
	}

	return true;
}

bool UT66LeaderboardSubsystem::SubmitCompletedRunTime(float Seconds, const FString& ExistingRunSummarySlotName)
{
	UGameInstance* GICheck = GetGameInstance();
	UT66GameInstance* T66GICheck = GICheck ? Cast<UT66GameInstance>(GICheck) : nullptr;
	if (T66GICheck && T66GICheck->bRunIneligibleForLeaderboard)
	{
		return false;
	}

	LoadOrCreateLocalSave();
	if (!LocalSave) return false;

	Seconds = FMath::Max(0.f, Seconds);
	bLastCompletedRunTimeWasNewBest = false;
	if (Seconds <= 0.01f)
	{
		return false;
	}

	if (!IsAccountEligibleForLeaderboard())
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

	FT66LocalCompletedRunTimeRecord* Record = FindLocalCompletedRunTimeRecordMutable(Diff, Party);
	if (!Record)
	{
		LocalSave->CompletedRunTimeRecords.AddDefaulted();
		Record = &LocalSave->CompletedRunTimeRecords.Last();
		Record->Difficulty = Diff;
		Record->PartySize = Party;
		Record->BestCompletedSeconds = 0.f;
		Record->bSubmittedAnonymous = false;
	}

	const bool bUnset = (Record->BestCompletedSeconds <= 0.01f);
	const bool bIsNewBest = bUnset || (Seconds < Record->BestCompletedSeconds);
	if (bIsNewBest)
	{
		bLastCompletedRunTimeWasNewBest = true;
		Record->BestCompletedSeconds = Seconds;
		Record->bSubmittedAnonymous = bAnon;
		Record->RunSummarySlotName = ResolvedRunSummarySlotName;
		Record->AchievedAtUtc = FDateTime::UtcNow();
		SaveLocalSave();
	}
	else if ((Record->RunSummarySlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(Record->RunSummarySlotName, 0))
		&& !ResolvedRunSummarySlotName.IsEmpty()
		&& FMath::IsNearlyEqual(Seconds, Record->BestCompletedSeconds))
	{
		Record->RunSummarySlotName = ResolvedRunSummarySlotName;
		if (Record->AchievedAtUtc == FDateTime::MinValue())
		{
			Record->AchievedAtUtc = FDateTime::UtcNow();
		}
		SaveLocalSave();
	}

	UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: submit local completed run time. Seconds=%.3f NewBest=%s Anonymous=%s"), Seconds, bIsNewBest ? TEXT("true") : TEXT("false"), bAnon ? TEXT("true") : TEXT("false"));
	if (Party != ET66PartySize::Solo)
	{
		UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: co-op completed run leaderboard submission remains disabled until full party payload support is implemented. Party=%s"), *PartySizeKey(Party));
	}
	else if (UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		if (Backend->IsBackendConfigured() && Backend->HasSteamTicket())
		{
			UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			const int32 Score = RunState ? RunState->GetCurrentScore() : 0;
			const int32 StageReached = RunState ? RunState->GetCurrentStage() : 1;
			const int32 TimeMs = FMath::RoundToInt(Seconds * 1000.0f);
			const FString DisplayName = bAnon ? TEXT("ANONYMOUS") : TEXT("Player");
			const FString HeroId = T66GI ? T66GI->SelectedHeroID.ToString() : TEXT("unknown");
			const FString CompanionId = T66GI ? T66GI->SelectedCompanionID.ToString() : TEXT("unknown");

			UT66LeaderboardRunSummarySaveGame* BackendSnapshot = CreateCurrentRunSummarySnapshot(ET66LeaderboardType::SpeedRun, Diff, Party, Score);
			Backend->SubmitRunToBackend(DisplayName, Score, TimeMs, Diff, Party, StageReached, HeroId, CompanionId, BackendSnapshot);
		}
	}
	return true;
}

bool UT66LeaderboardSubsystem::SubmitStageSpeedRunTime(int32 Stage, float Seconds)
{
	UGameInstance* GICheck = GetGameInstance();
	UT66GameInstance* T66GICheck = GICheck ? Cast<UT66GameInstance>(GICheck) : nullptr;
	if (T66GICheck && T66GICheck->bRunIneligibleForLeaderboard)
	{
		return false;
	}

	LoadOrCreateLocalSave();
	if (!LocalSave) return false;

	Stage = FMath::Clamp(Stage, 1, 23);
	Seconds = FMath::Max(0.f, Seconds);
	if (Seconds <= 0.01f) return false;
	bLastSpeedRunWasNewBest = false;
	LastSpeedRunSubmittedStage = Stage;

	if (!IsAccountEligibleForLeaderboard())
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

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	const ET66Difficulty Diff = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66PartySize Party = T66GI ? T66GI->SelectedPartySize : ET66PartySize::Solo;

	FT66LocalSpeedRunStageRecord* Record = nullptr;
	for (FT66LocalSpeedRunStageRecord& R : LocalSave->SpeedRunStageRecords)
	{
		if (R.Difficulty == Diff && R.PartySize == Party && R.Stage == Stage)
		{
			Record = &R;
			break;
		}
	}
	if (!Record)
	{
		LocalSave->SpeedRunStageRecords.AddDefaulted();
		Record = &LocalSave->SpeedRunStageRecords.Last();
		Record->Difficulty = Diff;
		Record->PartySize = Party;
		Record->Stage = Stage;
		Record->BestSeconds = 0.f;
		Record->bSubmittedAnonymous = false;
	}

	const bool bUnset = (Record->BestSeconds <= 0.01f);
	const bool bIsNewBest = bUnset || (Seconds < Record->BestSeconds);
	if (bIsNewBest)
	{
		bLastSpeedRunWasNewBest = true;
		Record->BestSeconds = Seconds;
		Record->bSubmittedAnonymous = bAnon;
		SaveLocalSave();
	}

	UE_LOG(LogT66Leaderboard, Log, TEXT("Leaderboard: submit local stage time. Stage=%d Seconds=%.3f NewBest=%s Anonymous=%s"), Stage, Seconds, bIsNewBest ? TEXT("true") : TEXT("false"), bAnon ? TEXT("true") : TEXT("false"));
	return true;
}

void UT66LeaderboardSubsystem::RequestOpenLocalBestScoreRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
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
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	if (!T66GI || !RunState)
	{
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
	Record.EndedAtUtc = FDateTime::UtcNow();
	Record.RunSummarySlotName = RunSummarySlotName;
	Record.Difficulty = T66GI->SelectedDifficulty;
	Record.PartySize = T66GI->SelectedPartySize;
	Record.HeroID = T66GI->SelectedHeroID;
	Record.CompanionID = T66GI->SelectedCompanionID;
	Record.Score = RunState->GetCurrentScore();
	Record.StageReached = RunState->GetCurrentStage();
	Record.DurationSeconds = RunState->GetFinalRunElapsedSeconds();
	Record.bWasFullClear = RunState->DidRunEndInVictory();
	Record.bWasSpeedRunMode = PS ? PS->GetSpeedRunMode() : false;

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
		if (Record.RunSummarySlotName == SlotName)
		{
			return true;
		}
	}

	for (const FT66LocalCompletedRunTimeRecord& Record : LocalSave->CompletedRunTimeRecords)
	{
		if (Record.RunSummarySlotName == SlotName)
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

bool UT66LeaderboardSubsystem::HasAccountRestrictionRunSummary() const
{
	const FT66AccountRestrictionRecord Rec = GetAccountRestrictionRecord();
	return !Rec.RunSummarySlotName.IsEmpty() && UGameplayStatics::DoesSaveGameExist(Rec.RunSummarySlotName, 0);
}

bool UT66LeaderboardSubsystem::CanSubmitAccountAppeal() const
{
	const FT66AccountRestrictionRecord Rec = GetAccountRestrictionRecord();
	if (Rec.Restriction != ET66AccountRestrictionKind::Suspicion)
	{
		return false;
	}
	// Only allow one submission in the local placeholder implementation.
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
				Backend->SubmitAppeal(Msg, Url);
			}
		}
	}

	LocalSave->AccountRestriction.LastAppealMessage = Msg;
	LocalSave->AccountRestriction.LastEvidenceUrl = Url;
	LocalSave->AccountRestriction.AppealStatus = ET66AppealReviewStatus::UnderReview;
	SaveLocalSave();

	UE_LOG(LogT66Leaderboard, Log, TEXT("AccountStatus: appeal submitted. EvidenceUrl=%s"), *Url);
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
		}
	}

	LocalSave->AccountRestriction = NewRecord;
	SaveLocalSave();
}

bool UT66LeaderboardSubsystem::RequestOpenAccountRestrictionRunSummary()
{
	const FT66AccountRestrictionRecord Rec = GetAccountRestrictionRecord();
	return RequestOpenRunSummarySlot(Rec.RunSummarySlotName, ET66ScreenType::AccountStatus);
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
	return 0;
}

int32 UT66LeaderboardSubsystem::GetLocalSpeedRunRank(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const
{
	return 0;
}

