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
#include "Styling/SlateBrush.h"
#include "UI/Components/T66MinigameMenuLayout.h"
#include "UI/Style/T66AnimatedStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UITypes.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const TCHAR* DeckMainMenuMockupPath()
	{
		return TEXT("SourceAssets/Deck/Backgrounds/Deck_MainMenu_Backdrop.png");
	}

	const TCHAR* DeckGameplayMockupPath()
	{
		return TEXT("SourceAssets/Deck/Backgrounds/Deck_Gameplay_Backdrop.png");
	}

	TAttribute<FText> MakeDeckTextAttribute(UT66DeckMainMenuScreen* Screen, FText (UT66DeckMainMenuScreen::*Getter)() const)
	{
		return TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(Screen, Getter));
	}

	TAttribute<TOptional<float>> MakeDeckPercentAttribute(UT66DeckMainMenuScreen* Screen, TOptional<float> (UT66DeckMainMenuScreen::*Getter)() const)
	{
		return TAttribute<TOptional<float>>::Create(TAttribute<TOptional<float>>::FGetter::CreateUObject(Screen, Getter));
	}

	FName MakeDeckTag(const TCHAR* Prefix, const FText& Label)
	{
		FString Token = Label.ToString().ToUpper();
		for (int32 Index = Token.Len() - 1; Index >= 0; --Index)
		{
			if (!FChar::IsAlnum(Token[Index]))
			{
				Token.RemoveAt(Index, 1, EAllowShrinking::No);
			}
		}

		if (Token.IsEmpty())
		{
			Token = TEXT("ITEM");
		}

		return FName(*(FString(Prefix) + TEXT(".") + Token));
	}

	TSharedRef<SWidget> MakeDeckChromePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding, const FLinearColor& Accent)
	{
		const ET66FlatState PanelState = Accent.R > 0.45f ? ET66FlatState::Selected : ET66FlatState::Default;
		return FT66FlatStyle::MakeFlatPanel(PanelState, Padding, Content);
	}

	TSharedPtr<FSlateBrush> FindOrLoadDeckLooseBrush(const FString& RelativePath)
	{
		static TMap<FString, TSharedPtr<FSlateBrush>> Brushes;
		static TArray<TStrongObjectPtr<UTexture2D>> RetainedTextures;

		if (RelativePath.IsEmpty())
		{
			return nullptr;
		}

		if (const TSharedPtr<FSlateBrush>* ExistingBrush = Brushes.Find(RelativePath))
		{
			return *ExistingBrush;
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Nearest, TEXT("DeckLooseTexture")))
			{
				RetainedTextures.Emplace(Texture);
				TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
				Brush->SetResourceObject(Texture);
				Brush->DrawAs = ESlateBrushDrawType::Image;
				Brush->Tiling = ESlateBrushTileType::NoTile;
				Brush->ImageSize = FVector2D(FMath::Max(1, Texture->GetSizeX()), FMath::Max(1, Texture->GetSizeY()));
				Brushes.Add(RelativePath, Brush);
				return Brush;
			}
		}

		Brushes.Add(RelativePath, nullptr);
		return nullptr;
	}

	TSharedRef<SWidget> MakeDeckLooseSprite(const FString& RelativePath, const FVector2D& Size, const FLinearColor& FallbackTint, const FName Tag)
	{
		TSharedRef<SWidget> SpriteContent = SNew(SSpacer);
		if (const TSharedPtr<FSlateBrush> Brush = FindOrLoadDeckLooseBrush(RelativePath))
		{
			SpriteContent = SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SImage)
					.Image(Brush.Get())
					.ColorAndOpacity(FLinearColor::White)
				];
		}

		TSharedRef<SWidget> Sprite = SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FallbackTint)
				.Padding(FMargin(0.f))
				[
					SpriteContent
				]
			];
		return FT66AnimatedStyle::AttachMetadata(Sprite, Tag, TEXT("Deck.Sprite"));
	}

	TSharedRef<SWidget> MakeDeckAnimatedProgressBar(const TAttribute<TOptional<float>>& Percent, const FName Tag)
	{
		TSharedRef<SWidget> ProgressBar = SNew(SProgressBar)
			.Percent(Percent);
		return FT66AnimatedStyle::AttachMetadata(ProgressBar, Tag, TEXT("Deck.ProgressBar"));
	}

	FString MakeDeckCardIconPath(const FName CardID)
	{
		return FString::Printf(TEXT("SourceAssets/Deck/Cards/Singles/Deck_Card_%s.png"), *CardID.ToString());
	}

	FText MakeDeckCardTypeText(const ET66DeckCardType CardType)
	{
		switch (CardType)
		{
		case ET66DeckCardType::Skill:
			return NSLOCTEXT("T66Deck.Gameplay", "CardTypeSkill", "SKILL");
		case ET66DeckCardType::Power:
			return NSLOCTEXT("T66Deck.Gameplay", "CardTypePower", "POWER");
		case ET66DeckCardType::Curse:
			return NSLOCTEXT("T66Deck.Gameplay", "CardTypeCurse", "CURSE");
		case ET66DeckCardType::Attack:
		default:
			return NSLOCTEXT("T66Deck.Gameplay", "CardTypeAttack", "ATTACK");
		}
	}

	FLinearColor MakeDeckCardTypeColor(const ET66DeckCardType CardType)
	{
		switch (CardType)
		{
		case ET66DeckCardType::Skill:
			return FLinearColor(0.22f, 0.48f, 0.90f, 1.0f);
		case ET66DeckCardType::Power:
			return FLinearColor(0.76f, 0.38f, 0.90f, 1.0f);
		case ET66DeckCardType::Curse:
			return FLinearColor(0.34f, 0.24f, 0.44f, 1.0f);
		case ET66DeckCardType::Attack:
		default:
			return FLinearColor(0.90f, 0.26f, 0.22f, 1.0f);
		}
	}

	FText MakeDeckRuntimeRulesText(const FString& FallbackRules, const int32 Damage, const int32 Block)
	{
		if (Damage > 0 && Block > 0)
		{
			return FText::Format(
				NSLOCTEXT("T66Deck.Gameplay", "CardRulesDamageAndBlock", "Deal {0} damage.\nGain {1} block."),
				FText::AsNumber(Damage),
				FText::AsNumber(Block));
		}
		if (Damage > 0)
		{
			return FText::Format(
				NSLOCTEXT("T66Deck.Gameplay", "CardRulesDamage", "Deal {0} damage."),
				FText::AsNumber(Damage));
		}
		if (Block > 0)
		{
			return FText::Format(
				NSLOCTEXT("T66Deck.Gameplay", "CardRulesBlock", "Gain {0} block."),
				FText::AsNumber(Block));
		}
		return FText::FromString(FallbackRules);
	}

	TSharedRef<SWidget> MakeDeckCardBadge(const FText& Label, const FText& Value, const FLinearColor& Accent)
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.06f, 0.92f))
			.Padding(FMargin(6.f, 4.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 5.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::MakeFont(TEXT("Bold"), 9))
					.ColorAndOpacity(Accent)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Value)
					.Font(FT66Style::MakeFont(TEXT("Bold"), 12))
					.ColorAndOpacity(FLinearColor(0.96f, 0.92f, 0.84f, 1.0f))
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
	const bool bStartGameplay = FParse::Param(FCommandLine::Get(), TEXT("T66DeckStartGameplay")) || FParse::Param(FCommandLine::Get(), TEXT("T66DeckStartCombat"));
	if (!bAppliedAutomationStart && bStartGameplay)
	{
		bAppliedAutomationStart = true;
		StartPlayableRun();
		if (FParse::Param(FCommandLine::Get(), TEXT("T66DeckStartCombat")))
		{
			EnterFirstAvailableEncounterForAutomation();
		}
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
	case EDeckViewMode::Summary:
		return BuildSummaryUI();
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
		.Title(NSLOCTEXT("T66Deck.MainMenu", "DeckSharedTitle", "CHADPOCALYPSE DECK BUILDER"))
		.Subtitle(NSLOCTEXT("T66Deck.MainMenu", "DeckSharedSubtitle", "Single-player dungeon descent"))
		.DailyTitle(NSLOCTEXT("T66Deck.MainMenu", "DeckDailyTitle", "TODAY'S DUNGEON"))
		.DailyBody(NSLOCTEXT("T66Deck.MainMenu", "DeckDailyBody", "One seeded descent per day. Choose a difficulty, draft through the dungeon, and post the best clear score."))
		.DailyRules(NSLOCTEXT("T66Deck.MainMenu", "DeckDailyRules", "Single player only.\nDaily score uses today's map and reward seed.\nLeaderboard ranks by score.\nDifficulty is the only rules toggle."))
		.AccentColor(FLinearColor(0.86f, 0.22f, 0.24f, 1.0f))
		.BackgroundColor(FLinearColor(0.026f, 0.014f, 0.016f, 1.0f))
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
				FLinearColor(0.50f, 0.14f, 0.18f, 0.88f))
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
	const FString HeroSpritePath = FString::Printf(TEXT("SourceAssets/Deck/Heroes/Singles/Deck_%s.png"), *SelectedHeroID.ToString());
	const FString EnemySpritePath = FString::Printf(
		TEXT("SourceAssets/Deck/Enemies/Singles/Deck_%s.png"),
		*(CurrentEnemyID != NAME_None ? CurrentEnemyID.ToString() : FString(TEXT("Dungeon_Fiend"))));

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
							.ColorAndOpacity(FLinearColor(0.98f, 0.84f, 0.82f, 1.0f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
						[
							MakeDeckAnimatedProgressBar(MakeDeckPercentAttribute(this, &UT66DeckMainMenuScreen::GetPlayerHealthPercent), FName(TEXT("Deck.Gameplay.PlayerHealthProgress")))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
						[
							MakeMeterPanel(NSLOCTEXT("T66Deck.Gameplay", "EnergyLabel", "ENERGY"), MakeDeckTextAttribute(this, &UT66DeckMainMenuScreen::GetEnergyText), FLinearColor(0.86f, 0.20f, 0.24f, 0.92f))
						],
						FMargin(20.f),
						FLinearColor(0.50f, 0.14f, 0.18f, 0.88f))
				]
				+ SHorizontalBox::Slot().FillWidth(0.40f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						MakeDeckLooseSprite(HeroSpritePath, FVector2D(128.f, 128.f), FLinearColor(0.30f, 0.16f, 0.18f, 0.72f), FName(TEXT("Deck.Gameplay.HeroSprite")))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(22.f, 0.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Deck.Gameplay", "BattleDivider", "VS"))
						.Font(FT66Style::MakeFont(TEXT("Black"), 22))
						.ColorAndOpacity(FLinearColor(0.94f, 0.78f, 0.48f, 1.0f))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						MakeDeckLooseSprite(EnemySpritePath, FVector2D(144.f, 144.f), FLinearColor(0.36f, 0.10f, 0.10f, 0.72f), FName(TEXT("Deck.Gameplay.EnemySprite")))
					]
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
							MakeDeckAnimatedProgressBar(MakeDeckPercentAttribute(this, &UT66DeckMainMenuScreen::GetEnemyHealthPercent), FName(TEXT("Deck.Gameplay.EnemyHealthProgress")))
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
					FLinearColor(0.42f, 0.16f, 0.18f, 0.88f))
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
					FLinearColor(0.42f, 0.14f, 0.16f, 0.88f))
			]
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildRewardUI()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	TSharedRef<SVerticalBox> RewardList = SNew(SVerticalBox);
	TSharedRef<SHorizontalBox> RewardCardRow = SNew(SHorizontalBox);
	bool bHasRewardCards = false;

	for (const FName CardID : RewardCardIDs)
	{
		if (const FT66DeckCardDefinition* Card = DataSubsystem ? DataSubsystem->FindCard(CardID) : nullptr)
		{
			bHasRewardCards = true;
			RewardCardRow->AddSlot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
			[
				MakeRewardCardWidget(*Card)
			];
		}
	}

	if (bHasRewardCards)
	{
		RewardList->AddSlot().AutoHeight()
		[
			RewardCardRow
		];
	}

	for (const FName ItemID : RewardItemIDs)
	{
		if (const FT66DeckItemDefinition* Item = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr)
		{
			RewardList->AddSlot().AutoHeight().Padding(0.f, bHasRewardCards ? 18.f : 0.f, 0.f, 10.f)
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

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildSummaryUI()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const FT66DeckStageDefinition* StageDefinition = DataSubsystem ? DataSubsystem->FindStageForFloor(FloorIndex) : nullptr;
	const int32 FinalFloor = StageDefinition ? FMath::Max(StageDefinition->BossFloor, FloorIndex) : 10;
	const int32 FloorReached = FMath::Clamp(FloorIndex, 1, FMath::Max(1, FinalFloor));
	const bool bVictory = !bRunDefeated && StageDefinition && StageDefinition->NextStageID.IsNone() && FloorIndex >= StageDefinition->BossFloor;
	const FText ResultTitle = bVictory
		? NSLOCTEXT("T66Deck.Summary", "VictoryTitle", "DESCENT CLEARED")
		: NSLOCTEXT("T66Deck.Summary", "DefeatTitle", "RUN ENDED");
	const FText ResultBody = bVictory
		? NSLOCTEXT("T66Deck.Summary", "VictoryBody", "Floor 10 boss defeated. The run is closed and ready for a new descent.")
		: NSLOCTEXT("T66Deck.Summary", "DefeatBody", "The descent ended early. Start a new run from the deck menu when ready.");

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BuildMockupBackdrop(DeckGameplayMockupPath(), FLinearColor(0.012f, 0.012f, 0.018f, 1.0f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
		]
		+ SOverlay::Slot().Padding(FMargin(170.f, 88.f, 170.f, 82.f))
		[
			MakeDeckChromePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(ResultTitle)
					.Font(FT66Style::MakeFont(TEXT("Black"), 30))
					.ColorAndOpacity(bVictory ? FLinearColor(0.92f, 0.82f, 0.42f, 1.0f) : FLinearColor(0.92f, 0.34f, 0.30f, 1.0f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 24.f)
				[
					SNew(STextBlock)
					.Text(ResultBody)
					.Font(FT66Style::MakeFont(TEXT("Regular"), 15))
					.ColorAndOpacity(FLinearColor(0.86f, 0.82f, 0.76f, 1.0f))
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeMeterPanel(NSLOCTEXT("T66Deck.Summary", "FloorLabel", "FLOOR"), FText::Format(NSLOCTEXT("T66Deck.Summary", "FloorValue", "{0}/{1}"), FText::AsNumber(FloorReached), FText::AsNumber(FinalFloor)), FLinearColor(0.80f, 0.58f, 0.26f, 1.0f))
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeMeterPanel(NSLOCTEXT("T66Deck.Summary", "GoldLabel", "GOLD"), FText::AsNumber(Gold), FLinearColor(0.92f, 0.76f, 0.28f, 1.0f))
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeMeterPanel(NSLOCTEXT("T66Deck.Summary", "DeckLabel", "DECK"), FText::Format(NSLOCTEXT("T66Deck.Summary", "DeckValue", "{0} cards"), FText::AsNumber(DeckCardIDs.Num())), FLinearColor(0.50f, 0.78f, 0.90f, 1.0f))
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						MakeMeterPanel(NSLOCTEXT("T66Deck.Summary", "StagesLabel", "BOSS"), FText::AsNumber(HighestStageIndexCleared), FLinearColor(0.72f, 0.86f, 0.40f, 1.0f))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 26.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 14.f, 0.f)
					[
						MakeDeckButton(NSLOCTEXT("T66Deck.Summary", "NewRun", "NEW RUN"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleNewRunClicked), 220.f, 56.f)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeDeckButton(NSLOCTEXT("T66Deck.Summary", "DeckMenu", "DECK MENU"), FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleGameplayBackClicked), 220.f, 56.f)
					]
				],
				FMargin(28.f),
				bVictory ? FLinearColor(0.34f, 0.28f, 0.14f, 0.92f) : FLinearColor(0.40f, 0.14f, 0.16f, 0.92f))
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::BuildMockupBackdrop(const FString& SourceRelativePath, const FLinearColor& FallbackColor) const
{
	if (const TSharedPtr<FSlateBrush> Brush = FindOrLoadDeckLooseBrush(SourceRelativePath))
	{
		return SNew(SScaleBox)
			.Stretch(EStretch::Fill)
			[
				SNew(SImage)
				.Image(Brush.Get())
				.ColorAndOpacity(FLinearColor::White)
			];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor);
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeDeckButton(const FText& Text, const FOnClicked& OnClicked, const float Width, const float Height, const FName Tag) const
{
	const FName EffectiveTag = Tag != NAME_None ? Tag : MakeDeckTag(TEXT("Deck.Button"), Text);
	return SNew(SBox)
		.WidthOverride(Width)
		.HeightOverride(Height)
		[
			FT66FlatStyle::MakeFlatButton(
				ET66FlatState::Default,
				Text,
				OnClicked,
				nullptr,
				nullptr,
				FMargin(14.f, 8.f),
				Width,
				Height,
				true,
				14,
				EffectiveTag)
		];
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeChoiceButton(const FText& Title, const FText& Body, const FLinearColor& Accent, const FOnClicked& OnClicked) const
{
	const ET66FlatState State = Accent.R > 0.70f || Accent.G > 0.70f ? ET66FlatState::Selected : ET66FlatState::Default;
	return FT66FlatStyle::MakeFlatToggleGroupButton(
		State,
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
		OnClicked,
		FMargin(14.f),
		0.f,
		0.f,
		true,
		MakeDeckTag(TEXT("Deck.Choice"), Title),
		FName(TEXT("DeckChoiceSelection")));
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeCardWidget(const int32 CardIndex)
{
	const bool bHasCard = Hand.IsValidIndex(CardIndex);
	const FRuntimeCard Card = bHasCard ? Hand[CardIndex] : FRuntimeCard();
	const bool bCanPlay = bHasCard && !bRunDefeated && Card.Cost <= Energy;

	return MakeCardPreviewWidget(
		bHasCard ? Card.CardID : NAME_None,
		bHasCard ? Card.Name : NSLOCTEXT("T66Deck.Gameplay", "EmptyCard", "EMPTY"),
		bHasCard ? Card.Rules : FText::GetEmpty(),
		bHasCard ? Card.CardType : ET66DeckCardType::Curse,
		bHasCard ? Card.Cost : 0,
		bHasCard ? Card.Damage : 0,
		bHasCard ? Card.Block : 0,
		bHasCard ? Card.RarityID : NAME_None,
		bHasCard ? Card.Accent : FLinearColor(0.24f, 0.22f, 0.26f, 0.92f),
		FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleCardClicked, CardIndex),
		bCanPlay,
		FName(*FString::Printf(TEXT("Deck.Card.%d"), CardIndex)));
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeRewardCardWidget(const FT66DeckCardDefinition& CardDefinition)
{
	return MakeCardPreviewWidget(
		CardDefinition.CardID,
		FText::FromString(CardDefinition.DisplayName),
		MakeDeckRuntimeRulesText(CardDefinition.RulesText, CardDefinition.Damage, CardDefinition.Block),
		CardDefinition.CardType,
		CardDefinition.EnergyCost,
		CardDefinition.Damage,
		CardDefinition.Block,
		CardDefinition.RarityID,
		CardDefinition.AccentColor,
		FOnClicked::CreateUObject(this, &UT66DeckMainMenuScreen::HandleRewardCardClicked, CardDefinition.CardID),
		true,
		FName(*FString::Printf(TEXT("Deck.RewardCard.%s"), *CardDefinition.CardID.ToString())));
}

TSharedRef<SWidget> UT66DeckMainMenuScreen::MakeCardPreviewWidget(
	const FName CardID,
	const FText& Name,
	const FText& Rules,
	const ET66DeckCardType CardType,
	const int32 EnergyCost,
	const int32 Damage,
	const int32 Block,
	const FName RarityID,
	const FLinearColor& Accent,
	const FOnClicked& OnClicked,
	const bool bCanUse,
	const FName Tag) const
{
	const FLinearColor TypeColor = MakeDeckCardTypeColor(CardType);
	const FLinearColor FrameColor = bCanUse ? Accent : FLinearColor(0.20f, 0.20f, 0.24f, 0.90f);
	const FLinearColor ArtTint = bCanUse ? FLinearColor::White : FLinearColor(0.42f, 0.42f, 0.46f, 1.0f);
	const FText RarityText = RarityID.IsNone() ? NSLOCTEXT("T66Deck.Gameplay", "CardRarityCommonFallback", "COMMON") : FText::FromName(RarityID);
	const bool bHasArt = CardID != NAME_None;

	TSharedRef<SHorizontalBox> BadgeRow = SNew(SHorizontalBox);
	if (Damage > 0)
	{
		BadgeRow->AddSlot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
		[
			MakeDeckCardBadge(NSLOCTEXT("T66Deck.Gameplay", "DamageBadge", "DMG"), FText::AsNumber(Damage), FLinearColor(0.96f, 0.32f, 0.24f, 1.0f))
		];
	}
	if (Block > 0)
	{
		BadgeRow->AddSlot().AutoWidth()
		[
			MakeDeckCardBadge(NSLOCTEXT("T66Deck.Gameplay", "BlockBadge", "BLK"), FText::AsNumber(Block), FLinearColor(0.34f, 0.62f, 1.0f, 1.0f))
		];
	}
	if (Damage <= 0 && Block <= 0)
	{
		BadgeRow->AddSlot().FillWidth(1.f)
		[
			SNew(SSpacer)
		];
	}

	TSharedRef<SWidget> CardArt = SNew(SSpacer);
	if (bHasArt)
	{
		CardArt = SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				SNew(SImage)
				.Image(FindOrLoadDeckLooseBrush(MakeDeckCardIconPath(CardID)).Get())
				.ColorAndOpacity(ArtTint)
			];
	}
	CardArt = FT66AnimatedStyle::AttachMetadata(
		CardArt,
		FName(*(Tag.ToString() + TEXT(".Art"))),
		TEXT("Deck.CardArt"));

	return SNew(SBox)
		.WidthOverride(210.f)
		.HeightOverride(304.f)
		[
			FT66FlatStyle::MakeFlatToggleGroupButton(
				bCanUse ? ET66FlatState::Default : ET66FlatState::Disabled,
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FrameColor)
					.Padding(FMargin(5.f))
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.045f, 0.042f, 0.052f, 0.98f))
						.Padding(FMargin(9.f, 8.f, 9.f, 9.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(28.f, 0.f, 0.f, 6.f)
							[
								SNew(STextBlock)
								.Text(Name)
								.Font(FT66Style::MakeFont(TEXT("Black"), 16))
								.ColorAndOpacity(bCanUse ? FLinearColor(0.96f, 0.90f, 0.82f, 1.0f) : FLinearColor(0.58f, 0.58f, 0.62f, 1.0f))
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.025f, 0.024f, 0.030f, 1.0f))
								.Padding(FMargin(3.f))
								[
									SNew(SBox)
									.HeightOverride(112.f)
									[
										CardArt
									]
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 7.f, 0.f, 7.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(TypeColor * (bCanUse ? 0.88f : 0.42f))
								.Padding(FMargin(8.f, 3.f))
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(MakeDeckCardTypeText(CardType))
										.Font(FT66Style::MakeFont(TEXT("Bold"), 10))
										.ColorAndOpacity(FLinearColor(0.98f, 0.96f, 0.90f, 1.0f))
									]
									+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(RarityText)
										.Font(FT66Style::MakeFont(TEXT("Bold"), 9))
										.ColorAndOpacity(FLinearColor(0.98f, 0.92f, 0.62f, 1.0f))
									]
								]
							]
							+ SVerticalBox::Slot().FillHeight(1.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.075f, 0.070f, 0.082f, 0.96f))
								.Padding(FMargin(8.f, 6.f))
								[
									SNew(STextBlock)
									.Text(Rules)
									.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
									.ColorAndOpacity(bCanUse ? FLinearColor(0.88f, 0.84f, 0.78f, 1.0f) : FLinearColor(0.54f, 0.54f, 0.58f, 1.0f))
									.AutoWrapText(true)
									.Justification(ETextJustify::Center)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 7.f, 0.f, 0.f)
							[
								BadgeRow
							]
						]
					]
				]
				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(0.f)
				[
					SNew(SBox)
					.WidthOverride(38.f)
					.HeightOverride(38.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bCanUse ? FLinearColor(0.16f, 0.36f, 0.92f, 1.0f) : FLinearColor(0.16f, 0.16f, 0.20f, 1.0f))
						.Padding(FMargin(0.f))
						[
							SNew(STextBlock)
							.Text(FText::AsNumber(EnergyCost))
							.Font(FT66Style::MakeFont(TEXT("Black"), 19))
							.ColorAndOpacity(FLinearColor::White)
							.Justification(ETextJustify::Center)
						]
					]
				],
				OnClicked,
				FMargin(0.f),
				210.f,
				304.f,
				bCanUse,
				Tag,
				FName(TEXT("DeckHandSelection")))
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
		Card.Cost = Definition->EnergyCost;
		Card.Damage = Definition->Damage > 0 ? Definition->Damage + BonusDamage : 0;
		Card.Block = Definition->Block;
		Card.Rules = MakeDeckRuntimeRulesText(Definition->RulesText, Card.Damage, Card.Block);
		Card.CardType = Definition->CardType;
		Card.RarityID = Definition->RarityID;
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

void UT66DeckMainMenuScreen::EnterFirstAvailableEncounterForAutomation()
{
	const UT66DeckDataSubsystem* DataSubsystem = GetDeckDataSubsystem();
	const TArray<const FT66DeckEncounterDefinition*> Encounters = DataSubsystem
		? DataSubsystem->GetEncountersForFloor(FloorIndex)
		: TArray<const FT66DeckEncounterDefinition*>();
	if (Encounters.Num() > 0 && Encounters[0])
	{
		EnterEncounter(Encounters[0]->EncounterID);
	}
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
	if (bBossClear && StageDefinition && StageDefinition->NextStageID.IsNone() && FloorIndex >= StageDefinition->BossFloor)
	{
		StatusText = FText::Format(
			NSLOCTEXT("T66Deck.Gameplay", "FinalEnemyClearStatus", "Final boss cleared. Gained {0} gold."),
			FText::AsNumber(Reward));
		FinishRun(true);
		return;
	}

	StatusText = FText::Format(
		NSLOCTEXT("T66Deck.Gameplay", "EnemyClearStatus", "Room cleared. Gained {0} gold. Floor {1} begins."),
		FText::AsNumber(Reward),
		FText::AsNumber(FloorIndex));
	ViewMode = EDeckViewMode::Reward;
	SaveCurrentRunState();
	ForceRebuildSlate();
}

void UT66DeckMainMenuScreen::FinishRun(const bool bWasVictory)
{
	bRunDefeated = !bWasVictory;
	RewardCardIDs.Reset();
	RewardItemIDs.Reset();
	SubmitLeaderboardProgressIfNeeded();
	ViewMode = EDeckViewMode::Summary;
	SaveCurrentRunState();
	bRunStarted = false;
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
	case static_cast<int32>(EDeckViewMode::Summary):
		ViewMode = EDeckViewMode::Summary;
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
		StatusText = NSLOCTEXT("T66Deck.Gameplay", "DefeatedStatus", "Defeated. Return to the menu and start another run.");
		FinishRun(false);
		return FReply::Handled();
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
