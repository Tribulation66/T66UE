// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66VendorOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66VendorBoss.h"
#include "Data/T66DataTypes.h"
#include "UI/Style/T66Style.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Styling/CoreStyle.h"

static UT66RunStateSubsystem* GetRunStateFromWorld(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

void UT66VendorOverlayWidget::NativeDestruct()
{
	// Stop any steal prompt ticking.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealTickTimerHandle);
	}

	// Unbind runstate delegates (safe to call even if not bound).
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->GoldChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
				RunState->DebtChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
				RunState->InventoryChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleInventoryChanged);
				RunState->VendorChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleVendorChanged);
			}
		}
	}

	// Safety: if destroyed unexpectedly, restore gameplay input.
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}
	Super::NativeDestruct();
}

void UT66VendorOverlayWidget::CloseOverlay()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealTickTimerHandle);
	}
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

TSharedRef<SWidget> UT66VendorOverlayWidget::RebuildWidget()
{
	bBoughtSomethingThisVisit = false;
	BorrowAmount = FMath::Max(0, BorrowAmount);
	PaybackAmount = FMath::Max(0, PaybackAmount);
	SelectedInventoryIndex = -1;

	UWorld* World = GetWorld();
	UT66LocalizationSubsystem* Loc = nullptr;
	if (World)
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}

	// Force stock generation now so we can build tiles.
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (RunState)
	{
		RunState->EnsureVendorStockForCurrentStage();
		bBoughtSomethingThisVisit = RunState->HasBoughtFromVendorThisStage();

		// Bind live updates (event-driven UI).
		RunState->GoldChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleInventoryChanged);
		RunState->VendorChanged.RemoveDynamic(this, &UT66VendorOverlayWidget::HandleVendorChanged);

		RunState->GoldChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
		RunState->DebtChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleGoldOrDebtChanged);
		RunState->InventoryChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleInventoryChanged);
		RunState->VendorChanged.AddDynamic(this, &UT66VendorOverlayWidget::HandleVendorChanged);
	}

	const ISlateStyle& Style = FT66Style::Get();
	const FButtonStyle& BtnPrimary = Style.GetWidgetStyle<FButtonStyle>("T66.Button.Primary");
	const FButtonStyle& BtnNeutral = Style.GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
	const FButtonStyle& BtnDanger = Style.GetWidgetStyle<FButtonStyle>("T66.Button.Danger");

	const FTextBlockStyle& TextTitle = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Title");
	const FTextBlockStyle& TextHeading = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TextBody = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FTextBlockStyle& TextChip = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");
	const FTextBlockStyle& TextButton = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");

	const FText VendorTitle = FText::FromString(TEXT("VENDOR"));
	const FText ShopTitle = FText::FromString(TEXT("SHOP"));
	const FText BankTitle = Loc ? Loc->GetText_Bank() : FText::FromString(TEXT("BANK"));
	const FText BackText = Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"));
	const FText AngerLabel = Loc ? Loc->GetText_Anger() : FText::FromString(TEXT("ANGER"));
	const FText InventoryTitle = FText::FromString(TEXT("YOUR ITEMS"));

	const TArray<FName> EmptyStock;
	const TArray<FName>& Stock = RunState ? RunState->GetVendorStockItemIDs() : EmptyStock;

	static constexpr int32 ShopSlotCount = 4;
	ItemNameTexts.SetNum(ShopSlotCount);
	ItemDescTexts.SetNum(ShopSlotCount);
	ItemPriceTexts.SetNum(ShopSlotCount);
	ItemTileBorders.SetNum(ShopSlotCount);
	ItemIconBorders.SetNum(ShopSlotCount);
	BuyButtons.SetNum(ShopSlotCount);
	StealButtons.SetNum(ShopSlotCount);
	BuyButtonTexts.SetNum(ShopSlotCount);

	InventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotButtons.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);

	TSharedRef<SUniformGridPanel> ShopGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(FT66Style::Tokens::Space4, FT66Style::Tokens::Space4));

	for (int32 i = 0; i < ShopSlotCount; ++i)
	{
		ShopGrid->AddSlot(i, 0)
		[
			SNew(SBox)
			.MinDesiredWidth(340.f)
			[
				SAssignNew(ItemTileBorders[i], SBorder)
				.BorderImage(Style.GetBrush("T66.Brush.Panel"))
				.Padding(FT66Style::Tokens::Space4)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 0.f, FT66Style::Tokens::Space3, 0.f)
						[
							SAssignNew(ItemIconBorders[i], SBorder)
							.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
							.Padding(0.f)
							[
								SNew(SBox)
								.WidthOverride(64.f)
								.HeightOverride(64.f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT(" ")))
									.TextStyle(&TextChip)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Top)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SAssignNew(ItemNameTexts[i], STextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&TextHeading)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("UPGRADE")))
								.TextStyle(&TextChip)
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						SAssignNew(ItemDescTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						SAssignNew(ItemPriceTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextChip)
						.ColorAndOpacity(FT66Style::Tokens::Accent2)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space2, 0.f)
						[
							SAssignNew(BuyButtons[i], SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBuySlot, i))
							.ButtonStyle(&BtnPrimary)
							.ContentPadding(FMargin(16.f, 12.f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SAssignNew(BuyButtonTexts[i], STextBlock)
								.Text(Loc ? Loc->GetText_Buy() : FText::FromString(TEXT("BUY")))
								.TextStyle(&TextButton)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							SAssignNew(StealButtons[i], SButton)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnStealSlot, i))
							.ButtonStyle(&BtnDanger)
							.ContentPadding(FMargin(16.f, 12.f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("STEAL")))
								.TextStyle(&TextButton)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
					]
				]
			]
		];
	}

	// Steal prompt (created once, overlaid and toggled visible when needed).
	TSharedRef<SWidget> StealPromptWidget =
		SAssignNew(StealPromptContainer, SBox)
		.WidthOverride(560.f)
		.HeightOverride(220.f)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(SBorder)
			.BorderImage(Style.GetBrush("T66.Brush.Panel"))
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(18.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("STEAL (TIMING)")))
					.TextStyle(&TextHeading)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(SBox)
					.WidthOverride(360.f)
					.HeightOverride(28.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
							.BorderBackgroundColor(FT66Style::Tokens::Panel2)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Fill)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SAssignNew(StealMarkerSpacerBox, SBox)
								.WidthOverride(0.f)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
									.BorderBackgroundColor(FT66Style::Tokens::Success)
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Press STOP near the center.")))
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnStealStop))
					.ButtonStyle(&BtnPrimary)
					.ContentPadding(FMargin(22.f, 12.f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("STOP")))
						.TextStyle(&TextButton)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
			]
		];

	TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(Style.GetBrush("T66.Brush.Bg"))
			.BorderBackgroundColor(FT66Style::Tokens::Scrim)
			.Padding(24.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(VendorTitle)
						.TextStyle(&TextTitle)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SSpacer)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(420.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 16.f, 0.f)
								[
									SAssignNew(GoldText, STextBlock)
									.Text(FText::FromString(TEXT("Gold: 0")))
									.TextStyle(&TextHeading)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SAssignNew(DebtText, STextBlock)
									.Text(FText::FromString(TEXT("Owe: 0")))
									.TextStyle(&TextBody)
									.ColorAndOpacity(FT66Style::Tokens::Danger)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[
									SNew(STextBlock)
									.Text(AngerLabel)
									.TextStyle(&TextChip)
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								]
								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									SNew(SBorder)
									.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
									.BorderBackgroundColor(FT66Style::Tokens::Panel2)
									.Padding(2.f)
									[
										SNew(SOverlay)
										+ SOverlay::Slot()
										[
											SAssignNew(AngerFillBox, SBox)
											.WidthOverride(0.f)
											[
												SNew(SBorder)
												.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
												.BorderBackgroundColor(FT66Style::Tokens::Danger)
											]
										]
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
								[
									SAssignNew(AngerText, STextBlock)
									.Text(FText::FromString(TEXT("0/100")))
									.TextStyle(&TextChip)
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
				[
					SAssignNew(StatusText, STextBlock)
					.Text(FText::GetEmpty())
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f)
					[
						SNew(SBorder)
						.BorderImage(Style.GetBrush("T66.Brush.Panel"))
						.BorderBackgroundColor(FT66Style::Tokens::Panel)
						.Padding(FT66Style::Tokens::Space6)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4)
							[
								SNew(STextBlock)
								.Text(ShopTitle)
								.TextStyle(&TextHeading)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot().FillHeight(1.f)
							[
								ShopGrid
							]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBorder)
						.BorderImage(Style.GetBrush("T66.Brush.Panel"))
						.BorderBackgroundColor(FT66Style::Tokens::Panel)
						.Padding(FT66Style::Tokens::Space6)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4)
							[
								SNew(STextBlock)
								.Text(BankTitle)
								.TextStyle(&TextHeading)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[ SNew(STextBlock).Text(Loc ? Loc->GetText_Borrow() : FText::FromString(TEXT("Borrow"))).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[
									SAssignNew(BorrowAmountSpin, SSpinBox<int32>)
									.MinValue(0).MaxValue(999999).Delta(10)
									.Value_Lambda([this]() { return BorrowAmount; })
									.OnValueChanged_Lambda([this](int32 V) { BorrowAmount = FMath::Max(0, V); })
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBorrowClicked))
									.ButtonStyle(&BtnNeutral)
									.ContentPadding(FMargin(16.f, 10.f))
									[ SNew(STextBlock).Text(Loc ? Loc->GetText_Borrow() : FText::FromString(TEXT("BORROW"))).TextStyle(&TextButton).ColorAndOpacity(FT66Style::Tokens::Text) ]
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[ SNew(STextBlock).Text(Loc ? Loc->GetText_Payback() : FText::FromString(TEXT("Payback"))).TextStyle(&TextBody).ColorAndOpacity(FT66Style::Tokens::Text) ]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[
									SAssignNew(PaybackAmountSpin, SSpinBox<int32>)
									.MinValue(0).MaxValue(999999).Delta(10)
									.Value_Lambda([this]() { return PaybackAmount; })
									.OnValueChanged_Lambda([this](int32 V) { PaybackAmount = FMath::Max(0, V); })
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnPaybackClicked))
									.ButtonStyle(&BtnNeutral)
									.ContentPadding(FMargin(16.f, 10.f))
									[ SNew(STextBlock).Text(Loc ? Loc->GetText_Payback() : FText::FromString(TEXT("PAYBACK"))).TextStyle(&TextButton).ColorAndOpacity(FT66Style::Tokens::Text) ]
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f)
				[
					SNew(SBorder)
					.BorderImage(Style.GetBrush("T66.Brush.Panel"))
					.BorderBackgroundColor(FT66Style::Tokens::Panel)
					.Padding(FT66Style::Tokens::Space6)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(InventoryTitle)
							.TextStyle(&TextHeading)
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space4, 0.f, 0.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f)
							[
								// Inventory slots
								SNew(SUniformGridPanel)
								.SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f))
								+ SUniformGridPanel::Slot(0, 0)
								[
									SAssignNew(InventorySlotButtons[0], SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSelectInventorySlot, 0))
									.ButtonStyle(&BtnNeutral)
									.ContentPadding(FMargin(0.f))
									[
										SAssignNew(InventorySlotBorders[0], SBorder)
										.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
										.Padding(FMargin(12.f, 10.f))
										[
											SAssignNew(InventorySlotTexts[0], STextBlock)
											.Text(FText::FromString(TEXT("-")))
											.TextStyle(&TextChip)
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
									]
								]
								+ SUniformGridPanel::Slot(1, 0)
								[
									SAssignNew(InventorySlotButtons[1], SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSelectInventorySlot, 1))
									.ButtonStyle(&BtnNeutral)
									.ContentPadding(FMargin(0.f))
									[
										SAssignNew(InventorySlotBorders[1], SBorder)
										.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
										.Padding(FMargin(12.f, 10.f))
										[
											SAssignNew(InventorySlotTexts[1], STextBlock)
											.Text(FText::FromString(TEXT("-")))
											.TextStyle(&TextChip)
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
									]
								]
								+ SUniformGridPanel::Slot(2, 0)
								[
									SAssignNew(InventorySlotButtons[2], SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSelectInventorySlot, 2))
									.ButtonStyle(&BtnNeutral)
									.ContentPadding(FMargin(0.f))
									[
										SAssignNew(InventorySlotBorders[2], SBorder)
										.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
										.Padding(FMargin(12.f, 10.f))
										[
											SAssignNew(InventorySlotTexts[2], STextBlock)
											.Text(FText::FromString(TEXT("-")))
											.TextStyle(&TextChip)
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
									]
								]
								+ SUniformGridPanel::Slot(3, 0)
								[
									SAssignNew(InventorySlotButtons[3], SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSelectInventorySlot, 3))
									.ButtonStyle(&BtnNeutral)
									.ContentPadding(FMargin(0.f))
									[
										SAssignNew(InventorySlotBorders[3], SBorder)
										.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
										.Padding(FMargin(12.f, 10.f))
										[
											SAssignNew(InventorySlotTexts[3], STextBlock)
											.Text(FText::FromString(TEXT("-")))
											.TextStyle(&TextChip)
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
									]
								]
								+ SUniformGridPanel::Slot(4, 0)
								[
									SAssignNew(InventorySlotButtons[4], SButton)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSelectInventorySlot, 4))
									.ButtonStyle(&BtnNeutral)
									.ContentPadding(FMargin(0.f))
									[
										SAssignNew(InventorySlotBorders[4], SBorder)
										.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
										.Padding(FMargin(12.f, 10.f))
										[
											SAssignNew(InventorySlotTexts[4], STextBlock)
											.Text(FText::FromString(TEXT("-")))
											.TextStyle(&TextChip)
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
									]
								]
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(FT66Style::Tokens::Space6, 0.f, 0.f, 0.f)
							[
								// Sell details for selected item
								SAssignNew(SellPanelContainer, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									SNew(SBorder)
									.BorderImage(Style.GetBrush("T66.Brush.Panel2"))
									.BorderBackgroundColor(FT66Style::Tokens::Panel2)
									.Padding(FT66Style::Tokens::Space4)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight()
										[
											SAssignNew(SellItemNameText, STextBlock)
											.Text(FText::GetEmpty())
											.TextStyle(&TextHeading)
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
										[
											SAssignNew(SellItemDescText, STextBlock)
											.Text(FText::GetEmpty())
											.TextStyle(&TextBody)
											.ColorAndOpacity(FT66Style::Tokens::TextMuted)
											.AutoWrapText(true)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
										[
											SAssignNew(SellItemPriceText, STextBlock)
											.Text(FText::GetEmpty())
											.TextStyle(&TextChip)
											.ColorAndOpacity(FT66Style::Tokens::Accent2)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
										[
											SAssignNew(SellItemButton, SButton)
											.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSellSelectedClicked))
											.ButtonStyle(&BtnPrimary)
											.ContentPadding(FMargin(18.f, 10.f))
											[
												SNew(STextBlock)
												.Text(Loc ? Loc->GetText_Sell() : FText::FromString(TEXT("SELL")))
												.TextStyle(&TextButton)
												.ColorAndOpacity(FT66Style::Tokens::Text)
											]
										]
									]
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 16.f, 0.f, 0.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBack))
					.ButtonStyle(&BtnNeutral)
					.ContentPadding(FMargin(20.f, 12.f))
					[
						SNew(STextBlock)
						.Text(BackText)
						.TextStyle(&TextButton)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			StealPromptWidget
		];

	RefreshAll();
	return Root;
}

void UT66VendorOverlayWidget::HandleGoldOrDebtChanged()
{
	RefreshTopBar();
}

void UT66VendorOverlayWidget::HandleInventoryChanged()
{
	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();
}

void UT66VendorOverlayWidget::HandleVendorChanged()
{
	RefreshTopBar();
	RefreshStock();
}

void UT66VendorOverlayWidget::RefreshAll()
{
	RefreshTopBar();
	RefreshStock();
	RefreshInventory();
	RefreshSellPanel();
}

void UT66VendorOverlayWidget::RefreshTopBar()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	if (GoldText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_GoldFormat() : FText::FromString(TEXT("Gold: {0}"));
		GoldText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentGold())));
	}
	if (DebtText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_OweFormat() : FText::FromString(TEXT("Owe: {0}"));
		DebtText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentDebt())));
	}

	const float Anger01 = RunState->GetVendorAnger01();
	if (AngerFillBox.IsValid())
	{
		const float Full = 240.f;
		AngerFillBox->SetWidthOverride(FMath::Clamp(Full * Anger01, 0.f, Full));
	}
	if (AngerText.IsValid())
	{
		AngerText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), RunState->GetVendorAngerGold(), UT66RunStateSubsystem::VendorAngerThresholdGold)));
	}
}

void UT66VendorOverlayWidget::RefreshStock()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	const TArray<FName>& Stock = RunState->GetVendorStockItemIDs();
	const int32 SlotCount = ItemNameTexts.Num();
	for (int32 i = 0; i < SlotCount; ++i)
	{
		const bool bHasItem = Stock.IsValidIndex(i) && !Stock[i].IsNone();
		const bool bSold = bHasItem ? RunState->IsVendorStockSlotSold(i) : true;
		FItemData D;
		const bool bHasData = bHasItem && GI && GI->GetItemData(Stock[i], D);

		if (ItemNameTexts.IsValidIndex(i) && ItemNameTexts[i].IsValid())
		{
			ItemNameTexts[i]->SetText(bHasItem ? FText::FromName(Stock[i]) : FText::FromString(TEXT("EMPTY")));
		}
		if (ItemDescTexts.IsValidIndex(i) && ItemDescTexts[i].IsValid())
		{
			if (!bHasData)
			{
				ItemDescTexts[i]->SetText(FText::GetEmpty());
			}
			else
			{
				// Combine tooltip lines into a single wrapped block.
				FString Combined;
				const TArray<FText> Lines = { D.EffectLine1, D.EffectLine2, D.EffectLine3 };
				for (const FText& L : Lines)
				{
					if (L.IsEmpty()) continue;
					if (!Combined.IsEmpty()) Combined += TEXT("\n");
					Combined += L.ToString();
				}
				ItemDescTexts[i]->SetText(FText::FromString(Combined));
			}
		}
		if (ItemPriceTexts.IsValidIndex(i) && ItemPriceTexts[i].IsValid())
		{
			if (!bHasData)
			{
				ItemPriceTexts[i]->SetText(FText::GetEmpty());
			}
			else
			{
				ItemPriceTexts[i]->SetText(FText::FromString(FString::Printf(TEXT("PRICE: %dg"), D.BuyValueGold)));
			}
		}
		if (ItemIconBorders.IsValidIndex(i) && ItemIconBorders[i].IsValid())
		{
			ItemIconBorders[i]->SetBorderBackgroundColor(bHasData ? D.PlaceholderColor : FT66Style::Tokens::Panel2);
		}
		if (ItemTileBorders.IsValidIndex(i) && ItemTileBorders[i].IsValid())
		{
			ItemTileBorders[i]->SetBorderBackgroundColor(bSold ? FT66Style::Tokens::Panel2 : FT66Style::Tokens::Panel);
		}
		if (BuyButtons.IsValidIndex(i) && BuyButtons[i].IsValid())
		{
			BuyButtons[i]->SetEnabled(bHasItem && !bSold && !IsBossActive());
		}
		if (BuyButtonTexts.IsValidIndex(i) && BuyButtonTexts[i].IsValid())
		{
			if (!bHasItem)
			{
				BuyButtonTexts[i]->SetText(FText::FromString(TEXT("BUY")));
			}
			else if (bSold)
			{
				BuyButtonTexts[i]->SetText(FText::FromString(TEXT("SOLD")));
			}
			else
			{
				const int32 Price = bHasData ? D.BuyValueGold : 0;
				BuyButtonTexts[i]->SetText(FText::FromString(FString::Printf(TEXT("BUY (%dg)"), Price)));
			}
		}
		if (StealButtons.IsValidIndex(i) && StealButtons[i].IsValid())
		{
			StealButtons[i]->SetEnabled(bHasItem && !bSold && bBoughtSomethingThisVisit && !IsBossActive());
		}
	}
}

void UT66VendorOverlayWidget::RefreshInventory()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	const TArray<FName>& Inv = RunState->GetInventory();
	if (SelectedInventoryIndex >= Inv.Num())
	{
		SelectedInventoryIndex = (Inv.Num() > 0) ? (Inv.Num() - 1) : -1;
	}

	for (int32 i = 0; i < InventorySlotTexts.Num(); ++i)
	{
		const bool bHasItem = Inv.IsValidIndex(i) && !Inv[i].IsNone();
		if (InventorySlotTexts[i].IsValid())
		{
			InventorySlotTexts[i]->SetText(bHasItem ? FText::FromName(Inv[i]) : FText::FromString(TEXT("-")));
		}
		if (InventorySlotButtons.IsValidIndex(i) && InventorySlotButtons[i].IsValid())
		{
			InventorySlotButtons[i]->SetEnabled(bHasItem && !IsBossActive());
		}
		if (InventorySlotBorders.IsValidIndex(i) && InventorySlotBorders[i].IsValid())
		{
			const bool bSelected = (i == SelectedInventoryIndex);
			InventorySlotBorders[i]->SetBorderBackgroundColor(bSelected ? FT66Style::Tokens::Accent : FT66Style::Tokens::Panel2);
		}
	}
}

void UT66VendorOverlayWidget::RefreshSellPanel()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	const TArray<FName>& Inv = RunState->GetInventory();
	const bool bValidSelection = Inv.IsValidIndex(SelectedInventoryIndex) && !Inv[SelectedInventoryIndex].IsNone();

	if (SellPanelContainer.IsValid())
	{
		SellPanelContainer->SetVisibility(bValidSelection ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (!bValidSelection) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	FItemData D;
	const bool bHasData = GI && GI->GetItemData(Inv[SelectedInventoryIndex], D);

	if (SellItemNameText.IsValid())
	{
		SellItemNameText->SetText(FText::FromName(Inv[SelectedInventoryIndex]));
	}
	if (SellItemDescText.IsValid())
	{
		if (!bHasData)
		{
			SellItemDescText->SetText(FText::GetEmpty());
		}
		else
		{
			FString Combined;
			const TArray<FText> Lines = { D.EffectLine1, D.EffectLine2, D.EffectLine3 };
			for (const FText& L : Lines)
			{
				if (L.IsEmpty()) continue;
				if (!Combined.IsEmpty()) Combined += TEXT("\n");
				Combined += L.ToString();
			}
			SellItemDescText->SetText(FText::FromString(Combined));
		}
	}
	if (SellItemPriceText.IsValid())
	{
		const int32 SellValue = bHasData ? D.SellValueGold : 0;
		SellItemPriceText->SetText(FText::FromString(FString::Printf(TEXT("SELL FOR: %dg"), SellValue)));
	}
	if (SellItemButton.IsValid())
	{
		SellItemButton->SetEnabled(!IsBossActive());
	}
}

FReply UT66VendorOverlayWidget::OnBack()
{
	CloseOverlay();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnBorrowClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	if (BorrowAmount <= 0)
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Borrow amount must be > 0.")));
		return FReply::Handled();
	}
	RunState->BorrowGold(BorrowAmount);
	if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Borrowed.")));
	RefreshTopBar();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnPaybackClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	if (PaybackAmount <= 0)
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Payback amount must be > 0.")));
		return FReply::Handled();
	}
	const int32 Paid = RunState->PayDebt(PaybackAmount);
	if (StatusText.IsValid()) StatusText->SetText(FText::FromString(FString::Printf(TEXT("Paid %d."), Paid)));
	RefreshTopBar();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnSelectInventorySlot(int32 InventoryIndex)
{
	SelectedInventoryIndex = InventoryIndex;
	RefreshInventory();
	RefreshSellPanel();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnSellSelectedClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	if (SelectedInventoryIndex < 0)
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Select an item to sell.")));
		return FReply::Handled();
	}

	const bool bSold = RunState->SellInventoryItemAt(SelectedInventoryIndex);
	if (bSold)
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Sold.")));
	}
	else
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Could not sell.")));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnBuySlot(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	if (IsBossActive())
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Boss is active.")));
		return FReply::Handled();
	}

	if (!RunState->HasInventorySpace())
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Inventory full.")));
		return FReply::Handled();
	}

	const bool bBought = RunState->TryBuyVendorStockSlot(SlotIndex);
	if (bBought)
	{
		bBoughtSomethingThisVisit = true;
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Purchased.")));
	}
	else
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Could not purchase.")));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnStealSlot(int32 SlotIndex)
{
	if (!bBoughtSomethingThisVisit)
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Buy one item before stealing.")));
		return FReply::Handled();
	}
	if (IsBossActive())
	{
		if (StatusText.IsValid()) StatusText->SetText(FText::FromString(TEXT("Boss is active.")));
		return FReply::Handled();
	}
	ShowStealPrompt(SlotIndex);
	return FReply::Handled();
}

void UT66VendorOverlayWidget::ShowStealPrompt(int32 SlotIndex)
{
	PendingStealIndex = SlotIndex;
	bStealPromptVisible = true;
	StealMarker01 = 0.f;
	bStealForward = true;

	if (StealMarkerSpacerBox.IsValid())
	{
		StealMarkerSpacerBox->SetWidthOverride(0.f);
	}
	if (StealPromptContainer.IsValid())
	{
		StealPromptContainer->SetVisibility(EVisibility::Visible);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealTickTimerHandle);
		World->GetTimerManager().SetTimer(StealTickTimerHandle, this, &UT66VendorOverlayWidget::TickStealBar, 0.016f, true);
	}
}

void UT66VendorOverlayWidget::HideStealPrompt()
{
	bStealPromptVisible = false;
	PendingStealIndex = -1;
	if (StealPromptContainer.IsValid())
	{
		StealPromptContainer->SetVisibility(EVisibility::Collapsed);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StealTickTimerHandle);
	}
}

void UT66VendorOverlayWidget::TickStealBar()
{
	// Bounce marker back and forth.
	const float Speed = 1.6f; // cycles per second-ish
	const float Delta = 0.016f;
	StealMarker01 += (bStealForward ? 1.f : -1.f) * Speed * Delta;
	if (StealMarker01 >= 1.f) { StealMarker01 = 1.f; bStealForward = false; }
	if (StealMarker01 <= 0.f) { StealMarker01 = 0.f; bStealForward = true; }

	// Bar width is 360, marker is 10, so travel range is 350.
	const float Travel = 350.f;
	if (StealMarkerSpacerBox.IsValid())
	{
		StealMarkerSpacerBox->SetWidthOverride(Travel * StealMarker01);
	}
}

FReply UT66VendorOverlayWidget::OnStealStop()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState)
	{
		HideStealPrompt();
		return FReply::Handled();
	}

	const float DistFromCenter = FMath::Abs(StealMarker01 - 0.5f);
	const bool bTimingHit = (DistFromCenter <= 0.12f);

	bool bRngSuccess = false;
	if (bTimingHit)
	{
		// v0 success odds: 65% if you hit timing window.
		bRngSuccess = (FMath::FRand() < 0.65f);
	}

	const bool bGranted = RunState->ResolveVendorStealAttempt(PendingStealIndex, bTimingHit, bRngSuccess);
	HideStealPrompt();

	if (StatusText.IsValid())
	{
		if (!bTimingHit) StatusText->SetText(FText::FromString(TEXT("Steal failed (miss). Anger increased.")));
		else if (!bRngSuccess) StatusText->SetText(FText::FromString(TEXT("Steal failed. Anger increased.")));
		else if (!bGranted) StatusText->SetText(FText::FromString(TEXT("Steal succeeded, but inventory is full.")));
		else StatusText->SetText(FText::FromString(TEXT("Stolen.")));
	}

	RefreshAll();
	TriggerVendorBossIfAngry();
	return FReply::Handled();
}

bool UT66VendorOverlayWidget::IsBossActive() const
{
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World))
		{
			return RunState->GetBossActive();
		}
	}
	return false;
}

void UT66VendorOverlayWidget::TriggerVendorBossIfAngry()
{
	UWorld* World = GetWorld();
	if (!World) return;
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;
	if (!RunState->IsVendorAngryEnoughToBoss()) return;
	if (RunState->GetBossActive()) return;

	// Spawn VendorBoss at vendor NPC location (and remove the NPC).
	FVector SpawnLoc = FVector::ZeroVector;
	AT66VendorNPC* Vendor = nullptr;
	for (TActorIterator<AT66VendorNPC> It(World); It; ++It)
	{
		Vendor = *It;
		break;
	}
	if (!Vendor)
	{
		return;
	}
	SpawnLoc = Vendor->GetActorLocation();
	Vendor->Destroy();

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66VendorBoss>(AT66VendorBoss::StaticClass(), SpawnLoc, FRotator::ZeroRotator, P);

	// Close UI immediately when boss triggers.
	CloseOverlay();
}

