// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AchievementsScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	bool T66IsPausedGameplayWidget(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		return PC && PC->IsPaused();
	}

	FLinearColor T66AchievementsShellFill()
	{
		return FLinearColor(0.004f, 0.005f, 0.010f, 0.985f);
	}

	FLinearColor T66AchievementsInsetFill()
	{
		return FLinearColor(0.024f, 0.025f, 0.030f, 1.0f);
	}

	FLinearColor T66AchievementsRowFill()
	{
		return FLinearColor(0.028f, 0.029f, 0.034f, 1.0f);
	}

	FLinearColor T66AchievementsUnlockedRowFill()
	{
		return FLinearColor(0.036f, 0.048f, 0.041f, 1.0f);
	}

	float T66AchievementProgress01(const FAchievementData& Achievement)
	{
		return Achievement.RequirementCount > 0
			? static_cast<float>(FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount))
				/ static_cast<float>(Achievement.RequirementCount)
			: (Achievement.bIsUnlocked ? 1.0f : 0.0f);
	}

	int32 T66AchievementRemainingCount(const FAchievementData& Achievement)
	{
		return FMath::Max(0, Achievement.RequirementCount - FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount));
	}

	int32 T66AchievementOrderKey(const FName AchievementID)
	{
		int32 NumericPart = 0;
		bool bHasDigit = false;
		for (const TCHAR Character : AchievementID.ToString())
		{
			if (FChar::IsDigit(Character))
			{
				NumericPart = (NumericPart * 10) + (Character - TEXT('0'));
				bHasDigit = true;
			}
		}

		return bHasDigit ? NumericPart : MAX_int32;
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
	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		AllAchievements = Achievements->GetAllAchievements();
		AllAchievements.Sort([Achievements](const FAchievementData& Left, const FAchievementData& Right)
		{
			const bool bLeftClaimed = Achievements ? Achievements->IsAchievementClaimed(Left.AchievementID) : false;
			const bool bRightClaimed = Achievements ? Achievements->IsAchievementClaimed(Right.AchievementID) : false;
			if (bLeftClaimed != bRightClaimed)
			{
				return !bLeftClaimed && bRightClaimed;
			}

			const float LeftProgress = T66AchievementProgress01(Left);
			const float RightProgress = T66AchievementProgress01(Right);
			if (!FMath::IsNearlyEqual(LeftProgress, RightProgress))
			{
				return LeftProgress > RightProgress;
			}

			const int32 LeftRemaining = T66AchievementRemainingCount(Left);
			const int32 RightRemaining = T66AchievementRemainingCount(Right);
			if (LeftRemaining != RightRemaining)
			{
				return LeftRemaining < RightRemaining;
			}

			const int32 LeftOrder = T66AchievementOrderKey(Left.AchievementID);
			const int32 RightOrder = T66AchievementOrderKey(Right.AchievementID);
			if (LeftOrder != RightOrder)
			{
				return LeftOrder < RightOrder;
			}
			return Left.AchievementID.LexicalLess(Right.AchievementID);
		});
	}
	else
	{
		AllAchievements.Reset();
	}
}

int32 UT66AchievementsScreen::GetUnlockedAchievementCount() const
{
	int32 UnlockedCount = 0;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.bIsUnlocked)
		{
			++UnlockedCount;
		}
	}
	return UnlockedCount;
}

TSharedRef<SWidget> UT66AchievementsScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FLinearColor ShellFill = T66AchievementsShellFill();
	const FLinearColor InsetFill = T66AchievementsInsetFill();
	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 22.f;
	const float TopInset = UIManager
		? FMath::Max(0.f, (UIManager->GetFrontendTopBarContentHeight() - TopBarOverlapPx) / ResponsiveScale)
		: 0.f;
	const bool bShowBackButton = !(UIManager && UIManager->IsFrontendTopBarVisible());

	RefreshAchievements();

	const TSharedRef<SWidget> Root =
		SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		[
			FT66Style::MakePanel(
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(24.f, 18.f, 24.f, 14.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(AchievementsText)
								.Font(FT66Style::Tokens::FontBold(40))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 14.f, 0.f, 0.f)
						[
							FT66Style::MakePanel(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(12.f, 0.f, 12.f, 8.f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.FillWidth(1.f)
									[
										SNew(STextBlock)
										.Text_Lambda([this]() -> FText
										{
											return FText::Format(
												NSLOCTEXT("T66.Achievements", "CompletionLabel", "{0}/{1} ACHIEVEMENTS"),
												FText::AsNumber(GetUnlockedAchievementCount()),
												FText::AsNumber(AllAchievements.Num()));
										})
										.Font(FT66Style::Tokens::FontBold(18))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Right)
									[
										SNew(STextBlock)
										.Text_Lambda([this]() -> FText
										{
											const float Progress = AllAchievements.Num() > 0
												? static_cast<float>(GetUnlockedAchievementCount()) / static_cast<float>(AllAchievements.Num())
												: 0.f;
											return FText::AsPercent(Progress);
										})
										.Font(FT66Style::Tokens::FontBold(17))
										.ColorAndOpacity(FT66Style::Tokens::TextMuted)
									]
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SProgressBar)
									.Percent_Lambda([this]() -> TOptional<float>
									{
										return AllAchievements.Num() > 0
											? static_cast<float>(GetUnlockedAchievementCount()) / static_cast<float>(AllAchievements.Num())
											: 0.f;
									})
									.FillColorAndOpacity(FLinearColor(0.40f, 0.68f, 0.41f, 1.0f))
								],
								FT66PanelParams(ET66PanelType::Panel)
									.SetColor(InsetFill)
									.SetPadding(FMargin(12.f, 12.f)))
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					.Padding(24.f, 0.f, 24.f, 16.f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(AchievementListBox, SVerticalBox)
						]
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				.Padding(18.f, 0.f, 0.f, 18.f)
				[
					SNew(SBox)
					.Visibility(bShowBackButton ? EVisibility::Visible : EVisibility::Collapsed)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(
								BackText,
								FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleBackClicked),
								ET66ButtonType::Neutral)
							.SetMinWidth(108.f)
							.SetFontSize(18))
					]
				],
				FT66PanelParams(ET66PanelType::Panel).SetColor(ShellFill))
		];

	RebuildAchievementList();
	return Root;
}

void UT66AchievementsScreen::RebuildAchievementList()
{
	if (!AchievementListBox.IsValid())
	{
		return;
	}

	AchievementListBox->ClearChildren();
	RefreshAchievements();

	UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem();
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	for (const FAchievementData& Achievement : AllAchievements)
	{
		const FLinearColor RowBackground = Achievement.bIsUnlocked
			? T66AchievementsUnlockedRowFill()
			: T66AchievementsRowFill();
		const FLinearColor ProgressColor = Achievement.bIsUnlocked
			? FT66Style::Tokens::Success
			: FT66Style::Tokens::TextMuted;

		const FString ProgressString = FString::Printf(
			TEXT("%d/%d"),
			FMath::Min(Achievement.CurrentProgress, Achievement.RequirementCount),
			FMath::Max(1, Achievement.RequirementCount));

		const bool bClaimed = Achievements ? Achievements->IsAchievementClaimed(Achievement.AchievementID) : false;
		const bool bCanClaim = Achievement.bIsUnlocked && !bClaimed;

		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 5.f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(RowBackground)
			.Padding(FMargin(16.f, 12.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.55f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(Achievement.DisplayName)
						.Font(FT66Style::Tokens::FontBold(19))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 3.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(Achievement.Description)
						.Font(FT66Style::Tokens::FontRegular(16))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.17f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ProgressString))
					.Font(FT66Style::Tokens::FontBold(19))
					.ColorAndOpacity(ProgressColor)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.28f)
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
						.Font(FT66Style::Tokens::FontBold(19))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(
								Loc ? Loc->GetText_Claim() : NSLOCTEXT("T66.Achievements", "Claim", "CLAIM"),
								FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleClaimClicked, Achievement.AchievementID),
								ET66ButtonType::Primary)
							.SetMinWidth(0.f)
							.SetFontSize(18)
							.SetEnabled(bCanClaim)
							.SetVisibility(bCanClaim ? EVisibility::Visible : EVisibility::Collapsed))
					]
				]
			]
		];
	}
}

void UT66AchievementsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	RebuildAchievementList();

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66AchievementsScreen::HandleLanguageChanged);
	}

	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->AchievementsStateChanged.AddUniqueDynamic(this, &UT66AchievementsScreen::HandleAchievementsStateChanged);
	}
}

void UT66AchievementsScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66AchievementsScreen::HandleLanguageChanged);
	}

	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->AchievementsStateChanged.RemoveDynamic(this, &UT66AchievementsScreen::HandleAchievementsStateChanged);
	}

	Super::OnScreenDeactivated_Implementation();
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

FReply UT66AchievementsScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleClaimClicked(FName AchievementID)
{
	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->TryClaimAchievement(AchievementID);
	}
	return FReply::Handled();
}

void UT66AchievementsScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}

void UT66AchievementsScreen::HandleAchievementsStateChanged()
{
	if (UIManager)
	{
		UIManager->RebuildAllVisibleUI();
		return;
	}

	FT66Style::DeferRebuild(this);
}
