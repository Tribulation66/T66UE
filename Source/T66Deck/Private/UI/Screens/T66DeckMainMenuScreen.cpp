// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66DeckMainMenuScreen.h"

#include "Core/T66BackendSubsystem.h"
#include "Core/T66DeckDataSubsystem.h"
#include "Core/T66DeckFrontendStateSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Engine/Texture2D.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Save/T66DeckSaveSubsystem.h"
#include "Save/T66DeckRunSaveGame.h"
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
	constexpr float DeckMockupBackdropOpacity = 0.62f;

	const TCHAR* DeckMainMenuMockupPath()
	{
		return TEXT("UI/screens/minigames/chadpocalypse_deckbuilder/reference/chadpocalypse_deckbuilder_main_menu_mockup_1920x1080_imagegen_20260503_v2.png");
	}

	const TCHAR* DeckGameplayMockupPath()
	{
		return TEXT("UI/screens/minigames/chadpocalypse_deckbuilder/reference/chadpocalypse_deckbuilder_gameplay_mockup_1920x1080_imagegen_20260503_v2.png");
	}

	TMap<FString, TStrongObjectPtr<UTexture2D>> GDeckMockupTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GDeckMockupBrushCache;

	TAttribute<FText> MakeDeckTextAttribute(UT66DeckMainMenuScreen* Screen, FText (UT66DeckMainMenuScreen::*Getter)() const)
	{
		return TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(Screen, Getter));
	}

	TAttribute<TOptional<float>> MakeDeckPercentAttribute(UT66DeckMainMenuScreen* Screen, TOptional<float> (UT66DeckMainMenuScreen::*Getter)() const)
	{
		return TAttribute<TOptional<float>>::Create(TAttribute<TOptional<float>>::FGetter::CreateUObject(Screen, Getter));
	}

	UTexture2D* LoadDeckMockupTexture(const FString& SourceRelativePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GDeckMockupTextureCache.Find(SourceRelativePath))
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
				TEXT("ChadpocalypseDeckbuilderMockup"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Nearest,
					true,
					TEXT("ChadpocalypseDeckbuilderMockup"));
			}

			if (Texture)
			{
				GDeckMockupTextureCache.Add(SourceRelativePath, TStrongObjectPtr<UTexture2D>(Texture));
				return Texture;
			}
		}

		return nullptr;
	}

	const FSlateBrush* ResolveDeckMockupBrush(const FString& SourceRelativePath)
	{
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GDeckMockupBrushCache.Find(SourceRelativePath))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = LoadDeckMockupTexture(SourceRelativePath);
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

		GDeckMockupBrushCache.Add(SourceRelativePath, Brush);
		return Brush.Get();
	}

	TSharedRef<SWidget> MakeDeckChromePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding, const FLinearColor& Accent)
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.018f, 0.016f, 0.024f, 0.88f))
			.Padding(1.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Accent)
				.Padding(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.030f, 0.026f, 0.038f, 0.93f))
					.Padding(Padding)
					[
						Content
					]
				]
			];
	}
}

UT66DeckMainMenuScreen::UT66DeckMainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::DeckMainMenu;
	bIsModal = false;
	StatusText = NSLOCTEXT("T66Deck.Gameplay", "MenuStatus", "Choose Play to descend.");
}

void UT66DeckMainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (!bAppliedAutomationStart && FParse::Param(FCommandLine::Get(), TEXT("T66DeckStartGameplay")))
	{
		bAppliedAutomationStart = true;
		StartPlayableRun();
		ForceRebuildSlate();
	}
}

void UT66DeckMainMenuScreen::OnScreenDeactivated_Implementation()
{
	Super::OnScreenDeactivated_Implementation();
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildSlateUI()
{
	switch (ViewMode)
	{
	case EDeckViewMode::HeroSelect:
		return BuildHeroSelectUI();
	case EDeckViewMode::Map:
		return BuildMapUI();
	case EDeckViewMode::Gameplay:
		return BuildGameplayUI();
	case EDeckViewMode::Reward:
		return BuildRewardUI();
	case EDeckViewMode::MainMenu:
	default:
		return BuildMainMenuUI();
	}
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildSharedMainMenuUI()
{
	const UT66DeckSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckSaveSubsystem>() : nullptr;
	bool bHasAnySave = false;
	if (SaveSubsystem)
	{
		for (const FT66DeckSaveSlotSummary& Summary : SaveSubsystem->BuildRunSlotSummaries())
		{
			if (Summary.bOccupied)
			{
				bHasAnySave = true;
				break;
			}
		}
	}

	return SAssignNew(SharedMenuLayout, ST66MinigameMenuLayout)
		.GameID(FName(TEXT("deck")))
		.Title(NSLOCTEXT("T66Deck.MainMenu", "DeckSharedTitle", "CHADPOCALYPSE DECKBUILDER"))
		.Subtitle(NSLOCTEXT("T66Deck.MainMenu", "DeckSharedSubtitle", "Single-player dungeon descent"))
		.DailyTitle(NSLOCTEXT("T66Deck.MainMenu", "DeckDailyTitle", "TODAY'S DUNGEON"))
		.DailyBody(NSLOCTEXT("T66Deck.MainMenu", "DeckDailyBody", "One seeded descent per day. Choose a difficulty, draft through the dungeon, and post the best clear score."))
		.DailyRules(NSLOCTEXT("T66Deck.MainMenu", "DeckDailyRules", "Single player only.\nDaily score uses today's map and reward seed.\nLeaderboard ranks by score.\nDifficulty is the only rules toggle."))
		.AccentColor(FLinearColor(0.78f, 0.58f, 0.96f, 1.0f))
		.BackgroundColor(FLinearColor(0.018f, 0.016f, 0.026f, 1.0f))
		.DifficultyOptions(BuildDifficultyOptions())
		.LoadGameEnabled(bHasAnySave)
		.BackendSubsystem(GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr)
		.OnNewGameClicked(FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandlePlayClicked))
		.OnLoadGameClicked(FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleLoadClicked))
		.OnDailyClicked(FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleDailyClicked))
		.OnBackClicked(FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleBackClicked))
		.OnBuildDailyEntries(FT66OnBuildMinigameLeaderboardEntries::CreateUObject(this, &UT66DeckMainMenuScreen::BuildDailyLeaderboardEntries))
		.OnBuildAllTimeEntries(FT66OnBuildMinigameLeaderboardEntries::CreateUObject(this, &UT66DeckMainMenuScreen::BuildAllTimeLeaderboardEntries))
		.OnGetDailyStatus(FT66OnGetMinigameLeaderboardStatus::CreateUObject(this, &UT66DeckMainMenuScreen::GetDailyLeaderboardStatus))
		.OnGetAllTimeStatus(FT66OnGetMinigameLeaderboardStatus::CreateUObject(this, &UT66DeckMainMenuScreen::GetAllTimeLeaderboardStatus));
}

TArray<FT66MinigameDifficultyOption> UT66DeckMainMenuScreen::BuildDifficultyOptions() const
{
	TArray<FT66MinigameDifficultyOption> Options;
	const TCHAR* DifficultyIds[] = { TEXT("Easy"), TEXT("Medium"), TEXT("Hard"), TEXT("VeryHard"), TEXT("Impossible") };
	const FText DifficultyLabels[] = {
		NSLOCTEXT("T66Deck.MainMenu", "DeckEasy", "Easy"),
		NSLOCTEXT("T66Deck.MainMenu", "DeckMedium", "Medium"),
		NSLOCTEXT("T66Deck.MainMenu", "DeckHard", "Hard"),
		NSLOCTEXT("T66Deck.MainMenu", "DeckVeryHard", "Very Hard"),
		NSLOCTEXT("T66Deck.MainMenu", "DeckImpossible", "Impossible")
	};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(DifficultyIds); ++Index)
	{
		FT66MinigameDifficultyOption& Option = Options.AddDefaulted_GetRef();
		Option.DifficultyID = FName(DifficultyIds[Index]);
		Option.DisplayName = DifficultyLabels[Index];
	}
	return Options;
}

TArray<FT66MinigameLeaderboardEntry> UT66DeckMainMenuScreen::BuildDailyLeaderboardEntries(FName DifficultyID) const
{
	return {};
}

TArray<FT66MinigameLeaderboardEntry> UT66DeckMainMenuScreen::BuildAllTimeLeaderboardEntries(FName DifficultyID) const
{
	TArray<FT66MinigameLeaderboardEntry> Entries;
	const UT66DeckSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckSaveSubsystem>() : nullptr;
	if (SaveSubsystem)
	{
		int32 BestFloor = 0;
		for (const FT66DeckSaveSlotSummary& Summary : SaveSubsystem->BuildRunSlotSummaries())
		{
			BestFloor = FMath::Max(BestFloor, Summary.FloorIndex);
		}
		if (BestFloor > 0)
		{
			FT66MinigameLeaderboardEntry& Entry = Entries.AddDefaulted_GetRef();
			Entry.Rank = 1;
			Entry.DisplayName = TEXT("Local Player");
			Entry.Score = BestFloor * 1000;
			Entry.bIsLocalPlayer = true;
		}
	}
	return Entries;
}

FText UT66DeckMainMenuScreen::GetDailyLeaderboardStatus(FName DifficultyID) const
{
	return NSLOCTEXT("T66Deck.MainMenu", "DeckDailyStatus", "Daily dungeon leaderboard is ready for backend entries.");
}

FText UT66DeckMainMenuScreen::GetAllTimeLeaderboardStatus(FName DifficultyID) const
{
	return NSLOCTEXT("T66Deck.MainMenu", "DeckAllTimeStatus", "Save or finish a descent to seed your all-time row.");
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildMainMenuUI()
{
	return BuildSharedMainMenuUI();

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66DeckDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66DeckDataSubsystem>() : nullptr;
	const int32 CardCount = DataSubsystem ? DataSubsystem->GetCards().Num() : 0;
	const int32 RelicCount = DataSubsystem ? DataSubsystem->GetRelics().Num() : 0;
	const int32 EncounterCount = DataSubsystem ? DataSubsystem->GetEncounters().Num() : 0;

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BuildMockupBackdrop(DeckMainMenuMockupPath(), FLinearColor(0.014f, 0.014f, 0.020f, 1.0f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
		]
		+ SOverlay::Slot()
		.Padding(FMargin(92.f, 118.f, 92.f, 70.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Bottom)
			[
				MakeDeckChromePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Deck.MainMenu", "Title", "CHADPOCALYPSE DECKBUILDER"))
						.Font(FT66Style::MakeFont(TEXT("Black"), 30))
						.ColorAndOpacity(FLinearColor(0.94f, 0.72f, 0.36f, 1.0f))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 24.f)
					[
						SNew(STextBlock)
						.Text(FText::Format(
							NSLOCTEXT("T66Deck.MainMenu", "Summary", "Prototype registry: {0} cards, {1} relics, {2} encounters."),
							FText::AsNumber(CardCount),
							FText::AsNumber(RelicCount),
							FText::AsNumber(EncounterCount)))
						.Font(FT66Style::MakeFont(TEXT("Regular"), 13))
						.ColorAndOpacity(FLinearColor(0.82f, 0.78f, 0.86f, 1.0f))
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeDeckButton(NSLOCTEXT("T66Deck.MainMenu", "Play", "PLAY"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandlePlayClicked), 430.f, 62.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						MakeDeckButton(NSLOCTEXT("T66Deck.MainMenu", "Collection", "COLLECTION"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleCollectionClicked), 430.f, 62.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						MakeDeckButton(NSLOCTEXT("T66Deck.MainMenu", "Options", "OPTIONS"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleOptionsClicked), 430.f, 62.f)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 24.f, 0.f, 0.f)
					[
						MakeDeckButton(NSLOCTEXT("T66Deck.MainMenu", "Back", "BACK TO MINIGAMES"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleBackClicked), 430.f, 48.f)
					],
					FMargin(28.f),
					FLinearColor(0.48f, 0.34f, 0.74f, 0.88f))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SSpacer)
			]
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildHeroSelectUI()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();

	TSharedRef<SVerticalBox> HeroList = SNew(SVerticalBox);
	if (DataSubsystem)
	{
		for (int32 Index = 0; Index < FMath::Min(3, DataSubsystem->GetHeroes().Num()); ++Index)
		{
			const FT66DeckHeroDefinition& Hero = DataSubsystem->GetHeroes()[Index];
			HeroList->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				MakeChoiceButton(
					FText::FromString(Hero.DisplayName),
					FText::Format(NSLOCTEXT("T66Deck.Setup", "HeroBody", "{0} HP | {1} gold"), FText::AsNumber(Hero.StartingHealth), FText::AsNumber(Hero.StartingGold)),
					SelectedHeroID == Hero.HeroID ? FLinearColor(0.78f, 0.58f, 0.28f, 0.96f) : Hero.AccentColor,
					FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleSelectHeroClicked, Hero.HeroID))
			];
		}
	}

	TSharedRef<SVerticalBox> CompanionList = SNew(SVerticalBox);
	if (DataSubsystem)
	{
		for (int32 Index = 0; Index < FMath::Min(3, DataSubsystem->GetCompanions().Num()); ++Index)
		{
			const FT66DeckCompanionDefinition& Companion = DataSubsystem->GetCompanions()[Index];
			CompanionList->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				MakeChoiceButton(
					FText::FromString(Companion.DisplayName),
					FText::Format(NSLOCTEXT("T66Deck.Setup", "CompanionBody", "{0} block | +{1} card damage"), FText::AsNumber(Companion.StartingBlock), FText::AsNumber(Companion.PassiveDamageBonus)),
					SelectedCompanionID == Companion.CompanionID ? FLinearColor(0.34f, 0.78f, 0.88f, 0.96f) : Companion.AccentColor,
					FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleSelectCompanionClicked, Companion.CompanionID))
			];
		}
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BuildMockupBackdrop(DeckMainMenuMockupPath(), FLinearColor(0.014f, 0.014f, 0.020f, 1.0f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.34f))
		]
		+ SOverlay::Slot().Padding(FMargin(84.f, 90.f, 84.f, 64.f))
		[
			MakeDeckChromePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Deck.Setup", "Title", "CHOOSE YOUR DESCENT"))
					.Font(FT66Style::MakeFont(TEXT("Black"), 28))
					.ColorAndOpacity(FLinearColor(0.94f, 0.76f, 0.44f, 1.0f))
				]
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 24.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 18.f, 0.f)
					[
						MakeDeckChromePanel(HeroList, FMargin(16.f), FLinearColor(0.42f, 0.28f, 0.16f, 0.88f))
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(18.f, 0.f, 0.f, 0.f)
					[
						MakeDeckChromePanel(CompanionList, FMargin(16.f), FLinearColor(0.18f, 0.34f, 0.46f, 0.88f))
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						MakeDeckButton(NSLOCTEXT("T66Deck.Setup", "Start", "START DESCENT"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleStartDescentClicked), 360.f, 58.f)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(16.f, 0.f, 0.f, 0.f)
					[
						MakeDeckButton(NSLOCTEXT("T66Deck.Setup", "Back", "BACK"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleGameplayBackClicked), 160.f, 58.f)
					]
				],
				FMargin(24.f),
				FLinearColor(0.46f, 0.32f, 0.72f, 0.88f))
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildMapUI()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	TArray<const FT66DeckEncounterDefinition*> Encounters = DataSubsystem ? DataSubsystem->GetEncountersForFloor(FloorIndex) : TArray<const FT66DeckEncounterDefinition*>();
	const int32 ChoiceCount = FMath::Min(GetDeckTuning().MapChoicesPerFloor, Encounters.Num());

	TSharedRef<SVerticalBox> ChoiceList = SNew(SVerticalBox);
	for (int32 Index = 0; Index < ChoiceCount; ++Index)
	{
		const FT66DeckEncounterDefinition* Encounter = Encounters[Index];
		ChoiceList->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			MakeChoiceButton(
				FText::FromString(Encounter->DisplayName),
				FText::Format(NSLOCTEXT("T66Deck.Map", "NodeBody", "Floor {0} | {1}"), FText::AsNumber(FloorIndex), FText::FromString(StaticEnum<ET66DeckNodeType>()->GetNameStringByValue(static_cast<int64>(Encounter->NodeType)))),
				FLinearColor(0.28f, 0.36f, 0.48f, 0.94f),
				FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleMapNodeClicked, Encounter->EncounterID))
		];
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BuildMockupBackdrop(DeckGameplayMockupPath(), FLinearColor(0.012f, 0.012f, 0.018f, 1.0f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.30f))
		]
		+ SOverlay::Slot().Padding(FMargin(120.f, 74.f, 120.f, 70.f))
		[
			MakeDeckChromePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Deck.Map", "Title", "CHOOSE A ROOM BELOW"))
					.Font(FT66Style::MakeFont(TEXT("Black"), 28))
					.ColorAndOpacity(FLinearColor(0.90f, 0.82f, 0.68f, 1.0f))
				]
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 24.f)
				[
					ChoiceList
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeDeckButton(NSLOCTEXT("T66Deck.Map", "Menu", "MENU"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleGameplayBackClicked), 180.f, 52.f)
				],
				FMargin(24.f),
				FLinearColor(0.22f, 0.26f, 0.34f, 0.90f))
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildGameplayUI()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BuildMockupBackdrop(DeckGameplayMockupPath(), FLinearColor(0.012f, 0.012f, 0.018f, 1.0f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.18f))
		]
		+ SOverlay::Slot()
		.Padding(FMargin(52.f, 44.f, 52.f, 36.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.30f).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 24.f, 120.f)
				[
					MakeDeckChromePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(MakeDeckTextAttribute(this, &UT66DeckMainMenuScreen::GetPlayerText))
							.Font(FT66Style::MakeFont(TEXT("Bold"), 19))
							.ColorAndOpacity(FLinearColor(0.92f, 0.84f, 0.98f, 1.0f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
						[
							SNew(SProgressBar)
							.Percent(MakeDeckPercentAttribute(this, &UT66DeckMainMenuScreen::GetPlayerHealthPercent))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
						[
							MakeMeterPanel(NSLOCTEXT("T66Deck.Gameplay", "EnergyLabel", "ENERGY"), MakeDeckTextAttribute(this, &UT66DeckMainMenuScreen::GetEnergyText), FLinearColor(0.58f, 0.36f, 0.95f, 0.92f))
						],
						FMargin(20.f),
						FLinearColor(0.42f, 0.28f, 0.72f, 0.88f))
				]
				+ SHorizontalBox::Slot().FillWidth(0.40f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot().FillWidth(0.30f).VAlign(VAlign_Top).Padding(24.f, 0.f, 0.f, 0.f)
				[
					MakeDeckChromePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(MakeDeckTextAttribute(this, &UT66DeckMainMenuScreen::GetEnemyText))
							.Font(FT66Style::MakeFont(TEXT("Bold"), 19))
							.ColorAndOpacity(FLinearColor(0.96f, 0.42f, 0.36f, 1.0f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
						[
							SNew(SProgressBar)
							.Percent(MakeDeckPercentAttribute(this, &UT66DeckMainMenuScreen::GetEnemyHealthPercent))
						],
						FMargin(20.f),
						FLinearColor(0.52f, 0.16f, 0.14f, 0.88f))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
			[
				MakeDeckChromePanel(
					SNew(STextBlock)
					.Text(MakeDeckTextAttribute(this, &UT66DeckMainMenuScreen::GetStatusText))
					.Font(FT66Style::MakeFont(TEXT("Regular"), 14))
					.ColorAndOpacity(FLinearColor(0.90f, 0.86f, 0.76f, 1.0f)),
					FMargin(18.f, 12.f),
					FLinearColor(0.34f, 0.26f, 0.42f, 0.88f))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeDeckChromePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeCardWidget(0)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeCardWidget(1)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeCardWidget(2)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeCardWidget(3)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 22.f, 0.f)
					[
						MakeCardWidget(4)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeDeckButton(NSLOCTEXT("T66Deck.Gameplay", "EndTurn", "END TURN"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleEndTurnClicked), 190.f, 62.f)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
						[
							MakeDeckButton(NSLOCTEXT("T66Deck.Gameplay", "Menu", "MENU"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleGameplayBackClicked), 190.f, 48.f)
						]
					],
					FMargin(18.f),
					FLinearColor(0.32f, 0.22f, 0.42f, 0.88f))
			]
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildRewardUI()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	TSharedRef<SVerticalBox> RewardList = SNew(SVerticalBox);

	for (const FName CardID : RewardCardIDs)
	{
		if (const FT66DeckCardDefinition* Card = DataSubsystem ? DataSubsystem->FindCard(CardID) : nullptr)
		{
			RewardList->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				MakeChoiceButton(
					FText::FromString(Card->DisplayName),
					FText::FromString(Card->RulesText),
					Card->AccentColor,
					FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleRewardCardClicked, Card->CardID))
			];
		}
	}

	for (const FName ItemID : RewardItemIDs)
	{
		if (const FT66DeckItemDefinition* Item = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr)
		{
			RewardList->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				MakeChoiceButton(
					FText::FromString(Item->DisplayName),
					FText::FromString(Item->RulesText),
					Item->AccentColor,
					FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleRewardItemClicked, Item->ItemID))
			];
		}
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BuildMockupBackdrop(DeckGameplayMockupPath(), FLinearColor(0.012f, 0.012f, 0.018f, 1.0f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.32f))
		]
		+ SOverlay::Slot().Padding(FMargin(150.f, 86.f, 150.f, 80.f))
		[
			MakeDeckChromePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Deck.Reward", "Title", "ROOM REWARD"))
					.Font(FT66Style::MakeFont(TEXT("Black"), 28))
					.ColorAndOpacity(FLinearColor(0.94f, 0.78f, 0.48f, 1.0f))
				]
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 24.f)
				[
					RewardList
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeDeckButton(NSLOCTEXT("T66Deck.Reward", "Skip", "SKIP"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleSkipRewardClicked), 180.f, 52.f)
				],
				FMargin(24.f),
				FLinearColor(0.36f, 0.26f, 0.18f, 0.90f))
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildMockupBackdrop(const FString& SourceRelativePath, const FLinearColor& FallbackColor) const
{
	if (const FSlateBrush* Brush = ResolveDeckMockupBrush(SourceRelativePath))
	{
		return SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, DeckMockupBackdropOpacity));
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor);
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeDeckButton(const FText& Text, const FOnClicked& OnClicked, const float Width, const float Height) const
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

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeChoiceButton(const FText& Title, const FText& Body, const FLinearColor& Accent, const FOnClicked& OnClicked) const
{
	return FT66Style::MakeBareButton(
		FT66BareButtonParams(
			OnClicked,
			MakeDeckChromePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Title)
					.Font(FT66Style::MakeFont(TEXT("Bold"), 16))
					.ColorAndOpacity(FLinearColor(0.96f, 0.90f, 0.82f, 1.0f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Body)
					.Font(FT66Style::MakeFont(TEXT("Regular"), 12))
					.ColorAndOpacity(FLinearColor(0.84f, 0.82f, 0.76f, 1.0f))
					.AutoWrapText(true)
				],
				FMargin(14.f),
				Accent))
		.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
		.SetPadding(FMargin(0.f)));
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeCardWidget(const int32 CardIndex)
{
	const bool bHasCard = Hand.IsValidIndex(CardIndex);
	const FRuntimeCard Card = bHasCard ? Hand[CardIndex] : FRuntimeCard();
	const bool bCanPlay = bHasCard && !bRunDefeated && Card.Cost <= Energy;
	const FLinearColor Accent = bCanPlay ? Card.Accent : FLinearColor(0.24f, 0.22f, 0.26f, 0.92f);

	return SNew(SBox)
		.WidthOverride(210.f)
		.HeightOverride(250.f)
		[
			FT66Style::MakeBareButton(
				FT66BareButtonParams(
					FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleCardClicked, CardIndex),
					MakeDeckChromePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(bHasCard ? Card.Name : NSLOCTEXT("T66Deck.Gameplay", "EmptyCard", "EMPTY"))
							.Font(FT66Style::MakeFont(TEXT("Bold"), 16))
							.ColorAndOpacity(FLinearColor(0.96f, 0.90f, 0.82f, 1.0f))
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 12.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.78f, 0.72f, 0.64f, 0.82f))
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(bHasCard ? Card.Rules : FText::GetEmpty())
							.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
							.ColorAndOpacity(FLinearColor(0.84f, 0.80f, 0.78f, 1.0f))
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(bHasCard ? FText::Format(NSLOCTEXT("T66Deck.Gameplay", "CardCost", "{0} energy"), FText::AsNumber(Card.Cost)) : FText::GetEmpty())
							.Font(FT66Style::MakeFont(TEXT("Regular"), 10))
							.ColorAndOpacity(FLinearColor(0.68f, 0.56f, 0.96f, 1.0f))
							.Justification(ETextJustify::Center)
						],
						FMargin(12.f),
						Accent))
				.SetEnabled(bCanPlay)
				.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.SetPadding(FMargin(0.f)))
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeMeterPanel(const FText& Label, const TAttribute<FText>& Value, const FLinearColor& Accent) const
{
	return MakeDeckChromePanel(
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
		FMargin(12.f),
		Accent * 0.64f);
}

void UT66DeckMainMenuScreen::StartPlayableRun()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckTuningDefinition& Tuning = GetDeckTuning();
	if (SelectedHeroID == NAME_None)
	{
		SelectedHeroID = ResolveStartingHeroID(DataSubsystem);
	}
	if (SelectedCompanionID == NAME_None)
	{
		SelectedCompanionID = ResolveStartingCompanionID(DataSubsystem);
	}
	SelectedStartingDeckID = ResolveStartingDeckID(DataSubsystem, SelectedHeroID);

	if (UT66DeckFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckFrontendStateSubsystem>() : nullptr)
	{
		if (!FrontendState->IsDailyRun())
		{
			FrontendState->BeginNewRun();
			FrontendState->SelectDifficulty(Tuning.DifficultyID);
		}
		FrontendState->SelectHero(SelectedHeroID);
		FrontendState->SelectStartingDeck(SelectedStartingDeckID);
	}

	const FT66DeckHeroDefinition* Hero = DataSubsystem ? DataSubsystem->FindHero(SelectedHeroID) : nullptr;
	const FT66DeckCompanionDefinition* Companion = DataSubsystem ? DataSubsystem->FindCompanion(SelectedCompanionID) : nullptr;
	const FT66DeckStartingDeckDefinition* StartingDeck = DataSubsystem ? DataSubsystem->FindStartingDeck(SelectedStartingDeckID) : nullptr;

	ViewMode = EDeckViewMode::Map;
	bRunStarted = true;
	bRunDefeated = false;
	ActiveSaveSlotIndex = 0;
	FloorIndex = FMath::Max(1, Tuning.StartingFloor);
	RefreshCurrentStageFromFloor();
	HighestStageIndexCleared = 0;
	PlayerMaxHealth = Hero ? Hero->StartingHealth : 80;
	PlayerHealth = PlayerMaxHealth;
	PlayerBlock = Companion ? Companion->StartingBlock : 0;
	Energy = Tuning.StartingEnergy;
	Gold = Hero ? Hero->StartingGold : 0;
	EnemyMaxHealth = 1;
	EnemyHealth = 1;
	EnemyIntent = 0;
	CurrentEncounterID = NAME_None;
	CurrentEnemyID = NAME_None;
	DeckCardIDs = StartingDeck ? StartingDeck->CardIDs : TArray<FName>();
	RewardCardIDs.Reset();
	RewardItemIDs.Reset();
	StatusText = NSLOCTEXT("T66Deck.Gameplay", "StartStatus", "Choose a room and keep descending.");
	BuildHandFromDeck();
	SaveCurrentRunState();
}

void UT66DeckMainMenuScreen::BuildHandFromDeck()
{
	Hand.Reset();
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckTuningDefinition& Tuning = GetDeckTuning();
	const FT66DeckCompanionDefinition* Companion = DataSubsystem ? DataSubsystem->FindCompanion(SelectedCompanionID) : nullptr;
	const int32 BonusDamage = Companion ? Companion->PassiveDamageBonus : 0;
	if (!DataSubsystem || DeckCardIDs.Num() == 0)
	{
		return;
	}

	for (int32 Index = 0; Index < FMath::Min(Tuning.HandSize, DeckCardIDs.Num()); ++Index)
	{
		const FName CardID = DeckCardIDs[(FloorIndex + Index - 1) % DeckCardIDs.Num()];
		const FT66DeckCardDefinition* Definition = DataSubsystem->FindCard(CardID);
		if (!Definition)
		{
			continue;
		}

		FRuntimeCard Card;
		Card.CardID = Definition->CardID;
		Card.Name = FText::FromString(Definition->DisplayName);
		Card.Rules = FText::FromString(Definition->RulesText);
		Card.Cost = Definition->EnergyCost;
		Card.Damage = Definition->Damage > 0 ? Definition->Damage + BonusDamage : 0;
		Card.Block = Definition->Block;
		Card.Accent = Definition->AccentColor;
		Hand.Add(MoveTemp(Card));
	}
}

void UT66DeckMainMenuScreen::EnterMap()
{
	RefreshCurrentStageFromFloor();
	ViewMode = EDeckViewMode::Map;
	RewardCardIDs.Reset();
	RewardItemIDs.Reset();
	SaveCurrentRunState();
	ForceRebuildSlate();
}

void UT66DeckMainMenuScreen::EnterEncounter(const FName EncounterID)
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckEncounterDefinition* Encounter = DataSubsystem ? DataSubsystem->FindEncounter(EncounterID) : nullptr;
	const FName EnemyID = Encounter && Encounter->EnemyIDs.Num() > 0 ? Encounter->EnemyIDs[0] : NAME_None;
	const FT66DeckEnemyDefinition* Enemy = DataSubsystem ? DataSubsystem->FindEnemy(EnemyID) : nullptr;
	if (!Encounter || !Enemy)
	{
		return;
	}

	CurrentEncounterID = Encounter->EncounterID;
	CurrentEnemyID = Enemy->EnemyID;
	EnemyMaxHealth = FMath::Max(1, Enemy->BaseHealth + FMath::Max(0, FloorIndex - 1) * Enemy->HealthPerFloor);
	EnemyHealth = EnemyMaxHealth;
	Energy = GetDeckTuning().StartingEnergy;
	PlayerBlock = 0;
	RefreshEnemyIntent();
	BuildHandFromDeck();
	StatusText = FText::Format(NSLOCTEXT("T66Deck.Gameplay", "EncounterStatus", "Entered {0}."), FText::FromString(Encounter->DisplayName));
	ViewMode = EDeckViewMode::Gameplay;
	SaveCurrentRunState();
}

void UT66DeckMainMenuScreen::CompleteEnemy()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckEncounterDefinition* Encounter = DataSubsystem ? DataSubsystem->FindEncounter(CurrentEncounterID) : nullptr;
	const FT66DeckEnemyDefinition* Enemy = DataSubsystem ? DataSubsystem->FindEnemy(CurrentEnemyID) : nullptr;
	const FT66DeckStageDefinition* StageDefinition = DataSubsystem ? DataSubsystem->FindStageForFloor(FloorIndex) : nullptr;
	const bool bBossClear = Encounter && Encounter->NodeType == ET66DeckNodeType::Boss;
	const int32 StageReward = bBossClear && StageDefinition ? FMath::Max(0, StageDefinition->ClearGoldReward) : 0;
	const int32 Reward = (Enemy ? Enemy->GoldReward : 0) + (Encounter ? Encounter->GoldRewardBonus : 0) + StageReward;
	Gold += FMath::Max(0, Reward);
	if (bBossClear && StageDefinition)
	{
		HighestStageIndexCleared = FMath::Max(HighestStageIndexCleared, StageDefinition->StageIndex);
	}
	PlayerBlock = 0;
	Energy = GetDeckTuning().StartingEnergy;
	RewardCardIDs = Encounter ? Encounter->RewardCardIDs : TArray<FName>();
	RewardItemIDs = Encounter ? Encounter->RewardItemIDs : TArray<FName>();
	SubmitLeaderboardProgressIfNeeded();
	StatusText = FText::Format(
		NSLOCTEXT("T66Deck.Gameplay", "EnemyClearStatus", "Room cleared. Gained {0} gold. Floor {1} begins."),
		FText::AsNumber(Reward),
		FText::AsNumber(FloorIndex));
	ViewMode = EDeckViewMode::Reward;
	SaveCurrentRunState();
	ForceRebuildSlate();
}

void UT66DeckMainMenuScreen::RefreshCurrentStageFromFloor()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	if (const FT66DeckStageDefinition* StageDefinition = DataSubsystem ? DataSubsystem->FindStageForFloor(FloorIndex) : nullptr)
	{
		CurrentStageID = StageDefinition->StageID;
		StageIndex = StageDefinition->StageIndex;
		return;
	}

	CurrentStageID = NAME_None;
	StageIndex = FMath::Max(1, FloorIndex);
}

void UT66DeckMainMenuScreen::SaveCurrentRunState()
{
	UT66DeckSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckSaveSubsystem>() : nullptr;
	UT66DeckRunSaveGame* RunSave = SaveSubsystem ? SaveSubsystem->CreateBlankRunSave() : nullptr;
	if (!SaveSubsystem || !RunSave || !bRunStarted)
	{
		return;
	}

	const UT66DeckFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckFrontendStateSubsystem>() : nullptr;
	RunSave->HeroID = SelectedHeroID;
	RunSave->CompanionID = SelectedCompanionID;
	RunSave->DifficultyID = FrontendState && FrontendState->GetSelectedDifficultyID() != NAME_None
		? FrontendState->GetSelectedDifficultyID()
		: GetDeckTuning().DifficultyID;
	RunSave->StartingDeckID = SelectedStartingDeckID;
	RunSave->ActIndex = 1;
	RunSave->FloorIndex = FloorIndex;
	RunSave->CurrentStageID = CurrentStageID;
	RunSave->StageIndex = StageIndex;
	RunSave->HighestStageIndexCleared = HighestStageIndexCleared;
	RunSave->CurrentHealth = PlayerHealth;
	RunSave->MaxHealth = PlayerMaxHealth;
	RunSave->Gold = Gold;
	RunSave->PlayerBlock = PlayerBlock;
	RunSave->Energy = Energy;
	RunSave->CurrentNodeID = CurrentEncounterID;
	RunSave->CurrentEnemyID = CurrentEnemyID;
	RunSave->EnemyHealth = EnemyHealth;
	RunSave->EnemyMaxHealth = EnemyMaxHealth;
	RunSave->EnemyIntent = EnemyIntent;
	RunSave->DrawPileCardIDs = DeckCardIDs;
	RunSave->RewardCardIDs = RewardCardIDs;
	RunSave->RewardItemIDs = RewardItemIDs;
	RunSave->SavedViewMode = static_cast<int32>(ViewMode);
	RunSave->bRunDefeated = bRunDefeated;
	SaveSubsystem->SaveRunToSlot(ActiveSaveSlotIndex, RunSave);
}

bool UT66DeckMainMenuScreen::RestoreRunFromSave(const UT66DeckRunSaveGame* RunSave)
{
	if (!RunSave)
	{
		return false;
	}

	SelectedHeroID = RunSave->HeroID;
	SelectedCompanionID = RunSave->CompanionID != NAME_None ? RunSave->CompanionID : ResolveStartingCompanionID(GetDeckDataSubsystem());
	SelectedStartingDeckID = RunSave->StartingDeckID;
	CurrentEncounterID = RunSave->CurrentNodeID;
	CurrentEnemyID = RunSave->CurrentEnemyID;
	FloorIndex = FMath::Max(1, RunSave->FloorIndex);
	CurrentStageID = RunSave->CurrentStageID;
	StageIndex = FMath::Max(1, RunSave->StageIndex);
	HighestStageIndexCleared = FMath::Max(0, RunSave->HighestStageIndexCleared);
	if (CurrentStageID == NAME_None)
	{
		RefreshCurrentStageFromFloor();
	}
	PlayerHealth = FMath::Max(1, RunSave->CurrentHealth);
	PlayerMaxHealth = FMath::Max(PlayerHealth, RunSave->MaxHealth);
	PlayerBlock = FMath::Max(0, RunSave->PlayerBlock);
	Energy = FMath::Max(0, RunSave->Energy);
	EnemyHealth = FMath::Max(0, RunSave->EnemyHealth);
	EnemyMaxHealth = FMath::Max(EnemyHealth, RunSave->EnemyMaxHealth);
	EnemyIntent = FMath::Max(0, RunSave->EnemyIntent);
	Gold = FMath::Max(0, RunSave->Gold);
	DeckCardIDs = RunSave->DrawPileCardIDs;
	RewardCardIDs = RunSave->RewardCardIDs;
	RewardItemIDs = RunSave->RewardItemIDs;
	bRunStarted = true;
	bRunDefeated = RunSave->bRunDefeated;

	switch (RunSave->SavedViewMode)
	{
	case static_cast<int32>(EDeckViewMode::Gameplay):
		ViewMode = EDeckViewMode::Gameplay;
		BuildHandFromDeck();
		break;
	case static_cast<int32>(EDeckViewMode::Reward):
		ViewMode = EDeckViewMode::Reward;
		break;
	case static_cast<int32>(EDeckViewMode::HeroSelect):
		ViewMode = EDeckViewMode::HeroSelect;
		break;
	default:
		ViewMode = EDeckViewMode::Map;
		break;
	}

	StatusText = NSLOCTEXT("T66Deck.Gameplay", "LoadedRunStatus", "Loaded saved descent.");
	return true;
}

void UT66DeckMainMenuScreen::SubmitLeaderboardProgressIfNeeded()
{
	UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		return;
	}

	const int32 Score = FMath::Max(0, (FloorIndex * 1000) + Gold);
	if (Score <= LastSubmittedLeaderboardScore)
	{
		return;
	}

	const UT66DeckFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckFrontendStateSubsystem>() : nullptr;
	const UT66SteamHelper* SteamHelper = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SteamHelper>() : nullptr;
	const bool bDailyRun = FrontendState && FrontendState->IsDailyRun();
	const FName DifficultyID = FrontendState && FrontendState->GetSelectedDifficultyID() != NAME_None
		? FrontendState->GetSelectedDifficultyID()
		: GetDeckTuning().DifficultyID;

	Backend->SubmitMinigameScore(
		SteamHelper ? SteamHelper->GetLocalDisplayName() : FString(TEXT("Player")),
		TEXT("deck"),
		bDailyRun ? TEXT("daily") : TEXT("alltime"),
		DifficultyID.ToString().ToLower(),
		Score,
		bDailyRun ? FrontendState->GetDailyChallengeId() : FString(),
		bDailyRun ? FrontendState->GetDailySeed() : 0);
	LastSubmittedLeaderboardScore = Score;
}

void UT66DeckMainMenuScreen::RefreshEnemyIntent()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckEnemyDefinition* Enemy = DataSubsystem ? DataSubsystem->FindEnemy(CurrentEnemyID) : nullptr;
	EnemyIntent = Enemy ? Enemy->BaseIntentDamage + FMath::Max(0, FloorIndex - 1) * Enemy->IntentDamagePerFloor : 0;
}

bool UT66DeckMainMenuScreen::IsRunActive() const
{
	return bRunStarted && !bRunDefeated && PlayerHealth > 0;
}

const UT66DeckDataSubsystem* UT66DeckMainMenuScreen::GetDeckDataSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckDataSubsystem>() : nullptr;
}

const FT66DeckTuningDefinition& UT66DeckMainMenuScreen::GetDeckTuning() const
{
	static const FT66DeckTuningDefinition FallbackTuning;
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	return DataSubsystem ? DataSubsystem->GetTuning() : FallbackTuning;
}

FName UT66DeckMainMenuScreen::ResolveStartingHeroID(const UT66DeckDataSubsystem* DataSubsystem) const
{
	const FT66DeckTuningDefinition& Tuning = GetDeckTuning();
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

FName UT66DeckMainMenuScreen::ResolveStartingCompanionID(const UT66DeckDataSubsystem* DataSubsystem) const
{
	const FT66DeckTuningDefinition& Tuning = GetDeckTuning();
	if (Tuning.StartingCompanionID != NAME_None)
	{
		return Tuning.StartingCompanionID;
	}
	if (DataSubsystem && DataSubsystem->GetCompanions().Num() > 0)
	{
		return DataSubsystem->GetCompanions()[0].CompanionID;
	}
	return NAME_None;
}

FName UT66DeckMainMenuScreen::ResolveStartingDeckID(const UT66DeckDataSubsystem* DataSubsystem, const FName HeroID) const
{
	const FT66DeckTuningDefinition& Tuning = GetDeckTuning();
	if (Tuning.StartingDeckID != NAME_None)
	{
		return Tuning.StartingDeckID;
	}
	if (DataSubsystem)
	{
		for (const FT66DeckStartingDeckDefinition& StartingDeck : DataSubsystem->GetStartingDecks())
		{
			if (StartingDeck.HeroID == HeroID || StartingDeck.HeroID == NAME_None)
			{
				return StartingDeck.StartingDeckID;
			}
		}
	}
	return NAME_None;
}

void UT66DeckMainMenuScreen::AddCardToDeck(const FName CardID)
{
	if (CardID != NAME_None)
	{
		DeckCardIDs.Add(CardID);
	}
}

void UT66DeckMainMenuScreen::ApplyRewardItem(const FName ItemID)
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckItemDefinition* Item = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr;
	if (!Item)
	{
		return;
	}

	PlayerMaxHealth += Item->BonusMaxHealth;
	PlayerHealth = FMath::Min(PlayerMaxHealth, PlayerHealth + Item->BonusMaxHealth);
	PlayerBlock += Item->BonusStartingBlock;
}

FText UT66DeckMainMenuScreen::GetEnemyText() const
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckEnemyDefinition* Enemy = DataSubsystem ? DataSubsystem->FindEnemy(CurrentEnemyID) : nullptr;
	return FText::Format(
		NSLOCTEXT("T66Deck.Gameplay", "EnemyValue", "{0} | {1}/{2} HP | Intent {3}"),
		FText::FromString(Enemy ? Enemy->DisplayName : FString(TEXT("Enemy"))),
		FText::AsNumber(FMath::Max(0, EnemyHealth)),
		FText::AsNumber(EnemyMaxHealth),
		FText::AsNumber(EnemyIntent));
}

FText UT66DeckMainMenuScreen::GetPlayerText() const
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckHeroDefinition* Hero = DataSubsystem ? DataSubsystem->FindHero(SelectedHeroID) : nullptr;
	return FText::Format(
		NSLOCTEXT("T66Deck.Gameplay", "PlayerValue", "{0} | {1}/{2} HP | {3} block | {4} gold"),
		FText::FromString(Hero ? Hero->DisplayName : FString(TEXT("Hero"))),
		FText::AsNumber(FMath::Max(0, PlayerHealth)),
		FText::AsNumber(PlayerMaxHealth),
		FText::AsNumber(PlayerBlock),
		FText::AsNumber(Gold));
}

FText UT66DeckMainMenuScreen::GetEnergyText() const
{
	return FText::Format(NSLOCTEXT("T66Deck.Gameplay", "EnergyValue", "{0}/{1}"), FText::AsNumber(Energy), FText::AsNumber(GetDeckTuning().StartingEnergy));
}

FText UT66DeckMainMenuScreen::GetStatusText() const
{
	return StatusText;
}

TOptional<float> UT66DeckMainMenuScreen::GetEnemyHealthPercent() const
{
	if (EnemyMaxHealth <= 0)
	{
		return 0.f;
	}
	return FMath::Clamp(static_cast<float>(EnemyHealth) / static_cast<float>(EnemyMaxHealth), 0.f, 1.f);
}

TOptional<float> UT66DeckMainMenuScreen::GetPlayerHealthPercent() const
{
	if (PlayerMaxHealth <= 0)
	{
		return 0.f;
	}
	return FMath::Clamp(static_cast<float>(PlayerHealth) / static_cast<float>(PlayerMaxHealth), 0.f, 1.f);
}

FReply UT66DeckMainMenuScreen::HandlePlayClicked()
{
	if (UT66DeckFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginNewRun();
	}

	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	if (SelectedHeroID == NAME_None)
	{
		SelectedHeroID = ResolveStartingHeroID(DataSubsystem);
	}
	if (SelectedCompanionID == NAME_None)
	{
		SelectedCompanionID = ResolveStartingCompanionID(DataSubsystem);
	}
	ViewMode = EDeckViewMode::HeroSelect;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleLoadClicked()
{
	const UT66DeckSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckSaveSubsystem>() : nullptr;
	if (!SaveSubsystem)
	{
		return FReply::Handled();
	}

	for (const FT66DeckSaveSlotSummary& Summary : SaveSubsystem->BuildRunSlotSummaries())
	{
		if (!Summary.bOccupied)
		{
			continue;
		}

		if (const UT66DeckRunSaveGame* RunSave = SaveSubsystem->LoadRunFromSlot(Summary.SlotIndex))
		{
			ActiveSaveSlotIndex = Summary.SlotIndex;
			if (RestoreRunFromSave(RunSave))
			{
				ForceRebuildSlate();
				return FReply::Handled();
			}
		}
	}

	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleDailyClicked()
{
	const FName DifficultyID = SharedMenuLayout.IsValid() ? SharedMenuLayout->GetSelectedDifficultyID() : FName(TEXT("Easy"));
	const FString DifficultyToken = DifficultyID.ToString().ToLower();
	const FString DateKey = FDateTime::UtcNow().ToString(TEXT("%Y%m%d"));
	FString ChallengeId = FString::Printf(TEXT("deck-%s-%s"), *DateKey, *DifficultyToken);
	int32 DailySeed = static_cast<int32>(GetTypeHash(ChallengeId));

	if (UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		FT66MinigameDailyChallengeData Challenge;
		const FString ChallengeKey = UT66BackendSubsystem::MakeMinigameDailyChallengeCacheKey(TEXT("deck"), DifficultyToken);
		if (Backend->GetCachedMinigameDailyChallenge(ChallengeKey, Challenge))
		{
			ChallengeId = Challenge.ChallengeId;
			DailySeed = Challenge.RunSeed;
		}
		else if (Backend->IsBackendConfigured())
		{
			Backend->FetchCurrentMinigameDailyChallenge(TEXT("deck"), DifficultyToken);
		}
	}

	if (UT66DeckFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DeckFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginDailyRun(DifficultyID, ChallengeId, DailySeed);
	}

	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	if (SelectedHeroID == NAME_None)
	{
		SelectedHeroID = ResolveStartingHeroID(DataSubsystem);
	}
	if (SelectedCompanionID == NAME_None)
	{
		SelectedCompanionID = ResolveStartingCompanionID(DataSubsystem);
	}
	ViewMode = EDeckViewMode::HeroSelect;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleSelectHeroClicked(const FName HeroID)
{
	SelectedHeroID = HeroID;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleSelectCompanionClicked(const FName CompanionID)
{
	SelectedCompanionID = CompanionID;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleStartDescentClicked()
{
	StartPlayableRun();
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleMapNodeClicked(const FName EncounterID)
{
	EnterEncounter(EncounterID);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleRewardCardClicked(const FName CardID)
{
	AddCardToDeck(CardID);
	++FloorIndex;
	SubmitLeaderboardProgressIfNeeded();
	EnterMap();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleRewardItemClicked(const FName ItemID)
{
	ApplyRewardItem(ItemID);
	++FloorIndex;
	SubmitLeaderboardProgressIfNeeded();
	EnterMap();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleSkipRewardClicked()
{
	++FloorIndex;
	SubmitLeaderboardProgressIfNeeded();
	EnterMap();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleCollectionClicked()
{
	StatusText = NSLOCTEXT("T66Deck.MainMenu", "CollectionStatus", "Collection view is deferred; the playable combat loop is wired first.");
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleOptionsClicked()
{
	StatusText = NSLOCTEXT("T66Deck.MainMenu", "OptionsStatus", "Options inherit the main settings flow for now.");
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleGameplayBackClicked()
{
	SaveCurrentRunState();
	ViewMode = EDeckViewMode::MainMenu;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleCardClicked(const int32 CardIndex)
{
	if (!IsRunActive() || !Hand.IsValidIndex(CardIndex))
	{
		return FReply::Handled();
	}

	const FRuntimeCard Card = Hand[CardIndex];
	if (Card.Cost > Energy)
	{
		StatusText = NSLOCTEXT("T66Deck.Gameplay", "NoEnergyStatus", "Not enough energy.");
		return FReply::Handled();
	}

	Energy -= Card.Cost;
	if (Card.Damage > 0)
	{
		EnemyHealth -= Card.Damage;
	}
	if (Card.Block > 0)
	{
		PlayerBlock += Card.Block;
	}

	StatusText = FText::Format(
		NSLOCTEXT("T66Deck.Gameplay", "PlayedCardStatus", "Played {0}."),
		Card.Name);
	Hand.RemoveAt(CardIndex);
	SaveCurrentRunState();

	if (EnemyHealth <= 0)
	{
		CompleteEnemy();
	}

	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleEndTurnClicked()
{
	if (!IsRunActive())
	{
		return FReply::Handled();
	}

	const int32 DamageTaken = FMath::Max(0, EnemyIntent - PlayerBlock);
	PlayerHealth -= DamageTaken;
	PlayerBlock = 0;
	Energy = GetDeckTuning().StartingEnergy;
	RefreshEnemyIntent();
	BuildHandFromDeck();

	if (PlayerHealth <= 0)
	{
		bRunDefeated = true;
		SubmitLeaderboardProgressIfNeeded();
		StatusText = NSLOCTEXT("T66Deck.Gameplay", "DefeatedStatus", "Defeated. Return to the menu and start another run.");
		SaveCurrentRunState();
	}
	else
	{
		StatusText = FText::Format(
			NSLOCTEXT("T66Deck.Gameplay", "EndTurnStatus", "Enemy hit for {0}. New hand drawn."),
			FText::AsNumber(DamageTaken));
		SaveCurrentRunState();
	}

	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleBackClicked()
{
	NavigateTo(ET66ScreenType::Minigames);
	return FReply::Handled();
}

FReply UT66DeckMainMenuScreen::HandleNewRunClicked()
{
	return HandlePlayClicked();
}
