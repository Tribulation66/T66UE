// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66DailyClimbScreen.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "UI/Style/T66FlatStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FString DifficultyLabel(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Medium: return TEXT("Medium");
		case ET66Difficulty::Hard: return TEXT("Hard");
		case ET66Difficulty::VeryHard: return TEXT("Very Hard");
		case ET66Difficulty::Impossible: return TEXT("Impossible");
		default: return TEXT("Easy");
		}
	}

	FText SeedQualityLabel(const int32 SeedQuality)
	{
		const int32 ClampedQuality = FMath::Clamp(SeedQuality, 0, 100);
		if (ClampedQuality >= 90) return NSLOCTEXT("T66.DailyClimb", "SeedQualityGodly", "Godly");
		if (ClampedQuality >= 75) return NSLOCTEXT("T66.DailyClimb", "SeedQualityLucky", "Lucky");
		if (ClampedQuality >= 60) return NSLOCTEXT("T66.DailyClimb", "SeedQualityFavorable", "Favorable");
		if (ClampedQuality >= 40) return NSLOCTEXT("T66.DailyClimb", "SeedQualityFair", "Fair");
		if (ClampedQuality >= 20) return NSLOCTEXT("T66.DailyClimb", "SeedQualityBadLuck", "Bad Luck");
		return NSLOCTEXT("T66.DailyClimb", "SeedQualityCursed", "Cursed");
	}

	FDateTime ParseDailySaveTimestamp(const FString& Timestamp)
	{
		FDateTime Parsed = FDateTime::MinValue();
		if (!Timestamp.IsEmpty())
		{
			FDateTime::ParseIso8601(*Timestamp, Parsed);
		}
		return Parsed;
	}
}

UT66DailyClimbScreen::UT66DailyClimbScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::DailyDescent;
	bIsModal = false;
}

void UT66DailyClimbScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	ContinueSaveSlotIndex = INDEX_NONE;
	bStartRequestInFlight = false;
	RefreshContinueAvailability();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnDailyClimbChallengeReady.RemoveAll(this);
			Backend->OnDailyClimbChallengeReady.AddUObject(this, &UT66DailyClimbScreen::HandleDailyClimbStatusReady);
			Backend->OnLeaderboardDataReady.RemoveAll(this);
			Backend->OnLeaderboardDataReady.AddUObject(this, &UT66DailyClimbScreen::HandleDailyLeaderboardReady);
			Backend->FetchCurrentDailyClimb();
			Backend->FetchDailyLeaderboard();
		}
	}
}

void UT66DailyClimbScreen::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnDailyClimbChallengeReady.RemoveAll(this);
			Backend->OnLeaderboardDataReady.RemoveAll(this);
		}
	}

	bStartRequestInFlight = false;
	ContinueSaveSlotIndex = INDEX_NONE;
	Super::OnScreenDeactivated_Implementation();
}

void UT66DailyClimbScreen::HandleDailyClimbStatusReady(const FString& RequestTag)
{
	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!Backend)
	{
		return;
	}

	if (Backend->HasCachedDailyClimbChallenge() && T66GI)
	{
		T66GI->CacheDailyClimbChallenge(Backend->GetCachedDailyClimbChallenge());
	}

	RefreshContinueAvailability();
	Backend->FetchDailyLeaderboard();

	if (RequestTag.Equals(TEXT("start"), ESearchCase::IgnoreCase))
	{
		bStartRequestInFlight = false;
		const FT66DailyClimbChallengeData& Challenge = Backend->GetCachedDailyClimbChallenge();
		if (T66GI && Challenge.IsValid() && Challenge.HasStartedAttempt() && !Challenge.HasCompletedAttempt())
		{
			T66GI->BeginDailyClimbRun(Challenge);
			T66GI->TransitionToGameplayLevel();
			return;
		}
	}

	ForceRebuildSlate();
}

void UT66DailyClimbScreen::HandleDailyLeaderboardReady(const FString& LeaderboardKey)
{
	if (LeaderboardKey.Equals(TEXT("daily_global"), ESearchCase::IgnoreCase))
	{
		ForceRebuildSlate();
	}
}

int32 UT66DailyClimbScreen::ComputeSeedQualityPreview(const int32 RunSeed) const
{
	const int32 SeedBase = (RunSeed != 0) ? RunSeed : 1;
	FRandomStream SeedLuckStream(SeedBase ^ 0x4C55434B);
	return FMath::Clamp(SeedLuckStream.RandRange(0, 100), 0, 100);
}

FReply UT66DailyClimbScreen::HandleBackClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

FReply UT66DailyClimbScreen::HandleContinueClicked()
{
	if (ContinueSaveSlotIndex == INDEX_NONE)
	{
		return FReply::Handled();
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!GI || !T66GI || !SaveSub)
	{
		return FReply::Handled();
	}

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(ContinueSaveSlotIndex);
	if (!Loaded || !Loaded->bIsDailyClimbRun || !Loaded->DailyClimbChallenge.IsValid() || !Loaded->RunSnapshot.bValid)
	{
		RefreshContinueAvailability();
		ForceRebuildSlate();
		return FReply::Handled();
	}

	if (T66GI->CachedDailyClimbChallenge.IsValid()
		&& !Loaded->DailyClimbChallenge.ChallengeId.Equals(T66GI->CachedDailyClimbChallenge.ChallengeId, ESearchCase::CaseSensitive))
	{
		RefreshContinueAvailability();
		ForceRebuildSlate();
		return FReply::Handled();
	}

	T66GI->SelectedHeroID = T66GI->ResolvePlayableHeroID(Loaded->HeroID);
	T66GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	T66GI->SelectedCompanionID = T66GI->ResolvePlayableCompanionID(Loaded->CompanionID);
	T66GI->SelectedDifficulty = T66GI->ResolvePlayableDifficulty(Loaded->Difficulty);
	T66GI->SelectedPartySize = ET66PartySize::Solo;
	T66GI->RunSeed = Loaded->RunSeed;
	T66GI->CachedDailyClimbChallenge = Loaded->DailyClimbChallenge;
	T66GI->ActiveDailyClimbChallenge = Loaded->DailyClimbChallenge;
	T66GI->SelectedRunMode = ET66RunMode::DailyClimb;
	T66GI->SelectedRunCategory = ET66RunCategory::Tower;
	T66GI->SelectedRunModifierKind = ET66RunModifierKind::None;
	T66GI->SelectedRunModifierID = NAME_None;
	T66GI->CurrentMainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;
	T66GI->PendingLoadedTransform = Loaded->PlayerTransform;
	T66GI->bApplyLoadedTransform = true;
	T66GI->PendingLoadedRunSnapshot = Loaded->RunSnapshot;
	T66GI->bApplyLoadedRunSnapshot = Loaded->RunSnapshot.bValid;
	T66GI->CurrentSaveSlotIndex = ContinueSaveSlotIndex;
	T66GI->bRunIneligibleForLeaderboard = Loaded->bRunIneligibleForLeaderboard;
	T66GI->CurrentRunOwnerPlayerId = Loaded->OwnerPlayerId;
	T66GI->CurrentRunOwnerDisplayName = Loaded->OwnerDisplayName;
	T66GI->CurrentRunPartyMemberIds = Loaded->PartyMemberIds;
	T66GI->CurrentRunPartyMemberDisplayNames = Loaded->PartyMemberDisplayNames;
	T66GI->bSaveSlotPreviewMode = false;

	if (UT66RunIntegritySubsystem* Integrity = GI->GetSubsystem<UT66RunIntegritySubsystem>())
	{
		Integrity->RestoreActiveRunContext(Loaded->IntegrityContext);
		Integrity->MarkLoadedSnapshot();
		T66GI->bRunIneligibleForLeaderboard = T66GI->bRunIneligibleForLeaderboard || !Integrity->GetCurrentContext().ShouldAllowRankedSubmission();
	}

	T66GI->PersistRememberedSelectionDefaults();
	T66GI->TransitionToGameplayLevel();
	return FReply::Handled();
}

FReply UT66DailyClimbScreen::HandleStartClicked()
{
	if (bStartRequestInFlight)
	{
		return FReply::Handled();
	}

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	const FT66DailyClimbChallengeData* Challenge = nullptr;
	if (Backend && Backend->HasCachedDailyClimbChallenge())
	{
		Challenge = &Backend->GetCachedDailyClimbChallenge();
	}
	else if (T66GI && T66GI->CachedDailyClimbChallenge.IsValid())
	{
		Challenge = &T66GI->CachedDailyClimbChallenge;
	}

	if (!Backend || !Challenge || !Challenge->IsValid() || Challenge->HasStartedAttempt() || Challenge->HasCompletedAttempt())
	{
		return FReply::Handled();
	}

	bStartRequestInFlight = true;
	Backend->StartDailyClimbAttempt();
	return FReply::Handled();
}

void UT66DailyClimbScreen::RefreshContinueAvailability()
{
	ContinueSaveSlotIndex = INDEX_NONE;

	UGameInstance* GI = GetGameInstance();
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!SaveSub)
	{
		return;
	}

	FString ActiveChallengeId;
	if (Backend && Backend->HasCachedDailyClimbChallenge())
	{
		ActiveChallengeId = Backend->GetCachedDailyClimbChallenge().ChallengeId;
	}
	else if (T66GI && T66GI->CachedDailyClimbChallenge.IsValid())
	{
		ActiveChallengeId = T66GI->CachedDailyClimbChallenge.ChallengeId;
	}

	FDateTime BestTimestamp = FDateTime::MinValue();
	for (int32 SlotIndex = 0; SlotIndex < UT66SaveSubsystem::MaxSlots; ++SlotIndex)
	{
		if (!SaveSub->DoesSlotExist(SlotIndex))
		{
			continue;
		}

		UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
		if (!Loaded
			|| !Loaded->bIsDailyClimbRun
			|| !Loaded->DailyClimbChallenge.IsValid()
			|| Loaded->DailyClimbChallenge.HasCompletedAttempt()
			|| !Loaded->RunSnapshot.bValid)
		{
			continue;
		}

		if (!ActiveChallengeId.IsEmpty()
			&& !Loaded->DailyClimbChallenge.ChallengeId.Equals(ActiveChallengeId, ESearchCase::CaseSensitive))
		{
			continue;
		}

		const FDateTime SaveTimestamp = ParseDailySaveTimestamp(Loaded->LastPlayedUtc);
		if (ContinueSaveSlotIndex == INDEX_NONE || SaveTimestamp >= BestTimestamp)
		{
			ContinueSaveSlotIndex = SlotIndex;
			BestTimestamp = SaveTimestamp;
		}
	}
}

TSharedRef<SWidget> UT66DailyClimbScreen::BuildSlateUI()
{
	{
		constexpr float CanvasW = 1920.f;
		constexpr float CanvasH = 1080.f;

		UGameInstance* GI = GetGameInstance();
		UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
		const FT66DailyClimbChallengeData* Challenge = nullptr;
		if (Backend && Backend->HasCachedDailyClimbChallenge())
		{
			Challenge = &Backend->GetCachedDailyClimbChallenge();
		}
		else if (T66GI && T66GI->CachedDailyClimbChallenge.IsValid())
		{
			Challenge = &T66GI->CachedDailyClimbChallenge;
		}

		const bool bHasChallenge = Challenge && Challenge->IsValid();
		const bool bAttemptStarted = bHasChallenge && Challenge->HasStartedAttempt();
		const bool bAttemptConsumed = bHasChallenge && Challenge->HasCompletedAttempt();
		const bool bCanContinueChallenge = ContinueSaveSlotIndex != INDEX_NONE;
		const bool bCanStartChallenge = bHasChallenge && !bAttemptStarted && !bAttemptConsumed && !bStartRequestInFlight;

		FString HeroName = TEXT("--");
		if (bHasChallenge && T66GI)
		{
			FHeroData HeroData;
			HeroName = T66GI->GetHeroData(Challenge->HeroID, HeroData)
				? HeroData.DisplayName.ToString()
				: Challenge->HeroID.ToString();
		}
		const int32 SeedQuality = bHasChallenge ? ComputeSeedQualityPreview(Challenge->RunSeed) : 50;
		const FText SeedQualityText = bHasChallenge
			? FText::Format(
				NSLOCTEXT("T66.DailyClimb", "FlatSeedQualityValue", "{0} ({1})"),
				SeedQualityLabel(SeedQuality),
				FText::AsNumber(SeedQuality))
			: FText::FromString(TEXT("--"));

		const FText StatusText =
			!Backend ? NSLOCTEXT("T66.DailyClimb", "FlatStatusOffline", "Backend unavailable.") :
			(!bHasChallenge
				? FText::FromString(Backend->GetLastDailyClimbMessage().IsEmpty() ? TEXT("Loading Daily Descent...") : Backend->GetLastDailyClimbMessage())
				: (bCanContinueChallenge
					? NSLOCTEXT("T66.DailyClimb", "FlatResumeReady", "Daily run in progress. Continue from the saved slot.")
					: (bAttemptConsumed
						? NSLOCTEXT("T66.DailyClimb", "FlatAttemptUsed", "Today's Daily Descent is already completed.")
						: (bAttemptStarted
							? NSLOCTEXT("T66.DailyClimb", "FlatAttemptStarted", "Daily attempt is active. Resume from the saved run.")
							: NSLOCTEXT("T66.DailyClimb", "FlatReady", "Everyone gets the same seed, hero, and rules. One attempt only.")))));

		const FText StartButtonText =
			bAttemptConsumed
				? NSLOCTEXT("T66.DailyClimb", "FlatAttemptUsedButton", "ATTEMPT USED")
				: (bAttemptStarted
					? NSLOCTEXT("T66.DailyClimb", "FlatAttemptLockedButton", "RUN IN PROGRESS")
					: (bStartRequestInFlight
						? NSLOCTEXT("T66.DailyClimb", "FlatStarting", "STARTING...")
						: NSLOCTEXT("T66.DailyClimb", "FlatStartDescent", "START DESCENT")));

		const FText ContinueButtonText = NSLOCTEXT("T66.DailyClimb", "FlatContinueDescent", "CONTINUE DESCENT");

		struct FDailyFlatRuleRow
		{
			FText Name;
			FText Description;
		};
		TArray<FDailyFlatRuleRow> RuleRows;
		if (bHasChallenge && Challenge->Rules.Num() > 0)
		{
			for (const FT66DailyClimbRule& Rule : Challenge->Rules)
			{
				RuleRows.Add({ FText::FromString(Rule.Label), FText::FromString(Rule.Description) });
				if (RuleRows.Num() >= 3)
				{
					break;
				}
			}
		}
		if (RuleRows.Num() == 0)
		{
			RuleRows.Add({
				NSLOCTEXT("T66.DailyClimb", "FlatNoRulesName", "Daily Rules"),
				bHasChallenge
					? NSLOCTEXT("T66.DailyClimb", "FlatNoRulesDescription", "No special rules are published for this Daily yet.")
					: NSLOCTEXT("T66.DailyClimb", "FlatLoadingRulesDescription", "Loading today's challenge rules.")
			});
		}

		const FString DailyLeaderboardKey(TEXT("daily_global"));
		const bool bDailyLeaderboardLoaded = Backend && Backend->HasCachedLeaderboard(DailyLeaderboardKey);
		const TArray<FLeaderboardEntry> DailyLeaderboardEntries =
			bDailyLeaderboardLoaded ? Backend->GetCachedLeaderboard(DailyLeaderboardKey) : TArray<FLeaderboardEntry>();
		FString LocalSteamId;
		if (UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr)
		{
			LocalSteamId = SteamHelper->GetLocalSteamId();
		}
		int32 LocalLeaderboardEntryIndex = INDEX_NONE;
		if (!LocalSteamId.IsEmpty())
		{
			for (int32 EntryIndex = 0; EntryIndex < DailyLeaderboardEntries.Num(); ++EntryIndex)
			{
				if (DailyLeaderboardEntries[EntryIndex].SteamId.Equals(LocalSteamId, ESearchCase::IgnoreCase))
				{
					LocalLeaderboardEntryIndex = EntryIndex;
					break;
				}
			}
		}

		auto DTag = [](const TCHAR* Name) -> FName
		{
			return FName(Name ? Name : TEXT(""));
		};

		auto MakeLabel = [DTag](
			const FName Tag,
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(STextBlock)
				.Text(Text)
				.Font(FontSize >= 20 ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds),
				Tag,
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);
		};

		auto MakePanel = [](const ET66FlatState State, const FName Tag) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatPanel(State, FMargin(0.f), SNew(SBox), nullptr, Tag);
		};

		auto MakeColorRect = [DTag](const FLinearColor& Color, const FName Tag) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Color),
				Tag,
				TEXT("Divider"),
				ET66FlatState::Selected,
				Color,
				false,
				NAME_None,
				false);
		};

		auto MakeInfoRow = [DTag, MakeLabel](const TCHAR* BaseTag, const FText& Label, const FText& Value) -> TSharedRef<SWidget>
		{
			const FString Base(BaseTag ? BaseTag : TEXT(""));
			return FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(12.f, 8.f),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
				[
					MakeLabel(FName(*(Base + TEXT(".Icon"))), FText::FromString(TEXT("i")), 18, FT66FlatStyle::PurpleAccent(), ETextJustify::Center)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					MakeLabel(FName(*(Base + TEXT(".Label"))), Label, 16, FT66FlatStyle::PurpleAccent())
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					MakeLabel(FName(*(Base + TEXT(".Value"))), Value, 18, FT66FlatStyle::PrimaryText(), ETextJustify::Right)
				],
				nullptr,
				FName(*Base));
		};

		auto MakeModifierRow = [DTag, MakeLabel](const TCHAR* BaseTag, const FText& Name, const FText& Description) -> TSharedRef<SWidget>
		{
			const FString Base(BaseTag ? BaseTag : TEXT(""));
			return FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(14.f, 8.f),
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeLabel(FName(*(Base + TEXT(".Name"))), Name, 17, FT66FlatStyle::PrimaryText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					MakeLabel(FName(*(Base + TEXT(".Description"))), Description, 14, FT66FlatStyle::SecondaryText())
				],
				nullptr,
				FName(*Base));
		};

		auto MakeRankRow = [DTag, MakeLabel](const TCHAR* BaseTag, const FText& Rank, const FText& Name, const FText& Score) -> TSharedRef<SWidget>
		{
			const FString Base(BaseTag ? BaseTag : TEXT(""));
			return FT66FlatStyle::AttachMetadata(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.18f).VAlign(VAlign_Center)
				[
					MakeLabel(FName(*(Base + TEXT(".Rank"))), Rank, 18, FT66FlatStyle::PurpleAccent())
				]
				+ SHorizontalBox::Slot().FillWidth(0.58f).VAlign(VAlign_Center)
				[
					MakeLabel(FName(*(Base + TEXT(".Name"))), Name, 18, FT66FlatStyle::PrimaryText())
				]
				+ SHorizontalBox::Slot().FillWidth(0.24f).VAlign(VAlign_Center)
				[
					MakeLabel(FName(*(Base + TEXT(".Score"))), Score, 18, FT66FlatStyle::PrimaryText(), ETextJustify::Right)
				],
				FName(*Base),
				TEXT("RankingRow"),
				ET66FlatState::Default);
		};

		TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
		auto AddN = [Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			Canvas->AddSlot()
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(FMargin(X * CanvasW, Y * CanvasH, W * CanvasW, H * CanvasH))
				[
					Widget
				];
		};

		AddN(0.000f, 0.000f, 1.000f, 1.000f, MakePanel(ET66FlatState::Default, DTag(TEXT("DailyDescent.Background"))));
		AddN(0.013f, 0.153f, 0.279f, 0.774f, MakePanel(ET66FlatState::Selected, DTag(TEXT("DailyDescent.LeftPanel"))));
		AddN(0.075f, 0.183f, 0.144f, 0.043f, MakeLabel(DTag(TEXT("DailyDescent.LeftPanel.Header")), NSLOCTEXT("T66.DailyClimb", "FlatRulesHeader", "RULES OF THE DAY"), 24, FT66FlatStyle::PrimaryText(), ETextJustify::Center));
		AddN(0.022f, 0.235f, 0.261f, 0.091f, MakePanel(ET66FlatState::Default, DTag(TEXT("DailyDescent.LeftPanel.IntroPanel"))));
		AddN(0.032f, 0.254f, 0.027f, 0.054f, MakeLabel(DTag(TEXT("DailyDescent.LeftPanel.IntroIcon")), FText::FromString(TEXT("i")), 22, FT66FlatStyle::PurpleAccent(), ETextJustify::Center));
		AddN(0.073f, 0.252f, 0.172f, 0.056f, MakeLabel(DTag(TEXT("DailyDescent.LeftPanel.IntroText")), StatusText, 14, FT66FlatStyle::SecondaryText()));
		AddN(0.023f, 0.342f, 0.257f, 0.068f, MakeInfoRow(TEXT("DailyDescent.LeftPanel.HeroRow"), NSLOCTEXT("T66.DailyClimb", "FlatHeroLabel", "Hero Selected"), FText::FromString(HeroName)));
		AddN(0.023f, 0.424f, 0.257f, 0.068f, MakeInfoRow(TEXT("DailyDescent.LeftPanel.DifficultyRow"), NSLOCTEXT("T66.DailyClimb", "FlatDifficultyLabel", "Difficulty"), FText::FromString(bHasChallenge ? DifficultyLabel(Challenge->Difficulty) : TEXT("--"))));
		AddN(0.023f, 0.506f, 0.257f, 0.068f, MakeInfoRow(TEXT("DailyDescent.LeftPanel.SeedQualityRow"), NSLOCTEXT("T66.DailyClimb", "FlatSeedQualityLabel", "Seed Quality"), SeedQualityText));
		AddN(0.029f, 0.598f, 0.126f, 0.036f, MakeLabel(DTag(TEXT("DailyDescent.LeftPanel.ModifiersHeader")), NSLOCTEXT("T66.DailyClimb", "FlatModifiersHeader", "MODIFIERS"), 20, FT66FlatStyle::PrimaryText()));
		for (int32 RuleIndex = 0; RuleIndex < RuleRows.Num(); ++RuleIndex)
		{
			const float RuleY = 0.646f + (RuleIndex * 0.092f);
			const float RuleHeight = RuleIndex == 2 ? 0.097f : 0.086f;
			const FString RuleTag = FString::Printf(TEXT("DailyDescent.LeftPanel.RuleRow%02d"), RuleIndex + 1);
			AddN(0.023f, RuleY, 0.257f, RuleHeight, MakeModifierRow(*RuleTag, RuleRows[RuleIndex].Name, RuleRows[RuleIndex].Description));
		}

		AddN(0.305f, 0.130f, 0.365f, 0.465f, MakePanel(ET66FlatState::Default, DTag(TEXT("DailyDescent.CenterArt"))));
		AddN(0.386f, 0.344f, 0.207f, 0.053f, MakeLabel(DTag(TEXT("DailyDescent.Title")), NSLOCTEXT("T66.DailyClimb", "FlatTitle", "DAILY DESCENT"), 36, FT66FlatStyle::PrimaryText(), ETextJustify::Center));
		AddN(0.384f, 0.404f, 0.211f, 0.035f, MakeLabel(DTag(TEXT("DailyDescent.Subtitle")), NSLOCTEXT("T66.DailyClimb", "FlatSubtitle", "One seed. One attempt."), 18, FT66FlatStyle::SecondaryText(), ETextJustify::Center));
		AddN(0.373f, 0.642f, 0.240f, 0.085f, FT66FlatStyle::MakeFlatButton(bCanStartChallenge ? ET66FlatState::Selected : ET66FlatState::Disabled, StartButtonText, FOnClicked::CreateUObject(this, &UT66DailyClimbScreen::HandleStartClicked), nullptr, nullptr, FMargin(14.f, 8.f), 200.f, 72.f, bCanStartChallenge, 22, DTag(TEXT("DailyDescent.StartButton"))));
		AddN(0.373f, 0.756f, 0.240f, 0.085f, FT66FlatStyle::MakeFlatButton(bCanContinueChallenge ? ET66FlatState::Selected : ET66FlatState::Default, ContinueButtonText, FOnClicked::CreateUObject(this, &UT66DailyClimbScreen::HandleContinueClicked), nullptr, nullptr, FMargin(14.f, 8.f), 200.f, 72.f, bCanContinueChallenge, 22, DTag(TEXT("DailyDescent.ContinueButton"))));

		AddN(0.690f, 0.233f, 0.291f, 0.707f, MakePanel(ET66FlatState::Default, DTag(TEXT("DailyDescent.RightLeaderboardPanel"))));
		AddN(0.701f, 0.160f, 0.268f, 0.067f, FT66FlatStyle::MakeFlatButton(ET66FlatState::Selected, NSLOCTEXT("T66.DailyClimb", "FlatLeaderboardTodayTab", "TODAY"), FOnClicked::CreateLambda([]() { return FReply::Handled(); }), nullptr, nullptr, FMargin(8.f), 360.f, 52.f, true, 16, DTag(TEXT("DailyDescent.LeaderboardTabs.TodayButton")), FName(TEXT("DailyLeaderboardTabs"))));
		AddN(0.731f, 0.258f, 0.208f, 0.038f, MakeLabel(DTag(TEXT("DailyDescent.RightLeaderboardPanel.Header")), NSLOCTEXT("T66.DailyClimb", "FlatLeaderboardHeader", "DAILY GLOBAL CHAD RANKINGS"), 22, FT66FlatStyle::PrimaryText(), ETextJustify::Center));
		const int32 VisibleLeaderboardRows = FMath::Min(8, DailyLeaderboardEntries.Num());
		if (!bDailyLeaderboardLoaded || DailyLeaderboardEntries.Num() == 0)
		{
			AddN(
				0.704f,
				0.382f,
				0.264f,
				0.072f,
				MakeLabel(
					DTag(TEXT("DailyDescent.RightLeaderboardPanel.EmptyState")),
					bDailyLeaderboardLoaded
						? NSLOCTEXT("T66.DailyClimb", "FlatLeaderboardEmpty", "No Daily submissions yet.")
						: NSLOCTEXT("T66.DailyClimb", "FlatLeaderboardLoading", "Loading Daily leaderboard..."),
					18,
					FT66FlatStyle::SecondaryText(),
					ETextJustify::Center));
		}
		for (int32 RowIndex = 0; RowIndex < VisibleLeaderboardRows; ++RowIndex)
		{
			const FLeaderboardEntry& Entry = DailyLeaderboardEntries[RowIndex];
			const FString RowTag = FString::Printf(TEXT("DailyDescent.RightLeaderboardPanel.Row%02d"), RowIndex + 1);
			const FString DisplayName = Entry.PlayerName.IsEmpty() ? TEXT("UNKNOWN") : Entry.PlayerName;
			AddN(
				0.704f,
				0.334f + (RowIndex * 0.061f),
				0.264f,
				0.048f,
				MakeRankRow(
					*RowTag,
					FText::FromString(FString::Printf(TEXT("#%d"), Entry.Rank > 0 ? Entry.Rank : RowIndex + 1)),
					FText::FromString(DisplayName),
					FText::AsNumber(Entry.Score)));
		}
		AddN(0.701f, 0.837f, 0.268f, 0.004f, MakeColorRect(FT66FlatStyle::SelectedBorder(), DTag(TEXT("DailyDescent.RightLeaderboardPanel.PlayerSeparator"))));
		if (LocalLeaderboardEntryIndex != INDEX_NONE)
		{
			const FLeaderboardEntry& LocalEntry = DailyLeaderboardEntries[LocalLeaderboardEntryIndex];
			AddN(
				0.704f,
				0.864f,
				0.264f,
				0.062f,
				MakeRankRow(
					TEXT("DailyDescent.RightLeaderboardPanel.PlayerRow"),
					FText::FromString(FString::Printf(TEXT("#%d"), LocalEntry.Rank > 0 ? LocalEntry.Rank : LocalLeaderboardEntryIndex + 1)),
					FText::FromString(LocalEntry.PlayerName.IsEmpty() ? TEXT("YOU") : LocalEntry.PlayerName),
					FText::AsNumber(LocalEntry.Score)));
		}
		else
		{
			AddN(0.704f, 0.864f, 0.264f, 0.062f, MakeRankRow(TEXT("DailyDescent.RightLeaderboardPanel.PlayerRow"), FText::FromString(TEXT("--")), FText::FromString(TEXT("YOU")), FText::FromString(TEXT("UNRANKED"))));
		}

		const TSharedRef<SWidget> ReferenceCanvas =
			SNew(SBox)
			.WidthOverride(CanvasW)
			.HeightOverride(CanvasH)
			[
				Canvas
			];

		return FT66FlatStyle::AttachMetadata(
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::Both)
				[
					ReferenceCanvas
				]
			],
			DTag(TEXT("DailyDescent.Root")),
			TEXT("ScreenRoot"),
			ET66FlatState::Default);
	}
}
