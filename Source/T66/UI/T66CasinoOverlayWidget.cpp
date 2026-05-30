// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CasinoOverlayWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/T66CasinoOverlayShared.h"
#include "UI/T66CasinoGamblerTabWidget.h"
#include "UI/T66ItemCardTextUtils.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66CasinoVendorTabWidget.h"
#include "UI/Style/T66FlatStyle.h"
#include "Input/DragAndDrop.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace SharedOverlay = T66CasinoOverlayShared;

namespace
{
	const FName CasinoTabToggleGroup(TEXT("CasinoOverlay.TabSelection"));

	void AddVendorCanvasSlot(
		const TSharedRef<SConstraintCanvas>& Canvas,
		const float X,
		const float Y,
		const float W,
		const float H,
		const TSharedRef<SWidget>& Widget)
	{
		const float UiScale = FMath::Max(0.1f, FT66FlatStyle::GetGlobalUIScale());
		Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X / UiScale, Y / UiScale, W / UiScale, H / UiScale))
		[
			Widget
		];
	}

	TSharedRef<SWidget> MakeVendorHeaderButton(
		const ET66FlatState State,
		const FText& Label,
		FOnClicked OnClicked,
		const FName Tag)
	{
		return FT66FlatStyle::MakeFlatButton(
			State,
			Label,
			MoveTemp(OnClicked),
			nullptr,
			nullptr,
			FMargin(12.f, 8.f),
			0.f,
			0.f,
			true,
			28,
			Tag,
			CasinoTabToggleGroup);
	}

	TSharedRef<SWidget> MakeTaggedCasinoText(
		const FText& Text,
		const int32 FontSize,
		const FName Tag,
		const FLinearColor Color = FT66FlatStyle::PrimaryText())
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
				.Text(Text)
				.Font(FT66FlatStyle::MakeBoldFont(FontSize))
				.ColorAndOpacity(Color),
			Tag,
			TEXT("Label.Body"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	}
}

TSharedRef<SWidget> UT66CasinoOverlayWidget::RebuildWidget()
{
	UT66RunStateSubsystem* RunState = GetRunState();

	SharedOverlay::EnsureShellTabWidgets(
		this,
		CasinoGamblerTabWidget,
		VendorTabWidget,
		[this](UT66CasinoGamblerTabWidget* Widget)
		{
			Widget->SetEmbeddedInCasinoShell(true);
			Widget->SetGamblingOnlyKiosk(OverlayMode == ECasinoOverlayMode::GamblerOnly);
			Widget->SetWinGoldAmount(PendingCasinoGamblerWinGoldAmount);
		},
		[this](UT66CasinoVendorTabWidget* Widget)
		{
			Widget->SetEmbeddedInCasinoShell(true);
			Widget->SetShopAllowsSteal(bShopAllowsSteal);
		});

	if (RunState)
	{
		RunState->ScoreChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleStageTimerChanged);
		RunState->ScoreChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleStageTimerChanged);
	}

	const FText GamblerTabText = NSLOCTEXT("T66.Casino", "TabGambler", "GAMBLER");
	const FText ShopTabText = NSLOCTEXT("T66.Casino", "TabVendor", "VENDOR");
	const FText CloseText = NSLOCTEXT("T66.Casino", "Close", "CLOSE");
	const bool bGamblerOnly = OverlayMode == ECasinoOverlayMode::GamblerOnly;
	const bool bVendorOnly = OverlayMode == ECasinoOverlayMode::VendorOnly;
	const FText StatusText = bGamblerOnly
		? NSLOCTEXT("T66.Casino", "GamblerOnlyStatus", "CHOOSE A GAME. THIS CASINO CAN BE PLAYED ONCE.")
		: ActiveTab == ECasinoTab::Gambling
			? NSLOCTEXT("T66.Casino", "GamblerStatus", "WELCOME TO THE GAMBLER. FORTUNE FAVORS THE BOLD. CHOOSE YOUR GAME.")
			: bVendorOnly
				? NSLOCTEXT("T66.Casino", "VendorOnlyStatus", "VENDOR IS OPEN. BROWSE GOODS OR STEAL AT YOUR OWN RISK.")
				: NSLOCTEXT("T66.Casino", "VendorStatus", "VENDOR IS OPEN. BROWSE GOODS, BUY UPGRADES, OR MANAGE DEBT.");

	TSharedRef<SWidget> ShopPage = VendorTabWidget ? VendorTabWidget->TakeWidget() : SNullWidget::NullWidget;
	TSharedRef<SWidget> GamblerPage = CasinoGamblerTabWidget ? CasinoGamblerTabWidget->TakeWidget() : SNullWidget::NullWidget;

	TSharedRef<SConstraintCanvas> RootCanvas = SNew(SConstraintCanvas);
	AddVendorCanvasSlot(
		RootCanvas,
		0.f,
		0.f,
		1920.f,
		1080.f,
		FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor()),
			FName(TEXT("CasinoOverlay.Backdrop")),
			TEXT("Panel"),
			ET66FlatState::Default));

	AddVendorCanvasSlot(
		RootCanvas,
		0.f,
		0.f,
		1920.f,
		1080.f,
		SAssignNew(TabSwitcher, SWidgetSwitcher)
		+ SWidgetSwitcher::Slot()
		[
			ShopPage
		]
		+ SWidgetSwitcher::Slot()
		[
			GamblerPage
		]);

	AddVendorCanvasSlot(
		RootCanvas,
		17.f,
		18.f,
		219.f,
		81.f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(18.f, 7.f),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTaggedCasinoText(NSLOCTEXT("T66.Casino", "HeaderScoreLabel", "SCORE"), 19, FName(TEXT("CasinoOverlay.ScoreLabel")))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(14.f, 0.f, 0.f, 0.f)
				[
					FT66FlatStyle::AttachMetadata(
						SAssignNew(HeaderScoreText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::MakeBoldFont(19))
							.ColorAndOpacity(FT66FlatStyle::PrimaryText()),
						FName(TEXT("CasinoOverlay.ScoreValue")),
						TEXT("Label.Body"),
						ET66FlatState::Default,
						TOptional<FLinearColor>(),
						false,
						NAME_None,
						true)
					]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeTaggedCasinoText(NSLOCTEXT("T66.Casino", "HeaderTimeLabel", "TIME"), 19, FName(TEXT("CasinoOverlay.TimeLabel")))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(22.f, 0.f, 0.f, 0.f)
				[
					FT66FlatStyle::AttachMetadata(
						SAssignNew(HeaderTimerText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::MakeBoldFont(19))
							.ColorAndOpacity(FT66FlatStyle::PrimaryText()),
						FName(TEXT("CasinoOverlay.TimeValue")),
						TEXT("Label.Body"),
						ET66FlatState::Default,
						TOptional<FLinearColor>(),
						false,
						NAME_None,
						true)
				]
			],
			nullptr,
			FName(TEXT("CasinoOverlay.ScorePanel"))));

	AddVendorCanvasSlot(
		RootCanvas,
		372.f,
		20.f,
		518.f,
		79.f,
		SNew(SBox)
		.Visibility(bGamblerOnly ? EVisibility::Collapsed : EVisibility::Visible)
		[
			MakeVendorHeaderButton(
				ActiveTab == ECasinoTab::Shop ? ET66FlatState::Selected : ET66FlatState::Default,
				ShopTabText,
				FOnClicked::CreateLambda([this]() { OpenVendorTab(); return FReply::Handled(); }),
				FName(TEXT("CasinoOverlay.VendorTabButton")))
		]);

	AddVendorCanvasSlot(
		RootCanvas,
		bGamblerOnly ? 702.f : 912.f,
		20.f,
		bGamblerOnly ? 518.f : 477.f,
		79.f,
		SNew(SBox)
		.Visibility(bVendorOnly ? EVisibility::Collapsed : EVisibility::Visible)
		[
			MakeVendorHeaderButton(
				ActiveTab == ECasinoTab::Gambling ? ET66FlatState::Selected : ET66FlatState::Default,
				GamblerTabText,
				FOnClicked::CreateLambda([this]() { OpenGamblerTab(); return FReply::Handled(); }),
				FName(TEXT("CasinoOverlay.GamblingTabButton")))
		]);

	AddVendorCanvasSlot(
		RootCanvas,
		1658.f,
		20.f,
		261.f,
		79.f,
		FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Selected,
			CloseText,
			FOnClicked::CreateLambda([this]() { CloseOverlay(); return FReply::Handled(); }),
			nullptr,
			nullptr,
			FMargin(12.f, 8.f),
			0.f,
			0.f,
			true,
			28,
			FName(TEXT("CasinoOverlay.CloseButton"))));

	AddVendorCanvasSlot(
		RootCanvas,
		16.f,
		116.f,
		1902.f,
		48.f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(12.f, 8.f),
			SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
			[
				MakeTaggedCasinoText(StatusText, 22, FName(TEXT("CasinoOverlay.StatusText")), FT66FlatStyle::SecondaryText())
			],
			nullptr,
			FName(TEXT("CasinoOverlay.StatusBar"))));

	RefreshHeaderSummary();
	SetActiveTab(ActiveTab);
	return FT66FlatStyle::AttachMetadata(RootCanvas, FName(TEXT("CasinoOverlay.Root")), TEXT("Overlay"), ET66FlatState::Default);
}

void UT66CasinoOverlayWidget::NativeDestruct()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->ScoreChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleStageTimerChanged);
	}

	SharedOverlay::RemoveFromParentAndReset(VendorTabWidget);
	SharedOverlay::RemoveFromParentAndReset(CasinoGamblerTabWidget);
	ReleaseCachedSlateResources();

	Super::NativeDestruct();
}

void UT66CasinoOverlayWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	ReleaseCachedSlateResources();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UT66CasinoOverlayWidget::ReleaseCachedSlateResources()
{
	TabSwitcher.Reset();
	HeaderTimerText.Reset();
	HeaderScoreText.Reset();

	AlchemyNetWorthText.Reset();
	AlchemyGoldText.Reset();
	AlchemyDebtText.Reset();
	AlchemyStatusText.Reset();
	AlchemyTargetText.Reset();
	AlchemyTargetDetailText.Reset();
	AlchemySacrificeText.Reset();
	AlchemySacrificeDetailText.Reset();
	AlchemyEmptyStateText.Reset();
	AlchemyResultText.Reset();
	AlchemyCardsRowContainer.Reset();
	AlchemyUpgradeButton.Reset();
	AlchemyTargetBorder.Reset();
	AlchemySacrificeBorder.Reset();
	AlchemyTargetIconImage.Reset();
	AlchemySacrificeIconImage.Reset();
	AlchemyTargetIconBrush.Reset();
	AlchemySacrificeIconBrush.Reset();

	AlchemyInventorySlotBorders.Reset();
	AlchemyInventorySlotCountTexts.Reset();
	AlchemyInventorySlotTexts.Reset();
	AlchemyInventorySlotImages.Reset();
	AlchemyInventorySlotBrushes.Reset();

	AlchemyTargetInventoryIndex = INDEX_NONE;
	AlchemySacrificeInventoryIndex = INDEX_NONE;
}

void UT66CasinoOverlayWidget::CloseOverlay()
{
	SharedOverlay::CloseOverlay(this);
}

void UT66CasinoOverlayWidget::OpenGamblerTab()
{
	if (OverlayMode == ECasinoOverlayMode::VendorOnly)
	{
		OpenVendorTab();
		return;
	}

	SharedOverlay::OpenGamblerTab(CasinoGamblerTabWidget, [this]() { SetActiveTab(ECasinoTab::Gambling); });
	if (IsInViewport())
	{
		FT66FlatStyle::DeferRebuild(this, 100);
	}
}

void UT66CasinoOverlayWidget::OpenVendorTab()
{
	if (OverlayMode == ECasinoOverlayMode::GamblerOnly)
	{
		OpenGamblerTab();
		return;
	}

	SharedOverlay::OpenVendorTab(VendorTabWidget, [this]() { SetActiveTab(ECasinoTab::Shop); });
	if (IsInViewport())
	{
		FT66FlatStyle::DeferRebuild(this, 100);
	}
}

void UT66CasinoOverlayWidget::OpenAlchemyTab()
{
	OpenVendorTab();
}

void UT66CasinoOverlayWidget::SetOverlayMode(const ECasinoOverlayMode InMode)
{
	OverlayMode = InMode;

	if (CasinoGamblerTabWidget)
	{
		CasinoGamblerTabWidget->SetGamblingOnlyKiosk(OverlayMode == ECasinoOverlayMode::GamblerOnly);
	}

	if (OverlayMode == ECasinoOverlayMode::GamblerOnly && ActiveTab != ECasinoTab::Gambling)
	{
		SetActiveTab(ECasinoTab::Gambling);
	}
	else if (OverlayMode == ECasinoOverlayMode::VendorOnly && ActiveTab != ECasinoTab::Shop)
	{
		SetActiveTab(ECasinoTab::Shop);
	}
}

void UT66CasinoOverlayWidget::SetCasinoGamblerWinGoldAmount(int32 InAmount)
{
	PendingCasinoGamblerWinGoldAmount = FMath::Max(0, InAmount);
	if (CasinoGamblerTabWidget)
	{
		CasinoGamblerTabWidget->SetWinGoldAmount(PendingCasinoGamblerWinGoldAmount);
	}
}

void UT66CasinoOverlayWidget::SetShopAllowsSteal(bool bInAllowsSteal)
{
	bShopAllowsSteal = bInAllowsSteal;
	if (VendorTabWidget)
	{
		VendorTabWidget->SetShopAllowsSteal(bShopAllowsSteal);
	}
}

TSharedRef<SWidget> UT66CasinoOverlayWidget::BuildAlchemyPage(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc)
{
	const FText NetWorthFmt = Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}");
	const FText GoldFmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
	const FText OweFmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");
	const float CardWidth = FT66FlatStyle::Tokens::NPCCompactShopCardWidth;
	const float CardHeight = FT66FlatStyle::Tokens::NPCCompactShopCardHeight;
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
				FT66FlatStyle::MakeFlatOverlayPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(HeadingText)
						.Font(FT66FlatStyle::Tokens::FontBold(SectionFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(CardNameBoxHeight)
						[
							SAssignNew(OutTitleText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::Tokens::FontBold(CardHeadingFontSize))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(CardWidth - CardPadding * 2.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							FT66FlatStyle::MakeFlatOverlayPanel(
								SNew(SBox)
								.WidthOverride(CardIconSize)
								.HeightOverride(CardIconSize)
								[
									FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
										SAssignNew(OutIconImage, SImage)
										.Image(IconBrush.Get())
										.ColorAndOpacity(FLinearColor::White)
										.Visibility(EVisibility::Hidden)),
										NAME_None,
										TEXT("Icon"))
								],
								ET66FlatOverlayChromeBrush::SlotNormal,
								FMargin(0.f))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(OutDetailText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66FlatStyle::Tokens::FontRegular(CardBodyFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(CardWidth - CardPadding * 2.f)
					],
					ET66FlatOverlayChromeBrush::OfferCardNormal,
					FMargin(CardPadding),
					&OutBorder)
			];
	};

	TSharedRef<SWidget> UpgradeButtonWidget = FT66FlatStyle::MakeFlatOverlayButton(
		FT66FlatStyle::MakeFlatOverlayButtonParams(
			UpgradeText,
			FOnClicked::CreateUObject(this, &UT66CasinoOverlayWidget::OnAlchemyTransmuteClicked),
			ET66FlatOverlayChromeButtonFamily::Primary)
	.SetMinWidth(CardWidth)
	.SetPadding(CompactButtonPadding)
	.SetFontSize(SectionFontSize)
);
	AlchemyUpgradeButton = UpgradeButtonWidget;

	TSharedRef<SWidget> RootWidget =
		FT66FlatStyle::MakeFlatOverlayPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Casino", "AlchemyTitle", "ALCHEMY"))
				.Font(FT66FlatStyle::Tokens::FontBold(TitleFontSize))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[
				FT66FlatStyle::MakeFlatOverlayPanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyNetWorthText, STextBlock)
						.Text(FText::Format(NetWorthFmt, FText::AsNumber(RunState ? RunState->GetNetWorth() : 0)))
						.Font(FT66FlatStyle::Tokens::FontBold(TopBarFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyGoldText, STextBlock)
						.Text(FText::Format(GoldFmt, FText::AsNumber(RunState ? RunState->GetCurrentGold() : 0)))
						.Font(FT66FlatStyle::Tokens::FontBold(TopBarFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyDebtText, STextBlock)
						.Text(FText::Format(OweFmt, FText::AsNumber(RunState ? RunState->GetCurrentDebt() : 0)))
						.Font(FT66FlatStyle::Tokens::FontBold(TopBarFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					],
					ET66FlatOverlayChromeBrush::HeaderSummaryBar,
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
						.Font(FT66FlatStyle::Tokens::FontBold(ArrowFontSize))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
				.Font(FT66FlatStyle::Tokens::FontBold(SectionFontSize))
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				.Visibility(EVisibility::Collapsed)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
			[
				SAssignNew(AlchemyStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66FlatStyle::Tokens::FontBold(SmallFontSize))
				.ColorAndOpacity(FLinearColor::White)
				.Justification(ETextJustify::Center)
			],
			ET66FlatOverlayChromeBrush::ContentPanelWide,
			FMargin(20.f)
		);

	return RootWidget;
}

void UT66CasinoOverlayWidget::SetActiveTab(const ECasinoTab NewTab)
{
	ActiveTab = NewTab;
	if (OverlayMode == ECasinoOverlayMode::GamblerOnly)
	{
		ActiveTab = ECasinoTab::Gambling;
	}
	else if (OverlayMode == ECasinoOverlayMode::VendorOnly)
	{
		ActiveTab = ECasinoTab::Shop;
	}

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
		AlchemyDebtText);
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

	ClearCard(AlchemyTargetBorder, AlchemyTargetIconImage, AlchemyTargetText, AlchemyTargetDetailText, AlchemyTargetIconBrush, FT66FlatStyle::Tokens::Panel2);
	ClearCard(AlchemySacrificeBorder, AlchemySacrificeIconImage, AlchemySacrificeText, AlchemySacrificeDetailText, AlchemySacrificeIconBrush, FT66FlatStyle::Tokens::Panel2);

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
		AlchemyTargetBorder->SetBorderBackgroundColor(FT66FlatStyle::Tokens::Panel2);
	}
	if (AlchemySacrificeBorder.IsValid())
	{
		AlchemySacrificeBorder->SetBorderBackgroundColor(FT66FlatStyle::Tokens::Panel2 * 0.55f + FT66FlatStyle::Tokens::Success * 0.45f);
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
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "AlchemySingleTargetHint", "Alchemy only needs one selected item. Matching copies are consumed automatically."), FLinearColor(0.92f, 0.82f, 0.78f, 1.f));
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
