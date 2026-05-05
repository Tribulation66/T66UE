// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66IdleMainMenuScreen.h"

#include "Core/T66BackendSubsystem.h"
#include "Core/T66IdleDataSubsystem.h"
#include "Core/T66IdleFrontendStateSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Engine/Texture2D.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Save/T66IdleSaveSubsystem.h"
#include "Styling/CoreStyle.h"
#include "UI/Components/T66MinigameMenuLayout.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UITypes.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float IdleMockupBackdropOpacity = 0.62f;

	const TCHAR* IdleMainMenuMockupPath()
	{
		return TEXT("UI/screens/minigames/idle_chadpocalypse/reference/idle_chadpocalypse_main_menu_mockup_1920x1080_imagegen_20260503_v2.png");
	}

	const TCHAR* IdleGameplayMockupPath()
	{
		return TEXT("UI/screens/minigames/idle_chadpocalypse/reference/idle_chadpocalypse_gameplay_mockup_1920x1080_imagegen_20260503_v2.png");
	}

	TMap<FString, TStrongObjectPtr<UTexture2D>> GIdleMockupTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GIdleMockupBrushCache;

	TAttribute<FText> MakeIdleTextAttribute(UT66IdleMainMenuScreen* Screen, FText (UT66IdleMainMenuScreen::*Getter)() const)
	{
		return TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(Screen, Getter));
	}

	TAttribute<TOptional<float>> MakeIdlePercentAttribute(UT66IdleMainMenuScreen* Screen, TOptional<float> (UT66IdleMainMenuScreen::*Getter)() const)
	{
		return TAttribute<TOptional<float>>::Create(TAttribute<TOptional<float>>::FGetter::CreateUObject(Screen, Getter));
	}

	UTexture2D* LoadIdleMockupTexture(const FString& SourceRelativePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GIdleMockupTextureCache.Find(SourceRelativePath))
		{
			return CachedTexture->Get();
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
				CandidatePath,
				TextureFilter::TF_Nearest,
				TEXT("IdleChadpocalypseMockup"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Nearest,
					true,
					TEXT("IdleChadpocalypseMockup"));
			}

			if (Texture)
			{
				GIdleMockupTextureCache.Add(SourceRelativePath, TStrongObjectPtr<UTexture2D>(Texture));
				return Texture;
			}
		}

		return nullptr;
	}

	const FSlateBrush* ResolveIdleMockupBrush(const FString& SourceRelativePath)
	{
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GIdleMockupBrushCache.Find(SourceRelativePath))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = LoadIdleMockupTexture(SourceRelativePath);
		if (!Texture)
		{
			return nullptr;
		}

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()));
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GIdleMockupBrushCache.Add(SourceRelativePath, Brush);
		return Brush.Get();
	}

	TSharedRef<SWidget> MakeIdleChromePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding, const FLinearColor& Accent)
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.018f, 0.016f, 0.020f, 0.86f))
			.Padding(1.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Accent)
				.Padding(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.030f, 0.028f, 0.032f, 0.92f))
					.Padding(Padding)
					[
						Content
					]
				]
			];
	}
}

UT66IdleMainMenuScreen::UT66IdleMainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::IdleMainMenu;
	bIsModal = false;
}

void UT66IdleMainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	EnsureProfileLoaded();
	if (!bAppliedAutomationStart && FParse::Param(FCommandLine::Get(), TEXT("T66IdleStartGameplay")))
	{
		bAppliedAutomationStart = true;
		StartPlayableRun();
		ForceRebuildSlate();
	}
}

void UT66IdleMainMenuScreen::OnScreenDeactivated_Implementation()
{
	SaveProfileState();
	Super::OnScreenDeactivated_Implementation();
}

void UT66IdleMainMenuScreen::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ViewMode == EIdleViewMode::Gameplay && bRunStarted)
	{
		TickIdleRun(InDeltaTime);
	}
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::BuildSlateUI()
{
	EnsureProfileLoaded();
	return ViewMode == EIdleViewMode::Gameplay ? BuildGameplayUI() : BuildMainMenuUI();
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::BuildSharedMainMenuUI()
{
	EnsureProfileLoaded();
	return SAssignNew(SharedMenuLayout, ST66MinigameMenuLayout)
		.GameID(FName(TEXT("idle")))
		.Title(NSLOCTEXT("T66Idle.MainMenu", "IdleSharedTitle", "IDLE CHADPOCALYPSE"))
		.Subtitle(NSLOCTEXT("T66Idle.MainMenu", "IdleSharedSubtitle", "Single-player idle progression"))
		.DailyTitle(NSLOCTEXT("T66Idle.MainMenu", "IdleDailyTitle", "TODAY'S IDLE CHALLENGE"))
		.DailyBody(NSLOCTEXT("T66Idle.MainMenu", "IdleDailyBody", "One daily snapshot run. Pick a difficulty, push stage progress, and cash out the best score before the reset."))
		.DailyRules(NSLOCTEXT("T66Idle.MainMenu", "IdleDailyRules", "Single player only.\nDaily score uses today's seed.\nLeaderboard ranks by highest stage and lifetime gold.\nOffline progress is capped by daily rules."))
		.AccentColor(FLinearColor(0.96f, 0.70f, 0.28f, 1.0f))
		.BackgroundColor(FLinearColor(0.024f, 0.020f, 0.018f, 1.0f))
		.DifficultyOptions(BuildDifficultyOptions())
		.LoadGameEnabled(true)
		.BackendSubsystem(GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr)
		.OnNewGameClicked(FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandlePlayClicked))
		.OnLoadGameClicked(FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleLoadClicked))
		.OnDailyClicked(FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleDailyClicked))
		.OnBackClicked(FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleBackClicked))
		.OnBuildDailyEntries(FT66OnBuildMinigameLeaderboardEntries::CreateUObject(this, &UT66IdleMainMenuScreen::BuildDailyLeaderboardEntries))
		.OnBuildAllTimeEntries(FT66OnBuildMinigameLeaderboardEntries::CreateUObject(this, &UT66IdleMainMenuScreen::BuildAllTimeLeaderboardEntries))
		.OnGetDailyStatus(FT66OnGetMinigameLeaderboardStatus::CreateUObject(this, &UT66IdleMainMenuScreen::GetDailyLeaderboardStatus))
		.OnGetAllTimeStatus(FT66OnGetMinigameLeaderboardStatus::CreateUObject(this, &UT66IdleMainMenuScreen::GetAllTimeLeaderboardStatus));
}

TArray<FT66MinigameDifficultyOption> UT66IdleMainMenuScreen::BuildDifficultyOptions() const
{
	TArray<FT66MinigameDifficultyOption> Options;
	const TCHAR* DifficultyIds[] = { TEXT("Easy"), TEXT("Medium"), TEXT("Hard"), TEXT("VeryHard"), TEXT("Impossible") };
	const FText DifficultyLabels[] = {
		NSLOCTEXT("T66Idle.MainMenu", "IdleEasy", "Easy"),
		NSLOCTEXT("T66Idle.MainMenu", "IdleMedium", "Medium"),
		NSLOCTEXT("T66Idle.MainMenu", "IdleHard", "Hard"),
		NSLOCTEXT("T66Idle.MainMenu", "IdleVeryHard", "Very Hard"),
		NSLOCTEXT("T66Idle.MainMenu", "IdleImpossible", "Impossible")
	};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(DifficultyIds); ++Index)
	{
		FT66MinigameDifficultyOption& Option = Options.AddDefaulted_GetRef();
		Option.DifficultyID = FName(DifficultyIds[Index]);
		Option.DisplayName = DifficultyLabels[Index];
	}
	return Options;
}

TArray<FT66MinigameLeaderboardEntry> UT66IdleMainMenuScreen::BuildDailyLeaderboardEntries(FName DifficultyID) const
{
	return {};
}

TArray<FT66MinigameLeaderboardEntry> UT66IdleMainMenuScreen::BuildAllTimeLeaderboardEntries(FName DifficultyID) const
{
	TArray<FT66MinigameLeaderboardEntry> Entries;
	const UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	const UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	if (ProfileSave && ProfileSave->Snapshot.HighestStageReached > 1)
	{
		FT66MinigameLeaderboardEntry& Entry = Entries.AddDefaulted_GetRef();
		Entry.Rank = 1;
		Entry.DisplayName = TEXT("Local Player");
		Entry.Score = static_cast<int64>(ProfileSave->Snapshot.HighestStageReached * 1000 + FMath::FloorToDouble(ProfileSave->Snapshot.LifetimeGold));
		Entry.bIsLocalPlayer = true;
	}
	return Entries;
}

FText UT66IdleMainMenuScreen::GetDailyLeaderboardStatus(FName DifficultyID) const
{
	return NSLOCTEXT("T66Idle.MainMenu", "IdleDailyStatus", "Daily challenge leaderboard is ready for backend entries.");
}

FText UT66IdleMainMenuScreen::GetAllTimeLeaderboardStatus(FName DifficultyID) const
{
	return NSLOCTEXT("T66Idle.MainMenu", "IdleAllTimeStatus", "Push past stage 1 to seed your all-time row.");
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::BuildMainMenuUI()
{
	return BuildSharedMainMenuUI();

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BuildMockupBackdrop(IdleMainMenuMockupPath(), FLinearColor(0.030f, 0.020f, 0.012f, 1.0f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.26f))
		]
		+ SOverlay::Slot()
		.Padding(FMargin(92.f, 108.f, 92.f, 70.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Bottom)
			[
				MakeIdleChromePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Idle.MainMenu", "Title", "IDLE CHADPOCALYPSE"))
						.Font(FT66Style::MakeFont(TEXT("Black"), 34))
						.ColorAndOpacity(FLinearColor(0.98f, 0.76f, 0.34f, 1.0f))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 24.f)
					[
						SNew(STextBlock)
						.Text(FText::Format(
							NSLOCTEXT("T66Idle.MainMenu", "Summary", "Stage {0} | {1}"),
							FText::AsNumber(CurrentStage),
							GetGoldText()))
						.Font(FT66Style::MakeFont(TEXT("Regular"), 13))
						.ColorAndOpacity(FLinearColor(0.82f, 0.78f, 0.70f, 1.0f))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.MainMenu", "Play", "PLAY"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandlePlayClicked), 420.f, 62.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.MainMenu", "Upgrade", "UPGRADES"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandlePlayClicked), 420.f, 62.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.MainMenu", "Options", "OPTIONS"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleOptionsClicked), 420.f, 62.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 24.f, 0.f, 0.f)
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.MainMenu", "Back", "BACK TO MINIGAMES"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleBackClicked), 420.f, 48.f)
					],
					FMargin(28.f),
					FLinearColor(0.74f, 0.51f, 0.18f, 0.92f))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::BuildGameplayUI()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BuildMockupBackdrop(IdleGameplayMockupPath(), FLinearColor(0.015f, 0.017f, 0.020f, 1.0f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.20f))
		]
		+ SOverlay::Slot()
		.Padding(FMargin(52.f, 42.f, 52.f, 38.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
				[
					MakeStatPanel(NSLOCTEXT("T66Idle.Gameplay", "GoldLabel", "GOLD"), MakeIdleTextAttribute(this, &UT66IdleMainMenuScreen::GetGoldText), FLinearColor(0.94f, 0.70f, 0.18f, 0.95f))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
				[
					MakeStatPanel(NSLOCTEXT("T66Idle.Gameplay", "StageLabel", "STAGE"), MakeIdleTextAttribute(this, &UT66IdleMainMenuScreen::GetStageText), FLinearColor(0.36f, 0.86f, 0.42f, 0.95f))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					MakeStatPanel(NSLOCTEXT("T66Idle.Gameplay", "PowerLabel", "POWER"), MakeIdleTextAttribute(this, &UT66IdleMainMenuScreen::GetPowerText), FLinearColor(0.56f, 0.38f, 0.96f, 0.95f))
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 48.f, 0.f, 28.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.27f).VAlign(VAlign_Center)
				[
					MakeIdleChromePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(MakeIdleTextAttribute(this, &UT66IdleMainMenuScreen::GetOwnedText))
							.Font(FT66Style::MakeFont(TEXT("Bold"), 22))
							.ColorAndOpacity(FLinearColor(0.98f, 0.78f, 0.42f, 1.0f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66Idle.Gameplay", "HeroBody", "Auto-fighting while the engine banks rewards. Tap and upgrade to push deeper."))
							.Font(FT66Style::MakeFont(TEXT("Regular"), 13))
							.ColorAndOpacity(FLinearColor(0.82f, 0.82f, 0.76f, 1.0f))
							.AutoWrapText(true)
						],
						FMargin(22.f),
						FLinearColor(0.40f, 0.28f, 0.11f, 0.88f))
				]
				+ SHorizontalBox::Slot().FillWidth(0.20f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot().FillWidth(0.27f).VAlign(VAlign_Center)
				[
					MakeIdleChromePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(MakeIdleTextAttribute(this, &UT66IdleMainMenuScreen::GetEnemyText))
							.Font(FT66Style::MakeFont(TEXT("Bold"), 20))
							.ColorAndOpacity(FLinearColor(0.96f, 0.40f, 0.32f, 1.0f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
						[
							SNew(SProgressBar)
							.Percent(MakeIdlePercentAttribute(this, &UT66IdleMainMenuScreen::GetEnemyHealthPercent))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(MakeIdleTextAttribute(this, &UT66IdleMainMenuScreen::GetProgressText))
							.Font(FT66Style::MakeFont(TEXT("Regular"), 13))
							.ColorAndOpacity(FLinearColor(0.78f, 0.86f, 0.74f, 1.0f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(SProgressBar)
							.Percent(MakeIdlePercentAttribute(this, &UT66IdleMainMenuScreen::GetEngineProgressPercent))
						],
						FMargin(22.f),
						FLinearColor(0.46f, 0.12f, 0.10f, 0.88f))
				]
				+ SHorizontalBox::Slot().FillWidth(0.26f).VAlign(VAlign_Center).Padding(24.f, 0.f, 0.f, 0.f)
				[
					MakePurchasePanel()
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeIdleChromePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.Gameplay", "Tap", "ATTACK"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleTapClicked), 300.f, 58.f)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.Gameplay", "UpgradeTap", "UPGRADE SWORD"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleUpgradeClicked), 300.f, 58.f)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.Gameplay", "UpgradeEngine", "UPGRADE ENGINE"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleEngineClicked), 300.f, 58.f)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.Gameplay", "Collect", "COLLECT"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleCollectClicked), 300.f, 58.f)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeIdleButton(NSLOCTEXT("T66Idle.Gameplay", "Back", "MENU"), FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleGameplayBackClicked), 160.f, 58.f)
					],
					FMargin(18.f),
					FLinearColor(0.28f, 0.24f, 0.18f, 0.88f))
			]
		];
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::BuildMockupBackdrop(const FString& SourceRelativePath, const FLinearColor& FallbackColor) const
{
	if (const FSlateBrush* Brush = ResolveIdleMockupBrush(SourceRelativePath))
	{
		return SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, IdleMockupBackdropOpacity));
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor);
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::MakeIdleButton(const FText& Text, const FOnClicked& OnClicked, const float Width, const float Height) const
{
	return SNew(SBox)
		.WidthOverride(Width)
		.HeightOverride(Height)
		[
			FT66Style::MakeButton(FT66ButtonParams(Text, OnClicked)
				.SetMinWidth(Width)
				.SetHeight(Height)
				.SetFontSize(14))
		];
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::MakeStatPanel(const FText& Label, const TAttribute<FText>& Value, const FLinearColor& Accent) const
{
	return MakeIdleChromePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FT66Style::MakeFont(TEXT("Bold"), 12))
			.ColorAndOpacity(Accent)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(Value)
			.Font(FT66Style::MakeFont(TEXT("Bold"), 17))
			.ColorAndOpacity(FLinearColor(0.96f, 0.92f, 0.84f, 1.0f))
		],
		FMargin(16.f, 12.f),
		Accent * 0.68f);
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::MakePurchasePanel()
{
	const UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	const UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	const FT66IdleHeroDefinition* NextHero = FindNextPurchasableHero(ProfileSave);
	const FT66IdleCompanionDefinition* NextCompanion = FindNextPurchasableCompanion(ProfileSave);
	const FT66IdleItemDefinition* NextItem = FindNextPurchasableItem(ProfileSave);
	const FT66IdleIdolDefinition* NextIdol = FindNextPurchasableIdol(ProfileSave);

	const auto MakeBody = [](const FString& Name, const double Cost, const FString& Fallback)
	{
		if (Name.IsEmpty())
		{
			return FText::FromString(Fallback);
		}
		return FText::Format(
			NSLOCTEXT("T66Idle.Gameplay", "PurchaseBody", "{0} | {1} gold"),
			FText::FromString(Name),
			FText::AsNumber(FMath::CeilToInt(Cost)));
	};

	return MakeIdleChromePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66Idle.Gameplay", "HireShopTitle", "POWER BAR"))
			.Font(FT66Style::MakeFont(TEXT("Bold"), 18))
			.ColorAndOpacity(FLinearColor(0.96f, 0.88f, 0.68f, 1.0f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
		[
			MakePurchaseButton(
				NSLOCTEXT("T66Idle.Gameplay", "HeroPurchase", "HERO"),
				MakeBody(NextHero ? NextHero->DisplayName : FString(), NextHero ? NextHero->HireCostGold : 0.0, TEXT("Roster complete")),
				FLinearColor(0.86f, 0.58f, 0.22f, 0.94f),
				FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleBuyHeroClicked))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
		[
			MakePurchaseButton(
				NSLOCTEXT("T66Idle.Gameplay", "CompanionPurchase", "COMPANION"),
				MakeBody(NextCompanion ? NextCompanion->DisplayName : FString(), NextCompanion ? NextCompanion->HireCostGold : 0.0, TEXT("Crew complete")),
				FLinearColor(0.30f, 0.62f, 0.78f, 0.94f),
				FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleBuyCompanionClicked))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
		[
			MakePurchaseButton(
				NSLOCTEXT("T66Idle.Gameplay", "ItemPurchase", "ITEM"),
				MakeBody(NextItem ? NextItem->DisplayName : FString(), NextItem ? NextItem->BaseCostGold : 0.0, TEXT("Inventory complete")),
				FLinearColor(0.62f, 0.48f, 0.22f, 0.94f),
				FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleBuyItemClicked))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
		[
			MakePurchaseButton(
				NSLOCTEXT("T66Idle.Gameplay", "IdolPurchase", "IDOL"),
				MakeBody(NextIdol ? NextIdol->DisplayName : FString(), NextIdol ? NextIdol->BaseCostGold : 0.0, TEXT("Altar complete")),
				FLinearColor(0.56f, 0.34f, 0.86f, 0.94f),
				FOnClicked::CreateUObject(this, &UT66IdleMainMenuScreen::HandleBuyIdolClicked))
		],
		FMargin(18.f),
		FLinearColor(0.24f, 0.22f, 0.20f, 0.90f));
}

TSharedRef<SWidget> UT66IdleMainMenuScreen::MakePurchaseButton(const FText& Label, const FText& Body, const FLinearColor& Accent, const FOnClicked& OnClicked) const
{
	return FT66Style::MakeBareButton(
		FT66BareButtonParams(
			OnClicked,
			MakeIdleChromePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::MakeFont(TEXT("Bold"), 12))
					.ColorAndOpacity(Accent)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Body)
					.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
					.ColorAndOpacity(FLinearColor(0.90f, 0.86f, 0.78f, 1.0f))
					.AutoWrapText(true)
				],
				FMargin(10.f),
				Accent * 0.64f))
		.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
		.SetPadding(FMargin(0.f)));
}

void UT66IdleMainMenuScreen::EnsureProfileLoaded()
{
	if (bProfileLoaded)
	{
		return;
	}

	bProfileLoaded = true;

	UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	const FT66IdleTuningDefinition& Tuning = GetIdleTuning();
	if (SaveSubsystem && ProfileSave)
	{
		EnsureStartingUnlocks(ProfileSave, DataSubsystem);
		SaveSubsystem->ApplyOfflineProgress(ProfileSave, DataSubsystem, FTimespan::FromHours(Tuning.OfflineProgressCapHours));
	}

	if (ProfileSave)
	{
		CurrentStage = FMath::Max(Tuning.StartingStage, ProfileSave->Snapshot.CurrentStage > 0 ? ProfileSave->Snapshot.CurrentStage : ProfileSave->Snapshot.HighestStageReached);
		Gold = FMath::Max(Tuning.StartingGold, ProfileSave->Snapshot.Gold);
		LifetimeGold = FMath::Max(0.0, ProfileSave->Snapshot.LifetimeGold);
		TapDamage = FMath::Max(Tuning.StartingTapDamage, ProfileSave->Snapshot.TapDamage);
		PassiveDamagePerSecond = FMath::Max(Tuning.StartingPassiveDamagePerSecond, ProfileSave->Snapshot.PassiveDamagePerSecond);
		EnemyHealth = FMath::Max(0.0, ProfileSave->Snapshot.EnemyHealth);
		EnemyMaxHealth = FMath::Max(0.0, ProfileSave->Snapshot.EnemyMaxHealth);
		EngineProgress = FMath::Clamp(ProfileSave->Snapshot.EngineProgress, 0.0, 0.999999);
		UncollectedProgress = FMath::Max(0.0, ProfileSave->Snapshot.UncollectedProgress);
		CurrentStageID = ProfileSave->Snapshot.CurrentStageID;
		CurrentEnemyID = ProfileSave->Snapshot.CurrentEnemyID;
		bCurrentEnemyIsStageBoss = ProfileSave->Snapshot.bCurrentEnemyIsStageBoss;
		BossStagesCleared = FMath::Max(0, ProfileSave->Snapshot.BossStagesCleared);
		RecalculatePowerFromOwned(ProfileSave);
	}

	SpawnEnemyForCurrentStage();
}

void UT66IdleMainMenuScreen::SaveProfileState()
{
	UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	if (!SaveSubsystem || !ProfileSave)
	{
		return;
	}

	if (UncollectedProgress > 0.0)
	{
		Gold += UncollectedProgress;
		UncollectedProgress = 0.0;
	}

	ProfileSave->Snapshot.HighestStageReached = FMath::Max(ProfileSave->Snapshot.HighestStageReached, CurrentStage);
	ProfileSave->Snapshot.CurrentStage = CurrentStage;
	ProfileSave->Snapshot.CurrentStageID = CurrentStageID;
	ProfileSave->Snapshot.CurrentEnemyID = CurrentEnemyID;
	ProfileSave->Snapshot.bCurrentEnemyIsStageBoss = bCurrentEnemyIsStageBoss;
	ProfileSave->Snapshot.EnemyHealth = EnemyHealth;
	ProfileSave->Snapshot.EnemyMaxHealth = EnemyMaxHealth;
	ProfileSave->Snapshot.EngineProgress = EngineProgress;
	ProfileSave->Snapshot.UncollectedProgress = UncollectedProgress;
	ProfileSave->Snapshot.Gold = Gold;
	ProfileSave->Snapshot.LifetimeGold = LifetimeGold;
	ProfileSave->Snapshot.BossStagesCleared = BossStagesCleared;
	ProfileSave->Snapshot.TapDamage = TapDamage;
	ProfileSave->Snapshot.PassiveDamagePerSecond = PassiveDamagePerSecond;
	SaveSubsystem->SaveProfile(ProfileSave);
	SubmitLeaderboardProgressIfNeeded();
}

void UT66IdleMainMenuScreen::StartPlayableRun()
{
	EnsureProfileLoaded();
	bRunStarted = true;
	if (UT66IdleFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleFrontendStateSubsystem>() : nullptr)
	{
		if (!FrontendState->IsDailySession())
		{
			FrontendState->BeginNewSession();
		}
		FrontendState->SelectHero(ResolveStartingHeroID(GetIdleDataSubsystem()));
	}
	ViewMode = EIdleViewMode::Gameplay;
}

void UT66IdleMainMenuScreen::SubmitLeaderboardProgressIfNeeded()
{
	UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		return;
	}

	const int64 RawScore = static_cast<int64>(CurrentStage) * 1000LL + static_cast<int64>(FMath::FloorToDouble(LifetimeGold));
	const int32 Score = FMath::Clamp<int64>(RawScore, 0, MAX_int32);
	if (Score <= LastSubmittedLeaderboardScore)
	{
		return;
	}

	const UT66IdleFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleFrontendStateSubsystem>() : nullptr;
	const UT66SteamHelper* SteamHelper = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SteamHelper>() : nullptr;
	const bool bDailySession = FrontendState && FrontendState->IsDailySession();
	const FName DifficultyID = FrontendState && FrontendState->GetSelectedDifficultyID() != NAME_None
		? FrontendState->GetSelectedDifficultyID()
		: FName(TEXT("Easy"));

	Backend->SubmitMinigameScore(
		SteamHelper ? SteamHelper->GetLocalDisplayName() : FString(TEXT("Player")),
		TEXT("idle"),
		bDailySession ? TEXT("daily") : TEXT("alltime"),
		DifficultyID.ToString().ToLower(),
		Score,
		bDailySession ? FrontendState->GetDailyChallengeId() : FString(),
		bDailySession ? FrontendState->GetDailySeed() : 0);
	LastSubmittedLeaderboardScore = Score;
}

void UT66IdleMainMenuScreen::SpawnEnemyForCurrentStage()
{
	const FName SavedEnemyID = CurrentEnemyID;
	const double SavedEnemyHealth = EnemyHealth;
	const double SavedEnemyMaxHealth = EnemyMaxHealth;
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	const FT66IdleStageDefinition* StageDefinition = DataSubsystem ? DataSubsystem->FindStageByIndex(CurrentStage) : nullptr;
	const FT66IdleZoneDefinition* Zone = DataSubsystem ? DataSubsystem->FindZoneForStage(CurrentStage) : nullptr;
	CurrentZoneDisplayName = StageDefinition && !StageDefinition->DisplayName.IsEmpty()
		? StageDefinition->DisplayName
		: (Zone ? Zone->DisplayName : FString());

	const FT66IdleEnemyDefinition* Enemy = nullptr;
	if (DataSubsystem && StageDefinition)
	{
		const FName StageEnemyID = !StageDefinition->BossEnemyID.IsNone() ? StageDefinition->BossEnemyID : StageDefinition->EnemyID;
		Enemy = StageEnemyID != NAME_None ? DataSubsystem->FindEnemy(StageEnemyID) : nullptr;
	}
	if (DataSubsystem && Zone && Zone->EnemyIDs.Num() > 0)
	{
		const int32 EnemyIndex = FMath::Abs(CurrentStage - Zone->FirstStage) % Zone->EnemyIDs.Num();
		Enemy = Enemy ? Enemy : DataSubsystem->FindEnemy(Zone->EnemyIDs[EnemyIndex]);
	}
	if (!Enemy && DataSubsystem && DataSubsystem->GetEnemies().Num() > 0)
	{
		Enemy = &DataSubsystem->GetEnemies()[0];
	}

	CurrentStageID = StageDefinition ? StageDefinition->StageID : NAME_None;
	CurrentEnemyID = Enemy ? Enemy->EnemyID : NAME_None;
	bCurrentEnemyIsStageBoss = StageDefinition && StageDefinition->bBossStage;
	CurrentEnemyDisplayName = Enemy ? Enemy->DisplayName : TEXT("Unknown Monster");

	const double BaseHealth = Enemy ? Enemy->BaseHealth : (Zone ? Zone->BaseEnemyHealth : 1.0);
	const double HealthPerStage = Enemy ? Enemy->HealthPerStage : 0.0;
	const double ZoneScalar = Zone ? Zone->EnemyHealthScalar : 1.0;
	EnemyMaxHealth = FMath::Max(1.0, (BaseHealth + static_cast<double>(CurrentStage - 1) * HealthPerStage) * ZoneScalar);
	if (SavedEnemyID == CurrentEnemyID && SavedEnemyHealth > 0.0 && SavedEnemyMaxHealth > 0.0)
	{
		EnemyHealth = FMath::Clamp(SavedEnemyHealth, 0.0, EnemyMaxHealth);
	}
	else
	{
		EnemyHealth = EnemyMaxHealth;
	}
}

void UT66IdleMainMenuScreen::AwardEnemyClear()
{
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	const FT66IdleZoneDefinition* Zone = DataSubsystem ? DataSubsystem->FindZoneForStage(CurrentStage) : nullptr;
	const FT66IdleEnemyDefinition* Enemy = DataSubsystem ? DataSubsystem->FindEnemy(CurrentEnemyID) : nullptr;
	const UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	const UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	const FT66IdleStageDefinition* StageDefinition = DataSubsystem ? DataSubsystem->FindStageByIndex(CurrentStage) : nullptr;
	const double BaseReward = Enemy ? Enemy->BaseGoldReward : (Zone ? Zone->BaseGoldReward : 1.0);
	const double GoldPerStage = Enemy ? Enemy->GoldPerStage : 0.0;
	const double ZoneScalar = Zone ? Zone->GoldRewardScalar : 1.0;
	const double StageReward = StageDefinition ? FMath::Max(0.0, StageDefinition->ClearGoldReward) : 0.0;
	const double Reward = FMath::Max(1.0, (BaseReward + static_cast<double>(CurrentStage) * GoldPerStage) * ZoneScalar * GetGoldRewardMultiplier(ProfileSave)) + StageReward;

	UncollectedProgress += Reward;
	LifetimeGold += Reward;
	if (bCurrentEnemyIsStageBoss)
	{
		++BossStagesCleared;
	}
	++CurrentStage;
}

void UT66IdleMainMenuScreen::TickIdleRun(const float DeltaSeconds)
{
	if (DeltaSeconds <= 0.f)
	{
		return;
	}

	const FT66IdleTuningDefinition& Tuning = GetIdleTuning();
	EnemyHealth -= PassiveDamagePerSecond * static_cast<double>(DeltaSeconds);
	EngineProgress += FMath::Clamp(PassiveDamagePerSecond * Tuning.EngineProgressPerPassiveDamageSecond * static_cast<double>(DeltaSeconds), 0.0, 0.35);
	if (EngineProgress >= 1.0)
	{
		EngineProgress = FMath::Fmod(EngineProgress, 1.0);
		const double EngineReward = FMath::Max(1.0, PassiveDamagePerSecond * Tuning.EngineGoldPayoutMultiplier);
		UncollectedProgress += EngineReward;
		LifetimeGold += EngineReward;
	}

	if (EnemyHealth <= 0.0)
	{
		AwardEnemyClear();
		SpawnEnemyForCurrentStage();
	}

	AutosaveAccumulator += DeltaSeconds;
	if (AutosaveAccumulator >= Tuning.AutosaveIntervalSeconds)
	{
		AutosaveAccumulator = 0.f;
		SaveProfileState();
	}
}

void UT66IdleMainMenuScreen::RecalculatePowerFromOwned(const UT66IdleProfileSaveGame* ProfileSave)
{
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	const FT66IdleTuningDefinition& Tuning = GetIdleTuning();
	if (!ProfileSave || !DataSubsystem)
	{
		return;
	}

	double DataTapDamage = Tuning.StartingTapDamage;
	double DataPassiveDamage = Tuning.StartingPassiveDamagePerSecond;
	double TapMultiplier = 1.0;
	double PassiveMultiplier = 1.0;
	CurrentHeroDisplayName.Reset();

	for (const FName HeroID : ProfileSave->UnlockedHeroIDs)
	{
		if (const FT66IdleHeroDefinition* Hero = DataSubsystem->FindHero(HeroID))
		{
			DataTapDamage += Hero->TapDamage;
			DataPassiveDamage += Hero->PassiveDamagePerSecond;
			if (CurrentHeroDisplayName.IsEmpty())
			{
				CurrentHeroDisplayName = Hero->DisplayName;
			}
		}
	}

	for (const FName CompanionID : ProfileSave->UnlockedCompanionIDs)
	{
		if (const FT66IdleCompanionDefinition* Companion = DataSubsystem->FindCompanion(CompanionID))
		{
			DataPassiveDamage += Companion->PassiveDamagePerSecond;
		}
	}

	for (const FName ItemID : ProfileSave->OwnedItemIDs)
	{
		if (const FT66IdleItemDefinition* Item = DataSubsystem->FindItem(ItemID))
		{
			DataTapDamage += Item->TapDamageBonus;
			DataPassiveDamage += Item->PassiveDamageBonus;
		}
	}

	for (const FName IdolID : ProfileSave->OwnedIdolIDs)
	{
		if (const FT66IdleIdolDefinition* Idol = DataSubsystem->FindIdol(IdolID))
		{
			TapMultiplier *= FMath::Max(0.01, Idol->TapDamageMultiplier);
			PassiveMultiplier *= FMath::Max(0.01, Idol->PassiveDamageMultiplier);
		}
	}

	TapDamage = FMath::Max(TapDamage, DataTapDamage * TapMultiplier);
	PassiveDamagePerSecond = FMath::Max(PassiveDamagePerSecond, DataPassiveDamage * PassiveMultiplier);
}

UT66IdleDataSubsystem* UT66IdleMainMenuScreen::GetIdleDataSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleDataSubsystem>() : nullptr;
}

const FT66IdleTuningDefinition& UT66IdleMainMenuScreen::GetIdleTuning() const
{
	static const FT66IdleTuningDefinition FallbackTuning;
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	return DataSubsystem ? DataSubsystem->GetTuning() : FallbackTuning;
}

double UT66IdleMainMenuScreen::GetGoldRewardMultiplier(const UT66IdleProfileSaveGame* ProfileSave) const
{
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	if (!ProfileSave || !DataSubsystem)
	{
		return 1.0;
	}

	double Multiplier = 1.0;
	for (const FName ItemID : ProfileSave->OwnedItemIDs)
	{
		if (const FT66IdleItemDefinition* Item = DataSubsystem->FindItem(ItemID))
		{
			Multiplier *= FMath::Max(0.01, Item->GoldRewardMultiplier);
		}
	}
	for (const FName IdolID : ProfileSave->OwnedIdolIDs)
	{
		if (const FT66IdleIdolDefinition* Idol = DataSubsystem->FindIdol(IdolID))
		{
			Multiplier *= FMath::Max(0.01, Idol->GoldRewardMultiplier);
		}
	}
	return Multiplier;
}

FName UT66IdleMainMenuScreen::ResolveStartingHeroID(const UT66IdleDataSubsystem* DataSubsystem) const
{
	const FT66IdleTuningDefinition& Tuning = GetIdleTuning();
	if (Tuning.StartingHeroID != NAME_None)
	{
		return Tuning.StartingHeroID;
	}
	if (DataSubsystem && DataSubsystem->GetHeroes().Num() > 0)
	{
		return DataSubsystem->GetHeroes()[0].HeroID;
	}
	return NAME_None;
}

void UT66IdleMainMenuScreen::EnsureStartingUnlocks(UT66IdleProfileSaveGame* ProfileSave, const UT66IdleDataSubsystem* DataSubsystem) const
{
	if (!ProfileSave || !DataSubsystem)
	{
		return;
	}

	const FName StartingHeroID = ResolveStartingHeroID(DataSubsystem);
	if (StartingHeroID != NAME_None && !ProfileSave->UnlockedHeroIDs.Contains(StartingHeroID))
	{
		ProfileSave->UnlockedHeroIDs.Add(StartingHeroID);
	}
}

bool UT66IdleMainMenuScreen::TrySpendGold(const double Cost)
{
	if (Gold < Cost)
	{
		return false;
	}

	Gold -= Cost;
	return true;
}

const FT66IdleHeroDefinition* UT66IdleMainMenuScreen::FindNextPurchasableHero(const UT66IdleProfileSaveGame* ProfileSave) const
{
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	if (!ProfileSave || !DataSubsystem)
	{
		return nullptr;
	}

	for (const FT66IdleHeroDefinition& Hero : DataSubsystem->GetHeroes())
	{
		if (!ProfileSave->UnlockedHeroIDs.Contains(Hero.HeroID))
		{
			return &Hero;
		}
	}
	return nullptr;
}

const FT66IdleCompanionDefinition* UT66IdleMainMenuScreen::FindNextPurchasableCompanion(const UT66IdleProfileSaveGame* ProfileSave) const
{
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	if (!ProfileSave || !DataSubsystem)
	{
		return nullptr;
	}

	for (const FT66IdleCompanionDefinition& Companion : DataSubsystem->GetCompanions())
	{
		if (!ProfileSave->UnlockedCompanionIDs.Contains(Companion.CompanionID))
		{
			return &Companion;
		}
	}
	return nullptr;
}

const FT66IdleItemDefinition* UT66IdleMainMenuScreen::FindNextPurchasableItem(const UT66IdleProfileSaveGame* ProfileSave) const
{
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	if (!ProfileSave || !DataSubsystem)
	{
		return nullptr;
	}

	for (const FT66IdleItemDefinition& Item : DataSubsystem->GetItems())
	{
		if (!ProfileSave->OwnedItemIDs.Contains(Item.ItemID))
		{
			return &Item;
		}
	}
	return nullptr;
}

const FT66IdleIdolDefinition* UT66IdleMainMenuScreen::FindNextPurchasableIdol(const UT66IdleProfileSaveGame* ProfileSave) const
{
	const UT66IdleDataSubsystem* DataSubsystem = GetIdleDataSubsystem();
	if (!ProfileSave || !DataSubsystem)
	{
		return nullptr;
	}

	for (const FT66IdleIdolDefinition& Idol : DataSubsystem->GetIdols())
	{
		if (!ProfileSave->OwnedIdolIDs.Contains(Idol.IdolID))
		{
			return &Idol;
		}
	}
	return nullptr;
}

FText UT66IdleMainMenuScreen::GetGoldText() const
{
	return FText::Format(
		NSLOCTEXT("T66Idle.Gameplay", "GoldValue", "{0} gold | {1} ready"),
		FText::AsNumber(FMath::FloorToInt(Gold)),
		FText::AsNumber(FMath::FloorToInt(UncollectedProgress)));
}

FText UT66IdleMainMenuScreen::GetStageText() const
{
	return FText::Format(NSLOCTEXT("T66Idle.Gameplay", "StageValue", "Stage {0}"), FText::AsNumber(CurrentStage));
}

FText UT66IdleMainMenuScreen::GetPowerText() const
{
	return FText::Format(
		NSLOCTEXT("T66Idle.Gameplay", "PowerValue", "Tap {0} | Auto {1}/s"),
		FText::AsNumber(FMath::FloorToInt(TapDamage)),
		FText::AsNumber(PassiveDamagePerSecond));
}

FText UT66IdleMainMenuScreen::GetEnemyText() const
{
	return FText::Format(
		NSLOCTEXT("T66Idle.Gameplay", "EnemyValue", "{0} | {1}/{2} HP"),
		FText::FromString(CurrentEnemyDisplayName.IsEmpty() ? FString(TEXT("Monster")) : CurrentEnemyDisplayName),
		FText::AsNumber(FMath::Max(0, FMath::CeilToInt(EnemyHealth))),
		FText::AsNumber(FMath::CeilToInt(EnemyMaxHealth)));
}

FText UT66IdleMainMenuScreen::GetProgressText() const
{
	return FText::Format(
		NSLOCTEXT("T66Idle.Gameplay", "ProgressValue", "Engine progress {0}%"),
		FText::AsNumber(FMath::RoundToInt(EngineProgress * 100.0)));
}

FText UT66IdleMainMenuScreen::GetOwnedText() const
{
	return FText::Format(
		NSLOCTEXT("T66Idle.Gameplay", "OwnedValue", "{0} | {1}"),
		FText::FromString(CurrentHeroDisplayName.IsEmpty() ? FString(TEXT("Hero")) : CurrentHeroDisplayName),
		FText::FromString(CurrentZoneDisplayName.IsEmpty() ? FString(TEXT("Dungeon")) : CurrentZoneDisplayName));
}

TOptional<float> UT66IdleMainMenuScreen::GetEnemyHealthPercent() const
{
	if (EnemyMaxHealth <= 0.0)
	{
		return 0.f;
	}
	return FMath::Clamp(static_cast<float>(EnemyHealth / EnemyMaxHealth), 0.f, 1.f);
}

TOptional<float> UT66IdleMainMenuScreen::GetEngineProgressPercent() const
{
	return FMath::Clamp(static_cast<float>(EngineProgress), 0.f, 1.f);
}

FReply UT66IdleMainMenuScreen::HandlePlayClicked()
{
	if (UT66IdleFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginNewSession();
	}
	StartPlayableRun();
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleLoadClicked()
{
	EnsureProfileLoaded();
	if (UT66IdleFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginNewSession();
	}
	StartPlayableRun();
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleDailyClicked()
{
	const FName DifficultyID = SharedMenuLayout.IsValid() ? SharedMenuLayout->GetSelectedDifficultyID() : FName(TEXT("Easy"));
	const FString DifficultyToken = DifficultyID.ToString().ToLower();
	const FString DateKey = FDateTime::UtcNow().ToString(TEXT("%Y%m%d"));
	FString ChallengeId = FString::Printf(TEXT("idle-%s-%s"), *DateKey, *DifficultyToken);
	int32 DailySeed = static_cast<int32>(GetTypeHash(ChallengeId));

	if (UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		FT66MinigameDailyChallengeData Challenge;
		const FString ChallengeKey = UT66BackendSubsystem::MakeMinigameDailyChallengeCacheKey(TEXT("idle"), DifficultyToken);
		if (Backend->GetCachedMinigameDailyChallenge(ChallengeKey, Challenge))
		{
			ChallengeId = Challenge.ChallengeId;
			DailySeed = Challenge.RunSeed;
		}
		else if (Backend->IsBackendConfigured())
		{
			Backend->FetchCurrentMinigameDailyChallenge(TEXT("idle"), DifficultyToken);
		}
	}

	if (UT66IdleFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginDailySession(DifficultyID, ChallengeId, DailySeed);
	}

	StartPlayableRun();
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleUpgradeClicked()
{
	const FT66IdleTuningDefinition& Tuning = GetIdleTuning();
	const double Cost = FMath::CeilToDouble(Tuning.TapUpgradeBaseCost + TapDamage * Tuning.TapUpgradeCostPerDamage);
	if (Gold >= Cost)
	{
		Gold -= Cost;
		TapDamage += Tuning.TapUpgradeDamageStep;
		SaveProfileState();
	}
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleEngineClicked()
{
	const FT66IdleTuningDefinition& Tuning = GetIdleTuning();
	const double Cost = FMath::CeilToDouble(Tuning.EngineUpgradeBaseCost + PassiveDamagePerSecond * Tuning.EngineUpgradeCostPerDps);
	if (Gold >= Cost)
	{
		Gold -= Cost;
		PassiveDamagePerSecond += Tuning.EngineUpgradeDpsStep;
		SaveProfileState();
	}
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleTapClicked()
{
	EnemyHealth -= TapDamage;
	if (EnemyHealth <= 0.0)
	{
		AwardEnemyClear();
		SpawnEnemyForCurrentStage();
	}
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleCollectClicked()
{
	if (UncollectedProgress > 0.0)
	{
		Gold += UncollectedProgress;
		UncollectedProgress = 0.0;
		SaveProfileState();
	}
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleBuyHeroClicked()
{
	UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	const FT66IdleHeroDefinition* Hero = FindNextPurchasableHero(ProfileSave);
	if (ProfileSave && Hero && TrySpendGold(Hero->HireCostGold))
	{
		ProfileSave->UnlockedHeroIDs.AddUnique(Hero->HeroID);
		RecalculatePowerFromOwned(ProfileSave);
		SaveProfileState();
		ForceRebuildSlate();
	}
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleBuyCompanionClicked()
{
	UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	const FT66IdleCompanionDefinition* Companion = FindNextPurchasableCompanion(ProfileSave);
	if (ProfileSave && Companion && TrySpendGold(Companion->HireCostGold))
	{
		ProfileSave->UnlockedCompanionIDs.AddUnique(Companion->CompanionID);
		RecalculatePowerFromOwned(ProfileSave);
		SaveProfileState();
		ForceRebuildSlate();
	}
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleBuyItemClicked()
{
	UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	const FT66IdleItemDefinition* Item = FindNextPurchasableItem(ProfileSave);
	if (ProfileSave && Item && TrySpendGold(Item->BaseCostGold))
	{
		ProfileSave->OwnedItemIDs.AddUnique(Item->ItemID);
		RecalculatePowerFromOwned(ProfileSave);
		SaveProfileState();
		ForceRebuildSlate();
	}
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleBuyIdolClicked()
{
	UT66IdleSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdleSaveSubsystem>() : nullptr;
	UT66IdleProfileSaveGame* ProfileSave = SaveSubsystem ? SaveSubsystem->LoadOrCreateProfileSave() : nullptr;
	const FT66IdleIdolDefinition* Idol = FindNextPurchasableIdol(ProfileSave);
	if (ProfileSave && Idol && TrySpendGold(Idol->BaseCostGold))
	{
		ProfileSave->OwnedIdolIDs.AddUnique(Idol->IdolID);
		RecalculatePowerFromOwned(ProfileSave);
		SaveProfileState();
		ForceRebuildSlate();
	}
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleOptionsClicked()
{
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleGameplayBackClicked()
{
	SaveProfileState();
	ViewMode = EIdleViewMode::MainMenu;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66IdleMainMenuScreen::HandleStartClicked()
{
	return HandlePlayClicked();
}

FReply UT66IdleMainMenuScreen::HandleBackClicked()
{
	SaveProfileState();
	NavigateTo(ET66ScreenType::Minigames);
	return FReply::Handled();
}
