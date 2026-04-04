// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CircusOverlayWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66ItemCardTextUtils.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66VendorOverlayWidget.h"
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

namespace
{
	DECLARE_DELEGATE_RetVal_TwoParams(FReply, FCircusAlchemyOnBeginDrag, const FGeometry&, const FPointerEvent&);

	static FString MakeAlchemyStackKey(const FT66InventorySlot& Slot)
	{
		return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
	}

	class SCircusAlchemyDropBorder : public SBorder
	{
	public:
		SLATE_BEGIN_ARGS(SCircusAlchemyDropBorder)
			: _BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			, _BorderBackgroundColor(FLinearColor::White)
			, _Padding(FMargin(0.f))
		{}
			SLATE_ARGUMENT(const FSlateBrush*, BorderImage)
			SLATE_ARGUMENT(FSlateColor, BorderBackgroundColor)
			SLATE_ARGUMENT(FMargin, Padding)
			SLATE_EVENT(FOnDrop, OnDropHandler)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			OnDropHandler = InArgs._OnDropHandler;
			SBorder::Construct(
				SBorder::FArguments()
				.BorderImage(InArgs._BorderImage)
				.BorderBackgroundColor(InArgs._BorderBackgroundColor)
				.Padding(InArgs._Padding)
				[
					InArgs._Content.Widget
				]);
		}

		virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
		{
			return OnDropHandler.IsBound() ? OnDropHandler.Execute(MyGeometry, DragDropEvent) : FReply::Unhandled();
		}

	private:
		FOnDrop OnDropHandler;
	};

	class SCircusAlchemyInventoryTile : public SBorder
	{
	public:
		SLATE_BEGIN_ARGS(SCircusAlchemyInventoryTile)
			: _BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			, _BorderBackgroundColor(FLinearColor::White)
			, _Padding(FMargin(0.f))
		{}
			SLATE_ARGUMENT(const FSlateBrush*, BorderImage)
			SLATE_ARGUMENT(FSlateColor, BorderBackgroundColor)
			SLATE_ARGUMENT(FMargin, Padding)
			SLATE_EVENT(FCircusAlchemyOnBeginDrag, OnBeginDragHandler)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			OnBeginDragHandler = InArgs._OnBeginDragHandler;
			SBorder::Construct(
				SBorder::FArguments()
				.BorderImage(InArgs._BorderImage)
				.BorderBackgroundColor(InArgs._BorderBackgroundColor)
				.Padding(InArgs._Padding)
				[
					InArgs._Content.Widget
				]);
		}

		virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
			}
			return SBorder::OnMouseButtonDown(MyGeometry, MouseEvent);
		}

		virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			return OnBeginDragHandler.IsBound() ? OnBeginDragHandler.Execute(MyGeometry, MouseEvent) : FReply::Unhandled();
		}

	private:
		FCircusAlchemyOnBeginDrag OnBeginDragHandler;
	};

	class FCircusAlchemyDragDropOp : public FDragDropOperation
	{
	public:
		DRAG_DROP_OPERATOR_TYPE(FCircusAlchemyDragDropOp, FDragDropOperation)

		int32 InventoryIndex = INDEX_NONE;
		FText Label;
		TSharedPtr<FSlateBrush> IconBrush;

		static TSharedRef<FCircusAlchemyDragDropOp> New(int32 InInventoryIndex, const FText& InLabel, const TSharedPtr<FSlateBrush>& InIconBrush)
		{
			TSharedRef<FCircusAlchemyDragDropOp> Op = MakeShared<FCircusAlchemyDragDropOp>();
			Op->InventoryIndex = InInventoryIndex;
			Op->Label = InLabel;
			Op->IconBrush = InIconBrush.IsValid() ? InIconBrush : MakeShared<FSlateBrush>();
			Op->Construct();
			return Op;
		}

		virtual void Construct() override
		{
			MouseCursor = EMouseCursor::GrabHandClosed;
			Decorator =
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 0.97f))
				.Padding(FMargin(12.f, 10.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(72.f)
						.HeightOverride(72.f)
						[
							SNew(SImage)
							.Image(IconBrush.Get())
							.ColorAndOpacity(FLinearColor::White)
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Label)
						.ColorAndOpacity(FLinearColor::White)
						.Font(FT66Style::Tokens::FontBold(16))
					]
				];

			// Let Slate create the cursor decorator window for this drag operation.
			FDragDropOperation::Construct();
		}

		virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
		{
			return Decorator;
		}

	private:
		TSharedPtr<SWidget> Decorator;
	};
}

TSharedRef<SWidget> UT66CircusOverlayWidget::RebuildWidget()
{
	UT66RunStateSubsystem* RunState = GetRunState();

	if (!GamblerTabWidget)
	{
		GamblerTabWidget = CreateWidget<UT66GamblerOverlayWidget>(GetOwningPlayer(), UT66GamblerOverlayWidget::StaticClass());
		if (GamblerTabWidget)
		{
			GamblerTabWidget->SetEmbeddedInCasinoShell(true);
		}
	}
	if (!VendorTabWidget)
	{
		VendorTabWidget = CreateWidget<UT66VendorOverlayWidget>(GetOwningPlayer(), UT66VendorOverlayWidget::StaticClass());
		if (VendorTabWidget)
		{
			VendorTabWidget->SetEmbeddedInCircusShell(true);
			VendorTabWidget->SetVendorAllowsSteal(true);
		}
	}
	if (RunState)
	{
		RunState->InventoryChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleInventoryChanged);
		RunState->GoldChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleGoldOrDebtChanged);
		RunState->GamblerAngerChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleAngerChanged);
		RunState->BossChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleBossChanged);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleStageTimerChanged);
		RunState->InventoryChanged.AddDynamic(this, &UT66CircusOverlayWidget::HandleInventoryChanged);
		RunState->GoldChanged.AddDynamic(this, &UT66CircusOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.AddDynamic(this, &UT66CircusOverlayWidget::HandleGoldOrDebtChanged);
		RunState->GamblerAngerChanged.AddDynamic(this, &UT66CircusOverlayWidget::HandleAngerChanged);
		RunState->BossChanged.AddDynamic(this, &UT66CircusOverlayWidget::HandleBossChanged);
		RunState->ScoreChanged.AddDynamic(this, &UT66CircusOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.AddDynamic(this, &UT66CircusOverlayWidget::HandleStageTimerChanged);
	}

	AlchemyInventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotCountTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotImages.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	for (int32 Index = 0; Index < UT66RunStateSubsystem::MaxInventorySlots; ++Index)
	{
		if (!AlchemyInventorySlotBrushes[Index].IsValid())
		{
			AlchemyInventorySlotBrushes[Index] = MakeShared<FSlateBrush>();
			AlchemyInventorySlotBrushes[Index]->ImageSize = FVector2D(56.f, 56.f);
		}
	}
	if (!AlchemyTargetIconBrush.IsValid())
	{
		AlchemyTargetIconBrush = MakeShared<FSlateBrush>();
		AlchemyTargetIconBrush->ImageSize = FVector2D(76.f, 76.f);
	}
	if (!AlchemySacrificeIconBrush.IsValid())
	{
		AlchemySacrificeIconBrush = MakeShared<FSlateBrush>();
		AlchemySacrificeIconBrush->ImageSize = FVector2D(76.f, 76.f);
	}

	const FText GamblingTabText = NSLOCTEXT("T66.Casino", "TabGambling", "GAMBLING");
	const FText VendorTabText = NSLOCTEXT("T66.Casino", "TabVendor", "VENDOR");
	const FText AlchemyTabText = NSLOCTEXT("T66.Casino", "TabAlchemy", "ALCHEMY");
	const FText CloseText = NSLOCTEXT("T66.Casino", "Close", "CLOSE");

	TSharedRef<SWidget> VendorPage = VendorTabWidget ? VendorTabWidget->TakeWidget() : SNullWidget::NullWidget;
	TSharedRef<SWidget> GamblingPage = GamblerTabWidget ? GamblerTabWidget->TakeWidget() : SNullWidget::NullWidget;
	TSharedRef<SWidget> AlchemyPage = BuildAlchemyPage(RunState, GetLocalization());
	TSharedRef<SWidget> HeaderSummaryPanel =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Casino", "HeaderScoreLabel", "SCORE"))
					.Font(FT66Style::Tokens::FontBold(15))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SAssignNew(HeaderScoreText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(18))
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
					.Font(FT66Style::Tokens::FontBold(15))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SAssignNew(HeaderTimerText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			],
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 10.f))
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

	TSharedRef<SWidget> Root =
		FT66Style::MakePanel(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Padding(VerticalSafeInsets)
			[
				SNew(SBox)
				.WidthOverride(SurfaceWidthAttr)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					.Padding(24.f, 24.f, 24.f, 0.f)
					[
						FT66Style::MakePanel(
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
									FT66Style::MakeButton(
										FT66ButtonParams(
											VendorTabText,
											FOnClicked::CreateLambda([this]() { OpenVendorTab(); return FReply::Handled(); }),
											ET66ButtonType::Primary)
										.SetPadding(FMargin(18.f, 10.f))
									)
								]
								+ SHorizontalBox::Slot().AutoWidth()
								[
									FT66Style::MakeButton(
										FT66ButtonParams(
											GamblingTabText,
											FOnClicked::CreateLambda([this]() { OpenGamblingTab(); return FReply::Handled(); }),
											ET66ButtonType::Primary)
										.SetPadding(FMargin(18.f, 10.f))
									)
								]
								+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
								[
									FT66Style::MakeButton(
										FT66ButtonParams(
											AlchemyTabText,
											FOnClicked::CreateLambda([this]() { OpenAlchemyTab(); return FReply::Handled(); }),
											ET66ButtonType::Primary)
										.SetPadding(FMargin(18.f, 10.f))
									)
								]
							]
							+ SHorizontalBox::Slot().FillWidth(1.f)
							[
								SNew(SSpacer)
							]
							+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right).VAlign(VAlign_Center)
							[
								FT66Style::MakeButton(
									FT66ButtonParams(
										CloseText,
										FOnClicked::CreateLambda([this]() { CloseOverlay(); return FReply::Handled(); }),
										ET66ButtonType::Danger)
									.SetPadding(FMargin(18.f, 10.f))
								)
							],
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(16.f, 12.f))
						)
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					.Padding(24.f, 16.f, 24.f, 24.f)
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
			],
			FT66PanelParams(ET66PanelType::Bg).SetPadding(0.f).SetColor(FT66Style::Tokens::Bg)
		);

	RefreshAlchemy();
	RefreshHeaderSummary();
	OpenVendorTab();
	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66CircusOverlayWidget::NativeDestruct()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->InventoryChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleInventoryChanged);
		RunState->GoldChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleGoldOrDebtChanged);
		RunState->GamblerAngerChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleAngerChanged);
		RunState->BossChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleBossChanged);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleScoreChanged);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66CircusOverlayWidget::HandleStageTimerChanged);
	}

	Super::NativeDestruct();
}

void UT66CircusOverlayWidget::CloseOverlay()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

void UT66CircusOverlayWidget::OpenGamblingTab()
{
	if (GamblerTabWidget)
	{
		GamblerTabWidget->OpenCasinoPage();
	}
	SetActiveTab(ECasinoTab::Gambling);
}

void UT66CircusOverlayWidget::OpenVendorTab()
{
	if (VendorTabWidget)
	{
		VendorTabWidget->OpenShopPage();
	}
	SetActiveTab(ECasinoTab::Vendor);
}

void UT66CircusOverlayWidget::OpenAlchemyTab()
{
	SetActiveTab(ECasinoTab::Alchemy);
	RefreshAlchemy();
}

TSharedRef<SWidget> UT66CircusOverlayWidget::BuildAlchemyPage(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc)
{
	const FText NetWorthFmt = Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}");
	const FText GoldFmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
	const FText OweFmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");

	TSharedRef<SUniformGridPanel> InventoryGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(8.f));

	for (int32 InventoryIndex = 0; InventoryIndex < UT66RunStateSubsystem::MaxInventorySlots; ++InventoryIndex)
	{
		TSharedPtr<SCircusAlchemyInventoryTile> TileBorder;
		TSharedRef<SCircusAlchemyInventoryTile> Tile =
			SAssignNew(TileBorder, SCircusAlchemyInventoryTile)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(0.f)
			.Cursor(EMouseCursor::GrabHand)
			.OnBeginDragHandler(FCircusAlchemyOnBeginDrag::CreateLambda([this, InventoryIndex](const FGeometry& Geometry, const FPointerEvent& MouseEvent)
			{
				return HandleAlchemyInventoryDragDetected(Geometry, MouseEvent, InventoryIndex);
			}))
			[
				SNew(SBox)
				.WidthOverride(84.f)
				.HeightOverride(84.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(AlchemyInventorySlotImages[InventoryIndex], SImage)
						.Image(AlchemyInventorySlotBrushes[InventoryIndex].Get())
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 4.f, 6.f, 0.f)
					[
						SAssignNew(AlchemyInventorySlotCountTexts[InventoryIndex], STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.ShadowOffset(FVector2D(1.f, 1.f))
						.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
						.Visibility(EVisibility::Hidden)
					]
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SAssignNew(AlchemyInventorySlotTexts[InventoryIndex], STextBlock)
						.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
			];

		AlchemyInventorySlotBorders[InventoryIndex] = StaticCastSharedPtr<SBorder>(TileBorder);
		InventoryGrid->AddSlot(InventoryIndex % 10, InventoryIndex / 10)
		[
			Tile
		];
	}

	const FText TargetText = NSLOCTEXT("T66.Casino", "FusionItem", "FUSE ITEM");
	const FText SacrificeText = NSLOCTEXT("T66.Casino", "FusionRecipe", "FUSION RESULT");
	const FText DragHereText = NSLOCTEXT("T66.Casino", "DragHere", "Drag an item here");
	const FText RecipeHintText = NSLOCTEXT("T66.Casino", "AlchemyRecipeHint", "Select one Black, Red, or Yellow item. Alchemy consumes 3 matching copies and returns 1 higher-tier copy with about 10% more total stats.");
	const FText UpgradeText = NSLOCTEXT("T66.Casino", "Transmute", "FUSE");
	const FText ClearText = NSLOCTEXT("T66.Common", "Clear", "CLEAR");
	TSharedPtr<SCircusAlchemyDropBorder> TargetDropBorder;
	TSharedPtr<SCircusAlchemyDropBorder> SacrificeDropBorder;

	TSharedRef<SWidget> RootWidget =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Casino", "AlchemyTitle", "ALCHEMY"))
				.Font(FT66Style::Tokens::FontBold(34))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
			[
				FT66Style::MakePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyNetWorthText, STextBlock)
						.Text(FText::Format(NetWorthFmt, FText::AsNumber(RunState ? RunState->GetNetWorth() : 0)))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyGoldText, STextBlock)
						.Text(FText::Format(GoldFmt, FText::AsNumber(RunState ? RunState->GetCurrentGold() : 0)))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(AlchemyDebtText, STextBlock)
						.Text(FText::Format(OweFmt, FText::AsNumber(RunState ? RunState->GetCurrentDebt() : 0)))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SAssignNew(AlchemyAngerText, STextBlock)
						.Text(FText::Format(NSLOCTEXT("T66.Casino", "AngerFormat", "ANGER: {0}%"), FText::AsNumber(RunState ? FMath::RoundToInt(RunState->GetCasinoAnger01() * 100.f) : 0)))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FLinearColor(0.95f, 0.65f, 0.20f, 1.f))
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f))
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 16.f, 0.f)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
						[
							SNew(STextBlock)
							.Text(TargetText)
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(TargetDropBorder, SCircusAlchemyDropBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FT66Style::Tokens::Panel2)
							.Padding(FMargin(12.f))
							.OnDropHandler(FOnDrop::CreateUObject(this, &UT66CircusOverlayWidget::HandleAlchemyDropTarget, true))
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f).VAlign(VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(76.f)
									.HeightOverride(76.f)
									[
										SAssignNew(AlchemyTargetIconImage, SImage)
										.Image(AlchemyTargetIconBrush.Get())
										.ColorAndOpacity(FLinearColor::White)
									]
								]
								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									SAssignNew(AlchemyTargetText, STextBlock)
									.Text(DragHereText)
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.AutoWrapText(true)
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									ClearText,
									FOnClicked::CreateUObject(this, &UT66CircusOverlayWidget::OnClearAlchemyTargetClicked),
									ET66ButtonType::Neutral)
								.SetPadding(FMargin(12.f, 8.f))
							)
						],
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f))
					)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 32.f, 16.f, 0.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Casino", "AlchemyArrow", "=>"))
					.Font(FT66Style::Tokens::FontBold(30))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
						[
							SNew(STextBlock)
							.Text(SacrificeText)
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(SacrificeDropBorder, SCircusAlchemyDropBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FT66Style::Tokens::Panel2)
							.Padding(FMargin(12.f))
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f).VAlign(VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(76.f)
									.HeightOverride(76.f)
									[
										SAssignNew(AlchemySacrificeIconImage, SImage)
										.Image(AlchemySacrificeIconBrush.Get())
										.ColorAndOpacity(FLinearColor::White)
									]
								]
								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									SAssignNew(AlchemySacrificeText, STextBlock)
									.Text(RecipeHintText)
									.Font(FT66Style::Tokens::FontBold(16))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.AutoWrapText(true)
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Casino", "AlchemyFusionRule", "3 matching copies => 1 next-tier item"))
							.Font(FT66Style::Tokens::FontBold(14))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						],
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f))
					)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
			[
				SAssignNew(AlchemyResultText, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(18))
				.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.92f, 1.f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						UpgradeText,
						FOnClicked::CreateUObject(this, &UT66CircusOverlayWidget::OnAlchemyTransmuteClicked),
						ET66ButtonType::Primary)
					.SetPadding(FMargin(18.f, 12.f))
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
			[
				SAssignNew(AlchemyStatusText, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(FLinearColor::White)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Casino", "InventoryTitle", "INVENTORY"))
						.Font(FT66Style::Tokens::FontBold(20))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						InventoryGrid
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f))
				)
			],
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(28.f))
		);

	AlchemyTargetBorder = StaticCastSharedPtr<SBorder>(TargetDropBorder);
	AlchemySacrificeBorder = StaticCastSharedPtr<SBorder>(SacrificeDropBorder);
	return RootWidget;
}

void UT66CircusOverlayWidget::SetActiveTab(const ECasinoTab NewTab)
{
	ActiveTab = NewTab;
	if (TabSwitcher.IsValid())
	{
		TabSwitcher->SetActiveWidgetIndex(static_cast<int32>(ActiveTab));
	}
}

void UT66CircusOverlayWidget::RefreshHeaderSummary()
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

void UT66CircusOverlayWidget::RefreshAlchemy()
{
	RefreshAlchemyTopBar();
	RefreshAlchemyInventory();
	RefreshAlchemyDropTargets();
	if (AlchemyStatusText.IsValid())
	{
		AlchemyStatusText->SetText(AlchemyStatusMessage);
		AlchemyStatusText->SetColorAndOpacity(AlchemyStatusColor);
	}
}

void UT66CircusOverlayWidget::RefreshAlchemyTopBar()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UT66LocalizationSubsystem* Loc = GetLocalization();
	if (!RunState)
	{
		return;
	}

	if (AlchemyNetWorthText.IsValid())
	{
		const int32 NetWorth = RunState->GetNetWorth();
		const FText Fmt = Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}");
		AlchemyNetWorthText->SetText(FText::Format(Fmt, FText::AsNumber(NetWorth)));
		const FLinearColor NetWorthColor = NetWorth > 0
			? FT66Style::Tokens::Success
			: (NetWorth < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text);
		AlchemyNetWorthText->SetColorAndOpacity(NetWorthColor);
	}

	if (AlchemyGoldText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
		AlchemyGoldText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentGold())));
	}
	if (AlchemyDebtText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");
		AlchemyDebtText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentDebt())));
	}
	if (AlchemyAngerText.IsValid())
	{
		const int32 Pct = FMath::RoundToInt(RunState->GetCasinoAnger01() * 100.f);
		AlchemyAngerText->SetText(FText::Format(NSLOCTEXT("T66.Casino", "AngerFormat", "ANGER: {0}%"), FText::AsNumber(Pct)));
	}
}

void UT66CircusOverlayWidget::RefreshAlchemyInventory()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	const TArray<FName> Inventory = RunState->GetInventory();
	const TArray<FT66InventorySlot>& InventorySlots = RunState->GetInventorySlots();
	TMap<FString, int32> StackCounts;
	for (const FT66InventorySlot& InventorySlotData : InventorySlots)
	{
		if (InventorySlotData.IsValid())
		{
			StackCounts.FindOrAdd(MakeAlchemyStackKey(InventorySlotData))++;
		}
	}

	for (int32 Index = 0; Index < AlchemyInventorySlotTexts.Num(); ++Index)
	{
		const bool bHasItem = Inventory.IsValidIndex(Index) && !Inventory[Index].IsNone() && InventorySlots.IsValidIndex(Index);
		FLinearColor Fill = FT66Style::Tokens::Panel2;
		FItemData ItemData;
		const bool bHasData = bHasItem && GI && GI->GetItemData(Inventory[Index], ItemData);

		if (AlchemyInventorySlotBorders.IsValidIndex(Index) && AlchemyInventorySlotBorders[Index].IsValid())
		{
			const bool bSelected = Index == AlchemyTargetInventoryIndex;
			AlchemyInventorySlotBorders[Index]->SetBorderBackgroundColor(bSelected ? (Fill * 0.45f + FT66Style::Tokens::Accent * 0.55f) : Fill);
		}
		const int32 StackCount = (bHasItem && InventorySlots.IsValidIndex(Index) && InventorySlots[Index].IsValid())
			? StackCounts.FindRef(MakeAlchemyStackKey(InventorySlots[Index]))
			: 0;
		if (AlchemyInventorySlotCountTexts.IsValidIndex(Index) && AlchemyInventorySlotCountTexts[Index].IsValid())
		{
			AlchemyInventorySlotCountTexts[Index]->SetText(
				StackCount > 1
					? FText::Format(NSLOCTEXT("T66.Inventory", "StackCountFormat", "{0}x"), FText::AsNumber(StackCount))
					: FText::GetEmpty());
			AlchemyInventorySlotCountTexts[Index]->SetVisibility(StackCount > 1 ? EVisibility::Visible : EVisibility::Hidden);
		}
		if (AlchemyInventorySlotTexts.IsValidIndex(Index) && AlchemyInventorySlotTexts[Index].IsValid())
		{
			AlchemyInventorySlotTexts[Index]->SetText(bHasItem ? FText::GetEmpty() : NSLOCTEXT("T66.Common", "Dash", "-"));
		}
		if (AlchemyInventorySlotImages.IsValidIndex(Index) && AlchemyInventorySlotImages[Index].IsValid())
		{
			const ET66ItemRarity SlotRarity = bHasItem ? InventorySlots[Index].Rarity : ET66ItemRarity::Black;
			const TSoftObjectPtr<UTexture2D> IconSoft = bHasData ? ItemData.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
			if (!IconSoft.IsNull() && TexPool && AlchemyInventorySlotBrushes.IsValidIndex(Index) && AlchemyInventorySlotBrushes[Index].IsValid())
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IconSoft, this, AlchemyInventorySlotBrushes[Index], FName(TEXT("CasinoAlchemyInv"), Index + 1), true);
			}
			else if (AlchemyInventorySlotBrushes.IsValidIndex(Index) && AlchemyInventorySlotBrushes[Index].IsValid())
			{
				AlchemyInventorySlotBrushes[Index]->SetResourceObject(nullptr);
			}
			AlchemyInventorySlotImages[Index]->SetVisibility(bHasData && !ItemData.GetIconForRarity(SlotRarity).IsNull() ? EVisibility::Visible : EVisibility::Hidden);
		}
	}

	if (AlchemyTargetInventoryIndex >= InventorySlots.Num())
	{
		AlchemyTargetInventoryIndex = INDEX_NONE;
	}
	if (AlchemyTargetInventoryIndex != INDEX_NONE)
	{
		const FT66InventorySlot& TargetSlot = InventorySlots[AlchemyTargetInventoryIndex];
		if (!TargetSlot.IsValid()
			|| TargetSlot.Rarity == ET66ItemRarity::White
			|| RunState->GetAlchemyMatchingInventoryCount(AlchemyTargetInventoryIndex) <= 0)
		{
			AlchemyTargetInventoryIndex = INDEX_NONE;
		}
	}
}

void UT66CircusOverlayWidget::RefreshAlchemyDropTargets()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	TArray<FName> Inventory;
	const TArray<FT66InventorySlot>* InventorySlots = nullptr;
	if (RunState)
	{
		Inventory = RunState->GetInventory();
		InventorySlots = &RunState->GetInventorySlots();
	}

	auto RefreshDropSlot = [&](int32 InventoryIndex, const TSharedPtr<SBorder>& Border, const TSharedPtr<SImage>& Image, const TSharedPtr<STextBlock>& Text, const TSharedPtr<FSlateBrush>& Brush)
	{
		const bool bHasItem = Inventory.IsValidIndex(InventoryIndex) && !Inventory[InventoryIndex].IsNone() && InventorySlots && InventorySlots->IsValidIndex(InventoryIndex);
		FLinearColor Fill = FT66Style::Tokens::Panel2;
		if (Text.IsValid())
		{
			Text->SetText(NSLOCTEXT("T66.Casino", "DragHere", "Drag an item here"));
		}
		if (Brush.IsValid())
		{
			Brush->SetResourceObject(nullptr);
		}
		if (Image.IsValid())
		{
			Image->SetVisibility(EVisibility::Hidden);
		}
		if (!bHasItem)
		{
			if (Border.IsValid())
			{
				Border->SetBorderBackgroundColor(Fill);
			}
			return;
		}

		FItemData ItemData;
		if (!GI || !GI->GetItemData(Inventory[InventoryIndex], ItemData))
		{
			return;
		}

		const FT66InventorySlot& Slot = (*InventorySlots)[InventoryIndex];
		if (Border.IsValid())
		{
			Border->SetBorderBackgroundColor(Fill);
		}
		if (Text.IsValid())
		{
			const FText ItemName = Loc ? Loc->GetText_ItemDisplayName(Slot.ItemTemplateID) : FText::FromName(Slot.ItemTemplateID);
			const FText RarityName = Loc ? Loc->GetText_ItemRarityName(Slot.Rarity) : FText::GetEmpty();
			Text->SetText(FText::Format(NSLOCTEXT("T66.Casino", "AlchemySlotLabel", "{0} ({1})"), ItemName, RarityName));
		}
		if (Image.IsValid())
		{
			const TSoftObjectPtr<UTexture2D> IconSoft = ItemData.GetIconForRarity(Slot.Rarity);
			if (!IconSoft.IsNull() && TexPool && Brush.IsValid())
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IconSoft, this, Brush, FName(TEXT("CasinoAlchemyTarget")), true);
				Image->SetVisibility(EVisibility::Visible);
			}
			else
			{
				Image->SetVisibility(EVisibility::Hidden);
			}
		}
	};

	RefreshDropSlot(AlchemyTargetInventoryIndex, AlchemyTargetBorder, AlchemyTargetIconImage, AlchemyTargetText, AlchemyTargetIconBrush);

	FT66InventorySlot PreviewSlot;
	int32 MatchingCount = 0;
	const bool bHasPreview = RunState && RunState->GetAlchemyUpgradePreviewAt(AlchemyTargetInventoryIndex, PreviewSlot, MatchingCount);
	const bool bHasTarget = RunState && InventorySlots && InventorySlots->IsValidIndex(AlchemyTargetInventoryIndex);
	const FT66InventorySlot* SelectedTargetSlot = bHasTarget ? &(*InventorySlots)[AlchemyTargetInventoryIndex] : nullptr;

	if (AlchemySacrificeBorder.IsValid())
	{
		AlchemySacrificeBorder->SetBorderBackgroundColor(FT66Style::Tokens::Panel2);
	}
	if (AlchemySacrificeIconBrush.IsValid())
	{
		AlchemySacrificeIconBrush->SetResourceObject(nullptr);
	}
	if (AlchemySacrificeIconImage.IsValid())
	{
		AlchemySacrificeIconImage->SetVisibility(EVisibility::Hidden);
	}
	if (AlchemySacrificeText.IsValid())
	{
		AlchemySacrificeText->SetText(NSLOCTEXT("T66.Casino", "AlchemyRecipeHint", "Select one Black, Red, or Yellow item. Alchemy consumes 3 matching copies and returns 1 higher-tier copy with about 10% more total stats."));
	}

	if (SelectedTargetSlot && SelectedTargetSlot->IsValid())
	{
		FItemData ItemData;
		if (GI && GI->GetItemData(SelectedTargetSlot->ItemTemplateID, ItemData))
		{
			const ET66ItemRarity NextRarity = UT66RunStateSubsystem::GetNextItemRarity(SelectedTargetSlot->Rarity);
			const FText ItemName = Loc ? Loc->GetText_ItemDisplayName(SelectedTargetSlot->ItemTemplateID) : FText::FromName(SelectedTargetSlot->ItemTemplateID);
			const FText FromRarity = Loc ? Loc->GetText_ItemRarityName(SelectedTargetSlot->Rarity) : FText::GetEmpty();
			const FText ToRarity = Loc ? Loc->GetText_ItemRarityName(NextRarity) : FText::GetEmpty();

			if (AlchemySacrificeText.IsValid())
			{
				if (bHasPreview)
				{
					AlchemySacrificeText->SetText(FText::Format(
						NSLOCTEXT("T66.Casino", "AlchemyReadyText", "Ready: {0}/{1} matching copies\n{2}: {3} -> {4}"),
						FText::AsNumber(MatchingCount),
						FText::AsNumber(UT66RunStateSubsystem::AlchemyCopiesRequired),
						ItemName,
						FromRarity,
						ToRarity));
				}
				else
				{
					AlchemySacrificeText->SetText(FText::Format(
						NSLOCTEXT("T66.Casino", "AlchemyNeedMoreText", "{0}/{1} matching copies\nNeed {2} more {3} {4} item(s)."),
						FText::AsNumber(MatchingCount),
						FText::AsNumber(UT66RunStateSubsystem::AlchemyCopiesRequired),
						FText::AsNumber(FMath::Max(0, UT66RunStateSubsystem::AlchemyCopiesRequired - MatchingCount)),
						FromRarity,
						ItemName));
				}
			}

			if (AlchemySacrificeIconImage.IsValid())
			{
				const TSoftObjectPtr<UTexture2D> PreviewIconSoft = ItemData.GetIconForRarity(NextRarity);
				if (!PreviewIconSoft.IsNull() && TexPool && AlchemySacrificeIconBrush.IsValid())
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PreviewIconSoft, this, AlchemySacrificeIconBrush, FName(TEXT("CasinoAlchemyPreview")), true);
					AlchemySacrificeIconImage->SetVisibility(EVisibility::Visible);
				}
			}
		}
	}

	if (AlchemyResultText.IsValid())
	{
		AlchemyResultText->SetText(FText::GetEmpty());
		if (bHasPreview && SelectedTargetSlot && GI)
		{
			FItemData ItemData;
			if (GI->GetItemData(SelectedTargetSlot->ItemTemplateID, ItemData))
			{
				const FText ItemName = Loc ? Loc->GetText_ItemDisplayName(PreviewSlot.ItemTemplateID) : FText::FromName(PreviewSlot.ItemTemplateID);
				const FText RarityName = Loc ? Loc->GetText_ItemRarityName(PreviewSlot.Rarity) : FText::GetEmpty();
				const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
				const FText Desc = T66ItemCardTextUtils::BuildItemCardDescription(Loc, ItemData, PreviewSlot.Rarity, PreviewSlot.Line1RolledValue, ScaleMult, PreviewSlot.GetLine2Multiplier());
				AlchemyResultText->SetText(FText::Format(
					NSLOCTEXT("T66.Casino", "AlchemyPreviewFormat", "{0} ({1})\n{2}"),
					ItemName,
					RarityName,
					Desc));
			}
		}
	}
}

void UT66CircusOverlayWidget::SetAlchemyStatus(const FText& Message, const FLinearColor& Color)
{
	AlchemyStatusMessage = Message;
	AlchemyStatusColor = Color;
	if (AlchemyStatusText.IsValid())
	{
		AlchemyStatusText->SetText(AlchemyStatusMessage);
		AlchemyStatusText->SetColorAndOpacity(AlchemyStatusColor);
	}
}

bool UT66CircusOverlayWidget::TryAssignAlchemySlot(const bool bIsTargetSlot, const int32 InventoryIndex)
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

FReply UT66CircusOverlayWidget::HandleAlchemyInventoryDragDetected(const FGeometry&, const FPointerEvent&, const int32 InventoryIndex)
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UT66LocalizationSubsystem* Loc = GetLocalization();
	if (!RunState)
	{
		return FReply::Unhandled();
	}

	const TArray<FName> Inventory = RunState->GetInventory();
	if (!Inventory.IsValidIndex(InventoryIndex) || Inventory[InventoryIndex].IsNone())
	{
		return FReply::Unhandled();
	}

	const FText Label = Loc ? Loc->GetText_ItemDisplayName(Inventory[InventoryIndex]) : FText::FromName(Inventory[InventoryIndex]);
	TSharedPtr<FSlateBrush> DragIconBrush = MakeShared<FSlateBrush>();
	DragIconBrush->DrawAs = ESlateBrushDrawType::Image;
	DragIconBrush->ImageSize = FVector2D(72.f, 72.f);
	if (AlchemyInventorySlotBrushes.IsValidIndex(InventoryIndex) && AlchemyInventorySlotBrushes[InventoryIndex].IsValid())
	{
		*DragIconBrush = *AlchemyInventorySlotBrushes[InventoryIndex];
		DragIconBrush->DrawAs = ESlateBrushDrawType::Image;
		DragIconBrush->ImageSize = FVector2D(72.f, 72.f);
	}

	return FReply::Handled().BeginDragDrop(FCircusAlchemyDragDropOp::New(
		InventoryIndex,
		Label,
		DragIconBrush));
}

FReply UT66CircusOverlayWidget::HandleAlchemyDropTarget(const FGeometry&, const FDragDropEvent& DragDropEvent, const bool bIsTargetSlot)
{
	const TSharedPtr<FCircusAlchemyDragDropOp> DragOp = DragDropEvent.GetOperationAs<FCircusAlchemyDragDropOp>();
	if (!DragOp.IsValid())
	{
		return FReply::Unhandled();
	}

	return TryAssignAlchemySlot(bIsTargetSlot, DragOp->InventoryIndex) ? FReply::Handled() : FReply::Unhandled();
}

FReply UT66CircusOverlayWidget::OnAlchemyTransmuteClicked()
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
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "AlchemyNeedTarget", "Drag one item into the alchemy slot to choose what to fuse."), FLinearColor(1.f, 0.35f, 0.35f, 1.f));
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
		FText::Format(NSLOCTEXT("T66.Casino", "AlchemySuccess", "{0} fused into {1}."), ItemName, RarityName),
		FLinearColor(0.30f, 1.f, 0.40f, 1.f));
	RefreshAlchemy();

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->TriggerCircusBossIfAngry();
	}
	return FReply::Handled();
}

FReply UT66CircusOverlayWidget::OnClearAlchemyTargetClicked()
{
	AlchemyTargetInventoryIndex = INDEX_NONE;
	AlchemySacrificeInventoryIndex = INDEX_NONE;
	RefreshAlchemy();
	return FReply::Handled();
}

FReply UT66CircusOverlayWidget::OnClearAlchemySacrificeClicked()
{
	AlchemySacrificeInventoryIndex = INDEX_NONE;
	RefreshAlchemy();
	return FReply::Handled();
}

UT66RunStateSubsystem* UT66CircusOverlayWidget::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

UT66LocalizationSubsystem* UT66CircusOverlayWidget::GetLocalization() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
}

void UT66CircusOverlayWidget::HandleInventoryChanged()
{
	AlchemyTargetInventoryIndex = INDEX_NONE;
	AlchemySacrificeInventoryIndex = INDEX_NONE;
	RefreshAlchemy();
}

void UT66CircusOverlayWidget::HandleGoldOrDebtChanged()
{
	RefreshAlchemyTopBar();
}

void UT66CircusOverlayWidget::HandleAngerChanged()
{
	RefreshAlchemyTopBar();
}

void UT66CircusOverlayWidget::HandleBossChanged()
{
	RefreshAlchemy();
}

void UT66CircusOverlayWidget::HandleScoreChanged()
{
	RefreshHeaderSummary();
}

void UT66CircusOverlayWidget::HandleStageTimerChanged()
{
	RefreshHeaderSummary();
}

