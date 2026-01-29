// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AchievementsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

UT66AchievementsScreen::UT66AchievementsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Achievements;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66AchievementsScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

void UT66AchievementsScreen::GeneratePlaceholderAchievements()
{
	AllAchievements.Empty();

	// Black tier (25 AC each)
	auto AddBlack = [this](const FString& ID, const FString& Name, const FString& Desc, int32 Req, int32 Prog, bool bDone)
	{
		FAchievementData A;
		A.AchievementID = FName(*ID);
		A.DisplayName = FText::FromString(Name);
		A.Description = FText::FromString(Desc);
		A.Tier = ET66AchievementTier::Black;
		A.RewardCoins = 25;
		A.RequirementCount = Req;
		A.CurrentProgress = Prog;
		A.bIsUnlocked = bDone;
		AllAchievements.Add(A);
	};

	AddBlack(TEXT("ACH_BLK_001"), TEXT("First Blood"), TEXT("Kill 1 enemy."), 1, 1, true);
	AddBlack(TEXT("ACH_BLK_002"), TEXT("Wave Clear"), TEXT("Clear your first wave."), 1, 1, true);
	AddBlack(TEXT("ACH_BLK_003"), TEXT("Gate Crasher"), TEXT("Cross the stage gate and start the stage timer."), 1, 1, true);
	AddBlack(TEXT("ACH_BLK_004"), TEXT("Summoning Stone"), TEXT("Summon a stage boss using the Summoning Stone."), 1, 0, false);
	AddBlack(TEXT("ACH_BLK_005"), TEXT("Item Pickup"), TEXT("Pick up 10 items."), 10, 7, false);
	AddBlack(TEXT("ACH_BLK_006"), TEXT("Gold Hoarder"), TEXT("Collect 1000 gold in one run."), 1000, 450, false);
	AddBlack(TEXT("ACH_BLK_007"), TEXT("Vendor Visit"), TEXT("Visit the Black House vendor."), 1, 1, true);
	AddBlack(TEXT("ACH_BLK_008"), TEXT("Tree of Life"), TEXT("Interact with the Tree of Life."), 1, 0, false);
	AddBlack(TEXT("ACH_BLK_009"), TEXT("First Dodge"), TEXT("Successfully dodge an enemy attack."), 1, 1, true);
	AddBlack(TEXT("ACH_BLK_010"), TEXT("Ultimate User"), TEXT("Use your Ultimate ability."), 1, 1, true);

	// Red tier (100 AC each)
	auto AddRed = [this](const FString& ID, const FString& Name, const FString& Desc, int32 Req, int32 Prog, bool bDone)
	{
		FAchievementData A;
		A.AchievementID = FName(*ID);
		A.DisplayName = FText::FromString(Name);
		A.Description = FText::FromString(Desc);
		A.Tier = ET66AchievementTier::Red;
		A.RewardCoins = 100;
		A.RequirementCount = Req;
		A.CurrentProgress = Prog;
		A.bIsUnlocked = bDone;
		AllAchievements.Add(A);
	};

	AddRed(TEXT("ACH_RED_001"), TEXT("Clean Stage"), TEXT("Clear a stage without taking any damage."), 1, 0, false);
	AddRed(TEXT("ACH_RED_002"), TEXT("Perfect Wave"), TEXT("Clear 10 waves without taking any damage."), 10, 3, false);
	AddRed(TEXT("ACH_RED_003"), TEXT("Boss Rush"), TEXT("Defeat a boss before the stage timer reaches 4:00."), 1, 0, false);
	AddRed(TEXT("ACH_RED_004"), TEXT("Last-Second Escape"), TEXT("Defeat a boss with 10 seconds or less left."), 1, 0, false);
	AddRed(TEXT("ACH_RED_005"), TEXT("Stage 10 Clear"), TEXT("Complete Stage 10."), 1, 1, true);
	AddRed(TEXT("ACH_RED_006"), TEXT("Stage 20 Clear"), TEXT("Complete Stage 20."), 1, 0, false);
	AddRed(TEXT("ACH_RED_007"), TEXT("Gambler's Friend"), TEXT("Win 5 bets at the Yellow House."), 5, 2, false);
	AddRed(TEXT("ACH_RED_008"), TEXT("Item Synergy"), TEXT("Trigger an item synergy effect."), 1, 0, false);
	AddRed(TEXT("ACH_RED_009"), TEXT("Companion Healed"), TEXT("Be healed by your companion 50 times."), 50, 23, false);
	AddRed(TEXT("ACH_RED_010"), TEXT("Dodge Master"), TEXT("Dodge 100 attacks total."), 100, 67, false);

	// Yellow tier (250 AC each)
	auto AddYellow = [this](const FString& ID, const FString& Name, const FString& Desc, int32 Req, int32 Prog, bool bDone)
	{
		FAchievementData A;
		A.AchievementID = FName(*ID);
		A.DisplayName = FText::FromString(Name);
		A.Description = FText::FromString(Desc);
		A.Tier = ET66AchievementTier::Yellow;
		A.RewardCoins = 250;
		A.RequirementCount = Req;
		A.CurrentProgress = Prog;
		A.bIsUnlocked = bDone;
		AllAchievements.Add(A);
	};

	AddYellow(TEXT("ACH_YEL_001"), TEXT("Ten Stages Untouched"), TEXT("Clear 10 stages in a row without taking damage."), 10, 0, false);
	AddYellow(TEXT("ACH_YEL_002"), TEXT("One-Heart Champion"), TEXT("Kill a stage boss while on exactly 1 Heart."), 1, 0, false);
	AddYellow(TEXT("ACH_YEL_003"), TEXT("Miasma Champion"), TEXT("Kill a boss with 15 seconds or less left."), 1, 0, false);
	AddYellow(TEXT("ACH_YEL_004"), TEXT("66-Second Stage"), TEXT("Clear a stage in under 66 seconds."), 1, 0, false);
	AddYellow(TEXT("ACH_YEL_005"), TEXT("Stage 40 Clear"), TEXT("Complete Stage 40."), 1, 0, false);
	AddYellow(TEXT("ACH_YEL_006"), TEXT("Stage 50 Clear"), TEXT("Complete Stage 50."), 1, 0, false);
	AddYellow(TEXT("ACH_YEL_007"), TEXT("All Heroes Played"), TEXT("Complete a run with each hero."), 5, 2, false);
	AddYellow(TEXT("ACH_YEL_008"), TEXT("Legendary Drop"), TEXT("Obtain a Yellow rarity item."), 1, 0, false);
	AddYellow(TEXT("ACH_YEL_009"), TEXT("Debt Collector"), TEXT("Pay off a 5000+ gold debt to the Vendor."), 1, 0, false);
	AddYellow(TEXT("ACH_YEL_010"), TEXT("Item Hoarder"), TEXT("Collect 30 items in one run."), 30, 12, false);

	// White tier (1000 AC each)
	auto AddWhite = [this](const FString& ID, const FString& Name, const FString& Desc, int32 Req, int32 Prog, bool bDone)
	{
		FAchievementData A;
		A.AchievementID = FName(*ID);
		A.DisplayName = FText::FromString(Name);
		A.Description = FText::FromString(Desc);
		A.Tier = ET66AchievementTier::White;
		A.RewardCoins = 1000;
		A.RequirementCount = Req;
		A.CurrentProgress = Prog;
		A.bIsUnlocked = bDone;
		AllAchievements.Add(A);
	};

	AddWhite(TEXT("ACH_WHI_001"), TEXT("Kromer Salvation"), TEXT("Beat Stage 66 and give the kromer to the Saint."), 1, 0, false);
	AddWhite(TEXT("ACH_WHI_002"), TEXT("Betrayal Proof"), TEXT("Beat Stage 66 with NO COMPANION equipped."), 1, 0, false);
	AddWhite(TEXT("ACH_WHI_003"), TEXT("Final Difficulty Conqueror"), TEXT("Beat Stage 66 on Final difficulty."), 1, 0, false);
	AddWhite(TEXT("ACH_WHI_004"), TEXT("Unscathed To The End"), TEXT("Beat Stage 66 without taking damage."), 1, 0, false);
	AddWhite(TEXT("ACH_WHI_005"), TEXT("Speed Demon"), TEXT("Beat Stage 66 in under 60 minutes."), 1, 0, false);
	AddWhite(TEXT("ACH_WHI_006"), TEXT("Minimalist"), TEXT("Beat Stage 66 with 5 or fewer items."), 1, 0, false);
	AddWhite(TEXT("ACH_WHI_007"), TEXT("True Gambler"), TEXT("Win 50 bets at the Yellow House."), 50, 8, false);
	AddWhite(TEXT("ACH_WHI_008"), TEXT("Duo Victory"), TEXT("Beat Stage 66 in Duo mode."), 1, 0, false);
	AddWhite(TEXT("ACH_WHI_009"), TEXT("Trio Victory"), TEXT("Beat Stage 66 in Trio mode."), 1, 0, false);
	AddWhite(TEXT("ACH_WHI_010"), TEXT("Completionist"), TEXT("Unlock all other achievements."), 39, 9, false);
}

TSharedRef<SWidget> UT66AchievementsScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	FText AchievementsText = Loc ? Loc->GetText_Achievements() : FText::FromString(TEXT("ACHIEVEMENTS"));
	FText BackText = Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"));

	GeneratePlaceholderAchievements();

	// Tier button colors
	auto GetTierColor = [](ET66AchievementTier Tier, bool bSelected) -> FLinearColor
	{
		if (bSelected)
		{
			switch (Tier)
			{
			case ET66AchievementTier::Black: return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
			case ET66AchievementTier::Red: return FLinearColor(0.6f, 0.15f, 0.15f, 1.0f);
			case ET66AchievementTier::Yellow: return FLinearColor(0.6f, 0.5f, 0.1f, 1.0f);
			case ET66AchievementTier::White: return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
			default: return FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
			}
		}
		return FLinearColor(0.1f, 0.1f, 0.15f, 1.0f);
	};

	auto GetTierTextColor = [](ET66AchievementTier Tier, bool bSelected) -> FLinearColor
	{
		if (bSelected && Tier == ET66AchievementTier::White)
			return FLinearColor::Black;
		return FLinearColor::White;
	};

	// Use dynamic lambdas so button colors update when tier changes
	auto MakeTierButton = [this, &GetTierColor, &GetTierTextColor](const FText& Text, ET66AchievementTier Tier) -> TSharedRef<SWidget>
	{
		return SNew(SBox).WidthOverride(100.0f).HeightOverride(40.0f).Padding(FMargin(4.0f, 0.0f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleTierClicked, Tier))
				.ButtonColorAndOpacity_Lambda([this, Tier, GetTierColor]() -> FSlateColor {
					bool bIsSelected = (CurrentTier == Tier);
					return GetTierColor(Tier, bIsSelected);
				})
				[
					SNew(STextBlock).Text(Text)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.ColorAndOpacity_Lambda([this, Tier, GetTierTextColor]() -> FSlateColor {
						bool bIsSelected = (CurrentTier == Tier);
						return GetTierTextColor(Tier, bIsSelected);
					})
				]
			];
	};

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.0f))
		[
			SNew(SVerticalBox)
			// Header row
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(30.0f, 25.0f, 30.0f, 20.0f)
			[
				SNew(SHorizontalBox)
				// Title
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(AchievementsText)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 42))
					.ColorAndOpacity(FLinearColor::White)
				]
				// Total AC display
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.15f, 0.12f, 0.05f, 1.0f))
					.Padding(FMargin(15.0f, 8.0f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("\U0001FA99"))) // Coin emoji
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
							.ColorAndOpacity(FLinearColor(1.0f, 0.85f, 0.0f, 1.0f))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.0f, 0.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(FText::Format(FText::FromString(TEXT("{0} AC")), FText::AsNumber(TotalAchievementCoins)))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
							.ColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.5f, 1.0f))
						]
					]
				]
			]
			// Tier tabs
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(30.0f, 0.0f, 30.0f, 15.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTierButton(FText::FromString(TEXT("BLACK")), ET66AchievementTier::Black)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTierButton(FText::FromString(TEXT("RED")), ET66AchievementTier::Red)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTierButton(FText::FromString(TEXT("YELLOW")), ET66AchievementTier::Yellow)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTierButton(FText::FromString(TEXT("WHITE")), ET66AchievementTier::White)
				]
			]
			// Achievement list
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(30.0f, 0.0f, 30.0f, 20.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(AchievementListBox, SVerticalBox)
				]
			]
			// Back button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(30.0f, 0.0f, 30.0f, 25.0f)
			[
				SNew(SBox).WidthOverride(120.0f).HeightOverride(45.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center).VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleBackClicked))
					.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
					[
						SNew(STextBlock).Text(BackText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
		];
}

void UT66AchievementsScreen::RebuildAchievementList()
{
	if (!AchievementListBox.IsValid()) return;

	AchievementListBox->ClearChildren();

	for (const FAchievementData& Achievement : AllAchievements)
	{
		// Filter by current tier
		if (Achievement.Tier != CurrentTier) continue;

		// Row styling
		FLinearColor RowBg = Achievement.bIsUnlocked
			? FLinearColor(0.1f, 0.15f, 0.1f, 1.0f)
			: FLinearColor(0.06f, 0.06f, 0.1f, 1.0f);

		FLinearColor ProgressColor = Achievement.bIsUnlocked
			? FLinearColor(0.4f, 0.8f, 0.4f, 1.0f)
			: FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

		FString ProgressStr = FString::Printf(TEXT("%d/%d"), 
			FMath::Min(Achievement.CurrentProgress, Achievement.RequirementCount), 
			Achievement.RequirementCount);

		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(RowBg)
			.Padding(FMargin(20.0f, 14.0f))
			[
				SNew(SHorizontalBox)
				// Description
				+ SHorizontalBox::Slot()
				.FillWidth(0.55f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Achievement.DisplayName)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(Achievement.Description)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
						.AutoWrapText(true)
					]
				]
				// Progress
				+ SHorizontalBox::Slot()
				.FillWidth(0.2f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ProgressStr))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.ColorAndOpacity(ProgressColor)
				]
				// Reward
				+ SHorizontalBox::Slot()
				.FillWidth(0.25f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("\U0001FA99")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
						.ColorAndOpacity(FLinearColor(1.0f, 0.85f, 0.0f, 1.0f))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(FText::Format(FText::FromString(TEXT("{0} AC")), FText::AsNumber(Achievement.RewardCoins)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.5f, 1.0f))
					]
				]
			]
		];
	}
}

void UT66AchievementsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	CurrentTier = ET66AchievementTier::Black;
	RebuildAchievementList();
}

void UT66AchievementsScreen::SwitchToTier(ET66AchievementTier Tier)
{
	CurrentTier = Tier;
	// Rebuild just the achievement list (button colors update via lambdas)
	RebuildAchievementList();
}

void UT66AchievementsScreen::OnBackClicked()
{
	NavigateBack();
}

FReply UT66AchievementsScreen::HandleTierClicked(ET66AchievementTier Tier)
{
	SwitchToTier(Tier);
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}
