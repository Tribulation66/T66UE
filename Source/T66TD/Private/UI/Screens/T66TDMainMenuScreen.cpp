// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TDMainMenuScreen.h"

#include "Core/T66BackendSubsystem.h"
#include "Core/T66TDDataSubsystem.h"
#include "Core/T66TDFrontendStateSubsystem.h"
#include "Misc/Paths.h"
#include "Save/T66TDProfileSaveGame.h"
#include "Save/T66TDRunSaveGame.h"
#include "Save/T66TDSaveSubsystem.h"
#include "Styling/CoreStyle.h"
#include "UI/Components/T66MinigameMenuLayout.h"
#include "UI/T66TDUIStyle.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SInvalidationPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D TDBgSize(1920.f, 1080.f);

	void EnsureBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	void SetupLooseBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* RelativePath,
		const FVector2D& ImageSize,
		const bool bGenerateMips,
		const TCHAR* DebugLabel)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}
		EnsureBrush(Brush, ImageSize);

		if (!TextureHandle.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				UTexture2D* Texture = bGenerateMips
					? T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, DebugLabel)
					: T66RuntimeUITextureAccess::ImportFileTexture(CandidatePath, TextureFilter::TF_Trilinear, false, DebugLabel);
				if (Texture)
				{
					TextureHandle.Reset(Texture);
					break;
				}
			}
		}

		Brush->SetResourceObject(TextureHandle.Get());
	}

	TSharedRef<SWidget> MakeOptionalImageLayer(const TSharedPtr<FSlateBrush>& Brush, const float Opacity)
	{
		if (!Brush.IsValid() || !Brush->GetResourceObject())
		{
			return SNew(SBox);
		}

		return SNew(SScaleBox)
			.Stretch(EStretch::Fill)
			[
				SNew(SImage)
				.Image(Brush.Get())
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
			];
	}
}

UT66TDMainMenuScreen::UT66TDMainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::TDMainMenu;
	bIsModal = false;
}

void UT66TDMainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
}

void UT66TDMainMenuScreen::OnScreenDeactivated_Implementation()
{
	ReleaseRetainedSlateState();
	Super::OnScreenDeactivated_Implementation();
}

void UT66TDMainMenuScreen::NativeDestruct()
{
	ReleaseRetainedSlateState();
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66TDMainMenuScreen::BuildSlateUI()
{
	RequestMenuTextures();
	return SAssignNew(SharedMenuLayout, ST66MinigameMenuLayout)
		.GameID(FName(TEXT("td")))
		.Title(NSLOCTEXT("T66TD.MainMenu", "TDSharedTitle", "CHADPOCALYPSE TOWER DEFENSE"))
		.Subtitle(NSLOCTEXT("T66TD.MainMenu", "TDSharedSubtitle", "Single-player tower defense"))
		.DailyTitle(NSLOCTEXT("T66TD.MainMenu", "TDDailyTitle", "TODAY'S TD CHALLENGE"))
		.DailyBody(NSLOCTEXT("T66TD.MainMenu", "TDDailyBody", "One seeded defense map per day. Choose a difficulty, survive the route, and bank the strongest score on the daily board."))
		.DailyRules(NSLOCTEXT("T66TD.MainMenu", "TDDailyRules", "Single player only.\nDaily score uses today's map seed.\nLeaderboard ranks by final score.\nDifficulty is the only rules toggle."))
		.AccentColor(FLinearColor(0.94f, 0.76f, 0.38f, 1.0f))
		.BackgroundColor(FLinearColor(0.026f, 0.020f, 0.014f, 1.0f))
		.DifficultyOptions(BuildDifficultyOptions())
		.LoadGameEnabled(false)
		.BackendSubsystem(GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr)
		.OnNewGameClicked(FOnClicked::CreateUObject(this, &UT66TDMainMenuScreen::HandleNewGameClicked))
		.OnLoadGameClicked(FOnClicked::CreateUObject(this, &UT66TDMainMenuScreen::HandleLoadGameClicked))
		.OnDailyClicked(FOnClicked::CreateUObject(this, &UT66TDMainMenuScreen::HandleDailyClicked))
		.OnBackClicked(FOnClicked::CreateUObject(this, &UT66TDMainMenuScreen::HandleBackToMainMenuClicked))
		.OnBuildDailyEntries(FT66OnBuildMinigameLeaderboardEntries::CreateUObject(this, &UT66TDMainMenuScreen::BuildDailyLeaderboardEntries))
		.OnBuildAllTimeEntries(FT66OnBuildMinigameLeaderboardEntries::CreateUObject(this, &UT66TDMainMenuScreen::BuildAllTimeLeaderboardEntries))
		.OnGetDailyStatus(FT66OnGetMinigameLeaderboardStatus::CreateUObject(this, &UT66TDMainMenuScreen::GetDailyLeaderboardStatus))
		.OnGetAllTimeStatus(FT66OnGetMinigameLeaderboardStatus::CreateUObject(this, &UT66TDMainMenuScreen::GetAllTimeLeaderboardStatus));
}

FReply UT66TDMainMenuScreen::HandleBackToMainMenuClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

TArray<FT66MinigameDifficultyOption> UT66TDMainMenuScreen::BuildDifficultyOptions() const
{
	TArray<FT66MinigameDifficultyOption> Options;
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66TDDataSubsystem* TDDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDDataSubsystem>() : nullptr;
	if (TDDataSubsystem)
	{
		for (const FT66TDDifficultyDefinition& Difficulty : TDDataSubsystem->GetDifficulties())
		{
			FT66MinigameDifficultyOption& Option = Options.AddDefaulted_GetRef();
			Option.DifficultyID = Difficulty.DifficultyID;
			Option.DisplayName = FText::FromString(Difficulty.DisplayName.IsEmpty() ? Difficulty.DifficultyID.ToString() : Difficulty.DisplayName);
		}
	}
	return Options;
}

TArray<FT66MinigameLeaderboardEntry> UT66TDMainMenuScreen::BuildDailyLeaderboardEntries(FName DifficultyID) const
{
	return {};
}

TArray<FT66MinigameLeaderboardEntry> UT66TDMainMenuScreen::BuildAllTimeLeaderboardEntries(FName DifficultyID) const
{
	TArray<FT66MinigameLeaderboardEntry> Entries;
	const UT66TDSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDSaveSubsystem>() : nullptr;
	const UT66TDProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	if (ProfileSave && ProfileSave->BestScore > 0)
	{
		FT66MinigameLeaderboardEntry Entry;
		Entry.Rank = 1;
		Entry.DisplayName = TEXT("You");
		Entry.Score = ProfileSave->BestScore;
		Entry.bIsLocalPlayer = true;
		Entries.Add(MoveTemp(Entry));
	}
	return Entries;
}

FText UT66TDMainMenuScreen::GetDailyLeaderboardStatus(FName DifficultyID) const
{
	return NSLOCTEXT("T66TD.MainMenu", "TDDailyStatus", "Daily challenge leaderboard is ready for backend entries.");
}

FText UT66TDMainMenuScreen::GetAllTimeLeaderboardStatus(FName DifficultyID) const
{
	return NSLOCTEXT("T66TD.MainMenu", "TDAllTimeStatus", "Clear a TD map to seed this all-time board.");
}

FReply UT66TDMainMenuScreen::HandleNewGameClicked()
{
	if (UT66TDFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginNewRun();
	}

	NavigateTo(ET66ScreenType::TDDifficultySelect);
	return FReply::Handled();
}

FReply UT66TDMainMenuScreen::HandleLoadGameClicked()
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66TDSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDSaveSubsystem>() : nullptr;
	UT66TDFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr;
	const UT66TDRunSaveGame* RunSave = SaveSubsystem ? SaveSubsystem->LoadRun() : nullptr;
	if (!RunSave || !FrontendState)
	{
		return HandleNewGameClicked();
	}

	FrontendState->BeginNewRun();
	FrontendState->SelectDifficulty(RunSave->DifficultyID);
	FrontendState->SelectMap(RunSave->MapID);
	FrontendState->SelectStage(RunSave->StageID);
	NavigateTo(ET66ScreenType::TDBattle);
	return FReply::Handled();
}

FReply UT66TDMainMenuScreen::HandleDailyClicked()
{
	const FName DifficultyID = SharedMenuLayout.IsValid() ? SharedMenuLayout->GetSelectedDifficultyID() : FName(TEXT("Easy"));
	const FString DifficultyToken = DifficultyID.ToString().ToLower();
	const FString DateKey = FDateTime::UtcNow().ToString(TEXT("%Y%m%d"));
	FString ChallengeId = FString::Printf(TEXT("td-%s-%s"), *DateKey, *DifficultyToken);
	int32 DailySeed = static_cast<int32>(GetTypeHash(ChallengeId));

	if (UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		FT66MinigameDailyChallengeData Challenge;
		const FString ChallengeKey = UT66BackendSubsystem::MakeMinigameDailyChallengeCacheKey(TEXT("td"), DifficultyToken);
		if (Backend->GetCachedMinigameDailyChallenge(ChallengeKey, Challenge))
		{
			ChallengeId = Challenge.ChallengeId;
			DailySeed = Challenge.RunSeed;
		}
		else if (Backend->IsBackendConfigured())
		{
			Backend->FetchCurrentMinigameDailyChallenge(TEXT("td"), DifficultyToken);
		}
	}

	if (UT66TDFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginDailyRun(DifficultyID, ChallengeId, DailySeed);
	}

	NavigateTo(ET66ScreenType::TDDifficultySelect);
	return FReply::Handled();
}

FReply UT66TDMainMenuScreen::HandleMapBrowserClicked()
{
	return HandleNewGameClicked();
}

void UT66TDMainMenuScreen::RequestMenuTextures()
{
	SetupLooseBrush(BackdropBrush, BackdropTexture, TEXT("SourceAssets/TD/UI/td_main_menu/Scene/scene_plate.png"), TDBgSize, true, TEXT("TDMenuBackdrop"));
	SetupLooseBrush(ForegroundBrush, ForegroundTexture, TEXT("SourceAssets/TD/Maps/Backgrounds/TD_Menu_Backdrop_01.png"), TDBgSize, true, TEXT("TDMenuForeground"));
	PrimaryCTAFillBrush.Reset();
	PrimaryCTAFillTexture.Reset();
}

void UT66TDMainMenuScreen::ReleaseRetainedSlateState()
{
	SharedMenuLayout.Reset();
	BackdropBrush.Reset();
	BackdropTexture.Reset();
	ForegroundBrush.Reset();
	ForegroundTexture.Reset();
	PrimaryCTAFillBrush.Reset();
	PrimaryCTAFillTexture.Reset();
}
