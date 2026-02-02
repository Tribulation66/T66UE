// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66LocalLeaderboardSaveGame.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"

#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/IConsoleManager.h"

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
	if (S.Equals(TEXT("Perdition"), ESearchCase::IgnoreCase)) { Out = ET66Difficulty::Perdition; return true; }
	if (S.Equals(TEXT("Final"), ESearchCase::IgnoreCase)) { Out = ET66Difficulty::Final; return true; }
	return false;
}

static bool ParsePartySize(const FString& In, ET66PartySize& Out)
{
	const FString S = In.TrimStartAndEnd();
	if (S.Equals(TEXT("Solo"), ESearchCase::IgnoreCase)) { Out = ET66PartySize::Solo; return true; }
	if (S.Equals(TEXT("Duo"), ESearchCase::IgnoreCase)) { Out = ET66PartySize::Duo; return true; }
	if (S.Equals(TEXT("Trio"), ESearchCase::IgnoreCase)) { Out = ET66PartySize::Trio; return true; }
	return false;
}

void UT66LeaderboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreateLocalSave();
	// Prefer DataTables (UE-friendly, packages cleanly). Fall back to CSV if missing.
	if (!LoadTargetsFromDataTablesIfPresent())
	{
		LoadTargetsFromCsv();
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
	}
}

void UT66LeaderboardSubsystem::SaveLocalSave() const
{
	if (!LocalSave) return;
	UGameplayStatics::SaveGameToSlot(LocalSave, LocalSaveSlotName, 0);
}

void UT66LeaderboardSubsystem::DebugClearLocalLeaderboard()
{
	// Delete the core local leaderboard save.
	(void)UGameplayStatics::DeleteGameInSlot(LocalSaveSlotName, 0);

	// Delete all local best bounty run summary snapshots (all supported difficulty/party combinations).
	static const ET66Difficulty Diffs[] =
	{
		ET66Difficulty::Easy,
		ET66Difficulty::Medium,
		ET66Difficulty::Hard,
		ET66Difficulty::VeryHard,
		ET66Difficulty::Impossible,
		ET66Difficulty::Perdition,
		ET66Difficulty::Final,
	};

	static const ET66PartySize Parties[] =
	{
		ET66PartySize::Solo,
		ET66PartySize::Duo,
		ET66PartySize::Trio,
	};

	for (const ET66Difficulty Diff : Diffs)
	{
		for (const ET66PartySize Party : Parties)
		{
			const FString Slot = MakeLocalBestBountyRunSummarySlotName(Diff, Party);
			(void)UGameplayStatics::DeleteGameInSlot(Slot, 0);
		}
	}

	// Reset transient state and recreate the save so UI can immediately read a valid object.
	LocalSave = nullptr;
	PendingRunSummarySlotName.Reset();
	PendingReturnModalAfterViewerRunSummary = ET66ScreenType::None;
	bLastHighScoreWasNewBest = false;
	bLastSpeedRunWasNewBest = false;
	LastSpeedRunSubmittedStage = 0;

	LoadOrCreateLocalSave();

	UE_LOG(LogTemp, Log, TEXT("Leaderboard: cleared local saves (LocalLeaderboard + LocalBestBountyRunSummary_*_*)"));
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
	case ET66Difficulty::Perdition: return TEXT("Perdition");
	case ET66Difficulty::Final: return TEXT("Final");
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
	default: return TEXT("Unknown");
	}
}

FString UT66LeaderboardSubsystem::MakeLocalBestBountyRunSummarySlotName(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
	return FString::Printf(TEXT("T66_LocalBestBountyRunSummary_%s_%s"), *DifficultyKey(Difficulty), *PartySizeKey(PartySize));
}

bool UT66LeaderboardSubsystem::HasLocalBestBountyRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	const FString SlotName = MakeLocalBestBountyRunSummarySlotName(Difficulty, PartySize);
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

bool UT66LeaderboardSubsystem::SaveLocalBestBountyRunSummarySnapshot(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Bounty) const
{
	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66SkillRatingSubsystem* Skill = GI ? GI->GetSubsystem<UT66SkillRatingSubsystem>() : nullptr;
	if (!T66GI || !RunState)
	{
		return false;
	}

	UT66LeaderboardRunSummarySaveGame* Snapshot =
		Cast<UT66LeaderboardRunSummarySaveGame>(UGameplayStatics::CreateSaveGameObject(UT66LeaderboardRunSummarySaveGame::StaticClass()));
	if (!Snapshot)
	{
		return false;
	}

	Snapshot->SchemaVersion = 4;
	Snapshot->LeaderboardType = ET66LeaderboardType::HighScore;
	Snapshot->Difficulty = Difficulty;
	Snapshot->PartySize = PartySize;
	Snapshot->SavedAtUtc = FDateTime::UtcNow();

	Snapshot->StageReached = FMath::Clamp(RunState->GetCurrentStage(), 1, 66);
	Snapshot->Bounty = FMath::Max(0, Bounty);

	Snapshot->HeroID = T66GI->SelectedHeroID;
	Snapshot->HeroBodyType = T66GI->SelectedHeroBodyType;
	Snapshot->CompanionID = T66GI->SelectedCompanionID;
	Snapshot->CompanionBodyType = T66GI->SelectedCompanionBodyType;

	Snapshot->HeroLevel = FMath::Max(1, RunState->GetHeroLevel());
	Snapshot->DamageStat = FMath::Max(1, RunState->GetDamageStat());
	Snapshot->AttackSpeedStat = FMath::Max(1, RunState->GetAttackSpeedStat());
	Snapshot->AttackSizeStat = FMath::Max(1, RunState->GetScaleStat());
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

	Snapshot->EquippedIdols = RunState->GetEquippedIdols();
	Snapshot->Inventory = RunState->GetInventory();
	Snapshot->EventLog = RunState->GetEventLog();

	const FString SlotName = MakeLocalBestBountyRunSummarySlotName(Difficulty, PartySize);
	return UGameplayStatics::SaveGameToSlot(Snapshot, SlotName, 0);
}

uint64 UT66LeaderboardSubsystem::MakeBountyKey(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
	return (static_cast<uint64>(Difficulty) << 8) | static_cast<uint64>(PartySize);
}

uint64 UT66LeaderboardSubsystem::MakeSpeedRunKey_DiffStage(ET66Difficulty Difficulty, int32 Stage)
{
	return (static_cast<uint64>(Difficulty) << 8) | static_cast<uint64>(FMath::Clamp(Stage, 0, 255));
}

void UT66LeaderboardSubsystem::LoadTargetsFromCsv()
{
	// CSVs live alongside other Data CSVs so they can be imported as DataTables later.
	const FString ContentDir = FPaths::ProjectContentDir();
	LoadBountyTargetsFromCsv(FPaths::Combine(ContentDir, TEXT("Data/Leaderboard_BountyTargets.csv")));
	LoadSpeedRunTargetsFromCsv(FPaths::Combine(ContentDir, TEXT("Data/Leaderboard_SpeedrunTargets.csv")));
}

bool UT66LeaderboardSubsystem::LoadTargetsFromDataTablesIfPresent()
{
	BountyTarget10ByKey.Reset();
	SpeedRunTarget10ByKey_DiffStage.Reset();

	TSoftObjectPtr<UDataTable> BountyDT = TSoftObjectPtr<UDataTable>(FSoftObjectPath(BountyTargetsDTPath));
	TSoftObjectPtr<UDataTable> SpeedDT = TSoftObjectPtr<UDataTable>(FSoftObjectPath(SpeedRunTargetsDTPath));

	UDataTable* BDT = BountyDT.LoadSynchronous();
	UDataTable* SDT = SpeedDT.LoadSynchronous();

	// If neither exists, caller should fall back to CSV.
	if (!BDT && !SDT)
	{
		return false;
	}

	if (BDT)
	{
		if (BDT->GetRowStruct() != FT66LeaderboardBountyTargetRow::StaticStruct())
		{
			UE_LOG(LogTemp, Warning, TEXT("DT_Leaderboard_BountyTargets exists but has wrong RowStruct. Expected FT66LeaderboardBountyTargetRow."));
		}
		else
		{
			static const FString Ctx(TEXT("BountyTargetsDT"));
			TArray<FT66LeaderboardBountyTargetRow*> Rows;
			BDT->GetAllRows<FT66LeaderboardBountyTargetRow>(Ctx, Rows);
			for (const FT66LeaderboardBountyTargetRow* R : Rows)
			{
				if (!R) continue;
				if (R->TargetBounty10 <= 0) continue;
				BountyTarget10ByKey.Add(MakeBountyKey(R->Difficulty, R->PartySize), R->TargetBounty10);
			}
		}
	}

	if (SDT)
	{
		if (SDT->GetRowStruct() != FT66LeaderboardSpeedRunTargetRow::StaticStruct())
		{
			UE_LOG(LogTemp, Warning, TEXT("DT_Leaderboard_SpeedrunTargets exists but has wrong RowStruct. Expected FT66LeaderboardSpeedRunTargetRow."));
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

	// Success if we loaded at least one target from DTs.
	return (BountyTarget10ByKey.Num() > 0) || (SpeedRunTarget10ByKey_DiffStage.Num() > 0);
}

void UT66LeaderboardSubsystem::LoadBountyTargetsFromCsv(const FString& AbsPath)
{
	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *AbsPath))
	{
		return;
	}

	// Header supported:
	// - Difficulty,PartySize,TargetBounty10 (legacy)
	// - Name,Difficulty,PartySize,TargetBounty10 (UE DataTable-friendly)
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

		BountyTarget10ByKey.Add(MakeBountyKey(Diff, Party), Target10);
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

		SpeedRunTarget10ByKey_DiffStage.Add(MakeSpeedRunKey_DiffStage(Diff, Stage), Target10);
	}
}

int64 UT66LeaderboardSubsystem::GetBountyTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	const uint64 Key = MakeBountyKey(Difficulty, PartySize);
	if (const int64* Found = BountyTarget10ByKey.Find(Key))
	{
		return *Found;
	}

	// Fallback tuning (lower numbers so the player can realistically break into Top 10).
	const int32 DiffIndex = FMath::Clamp(static_cast<int32>(Difficulty), 0, 999);
	const int32 PartyIndex = FMath::Clamp(static_cast<int32>(PartySize), 0, 2);
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
		const int32 S = FMath::Clamp(Stage, 1, 66);
		Base = 35.f + (static_cast<float>(S) * 12.f) + (static_cast<float>(DiffIndex) * 6.f);
	}

	const float PartyMult =
		(PartySize == ET66PartySize::Solo) ? 1.0f :
		(PartySize == ET66PartySize::Duo) ? 1.06f : 1.12f;

	return FMath::Max(1.f, Base * PartyMult);
}

bool UT66LeaderboardSubsystem::GetSpeedRunTarget10Seconds(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage, float& OutSeconds) const
{
	if (Stage <= 0) return false;
	OutSeconds = GetSpeedRunTarget10(Difficulty, PartySize, Stage);
	return OutSeconds > 0.f;
}

bool UT66LeaderboardSubsystem::SubmitRunBounty(int32 Bounty)
{
	LoadOrCreateLocalSave();
	if (!LocalSave) return false;

	Bounty = FMath::Max(0, Bounty);
	bLastHighScoreWasNewBest = false;

	bool bPractice = false;
	bool bAnon = false;
	(void)GetSettingsPracticeAndAnon(bPractice, bAnon);
	if (bPractice)
	{
		UE_LOG(LogTemp, Log, TEXT("Leaderboard: blocked (Practice Mode). Bounty=%d"), Bounty);
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	const ET66Difficulty Diff = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66PartySize Party = T66GI ? T66GI->SelectedPartySize : ET66PartySize::Solo;

	FT66LocalBountyRecord* Record = nullptr;
	for (FT66LocalBountyRecord& R : LocalSave->BountyRecords)
	{
		if (R.Difficulty == Diff && R.PartySize == Party)
		{
			Record = &R;
			break;
		}
	}
	if (!Record)
	{
		LocalSave->BountyRecords.AddDefaulted();
		Record = &LocalSave->BountyRecords.Last();
		Record->Difficulty = Diff;
		Record->PartySize = Party;
		Record->BestBounty = 0;
		Record->bSubmittedAnonymous = false;
	}

	const bool bIsNewBest = (static_cast<int64>(Bounty) > Record->BestBounty);
	if (bIsNewBest)
	{
		bLastHighScoreWasNewBest = true;
		Record->BestBounty = static_cast<int64>(Bounty);
		Record->bSubmittedAnonymous = bAnon;
		SaveLocalSave();

		// Persist a snapshot of this best run so the leaderboard row can open the Run Summary later.
		(void)SaveLocalBestBountyRunSummarySnapshot(Diff, Party, Bounty);
	}
	else
	{
		// If the player already has a best score (from an older build), we might not have a snapshot yet.
		// If the current run matches the stored best and the snapshot is missing, capture it now so clicking works.
		const bool bHasBest = (Record->BestBounty > 0);
		const bool bSnapshotMissing = !UGameplayStatics::DoesSaveGameExist(MakeLocalBestBountyRunSummarySlotName(Diff, Party), 0);
		if (bHasBest && bSnapshotMissing && static_cast<int64>(Bounty) == Record->BestBounty)
		{
			(void)SaveLocalBestBountyRunSummarySnapshot(Diff, Party, Bounty);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Leaderboard: submit local bounty. Bounty=%d NewBest=%s Anonymous=%s"), Bounty, bIsNewBest ? TEXT("true") : TEXT("false"), bAnon ? TEXT("true") : TEXT("false"));
	return true;
}

bool UT66LeaderboardSubsystem::SubmitStageSpeedRunTime(int32 Stage, float Seconds)
{
	LoadOrCreateLocalSave();
	if (!LocalSave) return false;

	Stage = FMath::Clamp(Stage, 1, 66);
	Seconds = FMath::Max(0.f, Seconds);
	if (Seconds <= 0.01f) return false;
	bLastSpeedRunWasNewBest = false;
	LastSpeedRunSubmittedStage = Stage;

	bool bPractice = false;
	bool bAnon = false;
	(void)GetSettingsPracticeAndAnon(bPractice, bAnon);
	if (bPractice)
	{
		UE_LOG(LogTemp, Log, TEXT("Leaderboard: blocked (Practice Mode). Stage=%d Seconds=%.3f"), Stage, Seconds);
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

	UE_LOG(LogTemp, Log, TEXT("Leaderboard: submit local stage time. Stage=%d Seconds=%.3f NewBest=%s Anonymous=%s"), Stage, Seconds, bIsNewBest ? TEXT("true") : TEXT("false"), bAnon ? TEXT("true") : TEXT("false"));
	return true;
}

void UT66LeaderboardSubsystem::RequestOpenLocalBestBountyRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize)
{
	PendingRunSummarySlotName = MakeLocalBestBountyRunSummarySlotName(Difficulty, PartySize);
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

bool UT66LeaderboardSubsystem::ShouldShowAccountStatusButton() const
{
	if (CVarT66AccountStatusForce.GetValueOnGameThread() > 0)
	{
		return true;
	}
	// Ensure local save is loaded (frontend may query early).
	if (!LocalSave)
	{
		const_cast<UT66LeaderboardSubsystem*>(this)->LoadOrCreateLocalSave();
	}
	return LocalSave && (LocalSave->AccountRestriction.Restriction != ET66AccountRestrictionKind::None);
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
	return LocalSave ? LocalSave->AccountRestriction : FT66AccountRestrictionRecord();
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
		UE_LOG(LogTemp, Warning, TEXT("AccountStatus: appeal not submitted (empty message)."));
		return;
	}

	LocalSave->AccountRestriction.LastAppealMessage = Msg;
	LocalSave->AccountRestriction.LastEvidenceUrl = Url;
	LocalSave->AccountRestriction.AppealStatus = ET66AppealReviewStatus::UnderReview;
	SaveLocalSave();

	UE_LOG(LogTemp, Log, TEXT("AccountStatus: appeal submitted (local placeholder). EvidenceUrl=%s"), *Url);
}

bool UT66LeaderboardSubsystem::RequestOpenAccountRestrictionRunSummary()
{
	const FT66AccountRestrictionRecord Rec = GetAccountRestrictionRecord();
	if (Rec.RunSummarySlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(Rec.RunSummarySlotName, 0))
	{
		return false;
	}

	PendingRunSummarySlotName = Rec.RunSummarySlotName;
	PendingReturnModalAfterViewerRunSummary = ET66ScreenType::AccountStatus;
	return true;
}

ET66ScreenType UT66LeaderboardSubsystem::ConsumePendingReturnModalAfterViewerRunSummary()
{
	const ET66ScreenType Out = PendingReturnModalAfterViewerRunSummary;
	PendingReturnModalAfterViewerRunSummary = ET66ScreenType::None;
	return Out;
}

TArray<FLeaderboardEntry> UT66LeaderboardSubsystem::BuildBountyEntries(ET66Difficulty Difficulty, ET66PartySize PartySize) const
{
	// Ensure local save is loaded so "YOU" row reflects persisted best score.
	if (!LocalSave)
	{
		const_cast<UT66LeaderboardSubsystem*>(this)->LoadOrCreateLocalSave();
	}

	TArray<FLeaderboardEntry> Out;
	Out.Reserve(11);

	// Placeholder "global" Top 10 (generated from the 10th-place target).
	const int64 Target10 = GetBountyTarget10(Difficulty, PartySize);
	static const TArray<FString> FakeNames = {
		TEXT("xX_DarkSlayer_Xx"),
		TEXT("ProGamer2024"),
		TEXT("TribulationMaster"),
		TEXT("CasualHero"),
		TEXT("NightmareKing"),
		TEXT("SilentStorm"),
		TEXT("BlazeFury"),
		TEXT("IronWill"),
		TEXT("ShadowDancer"),
		TEXT("SpeedRunner_99"),
	};

	for (int32 Rank = 1; Rank <= 10; ++Rank)
	{
		const float Mult = 1.0f + (static_cast<float>(10 - Rank) * 0.08f); // rank1 higher, rank10 baseline
		FLeaderboardEntry E;
		E.Rank = Rank;
		E.PlayerName = FakeNames[Rank - 1];
		E.Score = static_cast<int64>(FMath::RoundToInt64(static_cast<double>(Target10) * static_cast<double>(Mult)));
		E.TimeSeconds = 0.f;
		E.PartySize = PartySize;
		E.Difficulty = Difficulty;
		E.bIsLocalPlayer = false;
		Out.Add(E);
	}

	// Local entry.
	int64 LocalBest = 0;
	bool bLocalAnonStored = false;
	if (LocalSave)
	{
		for (const FT66LocalBountyRecord& R : LocalSave->BountyRecords)
		{
			if (R.Difficulty == Difficulty && R.PartySize == PartySize)
			{
				LocalBest = R.BestBounty;
				bLocalAnonStored = R.bSubmittedAnonymous;
				break;
			}
		}
	}

	bool bPractice = false;
	bool bAnonNow = false;
	(void)GetSettingsPracticeAndAnon(bPractice, bAnonNow);
	const bool bUseAnonName = (LocalBest > 0) ? bLocalAnonStored : bAnonNow;
	const FString LocalName = bUseAnonName ? TEXT("ANONYMOUS") : TEXT("YOU");

	// If local best beats into Top 10, insert; otherwise append rank 11.
	const int64 Tenth = Out.Last().Score;
	bool bLocalInTop10 = false;
	if (LocalBest >= Tenth && LocalBest > 0)
	{
		const int32 InsertIndex = [&]()
		{
			for (int32 i = 0; i < Out.Num(); ++i)
			{
				if (LocalBest > Out[i].Score)
				{
					return i;
				}
			}
			return Out.Num(); // best is not better than any; would be after rank 10
		}();

		if (InsertIndex < 10)
		{
			FLeaderboardEntry You;
			You.PlayerName = LocalName;
			You.Score = LocalBest;
			You.TimeSeconds = 0.f;
			You.PartySize = PartySize;
			You.Difficulty = Difficulty;
			You.bIsLocalPlayer = true;

			Out.Insert(You, InsertIndex);
			Out.SetNum(10);
			bLocalInTop10 = true;
		}
	}

	// Re-rank Top 10.
	for (int32 i = 0; i < Out.Num(); ++i)
	{
		Out[i].Rank = i + 1;
	}

	if (!bLocalInTop10)
	{
		FLeaderboardEntry You;
		You.Rank = 11;
		You.PlayerName = LocalName;
		You.Score = LocalBest;
		You.TimeSeconds = 0.f;
		You.PartySize = PartySize;
		You.Difficulty = Difficulty;
		You.bIsLocalPlayer = true;
		Out.Add(You);
	}

	return Out;
}

TArray<FLeaderboardEntry> UT66LeaderboardSubsystem::BuildSpeedRunEntries(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const
{
	// Ensure local save is loaded so "YOU" row reflects persisted best time.
	if (!LocalSave)
	{
		const_cast<UT66LeaderboardSubsystem*>(this)->LoadOrCreateLocalSave();
	}

	TArray<FLeaderboardEntry> Out;
	Out.Reserve(11);

	Stage = FMath::Clamp(Stage, 1, 66);

	const float Target10 = GetSpeedRunTarget10(Difficulty, PartySize, Stage);
	static const TArray<FString> FakeNames = {
		TEXT("xX_DarkSlayer_Xx"),
		TEXT("ProGamer2024"),
		TEXT("TribulationMaster"),
		TEXT("CasualHero"),
		TEXT("NightmareKing"),
		TEXT("SilentStorm"),
		TEXT("BlazeFury"),
		TEXT("IronWill"),
		TEXT("ShadowDancer"),
		TEXT("SpeedRunner_99"),
	};

	for (int32 Rank = 1; Rank <= 10; ++Rank)
	{
		const float Mult = 0.55f + (static_cast<float>(Rank - 1) * (0.45f / 9.f)); // rank1 fastest, rank10 baseline
		FLeaderboardEntry E;
		E.Rank = Rank;
		E.PlayerName = FakeNames[Rank - 1];
		E.Score = 0;
		E.TimeSeconds = FMath::Max(1.f, Target10 * Mult);
		E.StageReached = Stage;
		E.PartySize = PartySize;
		E.Difficulty = Difficulty;
		E.bIsLocalPlayer = false;
		Out.Add(E);
	}

	float LocalBest = 0.f;
	bool bLocalAnonStored = false;
	if (LocalSave)
	{
		for (const FT66LocalSpeedRunStageRecord& R : LocalSave->SpeedRunStageRecords)
		{
			if (R.Difficulty == Difficulty && R.PartySize == PartySize && R.Stage == Stage)
			{
				LocalBest = R.BestSeconds;
				bLocalAnonStored = R.bSubmittedAnonymous;
				break;
			}
		}
	}

	bool bPractice = false;
	bool bAnonNow = false;
	(void)GetSettingsPracticeAndAnon(bPractice, bAnonNow);
	const bool bUseAnonName = (LocalBest > 0.01f) ? bLocalAnonStored : bAnonNow;
	const FString LocalName = bUseAnonName ? TEXT("ANONYMOUS") : TEXT("YOU");

	const float Tenth = Out.Last().TimeSeconds;
	bool bLocalInTop10 = false;
	if (LocalBest > 0.01f && LocalBest <= Tenth)
	{
		const int32 InsertIndex = [&]()
		{
			for (int32 i = 0; i < Out.Num(); ++i)
			{
				if (LocalBest < Out[i].TimeSeconds)
				{
					return i;
				}
			}
			return Out.Num();
		}();

		if (InsertIndex < 10)
		{
			FLeaderboardEntry You;
			You.PlayerName = LocalName;
			You.Score = 0;
			You.TimeSeconds = LocalBest;
			You.StageReached = Stage;
			You.PartySize = PartySize;
			You.Difficulty = Difficulty;
			You.bIsLocalPlayer = true;

			Out.Insert(You, InsertIndex);
			Out.SetNum(10);
			bLocalInTop10 = true;
		}
	}

	for (int32 i = 0; i < Out.Num(); ++i)
	{
		Out[i].Rank = i + 1;
	}

	if (!bLocalInTop10)
	{
		FLeaderboardEntry You;
		You.Rank = 11;
		You.PlayerName = LocalName;
		You.Score = 0;
		You.TimeSeconds = (LocalBest > 0.01f) ? LocalBest : 0.f;
		You.StageReached = Stage;
		You.PartySize = PartySize;
		You.Difficulty = Difficulty;
		You.bIsLocalPlayer = true;
		Out.Add(You);
	}

	return Out;
}

