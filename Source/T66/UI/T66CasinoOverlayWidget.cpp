// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CasinoOverlayWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/T66GamblerOverlayWidget.h"
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
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	DECLARE_DELEGATE_RetVal_TwoParams(FReply, FCasinoAlchemyOnBeginDrag, const FGeometry&, const FPointerEvent&);

	static FString MakeAlchemyStackKey(const FT66InventorySlot& Slot)
	{
		return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
	}

	class SCasinoAlchemyDropBorder : public SBorder
	{
	public:
		SLATE_BEGIN_ARGS(SCasinoAlchemyDropBorder)
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

	class SCasinoAlchemyInventoryTile : public SBorder
	{
	public:
		SLATE_BEGIN_ARGS(SCasinoAlchemyInventoryTile)
			: _BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			, _BorderBackgroundColor(FLinearColor::White)
			, _Padding(FMargin(0.f))
		{}
			SLATE_ARGUMENT(const FSlateBrush*, BorderImage)
			SLATE_ARGUMENT(FSlateColor, BorderBackgroundColor)
			SLATE_ARGUMENT(FMargin, Padding)
			SLATE_EVENT(FCasinoAlchemyOnBeginDrag, OnBeginDragHandler)
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
		FCasinoAlchemyOnBeginDrag OnBeginDragHandler;
	};

	class FCasinoAlchemyDragDropOp : public FDragDropOperation
	{
	public:
		DRAG_DROP_OPERATOR_TYPE(FCasinoAlchemyDragDropOp, FDragDropOperation)

		int32 InventoryIndex = INDEX_NONE;
		FText Label;
		TSharedPtr<FSlateBrush> IconBrush;

		static TSharedRef<FCasinoAlchemyDragDropOp> New(int32 InInventoryIndex, const FText& InLabel, const TSharedPtr<FSlateBrush>& InIconBrush)
		{
			TSharedRef<FCasinoAlchemyDragDropOp> Op = MakeShared<FCasinoAlchemyDragDropOp>();
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

TSharedRef<SWidget> UT66CasinoOverlayWidget::RebuildWidget()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UT66LocalizationSubsystem* Loc = GetLocalization();

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
			VendorTabWidget->SetEmbeddedInCasinoShell(true);
		}
	}
	if (RunState)
	{
		RunState->InventoryChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleInventoryChanged);
		RunState->GoldChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleGoldOrDebtChanged);
		RunState->GamblerAngerChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleAngerChanged);
		RunState->BossChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleBossChanged);
		RunState->InventoryChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleInventoryChanged);
		RunState->GoldChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleGoldOrDebtChanged);
		RunState->GamblerAngerChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleAngerChanged);
		RunState->BossChanged.AddDynamic(this, &UT66CasinoOverlayWidget::HandleBossChanged);
	}

	AlchemyInventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotButtons.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotCountTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotImages.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	AlchemyInventorySlotBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	for (int32 Index = 0; Index < UT66RunStateSubsystem::MaxInventorySlots; ++Index)
	{
		if (!AlchemyInventorySlotBrushes[Index].IsValid())
		{
			AlchemyInventorySlotBrushes[Index] = MakeShared<FSlateBrush>();
			AlchemyInventorySlotBrushes[Index]->DrawAs = ESlateBrushDrawType::Image;
			AlchemyInventorySlotBrushes[Index]->ImageSize = FVector2D(FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize);
		}
	}
	if (!AlchemyTargetIconBrush.IsValid())
	{
		AlchemyTargetIconBrush = MakeShared<FSlateBrush>();
		AlchemyTargetIconBrush->DrawAs = ESlateBrushDrawType::Image;
		AlchemyTargetIconBrush->ImageSize = FVector2D(FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize);
	}
	if (!AlchemySacrificeIconBrush.IsValid())
	{
		AlchemySacrificeIconBrush = MakeShared<FSlateBrush>();
		AlchemySacrificeIconBrush->DrawAs = ESlateBrushDrawType::Image;
		AlchemySacrificeIconBrush->ImageSize = FVector2D(FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize);
	}

	const FText GamblingTabText = NSLOCTEXT("T66.Casino", "TabGambling", "GAMBLING");
	const FText VendorTabText = NSLOCTEXT("T66.Casino", "TabVendor", "VENDOR");
	const FText AlchemyTabText = NSLOCTEXT("T66.Casino", "TabAlchemy", "ALCHEMY");
	const FText CloseText = NSLOCTEXT("T66.Casino", "Close", "CLOSE");

	TSharedRef<SWidget> AlchemyPage = BuildAlchemyPage(RunState, Loc);
	TSharedRef<SWidget> VendorPage = VendorTabWidget ? VendorTabWidget->TakeWidget() : SNullWidget::NullWidget;
	TSharedRef<SWidget> GamblingPage = GamblerTabWidget ? GamblerTabWidget->TakeWidget() : SNullWidget::NullWidget;

	const TAttribute<FMargin> SafeHeaderPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafePadding(FMargin(24.f));
	});

	const TAttribute<FMargin> SafePagePadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafePadding(FMargin(24.f, 96.f, 24.f, 24.f));
	});

	TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Bg)
		]
		+ SOverlay::Slot()
		.Padding(SafePagePadding)
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
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(SafeHeaderPadding)
		[
			FT66Style::MakePanel(
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
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							GamblingTabText,
							FOnClicked::CreateLambda([this]() { OpenGamblingTab(); return FReply::Handled(); }),
							ET66ButtonType::Primary)
						.SetPadding(FMargin(18.f, 10.f))
					)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							AlchemyTabText,
							FOnClicked::CreateLambda([this]() { OpenAlchemyTab(); return FReply::Handled(); }),
							ET66ButtonType::Primary)
						.SetPadding(FMargin(18.f, 10.f))
					)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							CloseText,
							FOnClicked::CreateLambda([this]() { CloseOverlay(); return FReply::Handled(); }),
							ET66ButtonType::Danger)
						.SetPadding(FMargin(18.f, 10.f))
					)
				],
				FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f))
			)
		];

	RefreshAlchemy();
	OpenVendorTab();
	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66CasinoOverlayWidget::NativeDestruct()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->InventoryChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleInventoryChanged);
		RunState->GoldChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleGoldOrDebtChanged);
		RunState->GamblerAngerChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleAngerChanged);
		RunState->BossChanged.RemoveDynamic(this, &UT66CasinoOverlayWidget::HandleBossChanged);
	}

	if (VendorTabWidget)
	{
		VendorTabWidget->RemoveFromParent();
		VendorTabWidget = nullptr;
	}

	if (GamblerTabWidget)
	{
		GamblerTabWidget->RemoveFromParent();
		GamblerTabWidget = nullptr;
	}

	Super::NativeDestruct();
}

void UT66CasinoOverlayWidget::CloseOverlay()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

void UT66CasinoOverlayWidget::OpenGamblingTab()
{
	if (GamblerTabWidget)
	{
		GamblerTabWidget->OpenCasinoPage();
	}
	SetActiveTab(ECasinoTab::Gambling);
}

void UT66CasinoOverlayWidget::OpenVendorTab()
{
	if (VendorTabWidget)
	{
		VendorTabWidget->OpenShopPage();
	}
	SetActiveTab(ECasinoTab::Vendor);
}

void UT66CasinoOverlayWidget::OpenAlchemyTab()
{
	SetActiveTab(ECasinoTab::Alchemy);
	RefreshAlchemy();
}

TSharedRef<SWidget> UT66CasinoOverlayWidget::BuildAlchemyPage(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc)
{
	(void)RunState;
	(void)Loc;

	const float InventorySlotSize = FT66Style::Tokens::InventorySlotSize;
	const float UpgradePanelWidth = InventorySlotSize + 56.f;
	const FText UpgradeText = NSLOCTEXT("T66.Casino", "UpgradeButton", "UPGRADE");
	const FText EmptyTargetText = NSLOCTEXT("T66.Casino", "AlchemyEmptyTarget", "No upgradable item");
	const FText EmptyResultText = NSLOCTEXT("T66.Casino", "AlchemyEmptyResult", "Upgrade result");

	TSharedRef<SUniformGridPanel> InventoryGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f));

	for (int32 InventoryIndex = 0; InventoryIndex < UT66RunStateSubsystem::MaxInventorySlots; ++InventoryIndex)
	{
		TSharedRef<SWidget> SlotButton = FT66Style::MakeButton(
			FT66ButtonParams(
				FText::GetEmpty(),
				FOnClicked::CreateLambda([this, InventoryIndex]()
				{
					TryAssignAlchemySlot(true, InventoryIndex);
					return FReply::Handled();
				}),
				ET66ButtonType::Neutral)
			.SetMinWidth(InventorySlotSize)
			.SetHeight(InventorySlotSize)
			.SetPadding(FMargin(0.f))
			.SetContent(
				FT66Style::MakePanel(
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(AlchemyInventorySlotImages[InventoryIndex], SImage)
						.Image(AlchemyInventorySlotBrushes[InventoryIndex].Get())
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 6.f, 8.f, 0.f)
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
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(0.f)),
					&AlchemyInventorySlotBorders[InventoryIndex]))
		);

		AlchemyInventorySlotButtons[InventoryIndex] = SlotButton;
		InventoryGrid->AddSlot(InventoryIndex, 0)
		[
			SNew(SBox)
			.WidthOverride(InventorySlotSize)
			.HeightOverride(InventorySlotSize)
			[
				SlotButton
			]
		];
	}

	TSharedRef<SWidget> UpgradeButtonWidget = FT66Style::MakeButton(
		FT66ButtonParams(
			UpgradeText,
			FOnClicked::CreateUObject(this, &UT66CasinoOverlayWidget::OnAlchemyTransmuteClicked),
			ET66ButtonType::Success)
		.SetMinWidth(UpgradePanelWidth)
		.SetPadding(FMargin(18.f, 12.f))
	);
	AlchemyUpgradeButton = UpgradeButtonWidget;

	return FT66Style::MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 20.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.Casino", "AlchemyTitle", "ALCHEMY"))
			.Font(FT66Style::Tokens::FontBold(34))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 18.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(UpgradePanelWidth)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(InventorySlotSize)
							.HeightOverride(InventorySlotSize)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SAssignNew(AlchemyTargetIconImage, SImage)
									.Image(AlchemyTargetIconBrush.Get())
									.ColorAndOpacity(FLinearColor::White)
									.Visibility(EVisibility::Hidden)
								]
								+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 6.f, 8.f, 0.f)
								[
									SAssignNew(AlchemyTargetCountText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontBold(14))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.ShadowOffset(FVector2D(1.f, 1.f))
									.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
									.Visibility(EVisibility::Hidden)
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 0.f)
						[
							SAssignNew(AlchemyTargetText, STextBlock)
							.Text(EmptyTargetText)
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
						],
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(18.f)),
						&AlchemyTargetBorder)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(20.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Casino", "AlchemyArrow", "->"))
				.Font(FT66Style::Tokens::FontBold(42))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(UpgradePanelWidth)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(InventorySlotSize)
								.HeightOverride(InventorySlotSize)
								[
									SAssignNew(AlchemySacrificeIconImage, SImage)
									.Image(AlchemySacrificeIconBrush.Get())
									.ColorAndOpacity(FLinearColor::White)
									.Visibility(EVisibility::Hidden)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 0.f)
							[
								SAssignNew(AlchemySacrificeText, STextBlock)
								.Text(EmptyResultText)
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							],
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(18.f)),
							&AlchemySacrificeBorder)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						UpgradeButtonWidget
					]
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 18.f)
		[
			SAssignNew(AlchemyStatusText, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FT66Style::Tokens::FontBold(16))
			.ColorAndOpacity(FLinearColor::White)
			.Justification(ETextJustify::Center)
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Casino", "InventoryTitle", "INVENTORY"))
						.Font(FT66Style::Tokens::FontBold(20))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SSpacer)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
				[
					SNew(SScrollBox)
					.Orientation(Orient_Horizontal)
					.ScrollBarVisibility(EVisibility::Visible)
					+ SScrollBox::Slot()
					[
						InventoryGrid
					]
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel))
		],
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(28.f))
	);
}

void UT66CasinoOverlayWidget::SetActiveTab(const ECasinoTab NewTab)
{
	ActiveTab = NewTab;
	if (TabSwitcher.IsValid())
	{
		TabSwitcher->SetActiveWidgetIndex(static_cast<int32>(ActiveTab));
	}
}

void UT66CasinoOverlayWidget::RefreshAlchemy()
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

void UT66CasinoOverlayWidget::RefreshAlchemyTopBar()
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

void UT66CasinoOverlayWidget::RefreshAlchemyInventory()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	const TArray<FName> Inventory = RunState->GetInventory();
	const TArray<FT66InventorySlot>& InventorySlots = RunState->GetInventorySlots();
	if (AlchemyTargetInventoryIndex < 0
		|| AlchemyTargetInventoryIndex >= InventorySlots.Num()
		|| !RunState->CanAlchemyUpgradeInventoryItemAt(AlchemyTargetInventoryIndex))
	{
		AlchemyTargetInventoryIndex = INDEX_NONE;
	}
	if (AlchemyTargetInventoryIndex == INDEX_NONE)
	{
		for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
		{
			if (RunState->CanAlchemyUpgradeInventoryItemAt(Index))
			{
				AlchemyTargetInventoryIndex = Index;
				break;
			}
		}
	}

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
		FItemData ItemData;
		const bool bHasData = bHasItem && GI && GI->GetItemData(Inventory[Index], ItemData);

		if (AlchemyInventorySlotBorders.IsValidIndex(Index) && AlchemyInventorySlotBorders[Index].IsValid())
		{
			const bool bSelected = Index == AlchemyTargetInventoryIndex;
			const FLinearColor Fill = bSelected
				? (FT66Style::Tokens::Panel2 * 0.45f + FT66Style::Tokens::Accent * 0.55f)
				: FT66Style::Tokens::Panel2;
			AlchemyInventorySlotBorders[Index]->SetBorderBackgroundColor(Fill);
		}
		if (AlchemyInventorySlotButtons.IsValidIndex(Index) && AlchemyInventorySlotButtons[Index].IsValid())
		{
			AlchemyInventorySlotButtons[Index]->SetEnabled(bHasItem && !RunState->GetBossActive());
		}
		const int32 StackCount = (bHasItem && InventorySlots[Index].IsValid())
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
	const bool bHasTarget = InventorySlots && InventorySlots->IsValidIndex(AlchemyTargetInventoryIndex);
	FT66InventorySlot PreviewSlot;
	int32 MatchingCount = 0;
	const bool bHasPreview = RunState && RunState->GetAlchemyUpgradePreviewAt(AlchemyTargetInventoryIndex, PreviewSlot, MatchingCount);

	if (AlchemyTargetBorder.IsValid())
	{
		AlchemyTargetBorder->SetBorderBackgroundColor(FT66Style::Tokens::Panel2);
	}
	if (AlchemySacrificeBorder.IsValid())
	{
		AlchemySacrificeBorder->SetBorderBackgroundColor(bHasPreview ? (FT66Style::Tokens::Panel2 * 0.55f + FT66Style::Tokens::Success * 0.45f) : FT66Style::Tokens::Panel2);
	}
	if (AlchemyTargetText.IsValid())
	{
		AlchemyTargetText->SetText(NSLOCTEXT("T66.Casino", "AlchemyEmptyTarget", "No upgradable item"));
	}
	if (AlchemySacrificeText.IsValid())
	{
		AlchemySacrificeText->SetText(NSLOCTEXT("T66.Casino", "AlchemyEmptyResult", "Upgrade result"));
	}
	if (AlchemyTargetCountText.IsValid())
	{
		AlchemyTargetCountText->SetText(FText::GetEmpty());
		AlchemyTargetCountText->SetVisibility(EVisibility::Hidden);
	}
	if (AlchemyTargetIconBrush.IsValid())
	{
		AlchemyTargetIconBrush->SetResourceObject(nullptr);
	}
	if (AlchemySacrificeIconBrush.IsValid())
	{
		AlchemySacrificeIconBrush->SetResourceObject(nullptr);
	}
	if (AlchemyTargetIconImage.IsValid())
	{
		AlchemyTargetIconImage->SetVisibility(EVisibility::Hidden);
	}
	if (AlchemySacrificeIconImage.IsValid())
	{
		AlchemySacrificeIconImage->SetVisibility(EVisibility::Hidden);
	}

	if (bHasTarget)
	{
		const FT66InventorySlot& TargetSlot = (*InventorySlots)[AlchemyTargetInventoryIndex];
		const FText ItemName = Loc ? Loc->GetText_ItemDisplayNameForRarity(TargetSlot.ItemTemplateID, TargetSlot.Rarity) : FText::FromName(TargetSlot.ItemTemplateID);
		if (AlchemyTargetText.IsValid())
		{
			AlchemyTargetText->SetText(ItemName);
		}
		if (AlchemyTargetCountText.IsValid() && MatchingCount > 1)
		{
			AlchemyTargetCountText->SetText(FText::Format(NSLOCTEXT("T66.Inventory", "StackCountFormat", "{0}x"), FText::AsNumber(MatchingCount)));
			AlchemyTargetCountText->SetVisibility(EVisibility::Visible);
		}

		FItemData ItemData;
		if (GI && GI->GetItemData(TargetSlot.ItemTemplateID, ItemData))
		{
			const TSoftObjectPtr<UTexture2D> IconSoft = ItemData.GetIconForRarity(TargetSlot.Rarity);
			if (!IconSoft.IsNull() && TexPool && AlchemyTargetIconBrush.IsValid())
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IconSoft, this, AlchemyTargetIconBrush, FName(TEXT("CasinoAlchemyTarget")), true);
				if (AlchemyTargetIconImage.IsValid())
				{
					AlchemyTargetIconImage->SetVisibility(EVisibility::Visible);
				}
			}
		}
	}

	if (bHasPreview)
	{
		const FText ItemName = Loc ? Loc->GetText_ItemDisplayNameForRarity(PreviewSlot.ItemTemplateID, PreviewSlot.Rarity) : FText::FromName(PreviewSlot.ItemTemplateID);
		if (AlchemySacrificeText.IsValid())
		{
			AlchemySacrificeText->SetText(ItemName);
		}

		FItemData ItemData;
		if (GI && GI->GetItemData(PreviewSlot.ItemTemplateID, ItemData))
		{
			const TSoftObjectPtr<UTexture2D> PreviewIconSoft = ItemData.GetIconForRarity(PreviewSlot.Rarity);
			if (!PreviewIconSoft.IsNull() && TexPool && AlchemySacrificeIconBrush.IsValid())
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PreviewIconSoft, this, AlchemySacrificeIconBrush, FName(TEXT("CasinoAlchemyPreview")), true);
				if (AlchemySacrificeIconImage.IsValid())
				{
					AlchemySacrificeIconImage->SetVisibility(EVisibility::Visible);
				}
			}
		}
	}

	if (AlchemyUpgradeButton.IsValid())
	{
		AlchemyUpgradeButton->SetEnabled(bHasPreview && RunState && !RunState->GetBossActive());
	}
	if (AlchemyResultText.IsValid())
	{
		AlchemyResultText->SetText(FText::GetEmpty());
	}
}

void UT66CasinoOverlayWidget::SetAlchemyStatus(const FText& Message, const FLinearColor& Color)
{
	AlchemyStatusMessage = Message;
	AlchemyStatusColor = Color;
	if (AlchemyStatusText.IsValid())
	{
		AlchemyStatusText->SetText(AlchemyStatusMessage);
		AlchemyStatusText->SetColorAndOpacity(AlchemyStatusColor);
	}
}

bool UT66CasinoOverlayWidget::TryAssignAlchemySlot(const bool bIsTargetSlot, const int32 InventoryIndex)
{
	(void)bIsTargetSlot;

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
	if (!SelectedSlot.IsValid())
	{
		return false;
	}
	if (SelectedSlot.Rarity == ET66ItemRarity::White || RunState->GetAlchemyMatchingInventoryCount(InventoryIndex) <= 0)
	{
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "AlchemyInvalidItem", "Only regular Black, Red, or Yellow items can be upgraded."), FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return false;
	}
	if (!RunState->CanAlchemyUpgradeInventoryItemAt(InventoryIndex))
	{
		SetAlchemyStatus(
			FText::Format(
				NSLOCTEXT("T66.Casino", "AlchemyNeedThree", "Alchemy needs {0} matching copies of the same item and rarity."),
				FText::AsNumber(UT66RunStateSubsystem::AlchemyCopiesRequired)),
			FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return false;
	}

	AlchemyTargetInventoryIndex = InventoryIndex;
	AlchemySacrificeInventoryIndex = INDEX_NONE;
	SetAlchemyStatus(FText::GetEmpty(), FLinearColor::White);
	RefreshAlchemy();
	return true;
}

FReply UT66CasinoOverlayWidget::HandleAlchemyInventoryDragDetected(const FGeometry&, const FPointerEvent&, const int32 InventoryIndex)
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UT66LocalizationSubsystem* Loc = GetLocalization();
	if (!RunState)
	{
		return FReply::Unhandled();
	}

	const TArray<FName> Inventory = RunState->GetInventory();
	const TArray<FT66InventorySlot>& InventorySlots = RunState->GetInventorySlots();
	if (!Inventory.IsValidIndex(InventoryIndex) || Inventory[InventoryIndex].IsNone())
	{
		return FReply::Unhandled();
	}

	const ET66ItemRarity SlotRarity = InventorySlots.IsValidIndex(InventoryIndex) ? InventorySlots[InventoryIndex].Rarity : ET66ItemRarity::Black;
	const FText Label = Loc ? Loc->GetText_ItemDisplayNameForRarity(Inventory[InventoryIndex], SlotRarity) : FText::FromName(Inventory[InventoryIndex]);
	TSharedPtr<FSlateBrush> DragIconBrush = MakeShared<FSlateBrush>();
	DragIconBrush->DrawAs = ESlateBrushDrawType::Image;
	DragIconBrush->ImageSize = FVector2D(72.f, 72.f);
	if (AlchemyInventorySlotBrushes.IsValidIndex(InventoryIndex) && AlchemyInventorySlotBrushes[InventoryIndex].IsValid())
	{
		*DragIconBrush = *AlchemyInventorySlotBrushes[InventoryIndex];
		DragIconBrush->DrawAs = ESlateBrushDrawType::Image;
		DragIconBrush->ImageSize = FVector2D(72.f, 72.f);
	}

	return FReply::Handled().BeginDragDrop(FCasinoAlchemyDragDropOp::New(
		InventoryIndex,
		Label,
		DragIconBrush));
}

FReply UT66CasinoOverlayWidget::HandleAlchemyDropTarget(const FGeometry&, const FDragDropEvent& DragDropEvent, const bool bIsTargetSlot)
{
	const TSharedPtr<FCasinoAlchemyDragDropOp> DragOp = DragDropEvent.GetOperationAs<FCasinoAlchemyDragDropOp>();
	if (!DragOp.IsValid())
	{
		return FReply::Unhandled();
	}

	return TryAssignAlchemySlot(bIsTargetSlot, DragOp->InventoryIndex) ? FReply::Handled() : FReply::Unhandled();
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
		SetAlchemyStatus(NSLOCTEXT("T66.Casino", "AlchemyNeedTarget", "No upgradable item is ready."), FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return FReply::Handled();
	}

	FT66InventorySlot UpgradedSlot;
	int32 MatchingCount = 0;
	RunState->GetAlchemyUpgradePreviewAt(AlchemyTargetInventoryIndex, UpgradedSlot, MatchingCount);
	if (!RunState->TryAlchemyUpgradeInventoryItems(AlchemyTargetInventoryIndex, INDEX_NONE, UpgradedSlot))
	{
		SetAlchemyStatus(
			FText::Format(
				NSLOCTEXT("T66.Casino", "AlchemyNeedCopies", "Alchemy needs {0} matching copies. You currently have {1}."),
				FText::AsNumber(UT66RunStateSubsystem::AlchemyCopiesRequired),
				FText::AsNumber(MatchingCount)),
			FLinearColor(1.f, 0.35f, 0.35f, 1.f));
		return FReply::Handled();
	}

	AlchemyTargetInventoryIndex = INDEX_NONE;
	AlchemySacrificeInventoryIndex = INDEX_NONE;
	const FText ItemName = Loc ? Loc->GetText_ItemDisplayName(UpgradedSlot.ItemTemplateID) : FText::FromName(UpgradedSlot.ItemTemplateID);
	const FText RarityName = Loc ? Loc->GetText_ItemRarityName(UpgradedSlot.Rarity) : FText::GetEmpty();
	SetAlchemyStatus(
		FText::Format(NSLOCTEXT("T66.Casino", "AlchemySuccess", "{0} upgraded to {1}."), ItemName, RarityName),
		FLinearColor(0.30f, 1.f, 0.40f, 1.f));
	RefreshAlchemy();

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->TriggerCircusBossIfAngry();
	}
	return FReply::Handled();
}

FReply UT66CasinoOverlayWidget::OnClearAlchemyTargetClicked()
{
	AlchemyTargetInventoryIndex = INDEX_NONE;
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
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

UT66LocalizationSubsystem* UT66CasinoOverlayWidget::GetLocalization() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
}

void UT66CasinoOverlayWidget::HandleInventoryChanged()
{
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
