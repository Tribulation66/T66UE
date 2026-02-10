// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AchievementsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Style/T66Style.h"
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

UT66AchievementsSubsystem* UT66AchievementsScreen::GetAchievementsSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66AchievementsSubsystem>();
	}
	return nullptr;
}

void UT66AchievementsScreen::RefreshAchievements()
{
	if (UT66AchievementsSubsystem* Ach = GetAchievementsSubsystem())
	{
		AllAchievements = Ach->GetAllAchievements();
	}
	else
	{
		AllAchievements.Reset();
	}
}

TSharedRef<SWidget> UT66AchievementsScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66AchievementsSubsystem* Ach = GetAchievementsSubsystem();

	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	RefreshAchievements();

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
		const FTextBlockStyle& TxtChip = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");

		return FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleTierClicked, Tier))
			.SetMinWidth(120.f)
			.SetPadding(FMargin(12.f, 8.f))
			.SetColor(TAttribute<FSlateColor>::CreateLambda([this, Tier, GetTierColor]() -> FSlateColor {
				bool bIsSelected = (CurrentTier == Tier);
				return GetTierColor(Tier, bIsSelected);
			}))
			.SetContent(
				SNew(STextBlock).Text(Text)
				.TextStyle(&TxtChip)
				.ColorAndOpacity_Lambda([this, Tier, GetTierTextColor]() -> FSlateColor {
					bool bIsSelected = (CurrentTier == Tier);
					return GetTierTextColor(Tier, bIsSelected);
				})
			)
		);
	};

	return FT66Style::MakePanel(
		SNew(SOverlay)
			// Main content
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				// Header row (title centered, AC display right)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(30.0f, 25.0f, 30.0f, 20.0f)
				[
					SNew(SOverlay)
					// Centered title
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock).Text(AchievementsText)
						.Font(FT66Style::Tokens::FontBold(42))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					// AC display (right-aligned)
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						FT66Style::MakePanel(
							SNew(STextBlock)
								.Text_Lambda([this, Ach, Loc]() -> FText
								{
									const int32 Balance = Ach ? Ach->GetAchievementCoinsBalance() : 0;
									if (Loc)
									{
										return FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Balance));
									}
									return FText::Format(NSLOCTEXT("T66.Achievements", "CoinsFormat", "{0} AC"), FText::AsNumber(Balance));
								})
								.Font(FT66Style::Tokens::FontBold(22))
								.ColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.5f, 1.0f))
						,
						FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(15.0f, 8.0f)))
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
						MakeTierButton(Loc ? Loc->GetText_AchievementTierBlack() : NSLOCTEXT("T66.Achievements", "TierBlack", "BLACK"), ET66AchievementTier::Black)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeTierButton(Loc ? Loc->GetText_AchievementTierRed() : NSLOCTEXT("T66.Achievements", "TierRed", "RED"), ET66AchievementTier::Red)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeTierButton(Loc ? Loc->GetText_AchievementTierYellow() : NSLOCTEXT("T66.Achievements", "TierYellow", "YELLOW"), ET66AchievementTier::Yellow)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeTierButton(Loc ? Loc->GetText_AchievementTierWhite() : NSLOCTEXT("T66.Achievements", "TierWhite", "WHITE"), ET66AchievementTier::White)
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
			]
			// Back button (bottom-left overlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.0f, 0.0f, 0.0f, 20.0f)
			[
				FT66Style::MakeButton(FT66ButtonParams(BackText,
					FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleBackClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(120.f)
				)
			]
		,
		ET66PanelType::Panel);
}

void UT66AchievementsScreen::RebuildAchievementList()
{
	if (!AchievementListBox.IsValid()) return;

	AchievementListBox->ClearChildren();

	RefreshAchievements();
	UT66AchievementsSubsystem* Ach = GetAchievementsSubsystem();
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

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

		const bool bClaimed = Ach ? Ach->IsAchievementClaimed(Achievement.AchievementID) : false;
		const bool bCanClaim = Achievement.bIsUnlocked && !bClaimed;

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
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(Achievement.Description)
						.Font(FT66Style::Tokens::FontRegular(11))
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
					.Font(FT66Style::Tokens::FontBold(14))
					.ColorAndOpacity(ProgressColor)
				]
				// Reward
				+ SHorizontalBox::Slot()
				.FillWidth(0.25f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text_Lambda([Loc, Achievement, bClaimed]() -> FText
						{
							if (bClaimed)
							{
								return Loc ? Loc->GetText_Claimed() : NSLOCTEXT("T66.Achievements", "Claimed", "CLAIMED");
							}
							if (Loc)
							{
								return FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Achievement.RewardCoins));
							}
							return FText::Format(NSLOCTEXT("T66.Achievements", "CoinsFormat", "{0} AC"), FText::AsNumber(Achievement.RewardCoins));
						})
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(bClaimed ? FLinearColor(0.6f, 0.8f, 0.6f, 1.0f) : FLinearColor(1.0f, 0.9f, 0.5f, 1.0f))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.Padding(0.f, 6.f, 0.f, 0.f)
					[
					FT66Style::MakeButton(
						FT66ButtonParams(
							Loc ? Loc->GetText_Claim() : NSLOCTEXT("T66.Achievements", "Claim", "CLAIM"),
							FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleClaimClicked, Achievement.AchievementID),
							ET66ButtonType::Primary
						)
						.SetMinWidth(0.f)
						.SetFontSize(12)
						.SetTextColor(FLinearColor::White)
						.SetEnabled(bCanClaim)
						.SetVisibility(bCanClaim ? EVisibility::Visible : EVisibility::Collapsed)
					)
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

	// Subscribe to language changes (rebuild UI).
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66AchievementsScreen::HandleLanguageChanged);
	}
}

void UT66AchievementsScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66AchievementsScreen::HandleLanguageChanged);
	}
	Super::OnScreenDeactivated_Implementation();
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

FReply UT66AchievementsScreen::HandleClaimClicked(FName AchievementID)
{
	if (UT66AchievementsSubsystem* Ach = GetAchievementsSubsystem())
	{
		Ach->TryClaimAchievement(AchievementID);
	}
	RebuildAchievementList();
	return FReply::Handled();
}

void UT66AchievementsScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}
