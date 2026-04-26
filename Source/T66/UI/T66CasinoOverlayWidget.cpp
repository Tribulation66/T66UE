// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CasinoOverlayWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/T66CasinoOverlayShared.h"
#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66ItemCardTextUtils.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66VendorOverlayWidget.h"
#include "UI/Style/T66OverlayChromeStyle.h"
#include "UI/Style/T66Style.h"
#include "Input/DragAndDrop.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace SharedOverlay = T66CasinoOverlayShared;

TSharedRef<SWidget> UT66CasinoOverlayWidget::RebuildWidget()
{
	UT66RunStateSubsystem* RunState = GetRunState();

	SharedOverlay::EnsureShellTabWidgets(
		this,
		GamblerTabWidget,
		VendorTabWidget,
		[this](UT66GamblerOverlayWidget* Widget)
		{
			Widget->SetEmbeddedInCasinoShell(true);
			Widget->SetWinGoldAmount(PendingGamblingWinGoldAmount);
		},
		[this](UT66VendorOverlayWidget* Widget)
		{
			Widget->SetEmbeddedInCasinoShell(true);
			Widget->SetVendorAllowsSteal(bVendorAllowsSteal);
		});

	if (RunState)
	{
		T66_RESET_COMMON_OVERLAY_RUNSTATE_DELEGATES(RunState, this, UT66CasinoOverlayWidget);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleStageTimerChanged);
		RunState->ScoreChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleStageTimerChanged);
	}

	SharedOverlay::ResizeArrays(
		UT66RunStateSubsystem::MaxInventorySlots,
		AlchemyInventorySlotBorders,
		AlchemyInventorySlotCountTexts,
		AlchemyInventorySlotTexts,
		AlchemyInventorySlotImages);
	const float AlchemyCardIconSize = FT66Style::Tokens::NPCCompactShopCardWidth - 10.f;
	SharedOverlay::EnsureBrushArray(
		AlchemyInventorySlotBrushes,
		UT66RunStateSubsystem::MaxInventorySlots,
		FVector2D(48.f, 48.f));
	SharedOverlay::EnsureBrush(AlchemyTargetIconBrush, FVector2D(AlchemyCardIconSize, AlchemyCardIconSize));
	SharedOverlay::EnsureBrush(AlchemySacrificeIconBrush, FVector2D(AlchemyCardIconSize, AlchemyCardIconSize));

	const FText GamblingTabText = NSLOCTEXT("T66.Casino", "TabGambling", "GAMBLING");
	const FText VendorTabText = NSLOCTEXT("T66.Casino", "TabVendor", "VENDOR");
	const FText AlchemyTabText = NSLOCTEXT("T66.Casino", "TabAlchemy", "ALCHEMY");
	const FText CloseText = NSLOCTEXT("T66.Casino", "Close", "CLOSE");
	const float HeaderPanelPaddingX = 8.f;
	const float HeaderPanelPaddingY = 6.f;
	const float HeaderLabelFontSize = 9.f;
	const float HeaderValueFontSize = 11.f;
	const float ShellTopPadding = 16.f;
	const float ShellSectionGap = 10.f;
	const float ShellOuterPadding = 16.f;
	const float ShellButtonFontSize = 14.f;
	const FMargin ShellButtonPadding(10.f, 6.f);
	const FMargin ShellTopBarPadding(10.f, 8.f);

	TSharedRef<SWidget> VendorPage = VendorTabWidget ? VendorTabWidget->TakeWidget() : SNullWidget::NullWidget;
	TSharedRef<SWidget> GamblingPage = GamblerTabWidget ? GamblerTabWidget->TakeWidget() : SNullWidget::NullWidget;
	TSharedRef<SWidget> AlchemyPage = BuildAlchemyPage(RunState, GetLocalization());
	TSharedRef<SWidget> HeaderSummaryPanel =
		T66OverlayChromeStyle::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Casino", "HeaderScoreLabel", "SCORE"))
					.Font(FT66Style::Tokens::FontBold(HeaderLabelFontSize))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SAssignNew(HeaderScoreText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(HeaderValueFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Casino", "HeaderTimeLabel", "TIME"))
					.Font(FT66Style::Tokens::FontBold(HeaderLabelFontSize))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SAssignNew(HeaderTimerText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(HeaderValueFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			],
			ET66OverlayChromeBrush::HeaderSummaryBar,
			FMargin(HeaderPanelPaddingX, HeaderPanelPaddingY)
		);

	const TAttribute<FMargin> VerticalSafeInsets = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		const FMargin SafeInsets = FT66Style::GetSafeFrameInsets();
		return FMargin(0.f, SafeInsets.Top, 0.f, SafeInsets.Bottom);
	});

	const TAttribute<FOptionalSize> SurfaceWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		return FOptionalSize(FMath::Max(1.f, FT66Style::GetViewportLogicalSize().X));
	});

	TSharedRef<SWidget> ShellLayout =
		SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Padding(VerticalSafeInsets)
			[
				SNew(SBox)
				.WidthOverride(SurfaceWidthAttr)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					.Padding(ShellOuterPadding, ShellTopPadding, ShellOuterPadding, 0.f)
					[
						T66OverlayChromeStyle::MakePanel(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								HeaderSummaryPanel
							]
							+ SHorizontalBox::Slot().FillWidth(1.f)
							[
								SNew(SSpacer)
							]
							+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
								[
									T66OverlayChromeStyle::MakeButton(
										T66OverlayChromeStyle::MakeButtonParams(
											VendorTabText,
											FOnClicked::CreateLambda([this]() { OpenVendorTab(); return FReply::Handled(); }),
											ET66OverlayChromeButtonFamily::Tab)
										.SetPadding(ShellButtonPadding)
										.SetFontSize(ShellButtonFontSize)
										.SetSelected(TAttribute<bool>::CreateLambda([this]() { return ActiveTab == ECasinoTab::Vendor; }))
								)
								]
								+ SHorizontalBox::Slot().AutoWidth()
								[
									T66OverlayChromeStyle::MakeButton(
										T66OverlayChromeStyle::MakeButtonParams(
											GamblingTabText,
											FOnClicked::CreateLambda([this]() { OpenGamblingTab(); return FReply::Handled(); }),
											ET66OverlayChromeButtonFamily::Tab)
										.SetPadding(ShellButtonPadding)
										.SetFontSize(ShellButtonFontSize)
										.SetSelected(TAttribute<bool>::CreateLambda([this]() { return ActiveTab == ECasinoTab::Gambling; }))
								)
								]
								+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
								[
									T66OverlayChromeStyle::MakeButton(
										T66OverlayChromeStyle::MakeButtonParams(
											AlchemyTabText,
											FOnClicked::CreateLambda([this]() { OpenAlchemyTab(); return FReply::Handled(); }),
											ET66OverlayChromeButtonFamily::Tab)
										.SetPadding(ShellButtonPadding)
										.SetFontSize(ShellButtonFontSize)
										.SetSelected(TAttribute<bool>::CreateLambda([this]() { return ActiveTab == ECasinoTab::Alchemy; }))
								)
								]
							]
							+ SHorizontalBox::Slot().FillWidth(1.f)
							[
								SNew(SSpacer)
							]
							+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right).VAlign(VAlign_Center)
							[
								T66OverlayChromeStyle::MakeButton(
									T66OverlayChromeStyle::MakeButtonParams(
										CloseText,
										FOnClicked::CreateLambda([this]() { CloseOverlay(); return FReply::Handled(); }),
										ET66OverlayChromeButtonFamily::Danger)
									.SetPadding(ShellButtonPadding)
									.SetFontSize(ShellButtonFontSize)
							)
							],
							ET66OverlayChromeBrush::HeaderSummaryBar,
							ShellTopBarPadding
						)
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					.Padding(ShellOuterPadding, ShellSectionGap, ShellOuterPadding, ShellOuterPadding)
					[
						SAssignNew(TabSwitcher, SWidgetSwitcher)
						+ SWidgetSwitcher::Slot()
						[
							VendorPage
						]
						+ SWidgetSwitcher::Slot()
						[
							GamblingPage
						]
						+ SWidgetSwitcher::Slot()
						[
							AlchemyPage
						]
					]
				]
			];

	TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.010f, 0.013f, 0.022f, 0.97f))
		]
		+ SOverlay::Slot()
		[
			ShellLayout
		];

	RefreshAlchemy();
	RefreshHeaderSummary();
	OpenVendorTab();
	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66CasinoOverlayWidget::NativeDestruct()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		T66_REMOVE_COMMON_OVERLAY_RUNSTATE_DELEGATES(RunState, this, UT66CasinoOverlayWidget);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleStageTimerChanged);
	}

	SharedOverlay::RemoveFromParentAndReset(VendorTabWidget);
	SharedOverlay::RemoveFromParentAndReset(GamblerTabWidget);

	Super::NativeDestruct();
}

void UT66CasinoOverlayWidget::CloseOverlay()
{
	SharedOverlay::CloseOverlay(this);
}

void UT66CasinoOverlayWidget::OpenGamblingTab()
{
	SharedOverlay::OpenGamblingTab(GamblerTabWidget, [this]() { SetActiveTab(ECasinoTab::Gambling); });
}

void UT66CasinoOverlayWidget::OpenVendorTab()
{
	SharedOverlay::OpenVendorTab(VendorTabWidget, [this]() { SetActiveTab(ECasinoTab::Vendor); });
}

void UT66CasinoOverlayWidget::OpenAlchemyTab()
{
	SharedOverlay::OpenAlchemyTab(
		[this]() { SetActiveTab(ECasinoTab::Alchemy); },
		[this]() { RefreshAlchemy(); });
}

void UT66CasinoOverlayWidget::SetGamblingWinGoldAmount(int32 InAmount)
{
	PendingGamblingWinGoldAmount = FMath::Max(0, InAmount);
	if (GamblerTabWidget)
	{
		GamblerTabWidget->SetWinGoldAmount(PendingGamblingWinGoldAmount);
	}
}

void UT66CasinoOverlayWidget::SetVendorAllowsSteal(bool bInAllowsSteal)
{
	bVendorAllowsSteal = bInAllowsSteal;
	if (VendorTabWidget)
	{
		VendorTabWidget->SetVendorAllowsSteal(bVendorAllowsSteal);
	}
}

TSharedRef<SWidget> UT66CasinoOverlayWidget::BuildAlchemyPage(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc)
{
	const FText NetWorthFmt = Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}");
	const FText GoldFmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
	const FText OweFmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");
	const float CardWidth = FT66Style::Tokens::NPCCompactShopCardWidth;
	const float CardHeight = FT66Style::Tokens::NPCCompactShopCardHeight;
	const float CardPadding = 5.f;
	const float CardNameBoxHeight = 28.f;
	const float CardIconSize = CardWidth - CardPadding * 2.f;
	const int32 TopBarFontSize = 15;
	const int32 TitleFontSize = 24;
	const int32 SectionFontSize = 12;
	const int32 CardHeadingFontSize = 9;
	const int32 CardBodyFontSize = 7;
	const int32 SmallFontSize = 11;
	const int32 ArrowFontSize = 24;
	const FMargin CompactButtonPadding(10.f, 6.f);
	const FText SourceCardHeading = NSLOCTEXT("T66.Casino", "FusionItem", "FUSE ITEM");
	const FText ResultCardHeading = NSLOCTEXT("T66.Casino", "FusionRecipe", "FUSION RESULT");
	const FText EmptyStateText = NSLOCTEXT("T66.Casino", "AlchemyNothingToUpgrade", "You have nothing to upgrade");
	const FText UpgradeText = NSLOCTEXT("T66.Casino", "UpgradeButton", "UPGRADE");

	auto MakeAlchemyCard = [&](const FText& HeadingText,
		TSharedPtr<STextBlock>& OutTitleText,
		TSharedPtr<STextBlock>& OutDetailText,
		TSharedPtr<SImage>& OutIconImage,
		TSharedPtr<FSlateBrush>& IconBrush,
		TSharedPtr<SBorder>& OutBorder) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(CardWidth)
			.HeightOverride(CardHeight)
			[
				T66OverlayChromeStyle::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(HeadingText)
						.Font(FT66Style::Tokens::FontBold(SectionFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(CardNameBoxHeight)
						[
							SAssignNew(OutTitleText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(CardHeadingFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(CardWidth - CardPadding * 2.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							T66OverlayChromeStyle::MakePanel(
								SNew(SBox)
								.WidthOverride(CardIconSize)
								.HeightOverride(CardIconSize)
								[
									SAssignNew(OutIconImage, SImage)
									.Image(IconBrush.Get())
									.ColorAndOpacity(FLinearColor::White)
									.Visibility(EVisibility::Hidden)
								],
								ET66OverlayChromeBrush::SlotNormal,
								FMargin(0.f))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(OutDetailText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontRegular(CardBodyFontSize))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(CardWidth - CardPadding * 2.f)
					],
					ET66OverlayChromeBrush::OfferCardNormal,
					FMargin(CardPadding),
					&OutBorder)
			];
	};

	TSharedRef<SWidget> UpgradeButtonWidget = T66OverlayChromeStyle::MakeButton(
		T66OverlayChromeStyle::MakeButtonParams(
			UpgradeText,
			FOnClicked::CreateUObject(this, &UT66CasinoOverlayWidget::OnAlchemyTransmuteClicked),
			ET66OverlayChromeButtonFamily::Primary)
	.SetMinWidth(CardWidth)
	.SetPadding(CompactButtonPadding)
	.SetFontSize(SectionFontSize)
);
	AlchemyUpgradeButton = UpgradeButtonWidget;

	TSharedRef<SWidget> RootWidget =
		T66OverlayChromeStyle::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Casino", "AlchemyTitle", "ALCHEMY"))
				.Font(FT66Style::Tokens::FontBold(TitleFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				T66OverlayChromeStyle::MakePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyNetWorthText, STextBlock)
						.Text(FText::Format(NetWorthFmt, FText::AsNumber(RunState ? RunState->GetNetWorth() : 0)))
						.Font(FT66Style::Tokens::FontBold(TopBarFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyGoldText, STextBlock)
						.Text(FText::Format(GoldFmt, FText::AsNumber(RunState ? RunState->GetCurrentGold() : 0)))
						.Font(FT66Style::Tokens::FontBold(TopBarFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyDebtText, STextBlock)
						.Text(FText::Format(OweFmt, FText::AsNumber(RunState ? RunState->GetCurrentDebt() : 0)))
						.Font(FT66Style::Tokens::FontBold(TopBarFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SAssignNew(AlchemyAngerText, STextBlock)
						.Text(FText::Format(NSLOCTEXT("T66.Casino", "AngerFormat", "ANGER: {0}%"), FText::AsNumber(RunState ? FMath::RoundToInt(RunState->GetCasinoAnger01() * 100.f) : 0)))
						.Font(FT66Style::Tokens::FontBold(TopBarFontSize))
						.ColorAndOpacity(FLinearColor(0.95f, 0.65f, 0.20f, 1.f))
					],
					ET66OverlayChromeBrush::HeaderSummaryBar,
					FMargin(10.f)
				)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
			[
				SAssignNew(AlchemyCardsRowContainer, SBox)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeAlchemyCard(
							SourceCardHeading,
							AlchemyTargetText,
							AlchemyTargetDetailText,
							AlchemyTargetIconImage,
							AlchemyTargetIconBrush,
							AlchemyTargetBorder)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Casino", "AlchemyArrow", "=>"))
						.Font(FT66Style::Tokens::FontBold(ArrowFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeAlchemyCard(
								ResultCardHeading,
								AlchemySacrificeText,
								AlchemySacrificeDetailText,
								AlchemySacrificeIconImage,
								AlchemySacrificeIconBrush,
								AlchemySacrificeBorder)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(CardWidth)
							[
								UpgradeButtonWidget
							]
						]
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
			[
				SAssignNew(AlchemyEmptyStateText, STextBlock)
				.Text(EmptyStateText)
				.Font(FT66Style::Tokens::FontBold(SectionFontSize))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Visibility(EVisibility::Collapsed)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
			[
				SAssignNew(AlchemyStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(SmallFontSize))
				.ColorAndOpacity(FLinearColor::White)
				.Justification(ETextJustify::Center)
			],
			ET66OverlayChromeBrush::ContentPanelWide,
			FMargin(20.f)
		);

	return RootWidget;
}

void UT66CasinoOverlayWidget::SetActiveTab(const ECasinoTab NewTab)
{
	ActiveTab = NewTab;
	if (TabSwitcher.IsValid())
	{
		TabSwitcher->SetActiveWidgetIndex(static_cast<int32>(ActiveTab));
	}
}

void UT66CasinoOverlayWidget::RefreshHeaderSummary()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	const int32 Score = RunState ? RunState->GetCurrentScore() : 0;
	const int32 TotalSeconds = RunState ? FMath::Max(0, FMath::CeilToInt(RunState->GetStageTimerSecondsRemaining())) : 0;
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;
	FNumberFormattingOptions TwoDigits;
	TwoDigits.MinimumIntegralDigits = 2;

	if (HeaderTimerText.IsValid())
	{
		HeaderTimerText->SetText(FText::Format(
			NSLOCTEXT("T66.Casino", "HeaderTimerValue", "{0}:{1}"),
			FText::AsNumber(Minutes),
			FText::AsNumber(Seconds, &TwoDigits)));
	}

	if (HeaderScoreText.IsValid())
	{
		HeaderScoreText->SetText(FText::AsNumber(Score));
	}
}

void UT66CasinoOverlayWidget::RefreshAlchemy()
{
	SharedOverlay::RefreshAlchemy(
		[this]() { RefreshAlchemyTopBar(); },
		[this]() { RefreshAlchemyInventory(); },
		[this]() { RefreshAlchemyDropTargets(); },
		AlchemyStatusText,
		AlchemyStatusMessage,
		AlchemyStatusColor);
}

void UT66CasinoOverlayWidget::RefreshAlchemyTopBar()
{
	SharedOverlay::RefreshAlchemyTopBar(
		GetRunState(),
		GetLocalization(),
		AlchemyNetWorthText,
		AlchemyGoldText,
		AlchemyDebtText,
		AlchemyAngerText);
}

void UT66CasinoOverlayWidget::RefreshAlchemyInventory()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState)
	{
		AlchemyTargetInventoryIndex = INDEX_NONE;
		return;
	}

	const TArray<FT66InventorySlot>& InventorySlots = RunState->GetInventorySlots();
	AlchemyTargetInventoryIndex = INDEX_NONE;
	for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
	{
		if (RunState->CanAlchemyUpgradeInventoryItemAt(Index))
		{
			AlchemyTargetInventoryIndex = Index;
			break;
		}
	}
	AlchemySacrificeInventoryIndex = INDEX_NONE;
}

void UT66CasinoOverlayWidget::RefreshAlchemyDropTargets()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FT66InventorySlot>* InventorySlots = RunState ? &RunState->GetInventorySlots() : nullptr;
	const bool bHasTarget = RunState && InventorySlots && InventorySlots->IsValidIndex(AlchemyTargetInventoryIndex);
	const FT66InventorySlot* SelectedTargetSlot = bHasTarget ? &(*InventorySlots)[AlchemyTargetInventoryIndex] : nullptr;
	FT66InventorySlot PreviewSlot;
	int32 MatchingCount = 0;
	const bool bHasPreview = RunState && SelectedTargetSlot && RunState->GetAlchemyUpgradePreviewAt(AlchemyTargetInventoryIndex, PreviewSlot, MatchingCount);
	const bool bBossActive = RunState && RunState->GetBossActive();
	const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;

	auto ClearCard = [](const TSharedPtr<SBorder>& Border,
		const TSharedPtr<SImage>& Image,
		const TSharedPtr<STextBlock>& TitleText,
		const TSharedPtr<STextBlock>& DetailText,
		const TSharedPtr<FSlateBrush>& Brush,
		const FLinearColor& Fill)
	{
		if (Border.IsValid())
		{
			Border->SetBorderBackgroundColor(Fill);
		}
		if (Brush.IsValid())
		{
			Brush->SetResourceObject(nullptr);
		}
		if (Image.IsValid())
		{
			Image->SetVisibility(EVisibility::Hidden);
		}
		if (TitleText.IsValid())
		{
			TitleText->SetText(FText::GetEmpty());
		}
		if (DetailText.IsValid())
		{
			DetailText->SetText(FText::GetEmpty());
		}
	};

	ClearCard(AlchemyTargetBorder, AlchemyTargetIconImage, AlchemyTargetText, AlchemyTargetDetailText, AlchemyTargetIconBrush, FT66Style::Tokens::Panel2);
	ClearCard(AlchemySacrificeBorder, AlchemySacrificeIconImage, AlchemySacrificeText, AlchemySacrificeDetailText, AlchemySacrificeIconBrush, FT66Style::Tokens::Panel2);

	if (AlchemyCardsRowContainer.IsValid())
	{
		AlchemyCardsRowContainer->SetVisibility(bHasPreview ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (AlchemyEmptyStateText.IsValid())
	{
		AlchemyEmptyStateText->SetVisibility(bHasPreview ? EVisibility::Collapsed : EVisibility::Visible);
	}
	if (AlchemyUpgradeButton.IsValid())
	{
		AlchemyUpgradeButton->SetVisibility(bHasPreview ? EVisibility::Visible : EVisibility::Collapsed);
		AlchemyUpgradeButton->SetEnabled(bHasPreview && !bBossActive);
	}

	if (!bHasPreview || !SelectedTargetSlot || !GI)
	{
		return;
	}

	FItemData TargetItemData;
	if (!GI->GetItemData(SelectedTargetSlot->ItemTemplateID, TargetItemData))
	{
		return;
	}

	if (AlchemyTargetBorder.IsValid())
	{
		AlchemyTargetBorder->SetBorderBackgroundColor(FT66Style::Tokens::Panel2);
	}
	if (AlchemySacrificeBorder.IsValid())
	{
		AlchemySacrificeBorder->SetBorderBackgroundColor(FT66Style::Tokens::Panel2 * 0.55f + FT66Style::Tokens::Success * 0.45f);
	}

	const FText TargetName = Loc
		? Loc->GetText_ItemDisplayNameForRarity(SelectedTargetSlot->ItemTemplateID, SelectedTargetSlot->Rarity)
		: FText::FromName(SelectedTargetSlot->ItemTemplateID);
	const FText PreviewName = Loc
		? Loc->GetText_ItemDisplayNameForRarity(PreviewSlot.ItemTemplateID, PreviewSlot.Rarity)
		: FText::FromName(PreviewSlot.ItemTemplateID);
	const FText TargetDesc = T66ItemCardTextUtils::BuildItemCardDescription(
		Loc,
		TargetItemData,
		SelectedTargetSlot->Rarity,
		SelectedTargetSlot->Line1RolledValue,
		ScaleMult,
		SelectedTargetSlot->GetLine2Multiplier());
	const FText PreviewDesc = T66ItemCardTextUtils::BuildItemCardDescription(
		Loc,
		TargetItemData,
		PreviewSlot.Rarity,
		PreviewSlot.Line1RolledValue,
		ScaleMult,
		PreviewSlot.GetLine2Multiplier());

	if (AlchemyTargetText.IsValid())
	{
		AlchemyTargetText->SetText(TargetName);
	}
	if (AlchemyTargetDetailText.IsValid())
	{
		AlchemyTargetDetailText->SetText(FText::Format(
			NSLOCTEXT("T66.Casino", "AlchemySourceCardDetail", "{0}/{1} matching copies\n{2}"),
			FText::AsNumber(MatchingCount),
			FText::AsNumber(UT66RunStateSubsystem::AlchemyCopiesRequired),
			TargetDesc));
	}
	if (AlchemySacrificeText.IsValid())
	{
		AlchemySacrificeText->SetText(PreviewName);
	}
	if (AlchemySacrificeDetailText.IsValid())
	{
		AlchemySacrificeDetailText->SetText(PreviewDesc);
	}

	const TSoftObjectPtr<UTexture2D> TargetIconSoft = TargetItemData.GetIconForRarity(SelectedTargetSlot->Rarity);
	if (!TargetIconSoft.IsNull() && TexPool && AlchemyTargetIconBrush.IsValid())
	{
		T66SlateTexture::BindSharedBrushAsync(TexPool, TargetIconSoft, this, AlchemyTargetIconBrush, FName(TEXT("CasinoAlchemyTargetCard")), true);
		if (AlchemyTargetIconImage.IsValid())
		{
			AlchemyTargetIconImage->SetVisibility(EVisibility::Visible);
		}
	}

	const TSoftObjectPtr<UTexture2D> PreviewIconSoft = TargetItemData.GetIconForRarity(PreviewSlot.Rarity);
	if (!PreviewIconSoft.IsNull() && TexPool && AlchemySacrificeIconBrush.IsValid())
	{
		T66SlateTexture::BindSharedBrushAsync(TexPool, PreviewIconSoft, this, AlchemySacrificeIconBrush, FName(TEXT("CasinoAlchemyPreviewCard")), true);
		if (AlchemySacrificeIconImage.IsValid())
		{
			AlchemySacrificeIconImage->SetVisibility(EVisibility::Visible);
		}
	}
}

void UT66CasinoOverlayWidget::SetAlchemyStatus(const FText& Message, const FLinearColor& Color)
{
	AlchemyStatusMessage = Message;
	AlchemyStatusColor = Color;
	SharedOverlay::ApplyAlchemyStatus(AlchemyStatusText, AlchemyStatusMessage, AlchemyStatusColor);
}

bool UT66CasinoOverlayWidget::TryAssignAlchemySlot(const bool bIsTargetSlot, const int32 InventoryIndex)
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState)
	{
		return false;
	}
	if (InventoryIndex < 0 || InventoryIndex >= RunState->GetInventorySlots().Num())
	{
		return false;
	}

	const FT66InventorySlot& SelectedSlot = RunState->GetInventorySlots()[InventoryIndex];
	if (!SelectedSlot.IsValid() || SelectedSlot.Rarity == ET66ItemRarity::White)
	{
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "TargetAlreadyMax", "That item cannot be fused any further."), FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return false;
	}
	if (RunState->GetAlchemyMatchingInventoryCount(InventoryIndex) <= 0)
	{
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "AlchemyInvalidItem", "Only regular Black, Red, or Yellow items can be fused."), FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return false;
	}

	AlchemyTargetInventoryIndex = InventoryIndex;
	if (!bIsTargetSlot)
	{
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "AlchemySingleTargetHint", "Alchemy only needs one selected item. Matching copies are consumed automatically."), FLinearColor(0.85f, 0.85f, 0.92f, 1.f));
	}
	else
	{
		SetAlchemyStatus(FText::GetEmpty(), FLinearColor::White);
	}

	RefreshAlchemy();
	return true;
}

FReply UT66CasinoOverlayWidget::HandleAlchemyInventoryDragDetected(const FGeometry&, const FPointerEvent&, const int32 InventoryIndex)
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UT66LocalizationSubsystem* Loc = GetLocalization();
	return SharedOverlay::BeginAlchemyInventoryDrag(
		RunState,
		InventoryIndex,
		AlchemyInventorySlotBrushes,
		[Loc, InventoryIndex](const TArray<FName>& Inventory, const TArray<FT66InventorySlot>&)
		{
			return Loc ? Loc->GetText_ItemDisplayName(Inventory[InventoryIndex]) : FText::FromName(Inventory[InventoryIndex]);
		});
}

FReply UT66CasinoOverlayWidget::HandleAlchemyDropTarget(const FGeometry&, const FDragDropEvent& DragDropEvent, const bool bIsTargetSlot)
{
	return SharedOverlay::HandleAlchemyDropTarget(
		DragDropEvent,
		[this, bIsTargetSlot](const int32 InventoryIndex)
		{
			return TryAssignAlchemySlot(bIsTargetSlot, InventoryIndex);
		});
}

FReply UT66CasinoOverlayWidget::OnAlchemyTransmuteClicked()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UT66LocalizationSubsystem* Loc = GetLocalization();
	if (!RunState)
	{
		return FReply::Handled();
	}
	if (RunState->GetBossActive())
	{
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "BossActive", "Boss is active."), FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return FReply::Handled();
	}
	if (AlchemyTargetInventoryIndex == INDEX_NONE)
	{
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "AlchemyNeedTarget", "You have nothing to upgrade."), FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return FReply::Handled();
	}

	FT66InventorySlot UpgradedSlot;
	int32 MatchingCount = 0;
	RunState->GetAlchemyUpgradePreviewAt(AlchemyTargetInventoryIndex, UpgradedSlot, MatchingCount);
	if (!RunState->TryAlchemyUpgradeInventoryItems(AlchemyTargetInventoryIndex, INDEX_NONE, UpgradedSlot))
	{
		SetAlchemyStatus(
			FText::Format(
				NSLOCTEXT("T66.Casino", "AlchemyNeedThree", "Alchemy needs {0} matching copies of the same item and rarity. You currently have {1}."),
				FText::AsNumber(UT66RunStateSubsystem::AlchemyCopiesRequired),
				FText::AsNumber(MatchingCount)),
			FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return FReply::Handled();
	}

	AlchemyTargetInventoryIndex = INDEX_NONE;
	const FText ItemName = Loc ? Loc->GetText_ItemDisplayName(UpgradedSlot.ItemTemplateID) : FText::FromName(UpgradedSlot.ItemTemplateID);
	const FText RarityName = Loc ? Loc->GetText_ItemRarityName(UpgradedSlot.Rarity) : FText::GetEmpty();
	SetAlchemyStatus(
		FText::Format(NSLOCTEXT("T66.Casino", "AlchemySuccess", "{0} upgraded to {1}."), ItemName, RarityName),
		FLinearColor(0.30f, 1.f, 0.40f, 1.f));
	RefreshAlchemy();

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->TriggerCasinoBossIfAngry();
	}
	return FReply::Handled();
}

FReply UT66CasinoOverlayWidget::OnClearAlchemyTargetClicked()
{
	AlchemyTargetInventoryIndex = INDEX_NONE;
	AlchemySacrificeInventoryIndex = INDEX_NONE;
	RefreshAlchemy();
	return FReply::Handled();
}

FReply UT66CasinoOverlayWidget::OnClearAlchemySacrificeClicked()
{
	AlchemySacrificeInventoryIndex = INDEX_NONE;
	RefreshAlchemy();
	return FReply::Handled();
}

UT66RunStateSubsystem* UT66CasinoOverlayWidget::GetRunState() const
{
	return SharedOverlay::GetRunState(this);
}

UT66LocalizationSubsystem* UT66CasinoOverlayWidget::GetLocalization() const
{
	return SharedOverlay::GetLocalization(this);
}

void UT66CasinoOverlayWidget::HandleInventoryChanged()
{
	AlchemyTargetInventoryIndex = INDEX_NONE;
	AlchemySacrificeInventoryIndex = INDEX_NONE;
	RefreshAlchemy();
}

void UT66CasinoOverlayWidget::HandleGoldOrDebtChanged()
{
	RefreshAlchemyTopBar();
}

void UT66CasinoOverlayWidget::HandleAngerChanged()
{
	RefreshAlchemyTopBar();
}

void UT66CasinoOverlayWidget::HandleBossChanged()
{
	RefreshAlchemy();
}

void UT66CasinoOverlayWidget::HandleScoreChanged()
{
	RefreshHeaderSummary();
}

void UT66CasinoOverlayWidget::HandleStageTimerChanged()
{
	RefreshHeaderSummary();
}
