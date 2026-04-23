// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66DailyClimbScreen.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UI/ST66AnimatedBackground.h"
#include "UI/ST66AnimatedBorderGlow.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D T66DailyClimbBackgroundImageSize(1920.f, 1080.f);

	FLinearColor DailyShellFill()
	{
		return FLinearColor(0.01f, 0.01f, 0.02f, 0.97f);
	}

	FLinearColor DailyInsetFill()
	{
		return FLinearColor(0.08f, 0.10f, 0.12f, 0.98f);
	}

	FLinearColor DailyHighlightFill()
	{
		return FLinearColor(0.16f, 0.28f, 0.20f, 0.97f);
	}

	FLinearColor DailyMutedText()
	{
		return FLinearColor(0.70f, 0.76f, 0.80f, 1.0f);
	}

	void EnsureDailyClimbRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	void SetupDailyClimbLayerBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		const TCHAR* ObjectPath,
		const TCHAR* PackagePath,
		FName RequestKey)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
			EnsureDailyClimbRuntimeImageBrush(Brush, T66DailyClimbBackgroundImageSize);
		}
		else
		{
			EnsureDailyClimbRuntimeImageBrush(Brush, T66DailyClimbBackgroundImageSize);
		}

		if (!TextureHandle.IsValid())
		{
			if (UTexture2D* AssetTexture = T66RuntimeUITextureAccess::LoadAssetTexture(
				ObjectPath,
				TextureFilter::TF_Trilinear,
				TEXT("DailyClimbBackgroundLayer")))
			{
				TextureHandle.Reset(AssetTexture);
			}
		}

		if (TextureHandle.IsValid())
		{
			Brush->SetResourceObject(TextureHandle.Get());
			Brush->ImageSize = FVector2D(
				FMath::Max(1, TextureHandle->GetSizeX()),
				FMath::Max(1, TextureHandle->GetSizeY()));
		}
		else
		{
			Brush->SetResourceObject(nullptr);
			Brush->ImageSize = T66DailyClimbBackgroundImageSize;
		}

		if (!TexPool || !FPackageName::DoesPackageExist(PackagePath) || TextureHandle.IsValid())
		{
			return;
		}

		const TSoftObjectPtr<UTexture2D> Soft{ FSoftObjectPath(ObjectPath) };
		if (UTexture2D* LoadedTexture = TexPool->GetLoadedTexture(Soft))
		{
			Brush->SetResourceObject(LoadedTexture);
			Brush->ImageSize = FVector2D(
				FMath::Max(1, LoadedTexture->GetSizeX()),
				FMath::Max(1, LoadedTexture->GetSizeY()));
			return;
		}

		T66SlateTexture::BindSharedBrushAsync(TexPool, Soft, Requester, Brush, RequestKey, false);
	}

	void SetupDailyClimbRuntimeImageBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* AssetPath,
		const TCHAR* StagedRelativePath,
		const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}

		EnsureDailyClimbRuntimeImageBrush(Brush, ImageSize);

		if (!TextureHandle.IsValid())
		{
			if (AssetPath && *AssetPath)
			{
				if (UTexture2D* AssetTexture = T66RuntimeUITextureAccess::LoadAssetTexture(
					AssetPath,
					TextureFilter::TF_Trilinear,
					TEXT("DailyClimbRuntimeImage")))
				{
					TextureHandle.Reset(AssetTexture);
				}
			}

			if (!TextureHandle.IsValid() && StagedRelativePath && *StagedRelativePath)
			{
				for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(StagedRelativePath))
				{
					if (!FPaths::FileExists(CandidatePath))
					{
						continue;
					}

					if (UTexture2D* FileTexture = T66RuntimeUITextureAccess::ImportFileTexture(
						CandidatePath,
						TextureFilter::TF_Trilinear,
						false,
						TEXT("DailyClimbRuntimeImage")))
					{
						TextureHandle.Reset(FileTexture);
						break;
					}
				}
			}
		}

		if (!TextureHandle.IsValid())
		{
			Brush->SetResourceObject(nullptr);
			return;
		}

		Brush->SetResourceObject(TextureHandle.Get());
		Brush->ImageSize = FVector2D(
			FMath::Max(1, TextureHandle->GetSizeX()),
			FMath::Max(1, TextureHandle->GetSizeY()));
	}

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
	ScreenType = ET66ScreenType::DailyClimb;
	bIsModal = false;
}

void UT66DailyClimbScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	ContinueSaveSlotIndex = INDEX_NONE;
	bStartRequestInFlight = false;
	RefreshContinueAvailability();

	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
		LeaderboardPanel->RefreshLeaderboard();
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnDailyClimbChallengeReady.RemoveAll(this);
			Backend->OnDailyClimbChallengeReady.AddUObject(this, &UT66DailyClimbScreen::HandleDailyClimbStatusReady);
			Backend->FetchCurrentDailyClimb();
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

	T66GI->SelectedHeroID = Loaded->HeroID;
	T66GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	T66GI->SelectedCompanionID = Loaded->CompanionID;
	T66GI->SelectedDifficulty = Loaded->Difficulty;
	T66GI->SelectedPartySize = ET66PartySize::Solo;
	T66GI->RunSeed = Loaded->RunSeed;
	T66GI->CachedDailyClimbChallenge = Loaded->DailyClimbChallenge;
	T66GI->ActiveDailyClimbChallenge = Loaded->DailyClimbChallenge;
	T66GI->bIsDailyClimbRunActive = true;
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

void UT66DailyClimbScreen::RequestBackgroundTexture()
{
	UGameInstance* GI = GetGameInstance();
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	SetupDailyClimbLayerBrush(
		SkyBackgroundBrush,
		SkyBackgroundTexture,
		TexPool,
		this,
		TEXT("/Game/UI/MainMenu/sky_bg.sky_bg"),
		TEXT("/Game/UI/MainMenu/sky_bg"),
		FName(TEXT("DailyClimbSkyBg")));

	SetupDailyClimbLayerBrush(
		FireMoonBrush,
		FireMoonTexture,
		TexPool,
		this,
		TEXT("/Game/UI/MainMenu/fire_moon.fire_moon"),
		TEXT("/Game/UI/MainMenu/fire_moon"),
		FName(TEXT("DailyClimbFireMoon")));

	SetupDailyClimbLayerBrush(
		PyramidChadBrush,
		PyramidChadTexture,
		TexPool,
		this,
		TEXT("/Game/UI/MainMenu/pyramid_chad.pyramid_chad"),
		TEXT("/Game/UI/MainMenu/pyramid_chad"),
		FName(TEXT("DailyClimbPyramidChad")));

	SetupDailyClimbRuntimeImageBrush(
		PrimaryCTAFillBrush,
		PrimaryCTAFillTexture,
		nullptr,
		TEXT("RuntimeDependencies/T66/UI/MiniMainMenu/mini_mainmenu_cta_fill_green.png"),
		FVector2D(1024.f, 232.f));
}

TSharedRef<SWidget> UT66DailyClimbScreen::BuildSlateUI()
{
	RequestBackgroundTexture();

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;

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
	const int32 SeedPreview = bHasChallenge ? ComputeSeedQualityPreview(Challenge->RunSeed) : 0;

	FString HeroName = TEXT("Unknown Hero");
	if (bHasChallenge && T66GI)
	{
		FHeroData HeroData;
		if (T66GI->GetHeroData(Challenge->HeroID, HeroData))
		{
			HeroName = HeroData.DisplayName.ToString();
		}
		else
		{
			HeroName = Challenge->HeroID.ToString();
		}
	}

	const FText StatusText =
		!Backend ? NSLOCTEXT("T66.DailyClimb", "StatusOffline", "Backend unavailable.") :
		(!bHasChallenge
			? FText::FromString(Backend->GetLastDailyClimbMessage().IsEmpty() ? TEXT("Loading Daily Challenge...") : Backend->GetLastDailyClimbMessage())
			: (bCanContinueChallenge
				? NSLOCTEXT("T66.DailyClimb", "ResumeReady", "Your Daily Challenge run is in progress. Continue to resume it.")
				: (bAttemptConsumed
					? NSLOCTEXT("T66.DailyClimb", "AttemptUsed", "Today's Daily Challenge is already completed.")
					: (bAttemptStarted
						? NSLOCTEXT("T66.DailyClimb", "AttemptStarted", "Your Daily attempt is already active. Resume from your saved Daily run.")
						: NSLOCTEXT("T66.DailyClimb", "Ready", "Everyone gets the same seed, hero, and rules. One attempt only.")))));

	const FText StartButtonText =
		bAttemptConsumed
			? NSLOCTEXT("T66.DailyClimb", "AttemptUsedButton", "ATTEMPT USED")
			: (bAttemptStarted
				? NSLOCTEXT("T66.DailyClimb", "AttemptLockedButton", "RUN IN PROGRESS")
				: (bStartRequestInFlight
					? NSLOCTEXT("T66.DailyClimb", "Starting", "STARTING...")
					: NSLOCTEXT("T66.DailyClimb", "StartChallenge", "START CHALLENGE")));

	const FText ContinueButtonText = NSLOCTEXT("T66.DailyClimb", "ContinueChallenge", "CONTINUE CHALLENGE");

	TSharedRef<SVerticalBox> RulesBox = SNew(SVerticalBox);
	if (bHasChallenge && Challenge->Rules.Num() > 0)
	{
		for (const FT66DailyClimbRule& Rule : Challenge->Rules)
		{
			RulesBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(DailyInsetFill())
				.Padding(12.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Rule.Label))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Rule.Description))
						.Font(FT66Style::Tokens::FontRegular(15))
						.ColorAndOpacity(DailyMutedText())
						.AutoWrapText(true)
					]
				]
			];
		}
	}
	else
	{
		RulesBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.DailyClimb", "NoRules", "No special rules are published for this Daily yet."))
			.Font(FT66Style::Tokens::FontRegular(15))
			.ColorAndOpacity(DailyMutedText())
			.AutoWrapText(true)
		];
	}

	auto MakeInfoCard = [](const FText& Label, const FText& Value) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(DailyInsetFill())
			.Padding(12.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::Tokens::FontRegular(13))
					.ColorAndOpacity(DailyMutedText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Value)
					.Font(FT66Style::Tokens::FontBold(19))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.AutoWrapText(true)
				]
			];
	};

	auto MakeMenuButton = [this](const FText& Text, FReply (UT66DailyClimbScreen::*ClickFunc)(), bool bEnabled, bool bUseGlow) -> TSharedRef<SWidget>
	{
		FT66ButtonParams ButtonParams(Text, FOnClicked::CreateUObject(this, ClickFunc), ET66ButtonType::Success);
		ButtonParams
			.SetMinWidth(360.f)
			.SetHeight(88.f)
			.SetFontSize(26)
			.SetPadding(FMargin(16.f, 10.f, 16.f, 8.f))
			.SetUseGlow(false)
			.SetUseDotaPlateOverlay(true)
			.SetDotaPlateOverrideBrush(PrimaryCTAFillBrush.Get())
			.SetTextColor(FLinearColor(0.96f, 0.96f, 0.94f, 1.0f))
			.SetStateTextShadowColors(
				FLinearColor(0.f, 0.f, 0.f, 0.36f),
				FLinearColor(0.f, 0.f, 0.f, 0.42f),
				FLinearColor(0.f, 0.f, 0.f, 0.28f))
			.SetTextShadowOffset(FVector2D(0.f, 1.f))
			.SetEnabled(bEnabled);

		TSharedRef<SWidget> ButtonWidget = FT66Style::MakeButton(ButtonParams);
		if (bUseGlow)
		{
			ButtonWidget = SNew(ST66AnimatedBorderGlow)
				.GlowColor(FLinearColor(0.32f, 0.83f, 0.56f, 1.0f))
				.SweepColor(FLinearColor(0.78f, 1.00f, 0.87f, 1.0f))
				.BorderInset(2.0f)
				.InnerThickness(2.0f)
				.MidThickness(5.0f)
				.OuterThickness(9.0f)
				.SweepLengthFraction(0.26f)
				.SweepSpeed(0.12f)
				[
					ButtonWidget
				];
		}

		return SNew(SBox)
			.WidthOverride(360.f)
			[
				ButtonWidget
			];
	};

	TSharedRef<SWidget> DailyLeaderboardWidget =
		SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
		.LocalizationSubsystem(Loc)
		.LeaderboardSubsystem(LB)
		.UIManager(UIManager)
		.DailyChallengeMode(true);

	TArray<FT66AnimatedBackgroundLayer> BackgroundLayers;
	BackgroundLayers.Reserve(3);

	FT66AnimatedBackgroundLayer& SkyLayer = BackgroundLayers.AddDefaulted_GetRef();
	SkyLayer.Brush = SkyBackgroundBrush.Get();
	SkyLayer.SwayAmplitude = FVector2D(15.f, 5.f);
	SkyLayer.SwayFrequency = 0.15f;
	SkyLayer.OpacityMin = 1.f;
	SkyLayer.OpacityMax = 1.f;
	SkyLayer.PhaseOffset = 0.00f;

	FT66AnimatedBackgroundLayer& FireMoonLayer = BackgroundLayers.AddDefaulted_GetRef();
	FireMoonLayer.Brush = FireMoonBrush.Get();
	FireMoonLayer.BaseOffset = FVector2D(0.f, 90.f);
	FireMoonLayer.SwayAmplitude = FVector2D(3.f, 2.f);
	FireMoonLayer.SwayFrequency = 0.10f;
	FireMoonLayer.ScalePulseAmplitude = 0.05f;
	FireMoonLayer.ScalePulseFrequency = 0.30f;
	FireMoonLayer.OpacityMin = 0.70f;
	FireMoonLayer.OpacityMax = 1.00f;
	FireMoonLayer.OpacityFrequency = 0.40f;
	FireMoonLayer.PhaseOffset = 0.21f;

	FT66AnimatedBackgroundLayer& PyramidLayer = BackgroundLayers.AddDefaulted_GetRef();
	PyramidLayer.Brush = PyramidChadBrush.Get();
	PyramidLayer.BaseOffset = FVector2D(0.f, 126.f);
	PyramidLayer.SwayAmplitude = FVector2D(2.f, 1.f);
	PyramidLayer.SwayFrequency = 0.08f;
	PyramidLayer.OpacityMin = 1.f;
	PyramidLayer.OpacityMax = 1.f;
	PyramidLayer.PhaseOffset = 0.43f;

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66AnimatedBackground)
				.Layers(BackgroundLayers)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.40f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(20.f, 20.f, 0.f, 0.f))
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.DailyClimb", "Back", "BACK"),
						FOnClicked::CreateUObject(this, &UT66DailyClimbScreen::HandleBackClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(118.f)
					.SetHeight(38.f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.Padding(FMargin(0.f, 48.f, 0.f, 0.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.DailyClimb", "Title", "DAILY CHALLENGE"))
					.Font(FT66Style::MakeFont(TEXT("Black"), 46))
					.ColorAndOpacity(FLinearColor(0.98f, 0.82f, 0.52f, 0.96f))
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.DailyClimb", "Subtitle", "One seed. One attempt. Same puzzle for everyone."))
					.Font(FT66Style::Tokens::FontBold(17))
					.ColorAndOpacity(FLinearColor(0.95f, 0.86f, 0.70f, 0.96f))
					.Justification(ETextJustify::Center)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(20.f, 140.f, 20.f, 20.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBox)
					.WidthOverride(380.f)
					.HeightOverride(650.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(DailyShellFill())
						.Padding(FMargin(14.f, 14.f, 14.f, 12.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.DailyClimb", "RulesPanelTitle", "TODAY'S RULES"))
								.Font(FT66Style::Tokens::FontHeading())
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(DailyHighlightFill())
								.Padding(12.f)
								[
									SNew(STextBlock)
									.Text(StatusText)
									.Font(FT66Style::Tokens::FontRegular(15))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.AutoWrapText(true)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.f, 0.f, 6.f, 0.f)
								[
									MakeInfoCard(
										NSLOCTEXT("T66.DailyClimb", "HeroLabel", "Hero"),
										FText::FromString(bHasChallenge ? HeroName : TEXT("--")))
								]
								+ SHorizontalBox::Slot().FillWidth(1.0f)
								[
									MakeInfoCard(
										NSLOCTEXT("T66.DailyClimb", "DifficultyLabel", "Difficulty"),
										FText::FromString(bHasChallenge ? DifficultyLabel(Challenge->Difficulty) : TEXT("--")))
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.f, 0.f, 6.f, 0.f)
								[
									MakeInfoCard(
										NSLOCTEXT("T66.DailyClimb", "SeedQualityLabel", "Seed Quality"),
										FText::FromString(bHasChallenge ? FString::Printf(TEXT("%d / 100"), SeedPreview) : TEXT("--")))
								]
								+ SHorizontalBox::Slot().FillWidth(1.0f)
								[
									MakeInfoCard(
										NSLOCTEXT("T66.DailyClimb", "RewardLabel", "Reward"),
										FText::FromString(bHasChallenge ? FString::Printf(TEXT("%d Chad Coupons"), Challenge->CouponReward) : TEXT("--")))
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 8.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.DailyClimb", "RulesHeader", "Rule Stack"))
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot().FillHeight(1.0f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									RulesBox
								]
							]
						]
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(28.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(392.f)
					.HeightOverride(300.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.02f, 0.98f))
						.Padding(FMargin(18.f, 18.f, 18.f, 16.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								MakeMenuButton(StartButtonText, &UT66DailyClimbScreen::HandleStartClicked, bCanStartChallenge, true)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 16.f, 0.f, 0.f)
							[
								MakeMenuButton(ContinueButtonText, &UT66DailyClimbScreen::HandleContinueClicked, bCanContinueChallenge, false)
							]
							+ SVerticalBox::Slot().FillHeight(1.0f).VAlign(VAlign_Bottom).Padding(0.f, 16.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.DailyClimb", "ButtonNote", "Daily Climb is always Solo and scored on a separate global leaderboard."))
								.Font(FT66Style::Tokens::FontRegular(14))
								.ColorAndOpacity(DailyMutedText())
								.AutoWrapText(true)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBox)
					.WidthOverride(380.f)
					.HeightOverride(650.f)
					[
						DailyLeaderboardWidget
					]
				]
			]
		];
}
