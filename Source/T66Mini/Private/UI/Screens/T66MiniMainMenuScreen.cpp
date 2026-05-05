// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniMainMenuScreen.h"

#include "Core/T66BackendSubsystem.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Save/T66MiniProfileSaveGame.h"
#include "Styling/CoreStyle.h"
#include "UI/Components/T66MinigameMenuLayout.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"

namespace
{
	const FVector2D MiniBgSize(1920.f, 1080.f);

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
		if (TextureHandle.IsValid())
		{
			Brush->ImageSize = FVector2D(
				FMath::Max(1, TextureHandle->GetSizeX()),
				FMath::Max(1, TextureHandle->GetSizeY()));
		}
	}

	void SetupCookedBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* AssetPath,
		const FVector2D& ImageSize,
		const TCHAR* DebugLabel)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}
		EnsureBrush(Brush, ImageSize);

		if (!TextureHandle.IsValid())
		{
			if (UTexture2D* Texture = T66RuntimeUITextureAccess::LoadAssetTexture(
				AssetPath,
				TextureFilter::TF_Trilinear,
				DebugLabel))
			{
				TextureHandle.Reset(Texture);
			}
		}

		Brush->SetResourceObject(TextureHandle.Get());
		if (TextureHandle.IsValid())
		{
			Brush->ImageSize = FVector2D(
				FMath::Max(1, TextureHandle->GetSizeX()),
				FMath::Max(1, TextureHandle->GetSizeY()));
		}
	}
}

UT66MiniMainMenuScreen::UT66MiniMainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniMainMenu;
	bIsModal = false;
}

void UT66MiniMainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
}

void UT66MiniMainMenuScreen::OnScreenDeactivated_Implementation()
{
	ReleaseRetainedSlateState();
	Super::OnScreenDeactivated_Implementation();
}

void UT66MiniMainMenuScreen::NativeDestruct()
{
	ReleaseRetainedSlateState();
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66MiniMainMenuScreen::BuildSharedMainMenuUI()
{
	RequestMiniMenuTextures();

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* MiniDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;

	bool bHasAnyMiniSave = false;
	if (SaveSubsystem)
	{
		for (const FT66MiniSaveSlotSummary& Summary : SaveSubsystem->BuildRunSlotSummaries(MiniDataSubsystem))
		{
			if (Summary.bOccupied)
			{
				bHasAnyMiniSave = true;
				break;
			}
		}
	}

	return SAssignNew(SharedMenuLayout, ST66MinigameMenuLayout)
		.GameID(FName(TEXT("mini")))
		.Title(NSLOCTEXT("T66Mini.MainMenu", "MiniSharedTitle", "MINI CHADPOCALYPSE"))
		.Subtitle(NSLOCTEXT("T66Mini.MainMenu", "MiniSharedSubtitle", "Single-player arcade survival"))
		.DailyTitle(NSLOCTEXT("T66Mini.MainMenu", "MiniDailyTitle", "TODAY'S MINI CHALLENGE"))
		.DailyBody(NSLOCTEXT("T66Mini.MainMenu", "MiniDailyBody", "One seeded survival run per day. Pick the difficulty, lock the route, then push materials and wave progress as high as you can."))
		.DailyRules(NSLOCTEXT("T66Mini.MainMenu", "MiniDailyRules", "Single player only.\nDaily scores use today's seed.\nLeaderboard ranks by materials collected.\nDifficulty is the only rules toggle."))
		.AccentColor(FLinearColor(0.98f, 0.84f, 0.43f, 1.0f))
		.BackgroundColor(FLinearColor(0.018f, 0.026f, 0.034f, 1.0f))
		.DifficultyOptions(BuildDifficultyOptions())
		.LoadGameEnabled(bHasAnyMiniSave)
		.BackendSubsystem(GameInstance ? GameInstance->GetSubsystem<UT66BackendSubsystem>() : nullptr)
		.OnNewGameClicked(FOnClicked::CreateUObject(this, &UT66MiniMainMenuScreen::HandleNewGameClicked))
		.OnLoadGameClicked(FOnClicked::CreateUObject(this, &UT66MiniMainMenuScreen::HandleLoadGameClicked))
		.OnDailyClicked(FOnClicked::CreateUObject(this, &UT66MiniMainMenuScreen::HandleDailyClicked))
		.OnBackClicked(FOnClicked::CreateUObject(this, &UT66MiniMainMenuScreen::HandleBackToMainMenuClicked))
		.OnBuildDailyEntries(FT66OnBuildMinigameLeaderboardEntries::CreateUObject(this, &UT66MiniMainMenuScreen::BuildDailyLeaderboardEntries))
		.OnBuildAllTimeEntries(FT66OnBuildMinigameLeaderboardEntries::CreateUObject(this, &UT66MiniMainMenuScreen::BuildAllTimeLeaderboardEntries))
		.OnGetDailyStatus(FT66OnGetMinigameLeaderboardStatus::CreateUObject(this, &UT66MiniMainMenuScreen::GetDailyLeaderboardStatus))
		.OnGetAllTimeStatus(FT66OnGetMinigameLeaderboardStatus::CreateUObject(this, &UT66MiniMainMenuScreen::GetAllTimeLeaderboardStatus));
}

TArray<FT66MinigameDifficultyOption> UT66MiniMainMenuScreen::BuildDifficultyOptions() const
{
	TArray<FT66MinigameDifficultyOption> Options;
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* MiniDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	if (MiniDataSubsystem)
	{
		for (const FT66MiniDifficultyDefinition& Difficulty : MiniDataSubsystem->GetDifficulties())
		{
			FT66MinigameDifficultyOption& Option = Options.AddDefaulted_GetRef();
			Option.DifficultyID = Difficulty.DifficultyID;
			Option.DisplayName = FText::FromString(Difficulty.DisplayName.IsEmpty() ? Difficulty.DifficultyID.ToString() : Difficulty.DisplayName);
		}
	}
	return Options;
}

TArray<FT66MinigameLeaderboardEntry> UT66MiniMainMenuScreen::BuildDailyLeaderboardEntries(FName DifficultyID) const
{
	return {};
}

TArray<FT66MinigameLeaderboardEntry> UT66MiniMainMenuScreen::BuildAllTimeLeaderboardEntries(FName DifficultyID) const
{
	TArray<FT66MinigameLeaderboardEntry> Entries;
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* MiniDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	const UT66MiniProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave(MiniDataSubsystem) : nullptr;
	if (ProfileSave && ProfileSave->LifetimeMaterialsCollected > 0)
	{
		FT66MinigameLeaderboardEntry& Entry = Entries.AddDefaulted_GetRef();
		Entry.Rank = 1;
		Entry.DisplayName = TEXT("Local Player");
		Entry.Score = ProfileSave->LifetimeMaterialsCollected;
		Entry.bIsLocalPlayer = true;
	}
	return Entries;
}

FText UT66MiniMainMenuScreen::GetDailyLeaderboardStatus(FName DifficultyID) const
{
	return NSLOCTEXT("T66Mini.MainMenu", "MiniDailyStatus", "Daily challenge leaderboard is ready for backend entries.");
}

FText UT66MiniMainMenuScreen::GetAllTimeLeaderboardStatus(FName DifficultyID) const
{
	return NSLOCTEXT("T66Mini.MainMenu", "MiniAllTimeStatus", "Finish a Mini Chadpocalypse run to seed your all-time row.");
}

TSharedRef<SWidget> UT66MiniMainMenuScreen::BuildSlateUI()
{
	return BuildSharedMainMenuUI();
}

FReply UT66MiniMainMenuScreen::HandleBackToMainMenuClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

FReply UT66MiniMainMenuScreen::HandleNewGameClicked()
{
	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginNewRun();
	}

	NavigateTo(ET66ScreenType::MiniCharacterSelect);
	return FReply::Handled();
}

FReply UT66MiniMainMenuScreen::HandleLoadGameClicked()
{
	NavigateTo(ET66ScreenType::MiniSaveSlots);
	return FReply::Handled();
}

FReply UT66MiniMainMenuScreen::HandleDailyClicked()
{
	const FName DifficultyID = SharedMenuLayout.IsValid() ? SharedMenuLayout->GetSelectedDifficultyID() : FName(TEXT("Easy"));
	const FString DifficultyToken = DifficultyID.ToString().ToLower();
	const FString DateKey = FDateTime::UtcNow().ToString(TEXT("%Y%m%d"));
	FString ChallengeId = FString::Printf(TEXT("mini-%s-%s"), *DateKey, *DifficultyToken);
	int32 DailySeed = static_cast<int32>(GetTypeHash(ChallengeId));

	if (UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		FT66MinigameDailyChallengeData Challenge;
		const FString ChallengeKey = UT66BackendSubsystem::MakeMinigameDailyChallengeCacheKey(TEXT("mini"), DifficultyToken);
		if (Backend->GetCachedMinigameDailyChallenge(ChallengeKey, Challenge))
		{
			ChallengeId = Challenge.ChallengeId;
			DailySeed = Challenge.RunSeed;
		}
		else if (Backend->IsBackendConfigured())
		{
			Backend->FetchCurrentMinigameDailyChallenge(TEXT("mini"), DifficultyToken);
		}
	}

	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginDailyRun(DifficultyID, ChallengeId, DailySeed);
	}

	NavigateTo(ET66ScreenType::MiniCharacterSelect);
	return FReply::Handled();
}

void UT66MiniMainMenuScreen::RequestMiniMenuTextures()
{
	SetupLooseBrush(SkyBackgroundBrush, SkyBackgroundTexture, TEXT("SourceAssets/UI/MainMenuReference/scene_background_1920x1080.png"), MiniBgSize, true, TEXT("MiniMainMenuScene"));
	SetupLooseBrush(PrimaryCTAFillBrush, PrimaryCTAFillTexture, TEXT("SourceAssets/UI/Reference/Shared/Buttons/CTA/normal.png"), FVector2D(360.f, 104.f), false, TEXT("MiniMainMenuCTA"));
}

void UT66MiniMainMenuScreen::ReleaseRetainedSlateState()
{
	SharedMenuLayout.Reset();
	SkyBackgroundBrush.Reset();
	SkyBackgroundTexture.Reset();
	FireMoonBrush.Reset();
	FireMoonTexture.Reset();
	PyramidChadBrush.Reset();
	PyramidChadTexture.Reset();
	PrimaryCTAFillBrush.Reset();
	PrimaryCTAFillTexture.Reset();
}
