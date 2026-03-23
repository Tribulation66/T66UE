// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AchievementsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
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

namespace
{
	bool T66IsPausedGameplayWidget(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		return PC && PC->IsPaused();
	}

	int32 T66ExtractNumericSuffix(FName ID)
	{
		const FString AsString = ID.ToString();
		int32 UnderscoreIndex = INDEX_NONE;
		if (!AsString.FindLastChar(TEXT('_'), UnderscoreIndex))
		{
			return MAX_int32;
		}

		const FString Suffix = AsString.Mid(UnderscoreIndex + 1);
		return FCString::Atoi(*Suffix);
	}

	int32 T66CompanionUnlockStageFromID(FName CompanionID)
	{
		const int32 TargetIndex = T66ExtractNumericSuffix(CompanionID);
		if (TargetIndex <= 0)
		{
			return INDEX_NONE;
		}

		int32 CompanionCount = 0;
		for (int32 Stage = 1; Stage <= 30; ++Stage)
		{
			if ((Stage % 5) == 0)
			{
				continue;
			}

			++CompanionCount;
			if (CompanionCount == TargetIndex)
			{
				return Stage;
			}
		}

		return INDEX_NONE;
	}
}

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

	// Tier button colors (selected = rarity; unselected = theme panel)
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
		return FT66Style::Tokens::Panel2;
	};

	auto GetTierTextColor = [](ET66AchievementTier Tier, bool bSelected) -> FLinearColor
	{
		if (bSelected && Tier == ET66AchievementTier::White)
			return FLinearColor::Black;
		return FT66Style::Tokens::Text;
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

	auto MakeUnlocksButton = [this](const FText& Text) -> TSharedRef<SWidget>
	{
		const FTextBlockStyle& TxtChip = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");
		const FLinearColor SelectedColor(0.18f, 0.32f, 0.38f, 1.0f);

		return FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleUnlocksClicked))
			.SetMinWidth(140.f)
			.SetPadding(FMargin(12.f, 8.f))
			.SetColor(TAttribute<FSlateColor>::CreateLambda([this, SelectedColor]() -> FSlateColor
			{
				return bShowingUnlocksTab ? FSlateColor(SelectedColor) : FSlateColor(FT66Style::Tokens::Panel2);
			}))
			.SetContent(
				SNew(STextBlock).Text(Text)
				.TextStyle(&TxtChip)
				.ColorAndOpacity(FT66Style::Tokens::Text)
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
								.ColorAndOpacity(FT66Style::Tokens::Text)
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
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeUnlocksButton(NSLOCTEXT("T66.Achievements", "UnlocksTab", "UNLOCKS"))
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
	if (bShowingUnlocksTab)
	{
		RebuildUnlockList();
		return;
	}

	UT66AchievementsSubsystem* Ach = GetAchievementsSubsystem();
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	for (const FAchievementData& Achievement : AllAchievements)
	{
		// Filter by current tier
		if (Achievement.Tier != CurrentTier) continue;

		// Row styling (theme-aware)
		FLinearColor RowBg = Achievement.bIsUnlocked
			? (FT66Style::Tokens::Panel2 + FLinearColor(0.02f, 0.04f, 0.02f, 0.f))
			: FT66Style::Tokens::Panel;
		FLinearColor ProgressColor = Achievement.bIsUnlocked
			? FT66Style::Tokens::Success
			: FT66Style::Tokens::TextMuted;

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
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(Achievement.Description)
						.Font(FT66Style::Tokens::FontRegular(11))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
						.ColorAndOpacity(FT66Style::Tokens::Text)
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
						.SetEnabled(bCanClaim)
						.SetVisibility(bCanClaim ? EVisibility::Visible : EVisibility::Collapsed)
					)
					]
				]
			]
		];
	}
}

void UT66AchievementsScreen::RebuildUnlockList()
{
	if (!AchievementListBox.IsValid())
	{
		return;
	}

	AchievementListBox->ClearChildren();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66CompanionUnlockSubsystem* CompanionUnlocks = GI ? GI->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr;

	TArray<FName> LockedHeroIDs;
	TArray<FName> LockedCompanionIDs;

	if (GI)
	{
		TArray<FName> HeroIDs = GI->GetAllHeroIDs();
		HeroIDs.Sort([](const FName& A, const FName& B)
		{
			return T66ExtractNumericSuffix(A) < T66ExtractNumericSuffix(B);
		});

		for (const FName& HeroID : HeroIDs)
		{
			FHeroData HeroData;
			if (GI->GetHeroData(HeroID, HeroData) && !HeroData.bUnlockedByDefault)
			{
				LockedHeroIDs.Add(HeroID);
			}
		}

		TArray<FName> CompanionIDs = GI->GetAllCompanionIDs();
		CompanionIDs.Sort([](const FName& A, const FName& B)
		{
			return T66ExtractNumericSuffix(A) < T66ExtractNumericSuffix(B);
		});

		for (const FName& CompanionID : CompanionIDs)
		{
			if (!CompanionUnlocks || CompanionUnlocks->IsCompanionUnlocked(CompanionID))
			{
				continue;
			}

			LockedCompanionIDs.Add(CompanionID);
		}
	}

	auto AddSectionHeader = [this](const FText& Text)
	{
		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 12.f, 0.f, 8.f)
		[
			SNew(STextBlock)
			.Text(Text)
			.Font(FT66Style::Tokens::FontBold(20))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		];
	};

	auto AddUnlockRow = [this](const FText& KindText, const FText& NameText, const FText& ConditionText)
	{
		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel)
			.Padding(FMargin(18.f, 14.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 0.f, 14.f, 0.f)
				[
					SNew(STextBlock)
					.Text(KindText)
					.Font(FT66Style::Tokens::FontBold(12))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(NameText)
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(ConditionText)
						.Font(FT66Style::Tokens::FontRegular(12))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
				]
			]
		];
	};

	if (LockedHeroIDs.Num() == 0 && LockedCompanionIDs.Num() == 0)
	{
		AchievementListBox->AddSlot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Tokens::Panel)
			.Padding(FMargin(20.f, 18.f))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Achievements", "UnlocksAllUnlocked", "All currently defined heroes and companions are unlocked."))
				.Font(FT66Style::Tokens::FontRegular(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.AutoWrapText(true)
			]
		];
		return;
	}

	if (LockedHeroIDs.Num() > 0)
	{
		AddSectionHeader(NSLOCTEXT("T66.Achievements", "LockedHeroesHeader", "CHARACTERS"));
		for (const FName& HeroID : LockedHeroIDs)
		{
			const FText HeroName = Loc ? Loc->GetText_HeroName(HeroID) : FText::FromName(HeroID);
			AddUnlockRow(
				NSLOCTEXT("T66.Achievements", "UnlockKindHero", "HERO"),
				HeroName,
				NSLOCTEXT("T66.Achievements", "UnlockHeroConditionUnknown", "Unlock condition is not exposed by the current hero data."));
		}
	}

	if (LockedCompanionIDs.Num() > 0)
	{
		AddSectionHeader(NSLOCTEXT("T66.Achievements", "LockedCompanionsHeader", "COMPANIONS"));
		for (const FName& CompanionID : LockedCompanionIDs)
		{
			const int32 UnlockStage = T66CompanionUnlockStageFromID(CompanionID);
			const FText CompanionName = Loc ? Loc->GetText_CompanionName(CompanionID) : FText::FromName(CompanionID);
			const FText ConditionText = (UnlockStage != INDEX_NONE)
				? FText::Format(
					NSLOCTEXT("T66.Achievements", "UnlockCompanionStageCondition", "Clear Stage {0} for the first time."),
					FText::AsNumber(UnlockStage))
				: NSLOCTEXT("T66.Achievements", "UnlockCompanionConditionUnknown", "Unlock condition is not exposed by the current companion data.");

			AddUnlockRow(
				NSLOCTEXT("T66.Achievements", "UnlockKindCompanion", "COMPANION"),
				CompanionName,
				ConditionText);
		}
	}
}

void UT66AchievementsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	CurrentTier = ET66AchievementTier::Black;
	bShowingUnlocksTab = false;
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
	bShowingUnlocksTab = false;
	// Rebuild just the achievement list (button colors update via lambdas)
	RebuildAchievementList();
}

void UT66AchievementsScreen::SwitchToUnlocks()
{
	bShowingUnlocksTab = true;
	RebuildAchievementList();
}

void UT66AchievementsScreen::OnBackClicked()
{
	if (T66IsPausedGameplayWidget(this) && UIManager)
	{
		ShowModal(ET66ScreenType::PauseMenu);
		return;
	}

	NavigateBack();
}

FReply UT66AchievementsScreen::HandleTierClicked(ET66AchievementTier Tier)
{
	SwitchToTier(Tier);
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleUnlocksClicked()
{
	SwitchToUnlocks();
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
