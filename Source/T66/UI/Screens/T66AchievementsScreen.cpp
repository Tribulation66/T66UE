// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AchievementsScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 T66AchievementsFontDelta = -6;
	constexpr int32 T66SecretPlaceholderRowCount = 10;

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

	FLinearColor T66AchievementsTabActiveFill()
	{
		return FLinearColor(0.67f, 0.72f, 0.62f, 1.0f);
	}

	FLinearColor T66AchievementsTabInactiveFill()
	{
		return FLinearColor(0.10f, 0.11f, 0.15f, 1.0f);
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

	int32 AdjustAchievementsFontSize(int32 BaseSize)
	{
		return FMath::Max(8, BaseSize + T66AchievementsFontDelta);
	}

	FSlateFontInfo AchievementsBoldFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontBold(AdjustAchievementsFontSize(BaseSize));
	}

	FSlateFontInfo AchievementsRegularFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontRegular(AdjustAchievementsFontSize(BaseSize));
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

UT66PlayerSettingsSubsystem* UT66AchievementsScreen::GetPlayerSettingsSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66PlayerSettingsSubsystem>();
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

int32 UT66AchievementsScreen::GetUnlockedAchievementCountForCategory(const ET66AchievementCategory Category) const
{
	int32 UnlockedCount = 0;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.Category == Category && Achievement.bIsUnlocked)
		{
			++UnlockedCount;
		}
	}
	return UnlockedCount;
}

TArray<FAchievementData> UT66AchievementsScreen::GetAchievementsForCategory(const ET66AchievementCategory Category) const
{
	TArray<FAchievementData> FilteredAchievements;
	for (const FAchievementData& Achievement : AllAchievements)
	{
		if (Achievement.Category == Category)
		{
			FilteredAchievements.Add(Achievement);
		}
	}
	return FilteredAchievements;
}

FText UT66AchievementsScreen::BuildAchievementRewardText(const FAchievementData& Achievement) const
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	if (Achievement.RewardType == ET66AchievementRewardType::SkinUnlock)
	{
		const FText SkinName = Loc
			? Loc->GetText_SkinName(Achievement.RewardSkinID)
			: FText::FromName(Achievement.RewardSkinID);

		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (Achievement.RewardEntityType == ET66AchievementRewardEntityType::Hero)
			{
				FHeroData HeroData;
				if (T66GI->GetHeroData(Achievement.RewardEntityID, HeroData))
				{
					const FText HeroName = Loc
						? Loc->GetHeroDisplayName(HeroData)
						: (!HeroData.DisplayName.IsEmpty() ? HeroData.DisplayName : FText::FromName(Achievement.RewardEntityID));
					return FText::Format(
						NSLOCTEXT("T66.Achievements", "SkinRewardHeroFormat", "{0} Outfit: {1}"),
						SkinName,
						HeroName);
				}
			}
			else if (Achievement.RewardEntityType == ET66AchievementRewardEntityType::Companion)
			{
				FCompanionData CompanionData;
				if (T66GI->GetCompanionData(Achievement.RewardEntityID, CompanionData))
				{
					const FText CompanionName = Loc
						? Loc->GetCompanionDisplayName(CompanionData)
						: (!CompanionData.DisplayName.IsEmpty() ? CompanionData.DisplayName : FText::FromName(Achievement.RewardEntityID));
					return FText::Format(
						NSLOCTEXT("T66.Achievements", "SkinRewardCompanionFormat", "{0} Outfit: {1}"),
						SkinName,
						CompanionName);
				}
			}
		}

		return FText::Format(
			NSLOCTEXT("T66.Achievements", "SkinRewardFormat", "{0} Outfit"),
			SkinName);
	}

	if (Loc)
	{
		return FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Achievement.RewardCoins));
	}

	return FText::Format(NSLOCTEXT("T66.Achievements", "CoinsFormat", "{0} CC"), FText::AsNumber(Achievement.RewardCoins));
}

FText UT66AchievementsScreen::GetAchievementActionText(const FAchievementData& Achievement) const
{
	if (Achievement.RewardType == ET66AchievementRewardType::SkinUnlock)
	{
		return NSLOCTEXT("T66.Achievements", "UnlockReward", "UNLOCK");
	}

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		return Loc->GetText_Claim();
	}

	return NSLOCTEXT("T66.Achievements", "Claim", "CLAIM");
}

TSharedRef<SWidget> UT66AchievementsScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
	const FText SecretText = NSLOCTEXT("T66.Achievements", "SecretTab", "SECRET");
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
	const TArray<FAchievementData> StandardAchievements = GetAchievementsForCategory(ET66AchievementCategory::Standard);
	const int32 UnlockedStandardAchievements = GetUnlockedAchievementCountForCategory(ET66AchievementCategory::Standard);
	const float StandardProgress = StandardAchievements.Num() > 0
		? static_cast<float>(UnlockedStandardAchievements) / static_cast<float>(StandardAchievements.Num())
		: 0.0f;
	const bool bShowingSecret = ActiveTab == EAchievementTab::Secret;

	auto MakeTabButton = [this](const FText& Label, bool bActive, FReply(UT66AchievementsScreen::*Handler)()) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(
			FT66ButtonParams(Label, FOnClicked::CreateUObject(this, Handler), bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetMinWidth(172.f)
			.SetHeight(0.f)
			.SetFontSize(AdjustAchievementsFontSize(20))
			.SetPadding(FMargin(18.f, 11.f, 18.f, 9.f))
			.SetColor(bActive ? T66AchievementsTabActiveFill() : T66AchievementsTabInactiveFill())
			.SetTextColor(bActive ? FLinearColor(0.07f, 0.08f, 0.06f, 1.0f) : FT66Style::Tokens::Text)
			.SetUseGlow(false));
	};

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
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							[
								SNew(SSpacer)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.f, 0.f, 12.f, 0.f)
							[
								MakeTabButton(
									AchievementsText,
									ActiveTab == EAchievementTab::Achievements,
									&UT66AchievementsScreen::HandleAchievementsTabClicked)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								MakeTabButton(
									SecretText,
									ActiveTab == EAchievementTab::Secret,
									&UT66AchievementsScreen::HandleSecretTabClicked)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							[
								SNew(SSpacer)
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
										.Text_Lambda([bShowingSecret, UnlockedStandardAchievements, StandardAchievements]() -> FText
										{
											if (bShowingSecret)
											{
												return NSLOCTEXT("T66.Achievements", "SecretProgressMaskedLabel", "???");
											}

											return FText::Format(
												NSLOCTEXT("T66.Achievements", "CompletionLabel", "{0}/{1} ACHIEVEMENTS"),
												FText::AsNumber(UnlockedStandardAchievements),
												FText::AsNumber(StandardAchievements.Num()));
										})
										.Font(AchievementsBoldFont(18))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Right)
									[
										SNew(STextBlock)
										.Text_Lambda([bShowingSecret, StandardProgress]() -> FText
										{
											return bShowingSecret
												? NSLOCTEXT("T66.Achievements", "SecretProgressMaskedPercent", "???")
												: FText::AsPercent(StandardProgress);
										})
										.Font(AchievementsBoldFont(17))
										.ColorAndOpacity(FT66Style::Tokens::TextMuted)
									]
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SProgressBar)
									.Percent_Lambda([bShowingSecret, StandardProgress]() -> TOptional<float>
									{
										return bShowingSecret ? 0.0f : StandardProgress;
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
							.SetFontSize(AdjustAchievementsFontSize(18)))
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
	UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettingsSubsystem();
	const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
	const FText FavoriteAchievementTooltip = NSLOCTEXT("T66.Achievements", "FavoriteAchievementTooltip", "Favorite achievement");
	const FText UnfavoriteAchievementTooltip = NSLOCTEXT("T66.Achievements", "UnfavoriteAchievementTooltip", "Remove favorite");

	auto AddAchievementSection = [this, Achievements, PlayerSettings, &NoBorderButtonStyle, &FavoriteAchievementTooltip, &UnfavoriteAchievementTooltip](const FText& SectionTitle, const TArray<FAchievementData>& SectionAchievements)
	{
		if (!AchievementListBox.IsValid())
		{
			return;
		}

		int32 UnlockedInSection = 0;
		for (const FAchievementData& Achievement : SectionAchievements)
		{
			if (Achievement.bIsUnlocked)
			{
				++UnlockedInSection;
			}
		}

		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(FText::Format(
				NSLOCTEXT("T66.Achievements", "SectionTitleFormat", "{0}  {1}/{2}"),
				SectionTitle,
				FText::AsNumber(UnlockedInSection),
				FText::AsNumber(SectionAchievements.Num())))
			.Font(AchievementsBoldFont(22))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		];

		for (const FAchievementData& Achievement : SectionAchievements)
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
			const bool bIsFavorited = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Achievement.AchievementID);
			const FText RewardText = BuildAchievementRewardText(Achievement);
			const FText ActionText = GetAchievementActionText(Achievement);
			const FText FavoriteGlyph = FText::FromString(bIsFavorited ? TEXT("\u2605") : TEXT("\u2606"));
			const FLinearColor FavoriteGlyphColor = bIsFavorited
				? FLinearColor(0.94f, 0.78f, 0.28f, 1.0f)
				: FLinearColor(0.55f, 0.58f, 0.62f, 1.0f);

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
							.Font(AchievementsBoldFont(19))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 3.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(Achievement.Description)
							.Font(AchievementsRegularFont(16))
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
						.Font(AchievementsBoldFont(19))
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
						[  
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Right)
							[
								SNew(STextBlock)
								.Text(bClaimed
									? (GetLocSubsystem() ? GetLocSubsystem()->GetText_Claimed() : NSLOCTEXT("T66.Achievements", "Claimed", "CLAIMED"))
									: RewardText)
								.Font(AchievementsBoldFont(19))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(10.f, 0.f, 0.f, 0.f)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(28.f)
								.HeightOverride(28.f)
								[
									SNew(SButton)
									.ButtonStyle(&NoBorderButtonStyle)
									.ContentPadding(FMargin(0.f))
									.IsEnabled(PlayerSettings != nullptr)
									.ToolTipText(bIsFavorited ? UnfavoriteAchievementTooltip : FavoriteAchievementTooltip)
									.OnClicked(FOnClicked::CreateLambda([this, PlayerSettings, AchievementID = Achievement.AchievementID]()
									{
										if (PlayerSettings)
										{
											PlayerSettings->SetFavoriteAchievement(AchievementID, !PlayerSettings->IsFavoriteAchievement(AchievementID));
											ForceRebuildSlate();
										}
										return FReply::Handled();
									}))
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.10f, 0.11f, 0.14f, 0.94f))
										.Padding(FMargin(0.f))
										[
											SNew(STextBlock)
											.Text(FavoriteGlyph)
											.Font(FT66Style::Tokens::FontRegular(20))
											.ColorAndOpacity(FavoriteGlyphColor)
											.Justification(ETextJustify::Center)
										]
									]
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Right)
						.Padding(0.f, 4.f, 0.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									ActionText,
									FOnClicked::CreateUObject(this, &UT66AchievementsScreen::HandleClaimClicked, Achievement.AchievementID),
									ET66ButtonType::Primary)
								.SetMinWidth(0.f)
								.SetFontSize(AdjustAchievementsFontSize(18))
								.SetEnabled(bCanClaim)
								.SetVisibility(bCanClaim ? EVisibility::Visible : EVisibility::Collapsed))
						]
					]
				]
			];
		}
	};

	if (ActiveTab == EAchievementTab::Achievements)
	{
		AddAchievementSection(
			NSLOCTEXT("T66.Achievements", "AchievementsHeader", "ACHIEVEMENTS"),
			GetAchievementsForCategory(ET66AchievementCategory::Standard));
		return;
	}

	const FText MaskedText = NSLOCTEXT("T66.Achievements", "SecretMaskedText", "???");
	AchievementListBox->AddSlot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 10.f)
	[
		SNew(STextBlock)
		.Text(MaskedText)
		.Font(AchievementsBoldFont(22))
		.ColorAndOpacity(FT66Style::Tokens::Text)
	];

	for (int32 RowIndex = 0; RowIndex < T66SecretPlaceholderRowCount; ++RowIndex)
	{
		AchievementListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 5.f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(RowIndex % 2 == 0 ? T66AchievementsRowFill() : T66AchievementsUnlockedRowFill())
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
						.Text(MaskedText)
						.Font(AchievementsBoldFont(19))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 3.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(MaskedText)
						.Font(AchievementsRegularFont(16))
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
					.Text(MaskedText)
					.Font(AchievementsBoldFont(19))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
						.Text(MaskedText)
						.Font(AchievementsBoldFont(19))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(
								MaskedText,
								FOnClicked::CreateLambda([]()
								{
									return FReply::Handled();
								}),
								ET66ButtonType::Primary)
							.SetMinWidth(0.f)
							.SetFontSize(AdjustAchievementsFontSize(18))
							.SetEnabled(false))
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

FReply UT66AchievementsScreen::HandleAchievementsTabClicked()
{
	if (ActiveTab == EAchievementTab::Achievements)
	{
		return FReply::Handled();
	}

	ActiveTab = EAchievementTab::Achievements;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AchievementsScreen::HandleSecretTabClicked()
{
	if (ActiveTab == EAchievementTab::Secret)
	{
		return FReply::Handled();
	}

	ActiveTab = EAchievementTab::Secret;
	ForceRebuildSlate();
	return FReply::Handled();
}

void UT66AchievementsScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}

void UT66AchievementsScreen::HandleAchievementsStateChanged()
{
	if (!HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	RebuildAchievementList();
}
