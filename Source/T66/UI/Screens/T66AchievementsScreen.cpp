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

	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : FText::FromString(TEXT("ACHIEVEMENTS"));
	const FText BackText = Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"));

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
		const FButtonStyle& BtnNeutral = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
		const FTextBlockStyle& TxtChip = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");

		return SNew(SBox).MinDesiredWidth(120.0f).HeightOverride(40.0f).Padding(FMargin(4.0f, 0.0f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleTierClicked, Tier))
				.ButtonColorAndOpacity_Lambda([this, Tier, GetTierColor]() -> FSlateColor {
					bool bIsSelected = (CurrentTier == Tier);
					return GetTierColor(Tier, bIsSelected);
				})
				.ButtonStyle(&BtnNeutral)
				.ContentPadding(FMargin(12.f, 8.f))
				[
					SNew(STextBlock).Text(Text)
					.TextStyle(&TxtChip)
					.ColorAndOpacity_Lambda([this, Tier, GetTierTextColor]() -> FSlateColor {
						bool bIsSelected = (CurrentTier == Tier);
						return GetTierTextColor(Tier, bIsSelected);
					})
				]
			];
	};

	return SNew(SBorder)
		.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Bg"))
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
					.Font(FT66Style::Tokens::FontBold(42))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				// Total AC display
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
					.Padding(FMargin(15.0f, 8.0f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
					.Text_Lambda([this, Ach, Loc]() -> FText
					{
						const int32 Balance = Ach ? Ach->GetAchievementCoinsBalance() : 0;
						if (Loc)
						{
							return FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Balance));
						}
						return FText::Format(FText::FromString(TEXT("{0} AC")), FText::AsNumber(Balance));
					})
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
					MakeTierButton(Loc ? Loc->GetText_AchievementTierBlack() : FText::FromString(TEXT("BLACK")), ET66AchievementTier::Black)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTierButton(Loc ? Loc->GetText_AchievementTierRed() : FText::FromString(TEXT("RED")), ET66AchievementTier::Red)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTierButton(Loc ? Loc->GetText_AchievementTierYellow() : FText::FromString(TEXT("YELLOW")), ET66AchievementTier::Yellow)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTierButton(Loc ? Loc->GetText_AchievementTierWhite() : FText::FromString(TEXT("WHITE")), ET66AchievementTier::White)
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
								return Loc ? Loc->GetText_Claimed() : FText::FromString(TEXT("CLAIMED"));
							}
							if (Loc)
							{
								return FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Achievement.RewardCoins));
							}
							return FText::Format(FText::FromString(TEXT("{0} AC")), FText::AsNumber(Achievement.RewardCoins));
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(bClaimed ? FLinearColor(0.6f, 0.8f, 0.6f, 1.0f) : FLinearColor(1.0f, 0.9f, 0.5f, 1.0f))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.Padding(0.f, 6.f, 0.f, 0.f)
					[
						SNew(SButton)
						.IsEnabled(bCanClaim)
						.Visibility(bCanClaim ? EVisibility::Visible : EVisibility::Collapsed)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleClaimClicked, Achievement.AchievementID))
						.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_Claim() : FText::FromString(TEXT("CLAIM")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
							.ColorAndOpacity(FLinearColor::White)
						]
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
	TakeWidget();
}
