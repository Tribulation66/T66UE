// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66BackendSubsystem.h"
#include "Core/Backend/T66BackendPrivate.h"
#include "Core/Shutdown/T66ShutdownSubsystem.h"
#include "Core/T66SaveMigration.h"

DEFINE_LOG_CATEGORY(LogT66Backend);

TAutoConsoleVariable<float> CVarT66PartyInvitePollMinIntervalSeconds(
	TEXT("T66.Backend.PartyInvitePollMinIntervalSeconds"),
	0.15f,
	TEXT("Minimum interval between non-forced party invite polls."),
	ECVF_Default);

TAutoConsoleVariable<float> CVarT66PartyInvitePollTickerIntervalSeconds(
	TEXT("T66.Backend.PartyInvitePollTickerIntervalSeconds"),
	0.15f,
	TEXT("Ticker cadence used to drive invite polling while the backend subsystem is alive."),
	ECVF_Default);

namespace
{
	struct FT66DummyLeaderboardIdentity
	{
		FString Type;
		FString Time;
		FString Party;
		FString Difficulty;
		FString Filter;
	};

	bool T66TryParseDummyLeaderboardKey(const FString& Key, FT66DummyLeaderboardIdentity& OutIdentity)
	{
		TArray<FString> Tokens;
		Key.ParseIntoArray(Tokens, TEXT("_"));
		if (Tokens.Num() != 5)
		{
			return false;
		}

		OutIdentity.Type = Tokens[0].ToLower();
		OutIdentity.Time = Tokens[1].ToLower();
		OutIdentity.Party = Tokens[2].ToLower();
		OutIdentity.Difficulty = Tokens[3].ToLower();
		OutIdentity.Filter = Tokens[4].ToLower();

		const bool bValidType = OutIdentity.Type == TEXT("score") || OutIdentity.Type == TEXT("speedrun");
		const bool bValidTime = OutIdentity.Time == TEXT("weekly") || OutIdentity.Time == TEXT("alltime");
		const bool bValidFilter = OutIdentity.Filter == TEXT("global") || OutIdentity.Filter == TEXT("friends") || OutIdentity.Filter == TEXT("streamers");

		return bValidType
			&& bValidTime
			&& bValidFilter
			&& OutIdentity.Party == TEXT("solo")
			&& OutIdentity.Difficulty == TEXT("easy");
	}

	bool T66ShouldUseDevelopmentDummyLeaderboards()
	{
#if UE_BUILD_SHIPPING
		return false;
#else
		static const bool bUseDummyLeaderboards = []()
		{
			bool bEnabledFromConfig = false;
			if (GConfig)
			{
				GConfig->GetBool(
					TEXT("T66.Online"),
					TEXT("bEnableDevelopmentDummyLeaderboards"),
					bEnabledFromConfig,
					GGameIni);
			}

			return GIsEditor
				|| bEnabledFromConfig
				|| FParse::Param(FCommandLine::Get(), TEXT("T66EnableDummyLeaderboards"));
		}();

		return bUseDummyLeaderboards;
#endif
	}

	FString T66GetDummyPlayerName(const FString& Filter, int32 Index)
	{
		static const TArray<FString> GlobalNames = {
			TEXT("Ash Vale"), TEXT("Rune Kestrel"), TEXT("Iris Flint"), TEXT("Morrow Pike"), TEXT("Sable Crest")
		};
		static const TArray<FString> FriendNames = {
			TEXT("Niko Vale"), TEXT("Mina Torch"), TEXT("Oren Bloom"), TEXT("Pax Hollow"), TEXT("Rhea Vale")
		};
		static const TArray<FString> StreamerNames = {
			TEXT("Clip Monarch"), TEXT("Split Saint"), TEXT("Nova Route"), TEXT("Tempo Hex"), TEXT("Frame Witch")
		};

		const TArray<FString>* Pool = &GlobalNames;
		if (Filter == TEXT("friends"))
		{
			Pool = &FriendNames;
		}
		else if (Filter == TEXT("streamers"))
		{
			Pool = &StreamerNames;
		}

		return (*Pool)[Index % Pool->Num()];
	}

	int32 T66GetDummyFilterIndex(const FString& Filter)
	{
		if (Filter == TEXT("friends"))
		{
			return 1;
		}
		if (Filter == TEXT("streamers"))
		{
			return 2;
		}
		return 0;
	}

	FName T66GetDummyHeroId(int32 Index)
	{
		static const FName HeroIds[] = {
			FName(TEXT("Hero_1")),
			FName(TEXT("Hero_2")),
			FName(TEXT("Hero_3")),
			FName(TEXT("Hero_4")),
			FName(TEXT("Hero_5"))
		};

		return HeroIds[Index % UE_ARRAY_COUNT(HeroIds)];
	}

	TObjectPtr<UT66LeaderboardRunSummarySaveGame> T66BuildDummyRunSummary(
		UObject* Outer,
		const FT66DummyLeaderboardIdentity& Identity,
		const FLeaderboardEntry& Entry,
		const FString& DisplayName)
	{
		UT66LeaderboardRunSummarySaveGame* Summary = NewObject<UT66LeaderboardRunSummarySaveGame>(Outer);
		Summary->SchemaVersion = T66CurrentRunSummarySchemaVersion;
		Summary->EntryId = Entry.EntryId;
		Summary->OwnerSteamId = FString::Printf(TEXT("dummy_%s_%02d"), *Identity.Filter, Entry.Rank);
		Summary->LeaderboardType = (Identity.Type == TEXT("speedrun")) ? ET66LeaderboardType::SpeedRun : ET66LeaderboardType::Score;
		Summary->Difficulty = ET66Difficulty::Easy;
		Summary->PartySize = ET66PartySize::Solo;
		Summary->SavedAtUtc = FDateTime::UtcNow();
		Summary->RunEndedAtUtc = Summary->SavedAtUtc;
		Summary->RunDurationSeconds = Entry.TimeSeconds;
		Summary->bWasFullClear = true;
		Summary->bWasSpeedRunMode = (Summary->LeaderboardType == ET66LeaderboardType::SpeedRun);
		Summary->StageReached = 4;
		Summary->Score = static_cast<int32>(Entry.Score);
		Summary->ScoreRankAllTime = (Identity.Type == TEXT("score") && Identity.Time == TEXT("alltime")) ? Entry.Rank : 0;
		Summary->ScoreRankWeekly = (Identity.Type == TEXT("score") && Identity.Time == TEXT("weekly")) ? Entry.Rank : 0;
		Summary->SpeedRunRankAllTime = (Identity.Type == TEXT("speedrun") && Identity.Time == TEXT("alltime")) ? Entry.Rank : 0;
		Summary->SpeedRunRankWeekly = (Identity.Type == TEXT("speedrun") && Identity.Time == TEXT("weekly")) ? Entry.Rank : 0;
		Summary->HeroID = Entry.HeroID;
		Summary->CompanionID = NAME_None;
		Summary->HeroLevel = 1;
		Summary->DamageStat = 6;
		Summary->AttackSpeedStat = 5;
		Summary->AttackScaleStat = 4;
		Summary->AccuracyStat = 4;
		Summary->ArmorStat = 3;
		Summary->EvasionStat = 4;
		Summary->LuckStat = 5;
		Summary->SpeedStat = 6;
		Summary->LuckRating0To100 = 72 - Entry.Rank;
		Summary->LuckRatingQuantity0To100 = 68 - Entry.Rank;
		Summary->LuckRatingQuality0To100 = 74 - Entry.Rank;
		Summary->SkillRating0To100 = 80 - (Entry.Rank * 2);
		Summary->EquippedIdols = {
			FName(TEXT("Idol_Fire_AOE")),
			FName(TEXT("Idol_Ice_Summon")),
			FName(TEXT("Idol_Wind_Bounce"))
		};
		Summary->EquippedIdolTiers = { 1, 2, 3 };
		T66NormalizeEquippedIdolSaveArrays(Summary->EquippedIdols, Summary->EquippedIdolTiers);
		Summary->EquippedIdolElements = {
			ET66IdolElement::Fire,
			ET66IdolElement::Ice,
			ET66IdolElement::Wind
		};
		Summary->EquippedIdolCategories = {
			ET66AttackCategory::AOE,
			ET66AttackCategory::AOE,
			ET66AttackCategory::Bounce
		};
		Summary->Inventory = { FName(TEXT("Item_GoldTooth")), FName(TEXT("Item_TravelerBoots")), FName(TEXT("Item_LuckyCoin")) };
		Summary->InventorySlots = {
			FT66InventorySlot(FName(TEXT("Item_GoldTooth")), ET66ItemRarity::Black, 1, 0.f, 0, 1101),
			FT66InventorySlot(FName(TEXT("Item_TravelerBoots")), ET66ItemRarity::Red, 3, 0.f, 0, 1102),
			FT66InventorySlot(FName(TEXT("Item_LuckyCoin")), ET66ItemRarity::Yellow, 9, 0.f, 0, 1103)
		};
		Summary->NoIdolSelectionStacks = 1;
		Summary->NoIdolBaseStatBonuses.DamageTenths = 10;
		Summary->NoIdolBaseStatBonuses.AttackSpeedTenths = 10;
		Summary->NoIdolBaseStatBonuses.AttackScaleTenths = 10;
		Summary->MobLootDropsCollectedThisRun = 24 + Entry.Rank;
		Summary->MobLootQuantityCollectedThisRun = 64 + Entry.Rank;
		Summary->MobLootGoldValueCollectedThisRun = Summary->MobLootQuantityCollectedThisRun;
		Summary->MobLootQuantityCollectedByPlayerThisRun = 40 + Entry.Rank;
		Summary->MobLootQuantityCollectedByPetThisRun = Summary->MobLootQuantityCollectedThisRun - Summary->MobLootQuantityCollectedByPlayerThisRun;
		Summary->MobLootDropsCollectedByPetThisRun = 8;
		Summary->MobLootQuantitySoldThisRun = 50 + Entry.Rank;
		Summary->MobLootSaleGoldThisRun = Summary->MobLootQuantitySoldThisRun;
		Summary->MobLootRemainingStack = Summary->MobLootQuantityCollectedThisRun - Summary->MobLootQuantitySoldThisRun;
		Summary->CurrentGold = 125 + Entry.Rank;
		Summary->CurrentDebt = 0;
		Summary->InventorySellValueTotal = 90;
		Summary->NetWorth = Summary->CurrentGold + Summary->InventorySellValueTotal;
		Summary->ActiveVendorTokenStacks = 1;
		Summary->CurrentSellFraction = 0.725f;
		Summary->ShopStockCount = 4;
		Summary->BuybackPoolSize = 2;
		Summary->EquippedWeaponID = FName(TEXT("Weapon_Hero_1_Black_AOE"));
		Summary->EquippedWeaponBranch = ET66AttackCategory::AOE;
		Summary->EquippedWeaponRarity = ET66WeaponRarity::Black;
		Summary->EquippedWeaponAttackPatternID = FName(TEXT("TwinAxe"));
		Summary->EquippedWeaponProjectileCount = 2;
		Summary->EquippedWeaponSpreadAngleDegrees = 18.f;
		Summary->ActivePetID = FName(TEXT("Pet_Dungeon_Slime"));
		Summary->ActivePetSkinID = FName(TEXT("Default"));
		Summary->ActivePetBondStagesCleared = 3;
		Summary->ActivePetBondMovementSpeedMultiplier = 1.09f;
		Summary->PetMobLootQuantityCollectedThisRun = Summary->MobLootQuantityCollectedByPetThisRun;
		Summary->PetMobLootDropsCollectedThisRun = Summary->MobLootDropsCollectedByPetThisRun;
		Summary->bBossActiveAtSummary = false;
		Summary->ActiveBossID = FName(TEXT("Dungeon_SlimeKing"));
		Summary->BossMaxHP = 1000;
		Summary->BossCurrentHP = 0;
		FT66BossPartSnapshot& DummyBossPart = Summary->BossParts.AddDefaulted_GetRef();
		DummyBossPart.PartID = FName(TEXT("Core"));
		DummyBossPart.HitZoneType = ET66HitZoneType::Core;
		DummyBossPart.MaxHP = 1000;
		DummyBossPart.CurrentHP = 0;
		Summary->DamageBySource.Add(FName(TEXT("PrimaryAttack")), FMath::Max(2500, Summary->Score / 2));
		Summary->DamageBySource.Add(FName(TEXT("Ultimate")), FMath::Max(600, Summary->Score / 6));
		Summary->DamageReceivedBySource.Add(FName(TEXT("Slime")), 120);
		Summary->DamageReceivedBySource.Add(FName(TEXT("DungeonWallArrow")), 80);
		Summary->DisplayName = DisplayName;

		const TArray<float> TimeFractions = { 0.22f, 0.48f, 0.74f, 1.0f };
		const TArray<float> ScoreFractions = { 0.18f, 0.42f, 0.69f, 1.0f };
		for (int32 StageIndex = 0; StageIndex < 4; ++StageIndex)
		{
			const int32 StageNumber = StageIndex + 1;
			const float ElapsedSeconds = FMath::RoundToFloat(Entry.TimeSeconds * TimeFractions[StageIndex] * 100.0f) / 100.0f;
			const int32 StageScore = FMath::RoundToInt(static_cast<float>(Entry.Score) * ScoreFractions[StageIndex]);

			FT66StagePacingPoint& Point = Summary->StagePacingPoints.AddDefaulted_GetRef();
			Point.Stage = StageNumber;
			Point.Score = StageScore;
			Point.ElapsedSeconds = ElapsedSeconds;

			Summary->EventLog.Add(FString::Printf(
				TEXT("Stage %d cleared with %d score at %.2fs"),
				StageNumber,
				StageScore,
				ElapsedSeconds));
			Summary->EventLog.Add(T66LeaderboardPacing::MakeStageMarker(StageNumber, StageScore, ElapsedSeconds));
		}

		Summary->EventLog.Add(FString::Printf(TEXT("%s finished the dummy Easy solo run."), *DisplayName));
		return Summary;
	}
}

void UT66BackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UT66ShutdownSubsystem::StaticClass());
	Super::Initialize(Collection);

	GConfig->GetString(TEXT("T66.Online"), TEXT("BackendBaseUrl"), BackendBaseUrl, GGameIni);

	if (BackendBaseUrl.IsEmpty())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: BackendBaseUrl not configured in [T66.Online]. Online features disabled."));
	}
	else
	{
		while (BackendBaseUrl.EndsWith(TEXT("/")))
		{
			BackendBaseUrl.LeftChopInline(1);
		}
		UE_LOG(LogT66Backend, Log, TEXT("Backend: configured URL = %s"), *BackendBaseUrl);
	}

	SeedDevelopmentDummyLeaderboardsIfNeeded();

	PartyInvitePollTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UT66BackendSubsystem::HandlePartyInvitePollTicker),
		FMath::Max(0.05f, CVarT66PartyInvitePollTickerIntervalSeconds.GetValueOnGameThread()));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			ShutdownParticipantHandle = Shutdown->RegisterParticipant(
				this,
				FName(TEXT("Backend.NetworkRuntime")),
				ET66ShutdownPhase::NetworkPlatform,
				10,
				1.0,
				false,
				FT66ShutdownParticipantDelegate::CreateUObject(this, &UT66BackendSubsystem::HandleShutdown));
		}
	}
}

void UT66BackendSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			Shutdown->UnregisterParticipant(ShutdownParticipantHandle);
		}
	}
	ShutdownParticipantHandle.Reset();
	ShutdownRuntimeResources(TEXT("Deinitialize"));
	Super::Deinitialize();
}

bool UT66BackendSubsystem::HandleShutdown(const FT66ShutdownContext& /*Context*/)
{
	ShutdownRuntimeResources(TEXT("ShutdownSystem"));
	return true;
}

void UT66BackendSubsystem::ShutdownRuntimeResources(const TCHAR* /*Reason*/)
{
	if (PartyInvitePollTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PartyInvitePollTickerHandle);
		PartyInvitePollTickerHandle.Reset();
	}

	if (PendingCoopSubmitTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PendingCoopSubmitTickerHandle);
		PendingCoopSubmitTickerHandle.Reset();
	}

	if (TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> PendingInviteRequest = PendingPartyInvitePollRequest.Pin())
	{
		PendingInviteRequest->OnProcessRequestComplete().Unbind();
		PendingInviteRequest->CancelRequest();
		PendingPartyInvitePollRequest.Reset();
	}

	bPartyInvitePollInFlight = false;
	bPartyInvitePollRequestedWhileInFlight = false;
	PendingPartyInvites.Reset();
	PendingCoopSubmitRequests.Reset();
	PendingPartyInvitesChanged.Clear();
	PartyInviteActionComplete.Clear();
}

void UT66BackendSubsystem::SetSteamTicketHex(const FString& TicketHex)
{
	CachedSteamTicketHex = TicketHex;
	UE_LOG(LogT66Backend, Log, TEXT("Backend: Steam ticket set (%d hex chars)"), TicketHex.Len());
}

void UT66BackendSubsystem::SeedDevelopmentDummyLeaderboardsIfNeeded()
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (!T66ShouldUseDevelopmentDummyLeaderboards() || IsBackendConfigured())
	{
		return;
	}

	static const TCHAR* const KeysToSeed[] = {
		TEXT("score_weekly_solo_easy_global"),
		TEXT("score_alltime_solo_easy_global"),
		TEXT("score_weekly_solo_easy_friends"),
		TEXT("score_alltime_solo_easy_friends"),
		TEXT("score_weekly_solo_easy_streamers"),
		TEXT("score_alltime_solo_easy_streamers"),
		TEXT("speedrun_weekly_solo_easy_global"),
		TEXT("speedrun_alltime_solo_easy_global"),
		TEXT("speedrun_weekly_solo_easy_friends"),
		TEXT("speedrun_alltime_solo_easy_friends"),
		TEXT("speedrun_weekly_solo_easy_streamers"),
		TEXT("speedrun_alltime_solo_easy_streamers")
	};

	for (const TCHAR* Key : KeysToSeed)
	{
		TryPopulateDevelopmentDummyLeaderboard(Key);
	}
#endif
}

bool UT66BackendSubsystem::TryPopulateDevelopmentDummyLeaderboard(const FString& Key)
{
#if UE_BUILD_SHIPPING
	return false;
#else
	if (!T66ShouldUseDevelopmentDummyLeaderboards())
	{
		return false;
	}

	if (const FCachedLeaderboard* Existing = LeaderboardCache.Find(Key))
	{
		if (Existing->Entries.Num() > 0)
		{
			return true;
		}
	}

	FT66DummyLeaderboardIdentity Identity;
	if (!T66TryParseDummyLeaderboardKey(Key, Identity))
	{
		return false;
	}

	const bool bScoreBoard = Identity.Type == TEXT("score");
	const bool bAllTime = Identity.Time == TEXT("alltime");
	const int32 FilterIndex = T66GetDummyFilterIndex(Identity.Filter);
	const int32 BaseScore = bScoreBoard ? (bAllTime ? 9400 : 8200) : (bAllTime ? 7600 : 7000);
	const float BaseTime = bScoreBoard ? (bAllTime ? 244.0f : 258.0f) : (bAllTime ? 231.0f : 239.0f);

	FCachedLeaderboard Cached;
	Cached.TotalEntries = 5;

	for (int32 EntryIndex = 0; EntryIndex < 5; ++EntryIndex)
	{
		const int32 Rank = EntryIndex + 1;
		const int32 FilterScoreModifier = (FilterIndex == 0) ? 0 : (FilterIndex == 1 ? -220 : -360);
		const float FilterTimeModifier = (FilterIndex == 0) ? 0.f : (FilterIndex == 1 ? 6.5f : 11.0f);

		FLeaderboardEntry& Entry = Cached.Entries.AddDefaulted_GetRef();
		Entry.Rank = Rank;
		Entry.PlayerName = T66GetDummyPlayerName(Identity.Filter, EntryIndex);
		Entry.PlayerNames = { Entry.PlayerName };
		Entry.Score = FMath::Max<int64>(3500, BaseScore + FilterScoreModifier - (EntryIndex * 420));
		Entry.TimeSeconds = FMath::Max(60.0f, BaseTime + FilterTimeModifier + (EntryIndex * 8.75f));
		Entry.HeroID = T66GetDummyHeroId(EntryIndex + FilterIndex);
		Entry.PartySize = ET66PartySize::Solo;
		Entry.Difficulty = ET66Difficulty::Easy;
		Entry.StageReached = 5;
		Entry.bIsLocalPlayer = false;
		Entry.EntryId = FString::Printf(
			TEXT("dummy_%s_%s_%s_r%d"),
			*Identity.Type,
			*Identity.Time,
			*Identity.Filter,
			Rank);
		Entry.bHasRunSummary = true;
		Entry.AvatarUrl.Reset();

		RunSummaryCache.FindOrAdd(Entry.EntryId) = T66BuildDummyRunSummary(this, Identity, Entry, Entry.PlayerName);
	}

	LeaderboardCache.Add(Key, MoveTemp(Cached));
	UE_LOG(LogT66Backend, Log, TEXT("Backend: seeded dummy leaderboard for key=%s"), *Key);
	return true;
#endif
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UT66BackendSubsystem::CreateRequest(const FString& Verb, const FString& Endpoint) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(FString::Printf(TEXT("%s%s"), *BackendBaseUrl, *Endpoint));
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	return Request;
}

void UT66BackendSubsystem::SetAuthHeaders(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request) const
{
	if (!CachedSteamTicketHex.IsEmpty())
	{
		Request->SetHeader(TEXT("X-Steam-Ticket"), CachedSteamTicketHex);
	}
}

FString UT66BackendSubsystem::ExtractResponseMessage(const TSharedPtr<FJsonObject>& Json, const FString& FallbackMessage)
{
	if (!Json.IsValid())
	{
		return FallbackMessage;
	}

	FString Message;
	if (Json->TryGetStringField(TEXT("message"), Message) && !Message.IsEmpty())
	{
		return Message;
	}

	FString Status;
	if (Json->TryGetStringField(TEXT("status"), Status) && !Status.IsEmpty())
	{
		return Status;
	}

	return FallbackMessage;
}

FString UT66BackendSubsystem::DifficultyToApiString(ET66Difficulty Diff)
{
	switch (Diff)
	{
	case ET66Difficulty::Easy: return TEXT("easy");
	case ET66Difficulty::Medium: return TEXT("medium");
	case ET66Difficulty::Hard: return TEXT("hard");
	case ET66Difficulty::VeryHard: return TEXT("veryhard");
	case ET66Difficulty::Impossible: return TEXT("impossible");
	default: return TEXT("easy");
	}
}

FString UT66BackendSubsystem::PartySizeToApiString(ET66PartySize Party)
{
	switch (Party)
	{
	case ET66PartySize::Solo: return TEXT("solo");
	case ET66PartySize::Duo: return TEXT("duo");
	case ET66PartySize::Trio: return TEXT("trio");
	case ET66PartySize::Quad: return TEXT("quad");
	default: return TEXT("solo");
	}
}
