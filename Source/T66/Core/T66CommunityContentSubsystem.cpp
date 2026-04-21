// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66CommunityContentSaveGame.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66CommunityContent, Log, All);

namespace
{
	static constexpr TCHAR CommunityContentSaveSlotName[] = TEXT("T66CommunityContent");
	static constexpr int32 CommunityContentSaveUserIndex = 0;

	static FName MakeOfficialEntryId(const TCHAR* Name)
	{
		return FName(Name);
	}

	static FName MakeDraftEntryId()
	{
		return FName(*FString::Printf(TEXT("Draft_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
	}

	static void SetOptionalString(const TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FString& Value)
	{
		if (Json.IsValid() && !Value.IsEmpty())
		{
			Json->SetStringField(FieldName, Value);
		}
	}

	static const UEnum* GetPassiveEnum()
	{
		return StaticEnum<ET66PassiveType>();
	}

	static const UEnum* GetUltimateEnum()
	{
		return StaticEnum<ET66UltimateType>();
	}

	static FString PassiveToApiString(const ET66PassiveType PassiveType)
	{
		const UEnum* Enum = GetPassiveEnum();
		return Enum ? Enum->GetNameStringByValue(static_cast<int64>(PassiveType)) : TEXT("None");
	}

	static FString UltimateToApiString(const ET66UltimateType UltimateType)
	{
		const UEnum* Enum = GetUltimateEnum();
		return Enum ? Enum->GetNameStringByValue(static_cast<int64>(UltimateType)) : TEXT("None");
	}

	static ET66PassiveType PassiveFromApiString(const FString& Value)
	{
		const UEnum* Enum = GetPassiveEnum();
		if (!Enum)
		{
			return ET66PassiveType::None;
		}

		const int64 RawValue = Enum->GetValueByNameString(Value);
		return RawValue == INDEX_NONE ? ET66PassiveType::None : static_cast<ET66PassiveType>(RawValue);
	}

	static ET66UltimateType UltimateFromApiString(const FString& Value)
	{
		const UEnum* Enum = GetUltimateEnum();
		if (!Enum)
		{
			return ET66UltimateType::None;
		}

		const int64 RawValue = Enum->GetValueByNameString(Value);
		return RawValue == INDEX_NONE ? ET66UltimateType::None : static_cast<ET66UltimateType>(RawValue);
	}

	static void AppendStatBonusLine(TArray<FString>& Lines, const TCHAR* Label, const int32 Value)
	{
		if (Value != 0)
		{
			Lines.Add(FString::Printf(TEXT("%+d %s."), Value, Label));
		}
	}

	static FString ToFriendlySubmissionStatus(const FString& Status)
	{
		if (Status.Equals(TEXT("approved"), ESearchCase::IgnoreCase))
		{
			return TEXT("Approved");
		}
		if (Status.Equals(TEXT("pending"), ESearchCase::IgnoreCase))
		{
			return TEXT("Pending approval");
		}
		if (Status.Equals(TEXT("changes_requested"), ESearchCase::IgnoreCase))
		{
			return TEXT("Changes requested");
		}
		if (Status.Equals(TEXT("rejected"), ESearchCase::IgnoreCase))
		{
			return TEXT("Rejected");
		}
		if (Status.Equals(TEXT("unpublished"), ESearchCase::IgnoreCase))
		{
			return TEXT("Unpublished");
		}

		return Status.IsEmpty() ? TEXT("Not submitted") : Status;
	}

	static void T66_RunCommunityCatalogValidation(const TArray<FString>& Args, UWorld* World)
	{
		if (!World)
		{
			UE_LOG(LogT66CommunityContent, Warning, TEXT("T66.Community.ValidateCatalog requires a valid world."));
			return;
		}

		UGameInstance* GameInstance = World->GetGameInstance();
		UT66CommunityContentSubsystem* Community = GameInstance ? GameInstance->GetSubsystem<UT66CommunityContentSubsystem>() : nullptr;
		if (!Community)
		{
			UE_LOG(LogT66CommunityContent, Warning, TEXT("T66.Community.ValidateCatalog could not find the community content subsystem."));
			return;
		}

		const bool bIncludeDrafts = Args.ContainsByPredicate([](const FString& Arg)
		{
			return Arg.Equals(TEXT("drafts"), ESearchCase::IgnoreCase);
		});
		const bool bVerbose = Args.ContainsByPredicate([](const FString& Arg)
		{
			return Arg.Equals(TEXT("verbose"), ESearchCase::IgnoreCase);
		});

		Community->RunCatalogSmokeTest(bIncludeDrafts, bVerbose);
	}
}

static FAutoConsoleCommandWithWorldAndArgs T66CommunityValidateCatalogCommand(
	TEXT("T66.Community.ValidateCatalog"),
	TEXT("Validates official/community community-content entries. Optional args: drafts verbose"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_RunCommunityCatalogValidation));

void UT66CommunityContentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SeedOfficialContent();
	LoadOrCreateSave();
	RefreshCommunityCatalog(false);
}

const TArray<FT66CommunityContentEntry>& UT66CommunityContentSubsystem::GetOfficialEntries(const ET66CommunityContentKind Kind) const
{
	return Kind == ET66CommunityContentKind::Mod ? OfficialMods : OfficialChallenges;
}

const TArray<FT66CommunityContentEntry>& UT66CommunityContentSubsystem::GetCommunityEntries(const ET66CommunityContentKind Kind) const
{
	return Kind == ET66CommunityContentKind::Mod ? CommunityMods : CommunityChallenges;
}

const TArray<FT66CommunityContentEntry>& UT66CommunityContentSubsystem::GetDraftEntries(const ET66CommunityContentKind Kind) const
{
	return Kind == ET66CommunityContentKind::Mod ? DraftMods : DraftChallenges;
}

TArray<FT66CommunityContentEntry> UT66CommunityContentSubsystem::GetCommunityBrowserEntries(const ET66CommunityContentKind Kind) const
{
	TArray<FT66CommunityContentEntry> Result = GetDraftEntries(Kind);
	Result.Append(GetCommunityEntries(Kind));
	return Result;
}

FT66CommunityContentEntry UT66CommunityContentSubsystem::CreateDraftTemplate(const ET66CommunityContentKind Kind) const
{
	FT66CommunityContentEntry Draft;
	Draft.LocalId = MakeDraftEntryId();
	Draft.Kind = Kind;
	Draft.Origin = ET66CommunityContentOrigin::Draft;
	Draft.Title = Kind == ET66CommunityContentKind::Mod ? TEXT("New Mod") : TEXT("New Challenge");
	Draft.Description = TEXT("Describe what this ruleset changes.");
	Draft.AuthorDisplayName = TEXT("You");
	Draft.ModerationStatus = TEXT("local");
	Draft.SubmissionStatus = TEXT("Not submitted");
	Draft.bLocallyAuthored = true;
	Draft.Rules.bRequireFullClear = true;
	Draft.UpdatedAt = FDateTime::UtcNow();
	Draft.Sanitize();
	return Draft;
}

void UT66CommunityContentSubsystem::SanitizeEntryForStorage(FT66CommunityContentEntry& Entry) const
{
	Entry.Sanitize();

	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	if (T66GI && !Entry.Rules.StartingItemId.IsNone())
	{
		FItemData ItemData;
		if (!const_cast<UT66GameInstance*>(T66GI)->GetItemData(Entry.Rules.StartingItemId, ItemData))
		{
			UE_LOG(LogT66CommunityContent, Warning, TEXT("Clearing invalid community starting item '%s' from '%s'."), *Entry.Rules.StartingItemId.ToString(), *Entry.Title);
			Entry.Rules.StartingItemId = NAME_None;
		}
	}
}

void UT66CommunityContentSubsystem::ValidateEntryForRuntime(const FT66CommunityContentEntry& Entry, TArray<FString>& OutIssues) const
{
	OutIssues.Reset();

	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	if (T66GI && !Entry.Rules.StartingItemId.IsNone())
	{
		FItemData ItemData;
		if (!const_cast<UT66GameInstance*>(T66GI)->GetItemData(Entry.Rules.StartingItemId, ItemData))
		{
			OutIssues.Add(FString::Printf(TEXT("Unknown starting item: %s"), *Entry.Rules.StartingItemId.ToString()));
		}
	}

	if (Entry.Kind == ET66CommunityContentKind::Mod && !Entry.Rules.HasGameplayChanges())
	{
		OutIssues.Add(TEXT("Mod has no gameplay changes."));
	}

	if (Entry.Kind == ET66CommunityContentKind::Challenge
		&& !Entry.Rules.HasGameplayChanges()
		&& !Entry.Rules.bRequireNoDamage
		&& Entry.Rules.RequiredStageReached <= 0
		&& Entry.Rules.MaxRunTimeSeconds <= 0)
	{
		OutIssues.Add(TEXT("Challenge has no gameplay changes or explicit completion requirements."));
	}
}

void UT66CommunityContentSubsystem::SanitizeBucket(TArray<FT66CommunityContentEntry>& Bucket) const
{
	for (FT66CommunityContentEntry& Entry : Bucket)
	{
		SanitizeEntryForStorage(Entry);
	}
}

void UT66CommunityContentSubsystem::LogValidationIssues(const FT66CommunityContentEntry& Entry, const TArray<FString>& Issues, const TCHAR* Context) const
{
	for (const FString& Issue : Issues)
	{
		UE_LOG(LogT66CommunityContent, Warning, TEXT("[%s] %s (%s): %s"),
			Context,
			*Entry.Title,
			Entry.Kind == ET66CommunityContentKind::Mod ? TEXT("Mod") : TEXT("Challenge"),
			*Issue);
	}
}

void UT66CommunityContentSubsystem::RunCatalogSmokeTest(const bool bIncludeDrafts, const bool bVerbose) const
{
	int32 CheckedEntries = 0;
	int32 EntriesWithIssues = 0;

	auto ValidateBucket = [this, bVerbose, &CheckedEntries, &EntriesWithIssues](const TArray<FT66CommunityContentEntry>& Bucket, const TCHAR* Context)
	{
		for (const FT66CommunityContentEntry& StoredEntry : Bucket)
		{
			FT66CommunityContentEntry Entry = StoredEntry;
			SanitizeEntryForStorage(Entry);

			TArray<FString> Issues;
			ValidateEntryForRuntime(Entry, Issues);

			++CheckedEntries;
			if (Issues.Num() > 0)
			{
				++EntriesWithIssues;
				LogValidationIssues(Entry, Issues, Context);
			}
			else if (bVerbose)
			{
				UE_LOG(LogT66CommunityContent, Log, TEXT("[%s] %s validated cleanly."), Context, *Entry.Title);
			}
		}
	};

	ValidateBucket(OfficialChallenges, TEXT("OfficialChallenges"));
	ValidateBucket(OfficialMods, TEXT("OfficialMods"));
	ValidateBucket(CommunityChallenges, TEXT("CommunityChallenges"));
	ValidateBucket(CommunityMods, TEXT("CommunityMods"));
	if (bIncludeDrafts)
	{
		ValidateBucket(DraftChallenges, TEXT("DraftChallenges"));
		ValidateBucket(DraftMods, TEXT("DraftMods"));
	}

	UE_LOG(LogT66CommunityContent, Log, TEXT("Community catalog validation complete. Checked=%d EntriesWithIssues=%d IncludeDrafts=%d"),
		CheckedEntries,
		EntriesWithIssues,
		bIncludeDrafts ? 1 : 0);
}

bool UT66CommunityContentSubsystem::SaveDraft(const FT66CommunityContentEntry& Draft)
{
	if (Draft.LocalId.IsNone())
	{
		return false;
	}

	TArray<FT66CommunityContentEntry>& Bucket = GetMutableBucket(ET66CommunityContentOrigin::Draft, Draft.Kind);
	const int32 ExistingIndex = Bucket.IndexOfByPredicate([&Draft](const FT66CommunityContentEntry& Entry)
	{
		return Entry.LocalId == Draft.LocalId;
	});

	FT66CommunityContentEntry SavedDraft = Draft;
	SavedDraft.Origin = ET66CommunityContentOrigin::Draft;
	SavedDraft.bLocallyAuthored = true;
	SavedDraft.UpdatedAt = FDateTime::UtcNow();
	SanitizeEntryForStorage(SavedDraft);
	if (SavedDraft.SubmissionStatus.IsEmpty())
	{
		SavedDraft.SubmissionStatus = TEXT("Not submitted");
	}

	if (ExistingIndex != INDEX_NONE)
	{
		Bucket[ExistingIndex] = SavedDraft;
	}
	else
	{
		Bucket.Insert(SavedDraft, 0);
	}

	PersistSave();
	BroadcastContentChanged();
	return true;
}

bool UT66CommunityContentSubsystem::DeleteDraft(const FName LocalId)
{
	for (TArray<FT66CommunityContentEntry>* Bucket : { &DraftChallenges, &DraftMods })
	{
		const int32 ExistingIndex = Bucket->IndexOfByPredicate([LocalId](const FT66CommunityContentEntry& Entry)
		{
			return Entry.LocalId == LocalId;
		});
		if (ExistingIndex != INDEX_NONE)
		{
			Bucket->RemoveAt(ExistingIndex);
			if (ActiveEntryId == LocalId)
			{
				ActiveEntryId = NAME_None;
			}
			PersistSave();
			BroadcastContentChanged();
			return true;
		}
	}

	return false;
}

bool UT66CommunityContentSubsystem::FindEntryById(const FName LocalId, FT66CommunityContentEntry& OutEntry) const
{
	if (LocalId.IsNone())
	{
		return false;
	}

	for (const TArray<FT66CommunityContentEntry>* Bucket : { &DraftChallenges, &DraftMods, &CommunityChallenges, &CommunityMods, &OfficialChallenges, &OfficialMods })
	{
		const FT66CommunityContentEntry* Found = Bucket->FindByPredicate([LocalId](const FT66CommunityContentEntry& Entry)
		{
			return Entry.LocalId == LocalId;
		});
		if (Found)
		{
			OutEntry = *Found;
			return true;
		}
	}

	return false;
}

bool UT66CommunityContentSubsystem::ActivateEntry(const FName LocalId)
{
	FT66CommunityContentEntry Entry;
	if (!FindEntryById(LocalId, Entry))
	{
		return false;
	}

	SanitizeEntryForStorage(Entry);
	TArray<FString> ValidationIssues;
	ValidateEntryForRuntime(Entry, ValidationIssues);
	if (ValidationIssues.Num() > 0)
	{
		LogValidationIssues(Entry, ValidationIssues, TEXT("ActivateEntry"));
	}

	ActiveEntryId = LocalId;

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		T66GI->SelectedRunModifierKind =
			Entry.Kind == ET66CommunityContentKind::Mod
			? ET66RunModifierKind::Mod
			: ET66RunModifierKind::Challenge;
		T66GI->SelectedRunModifierID = Entry.LocalId;
	}

	LastStatusMessage = FString::Printf(TEXT("Armed %s."), *Entry.Title);
	BroadcastContentChanged();
	return true;
}

bool UT66CommunityContentSubsystem::GetActiveEntry(FT66CommunityContentEntry& OutEntry) const
{
	return ResolveActiveEntry(OutEntry);
}

void UT66CommunityContentSubsystem::ClearActiveEntry()
{
	ActiveEntryId = NAME_None;

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		T66GI->SelectedRunModifierKind = ET66RunModifierKind::None;
		T66GI->SelectedRunModifierID = NAME_None;
	}

	BroadcastContentChanged();
}

void UT66CommunityContentSubsystem::RefreshCommunityCatalog(const bool bForce)
{
	if (bCatalogRefreshInFlight && !bForce)
	{
		return;
	}

	const FString BackendBaseUrl = GetBackendBaseUrl();
	if (BackendBaseUrl.IsEmpty())
	{
		LastStatusMessage = TEXT("Community content backend is not configured.");
		return;
	}

	bCatalogRefreshInFlight = true;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(TEXT("GET"));
	Request->SetURL(BackendBaseUrl + TEXT("/api/community-content/catalog"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->OnProcessRequestComplete().BindUObject(this, &UT66CommunityContentSubsystem::OnCatalogResponseReceived);
	Request->ProcessRequest();
}

void UT66CommunityContentSubsystem::RefreshMySubmissionStates(const bool bForce)
{
	if (bMySubmissionsRefreshInFlight && !bForce)
	{
		return;
	}

	const FString BackendBaseUrl = GetBackendBaseUrl();
	const FString TicketHex = GetSteamTicketHex();
	if (BackendBaseUrl.IsEmpty() || TicketHex.IsEmpty())
	{
		return;
	}

	bMySubmissionsRefreshInFlight = true;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(TEXT("GET"));
	Request->SetURL(BackendBaseUrl + TEXT("/api/community-content/my-submissions"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("X-Steam-Ticket"), TicketHex);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66CommunityContentSubsystem::OnMySubmissionsResponseReceived);
	Request->ProcessRequest();
}

bool UT66CommunityContentSubsystem::SubmitDraftForApproval(const FName LocalId)
{
	if (bSubmitInFlight)
	{
		LastStatusMessage = TEXT("A community submission is already in flight.");
		return false;
	}

	FT66CommunityContentEntry Draft;
	if (!FindEntryById(LocalId, Draft) || !Draft.IsDraft())
	{
		LastStatusMessage = TEXT("Select a local draft before submitting.");
		return false;
	}
	if (!Draft.BackendId.IsEmpty() && Draft.ModerationStatus.Equals(TEXT("approved"), ESearchCase::IgnoreCase))
	{
		LastStatusMessage = TEXT("Approved submissions cannot be overwritten. Create a new draft to publish a variation.");
		BroadcastContentChanged();
		return false;
	}

	const FString BackendBaseUrl = GetBackendBaseUrl();
	const FString TicketHex = GetSteamTicketHex();
	if (BackendBaseUrl.IsEmpty() || TicketHex.IsEmpty())
	{
		LastStatusMessage = TEXT("Community submission requires the live backend and a Steam session ticket.");
		return false;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	if (!Draft.BackendId.IsEmpty())
	{
		Root->SetStringField(TEXT("submission_id"), Draft.BackendId);
	}
	Root->SetStringField(TEXT("kind"), KindToApiString(Draft.Kind));
	Root->SetStringField(TEXT("title"), Draft.Title);
	Root->SetStringField(TEXT("description"), Draft.Description);
	Root->SetStringField(TEXT("author_display_name"), Draft.AuthorDisplayName);
	Root->SetNumberField(TEXT("suggested_reward_chad_coupons"), FMath::Max(0, Draft.SuggestedRewardChadCoupons));

	TSharedPtr<FJsonObject> RulesJson = MakeShared<FJsonObject>();
	RulesJson->SetNumberField(TEXT("start_level_override"), FMath::Max(0, Draft.Rules.StartLevelOverride));
	RulesJson->SetBoolField(TEXT("set_max_hero_stats"), Draft.Rules.bSetMaxHeroStats);
	if (!Draft.Rules.StartingItemId.IsNone())
	{
		RulesJson->SetStringField(TEXT("starting_item_id"), Draft.Rules.StartingItemId.ToString());
	}
	SetOptionalString(RulesJson, TEXT("passive_override"), Draft.Rules.PassiveOverride != ET66PassiveType::None ? PassiveToApiString(Draft.Rules.PassiveOverride) : FString());
	SetOptionalString(RulesJson, TEXT("ultimate_override"), Draft.Rules.UltimateOverride != ET66UltimateType::None ? UltimateToApiString(Draft.Rules.UltimateOverride) : FString());
	RulesJson->SetBoolField(TEXT("require_full_clear"), Draft.Rules.bRequireFullClear);
	RulesJson->SetBoolField(TEXT("require_no_damage"), Draft.Rules.bRequireNoDamage);
	RulesJson->SetNumberField(TEXT("required_stage_reached"), FMath::Max(0, Draft.Rules.RequiredStageReached));
	RulesJson->SetNumberField(TEXT("max_run_time_seconds"), FMath::Max(0, Draft.Rules.MaxRunTimeSeconds));

	TSharedPtr<FJsonObject> BonusStatsJson = MakeShared<FJsonObject>();
	BonusStatsJson->SetNumberField(TEXT("damage"), Draft.Rules.BonusStats.Damage);
	BonusStatsJson->SetNumberField(TEXT("attack_speed"), Draft.Rules.BonusStats.AttackSpeed);
	BonusStatsJson->SetNumberField(TEXT("attack_scale"), Draft.Rules.BonusStats.AttackScale);
	BonusStatsJson->SetNumberField(TEXT("accuracy"), Draft.Rules.BonusStats.Accuracy);
	BonusStatsJson->SetNumberField(TEXT("armor"), Draft.Rules.BonusStats.Armor);
	BonusStatsJson->SetNumberField(TEXT("evasion"), Draft.Rules.BonusStats.Evasion);
	BonusStatsJson->SetNumberField(TEXT("luck"), Draft.Rules.BonusStats.Luck);
	BonusStatsJson->SetNumberField(TEXT("speed"), Draft.Rules.BonusStats.Speed);
	RulesJson->SetObjectField(TEXT("bonus_stats"), BonusStatsJson);
	Root->SetObjectField(TEXT("rules"), RulesJson);

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	bSubmitInFlight = true;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(TEXT("POST"));
	Request->SetURL(BackendBaseUrl + TEXT("/api/community-content/submit"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("X-Steam-Ticket"), TicketHex);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66CommunityContentSubsystem::OnSubmitResponseReceived, LocalId);
	Request->ProcessRequest();

	LastStatusMessage = FString::Printf(TEXT("Submitting %s for approval..."), *Draft.Title);
	BroadcastContentChanged();
	return true;
}

TArray<FString> UT66CommunityContentSubsystem::BuildRuleSummaryLines(const FT66CommunityContentEntry& Entry) const
{
	TArray<FString> Lines;

	if (Entry.Rules.bSetMaxHeroStats)
	{
		Lines.Add(TEXT("Start at level 99 with maxed hero stats."));
	}
	else if (Entry.Rules.StartLevelOverride > 0)
	{
		Lines.Add(FString::Printf(TEXT("Start at level %d."), Entry.Rules.StartLevelOverride));
	}

	AppendStatBonusLine(Lines, TEXT("Damage"), Entry.Rules.BonusStats.Damage);
	AppendStatBonusLine(Lines, TEXT("Attack Speed"), Entry.Rules.BonusStats.AttackSpeed);
	AppendStatBonusLine(Lines, TEXT("Attack Scale"), Entry.Rules.BonusStats.AttackScale);
	AppendStatBonusLine(Lines, TEXT("Accuracy"), Entry.Rules.BonusStats.Accuracy);
	AppendStatBonusLine(Lines, TEXT("Armor"), Entry.Rules.BonusStats.Armor);
	AppendStatBonusLine(Lines, TEXT("Evasion"), Entry.Rules.BonusStats.Evasion);
	AppendStatBonusLine(Lines, TEXT("Luck"), Entry.Rules.BonusStats.Luck);
	AppendStatBonusLine(Lines, TEXT("Speed"), Entry.Rules.BonusStats.Speed);

	if (!Entry.Rules.StartingItemId.IsNone())
	{
		Lines.Add(FString::Printf(TEXT("Start with %s."), *Entry.Rules.StartingItemId.ToString()));
	}

	if (Entry.Rules.PassiveOverride != ET66PassiveType::None)
	{
		const UEnum* Enum = GetPassiveEnum();
		const FString Label = Enum ? Enum->GetDisplayNameTextByValue(static_cast<int64>(Entry.Rules.PassiveOverride)).ToString() : TEXT("Passive Override");
		Lines.Add(FString::Printf(TEXT("Override passive to %s."), *Label));
	}

	if (Entry.Rules.UltimateOverride != ET66UltimateType::None)
	{
		const UEnum* Enum = GetUltimateEnum();
		const FString Label = Enum ? Enum->GetDisplayNameTextByValue(static_cast<int64>(Entry.Rules.UltimateOverride)).ToString() : TEXT("Ultimate Override");
		Lines.Add(FString::Printf(TEXT("Override ultimate to %s."), *Label));
	}

	if (Entry.Kind == ET66CommunityContentKind::Challenge)
	{
		if (Entry.Rules.bRequireFullClear)
		{
			Lines.Add(TEXT("Challenge only completes on a full clear."));
		}
		if (Entry.Rules.bRequireNoDamage)
		{
			Lines.Add(TEXT("Take no damage for the run."));
		}
		if (Entry.Rules.RequiredStageReached > 0)
		{
			Lines.Add(FString::Printf(TEXT("Reach at least stage %d."), Entry.Rules.RequiredStageReached));
		}
		if (Entry.Rules.MaxRunTimeSeconds > 0)
		{
			Lines.Add(FString::Printf(TEXT("Finish within %d seconds."), Entry.Rules.MaxRunTimeSeconds));
		}
	}

	if (Lines.Num() == 0)
	{
		Lines.Add(TEXT("No gameplay changes configured yet."));
	}

	return Lines;
}

FString UT66CommunityContentSubsystem::BuildRewardSummary(const FT66CommunityContentEntry& Entry) const
{
	if (Entry.Kind == ET66CommunityContentKind::Mod)
	{
		return TEXT("No Chad Coupon reward.");
	}

	if (Entry.HasApprovedReward())
	{
		return FString::Printf(TEXT("%d Chad Coupons"), FMath::Max(0, Entry.ApprovedRewardChadCoupons));
	}

	if (Entry.IsDraft())
	{
		return Entry.SuggestedRewardChadCoupons > 0
			? FString::Printf(TEXT("Suggested reward: %d Chad Coupons"), Entry.SuggestedRewardChadCoupons)
			: TEXT("Local draft - no reward until approved.");
	}

	if (Entry.SuggestedRewardChadCoupons > 0)
	{
		return FString::Printf(TEXT("Pending review - suggested reward %d"), Entry.SuggestedRewardChadCoupons);
	}

	return TEXT("No approved Chad Coupon reward.");
}

FString UT66CommunityContentSubsystem::BuildSelectionSummary(const FT66CommunityContentEntry& Entry) const
{
	if (Entry.IsDraft())
	{
		return TEXT("Local draft. Playable immediately. Submit it if you want it reviewed for the public Community tab.");
	}

	if (Entry.Origin == ET66CommunityContentOrigin::Community)
	{
		return Entry.IsCommunityApproved()
			? TEXT("Approved community entry. Selecting it arms the rules for the next run.")
			: TEXT("Pending community entry. You can still play your local draft version.");
	}

	return TEXT("Official entry prepared by the game build.");
}

bool UT66CommunityContentSubsystem::EvaluateChallengeCompletion(const FT66CommunityContentEntry& Entry, const UT66RunStateSubsystem* RunState, FString& OutFailureReason) const
{
	OutFailureReason.Reset();
	if (!RunState)
	{
		OutFailureReason = TEXT("Run state was unavailable.");
		return false;
	}

	if (Entry.Kind != ET66CommunityContentKind::Challenge)
	{
		OutFailureReason = TEXT("Only challenges can award Chad Coupons.");
		return false;
	}

	if (Entry.Rules.bRequireFullClear && !RunState->DidRunEndInVictory())
	{
		OutFailureReason = TEXT("This challenge requires a full clear.");
		return false;
	}

	if (Entry.Rules.bRequireNoDamage && RunState->GetDamageTakenHitCount() > 0)
	{
		OutFailureReason = TEXT("This challenge failed because damage was taken.");
		return false;
	}

	if (Entry.Rules.RequiredStageReached > 0 && RunState->GetCurrentStage() < Entry.Rules.RequiredStageReached)
	{
		OutFailureReason = FString::Printf(TEXT("This challenge requires reaching stage %d."), Entry.Rules.RequiredStageReached);
		return false;
	}

	if (Entry.Rules.MaxRunTimeSeconds > 0 && RunState->GetFinalRunElapsedSeconds() > static_cast<float>(Entry.Rules.MaxRunTimeSeconds))
	{
		OutFailureReason = FString::Printf(TEXT("This challenge must be finished within %d seconds."), Entry.Rules.MaxRunTimeSeconds);
		return false;
	}

	return true;
}

int32 UT66CommunityContentSubsystem::GetApprovedRewardForActiveChallenge(const UT66RunStateSubsystem* RunState, FString* OutRewardLabel, FString* OutFailureReason) const
{
	if (OutRewardLabel)
	{
		OutRewardLabel->Reset();
	}
	if (OutFailureReason)
	{
		OutFailureReason->Reset();
	}

	FT66CommunityContentEntry ActiveEntry;
	if (!ResolveActiveEntry(ActiveEntry) || !ActiveEntry.HasApprovedReward())
	{
		return 0;
	}

	FString FailureReason;
	if (!EvaluateChallengeCompletion(ActiveEntry, RunState, FailureReason))
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FailureReason;
		}
		return 0;
	}

	if (OutRewardLabel)
	{
		*OutRewardLabel = ActiveEntry.Title;
	}

	return FMath::Max(0, ActiveEntry.ApprovedRewardChadCoupons);
}

ET66PassiveType UT66CommunityContentSubsystem::GetActivePassiveOverride() const
{
	FT66CommunityContentEntry ActiveEntry;
	return ResolveActiveEntry(ActiveEntry) ? ActiveEntry.Rules.PassiveOverride : ET66PassiveType::None;
}

ET66UltimateType UT66CommunityContentSubsystem::GetActiveUltimateOverride() const
{
	FT66CommunityContentEntry ActiveEntry;
	return ResolveActiveEntry(ActiveEntry) ? ActiveEntry.Rules.UltimateOverride : ET66UltimateType::None;
}

void UT66CommunityContentSubsystem::SeedOfficialContent()
{
	OfficialChallenges.Reset();
	OfficialMods.Reset();

	FT66CommunityContentEntry GlassRoute;
	GlassRoute.LocalId = MakeOfficialEntryId(TEXT("Challenge_GlassRoute"));
	GlassRoute.Kind = ET66CommunityContentKind::Challenge;
	GlassRoute.Origin = ET66CommunityContentOrigin::Official;
	GlassRoute.Title = TEXT("Glass Route");
	GlassRoute.Description = TEXT("Clear the run without taking a single hit.");
	GlassRoute.AuthorDisplayName = TEXT("Tribulation 66");
	GlassRoute.ModerationStatus = TEXT("approved");
	GlassRoute.ApprovedRewardChadCoupons = 40;
	GlassRoute.Rules.bRequireFullClear = true;
	GlassRoute.Rules.bRequireNoDamage = true;
	OfficialChallenges.Add(GlassRoute);

	FT66CommunityContentEntry SpeedClear;
	SpeedClear.LocalId = MakeOfficialEntryId(TEXT("Challenge_SpeedClear"));
	SpeedClear.Kind = ET66CommunityContentKind::Challenge;
	SpeedClear.Origin = ET66CommunityContentOrigin::Official;
	SpeedClear.Title = TEXT("Pressure Run");
	SpeedClear.Description = TEXT("Finish a full clear before the timer budget expires.");
	SpeedClear.AuthorDisplayName = TEXT("Tribulation 66");
	SpeedClear.ModerationStatus = TEXT("approved");
	SpeedClear.ApprovedRewardChadCoupons = 30;
	SpeedClear.Rules.bRequireFullClear = true;
	SpeedClear.Rules.MaxRunTimeSeconds = 420;
	OfficialChallenges.Add(SpeedClear);

	FT66CommunityContentEntry MaxPower;
	MaxPower.LocalId = MakeOfficialEntryId(TEXT("Mod_MaxHeroStats"));
	MaxPower.Kind = ET66CommunityContentKind::Mod;
	MaxPower.Origin = ET66CommunityContentOrigin::Official;
	MaxPower.Title = TEXT("Max Power");
	MaxPower.Description = TEXT("Start at full level with every stat slammed to the ceiling.");
	MaxPower.AuthorDisplayName = TEXT("Tribulation 66");
	MaxPower.ModerationStatus = TEXT("approved");
	MaxPower.Rules.bSetMaxHeroStats = true;
	MaxPower.Rules.StartLevelOverride = 99;
	OfficialMods.Add(MaxPower);

	FT66CommunityContentEntry LoadedDice;
	LoadedDice.LocalId = MakeOfficialEntryId(TEXT("Mod_LoadedDice"));
	LoadedDice.Kind = ET66CommunityContentKind::Mod;
	LoadedDice.Origin = ET66CommunityContentOrigin::Official;
	LoadedDice.Title = TEXT("Loaded Dice");
	LoadedDice.Description = TEXT("Push a luck-heavy build from the first room.");
	LoadedDice.AuthorDisplayName = TEXT("Tribulation 66");
	LoadedDice.ModerationStatus = TEXT("approved");
	LoadedDice.Rules.BonusStats.Luck = 25;
	LoadedDice.Rules.StartingItemId = FName(TEXT("Item_GamblersToken"));
	OfficialMods.Add(LoadedDice);

	SanitizeBucket(OfficialChallenges);
	SanitizeBucket(OfficialMods);
}

void UT66CommunityContentSubsystem::LoadOrCreateSave()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(CommunityContentSaveSlotName, CommunityContentSaveUserIndex);
	SaveData = Cast<UT66CommunityContentSaveGame>(Loaded);
	if (!SaveData)
	{
		SaveData = NewObject<UT66CommunityContentSaveGame>(this);
	}

	DraftChallenges.Reset();
	DraftMods.Reset();
	CommunityChallenges.Reset();
	CommunityMods.Reset();

	for (const FT66CommunityContentEntry& Entry : SaveData->DraftEntries)
	{
		FT66CommunityContentEntry SanitizedEntry = Entry;
		SanitizeEntryForStorage(SanitizedEntry);
		GetMutableBucket(ET66CommunityContentOrigin::Draft, SanitizedEntry.Kind).Add(SanitizedEntry);
	}

	for (const FT66CommunityContentEntry& Entry : SaveData->CachedCommunityEntries)
	{
		FT66CommunityContentEntry SanitizedEntry = Entry;
		SanitizeEntryForStorage(SanitizedEntry);
		GetMutableBucket(ET66CommunityContentOrigin::Community, SanitizedEntry.Kind).Add(SanitizedEntry);
	}
}

void UT66CommunityContentSubsystem::PersistSave()
{
	if (!SaveData)
	{
		SaveData = NewObject<UT66CommunityContentSaveGame>(this);
	}

	SaveData->DraftEntries = DraftChallenges;
	SaveData->DraftEntries.Append(DraftMods);
	SaveData->CachedCommunityEntries = CommunityChallenges;
	SaveData->CachedCommunityEntries.Append(CommunityMods);
	UGameplayStatics::SaveGameToSlot(SaveData, CommunityContentSaveSlotName, CommunityContentSaveUserIndex);
}

TArray<FT66CommunityContentEntry>& UT66CommunityContentSubsystem::GetMutableBucket(const ET66CommunityContentOrigin Origin, const ET66CommunityContentKind Kind)
{
	switch (Origin)
	{
	case ET66CommunityContentOrigin::Community:
		return Kind == ET66CommunityContentKind::Mod ? CommunityMods : CommunityChallenges;
	case ET66CommunityContentOrigin::Draft:
		return Kind == ET66CommunityContentKind::Mod ? DraftMods : DraftChallenges;
	case ET66CommunityContentOrigin::Official:
	default:
		return Kind == ET66CommunityContentKind::Mod ? OfficialMods : OfficialChallenges;
	}
}

const TArray<FT66CommunityContentEntry>& UT66CommunityContentSubsystem::GetBucket(const ET66CommunityContentOrigin Origin, const ET66CommunityContentKind Kind) const
{
	switch (Origin)
	{
	case ET66CommunityContentOrigin::Community:
		return Kind == ET66CommunityContentKind::Mod ? CommunityMods : CommunityChallenges;
	case ET66CommunityContentOrigin::Draft:
		return Kind == ET66CommunityContentKind::Mod ? DraftMods : DraftChallenges;
	case ET66CommunityContentOrigin::Official:
	default:
		return Kind == ET66CommunityContentKind::Mod ? OfficialMods : OfficialChallenges;
	}
}

bool UT66CommunityContentSubsystem::ResolveActiveEntry(FT66CommunityContentEntry& OutEntry) const
{
	if (FindEntryById(ActiveEntryId, OutEntry))
	{
		return true;
	}

	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	return T66GI && FindEntryById(T66GI->SelectedRunModifierID, OutEntry);
}

FString UT66CommunityContentSubsystem::GetBackendBaseUrl() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	return Backend ? Backend->GetBackendBaseUrl() : FString();
}

FString UT66CommunityContentSubsystem::GetSteamTicketHex() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	return SteamHelper ? SteamHelper->GetTicketHex() : FString();
}

void UT66CommunityContentSubsystem::BroadcastContentChanged()
{
	ContentChanged.Broadcast();
}

void UT66CommunityContentSubsystem::OnCatalogResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	(void)Request;
	bCatalogRefreshInFlight = false;

	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		LastStatusMessage = TEXT("Unable to refresh the Community tab right now.");
		BroadcastContentChanged();
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		LastStatusMessage = TEXT("Community catalog response could not be parsed.");
		BroadcastContentChanged();
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* EntriesArray = nullptr;
	if (!Json->TryGetArrayField(TEXT("entries"), EntriesArray) || !EntriesArray)
	{
		LastStatusMessage = TEXT("Community catalog response was missing entries.");
		BroadcastContentChanged();
		return;
	}

	CommunityChallenges.Reset();
	CommunityMods.Reset();

	for (const TSharedPtr<FJsonValue>& Value : *EntriesArray)
	{
		const TSharedPtr<FJsonObject>* EntryObject = nullptr;
		if (!Value.IsValid() || !Value->TryGetObject(EntryObject) || !EntryObject || !EntryObject->IsValid())
		{
			continue;
		}

		FT66CommunityContentEntry ParsedEntry;
		if (TryParseApiEntry(*EntryObject, ParsedEntry))
		{
			SanitizeEntryForStorage(ParsedEntry);
			GetMutableBucket(ET66CommunityContentOrigin::Community, ParsedEntry.Kind).Add(ParsedEntry);
		}
	}

	PersistSave();
	LastStatusMessage = FString::Printf(TEXT("Community catalog refreshed (%d entries)."), CommunityChallenges.Num() + CommunityMods.Num());
	BroadcastContentChanged();
}

void UT66CommunityContentSubsystem::OnSubmitResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, const FName DraftId)
{
	(void)Request;
	bSubmitInFlight = false;

	FT66CommunityContentEntry Draft;
	if (!FindEntryById(DraftId, Draft))
	{
		LastStatusMessage = TEXT("Submission finished, but the local draft could no longer be found.");
		BroadcastContentChanged();
		return;
	}

	if (!bConnectedSuccessfully || !Response.IsValid() || (Response->GetResponseCode() != 200 && Response->GetResponseCode() != 201))
	{
		Draft.SubmissionStatus = TEXT("Submission failed");
		SaveDraft(Draft);
		LastStatusMessage = TEXT("Community submission failed.");
		BroadcastContentChanged();
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		Draft.SubmissionStatus = TEXT("Submitted");
		SaveDraft(Draft);
		LastStatusMessage = TEXT("Submitted for approval.");
		BroadcastContentChanged();
		return;
	}

	Json->TryGetStringField(TEXT("id"), Draft.BackendId);
	FString Status;
	Json->TryGetStringField(TEXT("status"), Status);
	Draft.SubmissionStatus = Status.IsEmpty() ? TEXT("Pending approval") : Status;
	Draft.ModerationStatus = TEXT("pending");
	SaveDraft(Draft);

	LastStatusMessage = FString::Printf(TEXT("%s submitted for approval."), *Draft.Title);
	RefreshCommunityCatalog(true);
	RefreshMySubmissionStates(true);
	BroadcastContentChanged();
}

void UT66CommunityContentSubsystem::OnMySubmissionsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	(void)Request;
	bMySubmissionsRefreshInFlight = false;

	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* EntriesArray = nullptr;
	if (!Json->TryGetArrayField(TEXT("entries"), EntriesArray) || !EntriesArray)
	{
		return;
	}

	bool bDraftsChanged = false;
	for (const TSharedPtr<FJsonValue>& Value : *EntriesArray)
	{
		const TSharedPtr<FJsonObject>* EntryObject = nullptr;
		if (!Value.IsValid() || !Value->TryGetObject(EntryObject) || !EntryObject || !EntryObject->IsValid())
		{
			continue;
		}

		FString BackendId;
		if (!(*EntryObject)->TryGetStringField(TEXT("id"), BackendId) || BackendId.IsEmpty())
		{
			continue;
		}

		FString Status;
		(*EntryObject)->TryGetStringField(TEXT("status"), Status);
		FString ReviewNote;
		(*EntryObject)->TryGetStringField(TEXT("review_note"), ReviewNote);
		const int32 ApprovedReward = (*EntryObject)->HasField(TEXT("approved_reward_chad_coupons"))
			? static_cast<int32>((*EntryObject)->GetNumberField(TEXT("approved_reward_chad_coupons")))
			: 0;

		for (TArray<FT66CommunityContentEntry>* Bucket : { &DraftChallenges, &DraftMods })
		{
			for (FT66CommunityContentEntry& Draft : *Bucket)
			{
				if (Draft.BackendId != BackendId)
				{
					continue;
				}

				Draft.ModerationStatus = Status;
				Draft.SubmissionStatus = ToFriendlySubmissionStatus(Status);
				Draft.ReviewNote = ReviewNote;
				Draft.ApprovedRewardChadCoupons = ApprovedReward;
				Draft.UpdatedAt = FDateTime::UtcNow();
				SanitizeEntryForStorage(Draft);
				bDraftsChanged = true;
			}
		}
	}

	if (bDraftsChanged)
	{
		PersistSave();
		BroadcastContentChanged();
	}
}

FString UT66CommunityContentSubsystem::KindToApiString(const ET66CommunityContentKind Kind)
{
	return Kind == ET66CommunityContentKind::Mod ? TEXT("mod") : TEXT("challenge");
}

bool UT66CommunityContentSubsystem::TryParseApiEntry(const TSharedPtr<FJsonObject>& Json, FT66CommunityContentEntry& OutEntry)
{
	if (!Json.IsValid())
	{
		return false;
	}

	FString Id;
	FString Kind;
	FString Title;
	FString Description;
	if (!Json->TryGetStringField(TEXT("id"), Id)
		|| !Json->TryGetStringField(TEXT("kind"), Kind)
		|| !Json->TryGetStringField(TEXT("title"), Title)
		|| !Json->TryGetStringField(TEXT("description"), Description))
	{
		return false;
	}

	OutEntry = FT66CommunityContentEntry{};
	OutEntry.BackendId = Id;
	OutEntry.LocalId = FName(*FString::Printf(TEXT("Community_%s"), *Id));
	OutEntry.Kind = Kind.Equals(TEXT("mod"), ESearchCase::IgnoreCase) ? ET66CommunityContentKind::Mod : ET66CommunityContentKind::Challenge;
	OutEntry.Origin = ET66CommunityContentOrigin::Community;
	OutEntry.Title = Title;
	OutEntry.Description = Description;
	Json->TryGetStringField(TEXT("author_display_name"), OutEntry.AuthorDisplayName);
	Json->TryGetStringField(TEXT("author_avatar_url"), OutEntry.AuthorAvatarUrl);
	Json->TryGetStringField(TEXT("status"), OutEntry.ModerationStatus);
	OutEntry.SuggestedRewardChadCoupons = Json->HasField(TEXT("suggested_reward_chad_coupons"))
		? static_cast<int32>(Json->GetNumberField(TEXT("suggested_reward_chad_coupons")))
		: 0;
	if (Json->HasField(TEXT("approved_reward_chad_coupons")))
	{
		OutEntry.ApprovedRewardChadCoupons = static_cast<int32>(Json->GetNumberField(TEXT("approved_reward_chad_coupons")));
	}
	Json->TryGetStringField(TEXT("review_note"), OutEntry.ReviewNote);

	const TSharedPtr<FJsonObject>* RulesObject = nullptr;
	if (Json->TryGetObjectField(TEXT("rules"), RulesObject) && RulesObject && (*RulesObject).IsValid())
	{
		const TSharedPtr<FJsonObject>& Rules = *RulesObject;
		OutEntry.Rules.StartLevelOverride = Rules->HasField(TEXT("start_level_override")) ? static_cast<int32>(Rules->GetNumberField(TEXT("start_level_override"))) : 0;
		OutEntry.Rules.bSetMaxHeroStats = Rules->HasField(TEXT("set_max_hero_stats")) && Rules->GetBoolField(TEXT("set_max_hero_stats"));
		FString StartingItemId;
		if (Rules->TryGetStringField(TEXT("starting_item_id"), StartingItemId) && !StartingItemId.IsEmpty())
		{
			OutEntry.Rules.StartingItemId = FName(*StartingItemId);
		}

		FString PassiveOverride;
		if (Rules->TryGetStringField(TEXT("passive_override"), PassiveOverride))
		{
			OutEntry.Rules.PassiveOverride = PassiveFromApiString(PassiveOverride);
		}

		FString UltimateOverride;
		if (Rules->TryGetStringField(TEXT("ultimate_override"), UltimateOverride))
		{
			OutEntry.Rules.UltimateOverride = UltimateFromApiString(UltimateOverride);
		}

		OutEntry.Rules.bRequireFullClear = !Rules->HasField(TEXT("require_full_clear")) || Rules->GetBoolField(TEXT("require_full_clear"));
		OutEntry.Rules.bRequireNoDamage = Rules->HasField(TEXT("require_no_damage")) && Rules->GetBoolField(TEXT("require_no_damage"));
		OutEntry.Rules.RequiredStageReached = Rules->HasField(TEXT("required_stage_reached")) ? static_cast<int32>(Rules->GetNumberField(TEXT("required_stage_reached"))) : 0;
		OutEntry.Rules.MaxRunTimeSeconds = Rules->HasField(TEXT("max_run_time_seconds")) ? static_cast<int32>(Rules->GetNumberField(TEXT("max_run_time_seconds"))) : 0;

		const TSharedPtr<FJsonObject>* BonusStats = nullptr;
		if (Rules->TryGetObjectField(TEXT("bonus_stats"), BonusStats) && BonusStats && (*BonusStats).IsValid())
		{
			const TSharedPtr<FJsonObject>& BonusStatsObject = *BonusStats;
			OutEntry.Rules.BonusStats.Damage = BonusStatsObject->HasField(TEXT("damage")) ? static_cast<int32>(BonusStatsObject->GetNumberField(TEXT("damage"))) : 0;
			OutEntry.Rules.BonusStats.AttackSpeed = BonusStatsObject->HasField(TEXT("attack_speed")) ? static_cast<int32>(BonusStatsObject->GetNumberField(TEXT("attack_speed"))) : 0;
			OutEntry.Rules.BonusStats.AttackScale = BonusStatsObject->HasField(TEXT("attack_scale")) ? static_cast<int32>(BonusStatsObject->GetNumberField(TEXT("attack_scale"))) : 0;
			OutEntry.Rules.BonusStats.Accuracy = BonusStatsObject->HasField(TEXT("accuracy")) ? static_cast<int32>(BonusStatsObject->GetNumberField(TEXT("accuracy"))) : 0;
			OutEntry.Rules.BonusStats.Armor = BonusStatsObject->HasField(TEXT("armor")) ? static_cast<int32>(BonusStatsObject->GetNumberField(TEXT("armor"))) : 0;
			OutEntry.Rules.BonusStats.Evasion = BonusStatsObject->HasField(TEXT("evasion")) ? static_cast<int32>(BonusStatsObject->GetNumberField(TEXT("evasion"))) : 0;
			OutEntry.Rules.BonusStats.Luck = BonusStatsObject->HasField(TEXT("luck")) ? static_cast<int32>(BonusStatsObject->GetNumberField(TEXT("luck"))) : 0;
			OutEntry.Rules.BonusStats.Speed = BonusStatsObject->HasField(TEXT("speed")) ? static_cast<int32>(BonusStatsObject->GetNumberField(TEXT("speed"))) : 0;
		}
	}

	OutEntry.Sanitize();
	return true;
}
