// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66VendorOverlayWidget.h"
#include "UI/T66StatsPanelSlate.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66VendorBoss.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Data/T66DataTypes.h"
#include "UI/T66SlateTextureHelpers.h"
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
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SNullWidget.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Engine/Texture2D.h"

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

	const FTextBlockStyle& TextTitle = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Title");
	const FTextBlockStyle& TextHeading = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TextBody = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FTextBlockStyle& TextChip = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");

	const FText VendorTitle = Loc ? Loc->GetText_Vendor() : NSLOCTEXT("T66.Vendor", "VendorTitle", "VENDOR");
	const FText ShopTitle = Loc ? Loc->GetText_Shop() : NSLOCTEXT("T66.Vendor", "ShopTitle", "SHOP");
	const FText BankTitle = Loc ? Loc->GetText_Bank() : NSLOCTEXT("T66.Vendor", "BankTitle", "BANK");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText InventoryTitle = Loc ? Loc->GetText_YourItems() : NSLOCTEXT("T66.Vendor", "InventoryTitle", "INVENTORY");
	const FText RerollText = Loc ? Loc->GetText_Reroll() : NSLOCTEXT("T66.Vendor", "Reroll", "REROLL");

	const TArray<FName> EmptyStock;
	const TArray<FName>& Stock = RunState ? RunState->GetVendorStockItemIDs() : EmptyStock;

	static constexpr int32 ShopSlotCount = 4;
	ItemNameTexts.SetNum(ShopSlotCount);
	ItemDescTexts.SetNum(ShopSlotCount);
	ItemPriceTexts.SetNum(ShopSlotCount);
	ItemTileBorders.SetNum(ShopSlotCount);
	ItemIconBorders.SetNum(ShopSlotCount);
	ItemIconImages.SetNum(ShopSlotCount);
	ItemIconBrushes.SetNum(ShopSlotCount);
	BuyButtons.SetNum(ShopSlotCount);
	StealButtons.SetNum(ShopSlotCount);
	BuyButtonTexts.SetNum(ShopSlotCount);

	InventorySlotBorders.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotButtons.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotTexts.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotIconImages.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	InventorySlotIconBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);

	for (int32 i = 0; i < ShopSlotCount; ++i)
	{
		ItemIconBrushes[i] = MakeShared<FSlateBrush>();
		ItemIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		ItemIconBrushes[i]->ImageSize = FVector2D(64.f, 64.f);
	}
	for (int32 i = 0; i < InventorySlotIconBrushes.Num(); ++i)
	{
		InventorySlotIconBrushes[i] = MakeShared<FSlateBrush>();
		InventorySlotIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		InventorySlotIconBrushes[i]->ImageSize = FVector2D(148.f, 148.f);
	}

	TSharedRef<SUniformGridPanel> ShopGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(FT66Style::Tokens::Space4, FT66Style::Tokens::Space4));

	for (int32 i = 0; i < ShopSlotCount; ++i)
	{
		// Layout (reserve the left side for TikTok):
		// [Slot 1] [Slot 2]
		// [Slot 0] [Slot 3]
		int32 Col = 0;
		int32 Row = 0;
		switch (i)
		{
			case 0: Col = 0; Row = 1; break;
			case 1: Col = 0; Row = 0; break;
			case 2: Col = 1; Row = 0; break;
			case 3: Col = 1; Row = 1; break;
			default: Col = 0; Row = 0; break;
		}

		TSharedRef<SWidget> BuyBtnWidget = FT66Style::MakeButton(
			FT66ButtonParams(
				Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"),
				FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBuySlot, i),
				ET66ButtonType::Primary)
			.SetMinWidth(260.f)
			.SetPadding(FMargin(12.f, 10.f))
			.SetContent(
				SAssignNew(BuyButtonTexts[i], STextBlock)
				.Text(Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"))
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			)
		);
		BuyButtons[i] = BuyBtnWidget;

		TSharedRef<SWidget> StealBtnWidget = FT66Style::MakeButton(
			FT66ButtonParams(
				Loc ? Loc->GetText_Steal() : NSLOCTEXT("T66.Vendor", "Steal", "STEAL"),
				FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnStealSlot, i),
				ET66ButtonType::Danger)
			.SetMinWidth(260.f)
			.SetPadding(FMargin(12.f, 10.f))
			.SetFontSize(14)
		);
		StealButtons[i] = StealBtnWidget;

		ShopGrid->AddSlot(Col, Row)
		[
			SNew(SBox)
			.MinDesiredWidth(400.f)
			.MinDesiredHeight(220.f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 0.f, FT66Style::Tokens::Space3, 0.f)
						[
							FT66Style::MakePanel(
								SNew(SBox)
								.WidthOverride(64.f)
								.HeightOverride(64.f)
								[
									SAssignNew(ItemIconImages[i], SImage)
									.Image(ItemIconBrushes[i].Get())
									.ColorAndOpacity(FLinearColor::White)
								]
							,
								FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f),
								&ItemIconBorders[i])
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
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[ BuyBtnWidget ]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
						[ StealBtnWidget ]
					]
				,
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4),
					&ItemTileBorders[i])
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
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Vendor", "StealTimingTitle", "STEAL (TIMING)"))
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
							FT66Style::MakePanel(SNullWidget::NullWidget, FT66PanelParams(ET66PanelType::Panel2))
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
									FT66Style::MakePanel(
										SNullWidget::NullWidget,
										FT66PanelParams(ET66PanelType::Panel2).SetColor(FT66Style::Tokens::Success))
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Vendor", "StealTimingInstructions", "Press STOP near the center."))
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							NSLOCTEXT("T66.Vendor", "Stop", "STOP"),
							FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnStealStop),
							ET66ButtonType::Primary)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(22.f, 12.f))
					)
				]
			,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(18.f).SetColor(FT66Style::Tokens::Panel2))
		];

	auto MakeTitle = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.TextStyle(&TextTitle)
			.ColorAndOpacity(FT66Style::Tokens::Text);
	};

	const FText VendorPrompt = Loc ? Loc->GetText_VendorDialoguePrompt() : NSLOCTEXT("T66.Vendor", "VendorDialoguePrompt", "What do you want?");
	const FText ShopChoice = Loc ? Loc->GetText_IWantToShop() : NSLOCTEXT("T66.Vendor", "IWantToShop", "I want to shop");
	const FText TeleportChoice = Loc ? Loc->GetText_TeleportMeToYourBrother() : NSLOCTEXT("T66.Gambler", "TeleportMeToYourBrother", "Teleport me to your brother");

	TSharedRef<SWidget> DialoguePage =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 20.f)
		[ MakeTitle(VendorTitle) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(VendorPrompt)
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(ShopChoice,
					FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnDialogueShop),
					ET66ButtonType::Primary)
				.SetMinWidth(420.f)
				.SetPadding(FMargin(18.f, 10.f))
				.SetEnabled(TAttribute<bool>::CreateLambda([this]() { return !IsBossActive(); }))
			)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(TeleportChoice,
					FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnDialogueTeleport),
					ET66ButtonType::Neutral)
				.SetMinWidth(420.f)
				.SetPadding(FMargin(18.f, 10.f))
				.SetEnabled(TAttribute<bool>::CreateLambda([this]() { return !IsBossActive(); }))
			)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(BackText,
					FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBack),
					ET66ButtonType::Neutral)
				.SetMinWidth(420.f)
				.SetPadding(FMargin(18.f, 10.f))
			)
		];

	// Pre-create inventory grid (buttons use centralized MakeButton).
	TSharedRef<SUniformGridPanel> InventoryGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f));

	for (int32 Inv = 0; Inv < UT66RunStateSubsystem::MaxInventorySlots; ++Inv)
	{
		TSharedRef<SWidget> SlotBtn = FT66Style::MakeButton(
			FT66ButtonParams(
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSelectInventorySlot, Inv),
				ET66ButtonType::Neutral)
			.SetMinWidth(160.f).SetHeight(160.f)
			.SetPadding(FMargin(0.f))
			.SetContent(
				FT66Style::MakePanel(
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(InventorySlotIconImages[Inv], SImage)
						.Image(InventorySlotIconBrushes[Inv].Get())
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SAssignNew(InventorySlotTexts[Inv], STextBlock)
						.Text(NSLOCTEXT("T66.Common", "Dash", "-"))
						.TextStyle(&TextChip)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				,
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f, 10.f)),
					&InventorySlotBorders[Inv])
			)
		);
		InventorySlotButtons[Inv] = SlotBtn;
		InventoryGrid->AddSlot(Inv, 0)[SlotBtn];
	}

	// Pre-create sell button (needs member reference for later SetEnabled).
	TSharedRef<SWidget> SellBtnWidget = FT66Style::MakeButton(
		FT66ButtonParams(
			Loc ? Loc->GetText_Sell() : NSLOCTEXT("T66.Common", "Sell", "SELL"),
			FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSellSelectedClicked),
			ET66ButtonType::Primary)
		.SetMinWidth(0.f)
		.SetPadding(FMargin(18.f, 10.f))
	);
	SellItemButton = SellBtnWidget;

	TSharedRef<SWidget> ShopPage =
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
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f)
			[
				SAssignNew(StatsPanelBox, SBox)
				[
					T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc)
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ShopTitle)
							.TextStyle(&TextHeading)
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							SNew(SSpacer)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(RerollText,
									FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnReroll),
									ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetPadding(FMargin(16.f, 10.f))
								.SetEnabled(TAttribute<bool>::CreateLambda([this]() { return !IsBossActive(); }))
							)
						]
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							SNew(SSpacer)
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							ShopGrid
						]
					]
				,
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					// Anger circle (top)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 14.f)
					[
						SNew(SBox)
						.WidthOverride(220.f)
						.HeightOverride(220.f)
						[
							SAssignNew(AngerCircleImage, SImage)
							.Image(Style.GetBrush("T66.Brush.Circle"))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
					// Bank (bottom, separate panel)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f)
					[
						FT66Style::MakePanel(
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
								[
									SNew(SBox)
									.WidthOverride(110.f)
									.HeightOverride(44.f)
									[
										SAssignNew(BorrowAmountSpin, SSpinBox<int32>)
										.MinValue(0).MaxValue(999999).Delta(10)
										.Font(FT66Style::Tokens::FontBold(16))
										.Value_Lambda([this]() { return BorrowAmount; })
										.OnValueChanged_Lambda([this](int32 V) { BorrowAmount = FMath::Max(0, V); })
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									FT66Style::MakeButton(
										FT66ButtonParams(
											Loc ? Loc->GetText_Borrow() : NSLOCTEXT("T66.Vendor", "Borrow_Button", "BORROW"),
											FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBorrowClicked),
											ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetPadding(FMargin(16.f, 10.f))
									)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[
									SNew(SBox)
									.WidthOverride(110.f)
									.HeightOverride(44.f)
									[
										SAssignNew(PaybackAmountSpin, SSpinBox<int32>)
										.MinValue(0).MaxValue(999999).Delta(10)
										.Font(FT66Style::Tokens::FontBold(16))
										.Value_Lambda([this]() { return PaybackAmount; })
										.OnValueChanged_Lambda([this](int32 V) { PaybackAmount = FMath::Max(0, V); })
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[
									FT66Style::MakeButton(
										FT66ButtonParams(
											Loc ? Loc->GetText_Max() : NSLOCTEXT("T66.Common", "Max", "MAX"),
											FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnPaybackMax),
											ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetPadding(FMargin(16.f, 10.f))
									)
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									FT66Style::MakeButton(
										FT66ButtonParams(
											Loc ? Loc->GetText_Payback() : NSLOCTEXT("T66.Vendor", "Payback_Button", "PAYBACK"),
											FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnPaybackClicked),
											ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetPadding(FMargin(16.f, 10.f))
									)
								]
							]
						,
							FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space5))
					]
				,
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 16.f, 0.f, 0.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(BackText,
					FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBack),
					ET66ButtonType::Neutral)
				.SetMinWidth(0.f)
				.SetPadding(FMargin(20.f, 12.f))
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(InventoryTitle)
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 16.f, 0.f)
					[
						SAssignNew(GoldText, STextBlock)
						.Text(FText::Format(
							Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}"),
							FText::AsNumber(0)))
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(DebtText, STextBlock)
						.Text(FText::Format(
							Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Owe: {0}"),
							FText::AsNumber(0)))
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Danger)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SSpacer)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space4, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						InventoryGrid
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(FT66Style::Tokens::Space6, 0.f, 0.f, 0.f)
					[
						// Sell details for selected item
						SAssignNew(SellPanelContainer, SBox)
								.Visibility(EVisibility::Visible)
						[
							FT66Style::MakePanel(
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
								[ SellBtnWidget ]
							,
								FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space4))
						]
					]
				]
			,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel))
		];

	TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			FT66Style::MakePanel(
				SAssignNew(PageSwitcher, SWidgetSwitcher)
				+ SWidgetSwitcher::Slot()
				[
					DialoguePage
				]
				+ SWidgetSwitcher::Slot()
				[
					ShopPage
				]
			,
				FT66PanelParams(ET66PanelType::Bg).SetPadding(24.f).SetColor(FT66Style::Tokens::Bg))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			StealPromptWidget
		];

	SetPage(EVendorPage::Dialogue);
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
	RefreshStatsPanel();
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
	RefreshStatsPanel();
}

void UT66VendorOverlayWidget::RefreshStatsPanel()
{
	if (!StatsPanelBox.IsValid()) return;
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	StatsPanelBox->SetContent(T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc));
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
		const FText Fmt = Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
		GoldText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentGold())));
	}
	if (DebtText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Owe: {0}");
		DebtText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentDebt())));
	}

	// Anger circle color rules (discrete):
	// 0 => white
	// 1-50 => pink
	// 51-99 => purple
	// 100 => red (boss triggers)
	if (AngerCircleImage.IsValid())
	{
		const int32 Anger = FMath::Clamp(RunState->GetVendorAngerGold(), 0, UT66RunStateSubsystem::VendorAngerThresholdGold);
		FLinearColor C = FLinearColor::White;
		if (Anger >= UT66RunStateSubsystem::VendorAngerThresholdGold)
		{
			C = FT66Style::Tokens::Danger;
		}
		else if (Anger >= 51)
		{
			C = FLinearColor(0.60f, 0.25f, 0.90f, 1.f); // purple
		}
		else if (Anger >= 1)
		{
			C = FLinearColor(0.95f, 0.35f, 0.65f, 1.f); // pink
		}
		AngerCircleImage->SetColorAndOpacity(FSlateColor(C));
	}
}

FReply UT66VendorOverlayWidget::OnReroll()
{
	if (IsBossActive())
	{
		return FReply::Handled();
	}
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World))
		{
			RunState->RerollVendorStockForCurrentStage();
		}
	}
	RefreshAll();
	return FReply::Handled();
}

void UT66VendorOverlayWidget::RefreshStock()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
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
			ItemNameTexts[i]->SetText(bHasItem
				? (Loc ? Loc->GetText_ItemDisplayName(Stock[i]) : FText::FromName(Stock[i]))
				: NSLOCTEXT("T66.Common", "Empty", "EMPTY"));
		}
		if (ItemDescTexts.IsValidIndex(i) && ItemDescTexts[i].IsValid())
		{
			if (!bHasData)
			{
				ItemDescTexts[i]->SetText(FText::GetEmpty());
			}
			else
			{
				// Items v1: show only the single main stat line (flat numeric bonus).
				auto StatLabel = [&](ET66HeroStatType Type) -> FText
				{
					if (Loc)
					{
						switch (Type)
						{
							case ET66HeroStatType::Damage: return Loc->GetText_Stat_Damage();
							case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
							case ET66HeroStatType::AttackSize: return Loc->GetText_Stat_AttackSize();
							case ET66HeroStatType::Armor: return Loc->GetText_Stat_Armor();
							case ET66HeroStatType::Evasion: return Loc->GetText_Stat_Evasion();
							case ET66HeroStatType::Luck: return Loc->GetText_Stat_Luck();
							default: break;
						}
					}
					switch (Type)
					{
						case ET66HeroStatType::Damage: return NSLOCTEXT("T66.Stats", "Damage", "Damage");
						case ET66HeroStatType::AttackSpeed: return NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed");
						case ET66HeroStatType::AttackSize: return NSLOCTEXT("T66.Stats", "AttackSize", "Attack Size");
						case ET66HeroStatType::Armor: return NSLOCTEXT("T66.Stats", "Armor", "Armor");
						case ET66HeroStatType::Evasion: return NSLOCTEXT("T66.Stats", "Evasion", "Evasion");
						case ET66HeroStatType::Luck: return NSLOCTEXT("T66.Stats", "Luck", "Luck");
						default: return FText::GetEmpty();
					}
				};

				ET66HeroStatType MainType = D.MainStatType;
				int32 MainValue = D.MainStatValue;
				if (MainValue == 0)
				{
					// Derive from legacy v0 fields until DT_Items is updated.
					switch (D.EffectType)
					{
						case ET66ItemEffectType::BonusDamagePct: MainType = ET66HeroStatType::Damage; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f); break;
						case ET66ItemEffectType::BonusAttackSpeedPct: MainType = ET66HeroStatType::AttackSpeed; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f); break;
						case ET66ItemEffectType::BonusArmorPctPoints: MainType = ET66HeroStatType::Armor; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f); break;
						case ET66ItemEffectType::BonusEvasionPctPoints: MainType = ET66HeroStatType::Evasion; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f); break;
						case ET66ItemEffectType::BonusLuckFlat: MainType = ET66HeroStatType::Luck; MainValue = FMath::RoundToInt(FMath::Max(0.f, D.EffectMagnitude)); break;
						case ET66ItemEffectType::BonusMoveSpeedPct:
						case ET66ItemEffectType::DashCooldownReductionPct:
							MainType = ET66HeroStatType::Evasion;
							MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f);
							break;
						default: break;
					}
				}

				ItemDescTexts[i]->SetText((MainValue > 0)
					? FText::Format(NSLOCTEXT("T66.ItemTooltip", "MainStatLineFormat", "{0}: +{1}"), StatLabel(MainType), FText::AsNumber(MainValue))
					: FText::GetEmpty());
			}
		}
		if (ItemIconBorders.IsValidIndex(i) && ItemIconBorders[i].IsValid())
		{
			ItemIconBorders[i]->SetBorderBackgroundColor(bHasData ? D.PlaceholderColor : FT66Style::Tokens::Panel2);
		}
		if (ItemIconBrushes.IsValidIndex(i) && ItemIconBrushes[i].IsValid())
		{
			if (bHasData && !D.Icon.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, D.Icon, this, ItemIconBrushes[i], FName(TEXT("VendorStock"), i + 1), /*bClearWhileLoading*/ true);
			}
			else
			{
				ItemIconBrushes[i]->SetResourceObject(nullptr);
			}
		}
		if (ItemIconImages.IsValidIndex(i) && ItemIconImages[i].IsValid())
		{
			const bool bHasIcon = bHasData && !D.Icon.IsNull();
			ItemIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
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
				BuyButtonTexts[i]->SetText(Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY"));
			}
			else if (bSold)
			{
				BuyButtonTexts[i]->SetText(NSLOCTEXT("T66.Vendor", "Sold", "SOLD"));
			}
			else
			{
				const int32 Price = bHasData ? D.BuyValueGold : 0;
				BuyButtonTexts[i]->SetText(FText::Format(
					NSLOCTEXT("T66.Vendor", "BuyPriceFormat", "BUY ({0}g)"),
					FText::AsNumber(Price)));
			}
		}
		if (StealButtons.IsValidIndex(i) && StealButtons[i].IsValid())
		{
			StealButtons[i]->SetEnabled(bHasItem && !bSold && bBoughtSomethingThisVisit && !IsBossActive());
		}
	}
}

void UT66VendorOverlayWidget::SetPage(EVendorPage Page)
{
	if (!PageSwitcher.IsValid()) return;
	PageSwitcher->SetActiveWidgetIndex(static_cast<int32>(Page));
}

void UT66VendorOverlayWidget::OpenShopPage()
{
	SetPage(EVendorPage::Shop);
	RefreshAll();
}

FReply UT66VendorOverlayWidget::OnDialogueShop()
{
	SetPage(EVendorPage::Shop);
	RefreshAll();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnDialogueTeleport()
{
	if (IsBossActive())
	{
		// Keep UI open but report status.
		if (StatusText.IsValid())
		{
			UT66LocalizationSubsystem* Loc2 = nullptr;
			if (UWorld* World = GetWorld())
			{
				if (UGameInstance* GI = World->GetGameInstance())
				{
					Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
				}
			}
			StatusText->SetText(Loc2 ? Loc2->GetText_TeleportDisabledBossActive() : NSLOCTEXT("T66.Vendor", "TeleportDisabledBossActive", "Teleport disabled (boss encounter started)."));
		}
		return FReply::Handled();
	}

	TeleportToGambler();
	CloseOverlay();
	return FReply::Handled();
}

void UT66VendorOverlayWidget::TeleportToGambler()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return;

	AT66GamblerNPC* Gambler = nullptr;
	for (TActorIterator<AT66GamblerNPC> It(World); It; ++It)
	{
		Gambler = *It;
		break;
	}
	if (!Gambler) return;

	const FVector GamblerLoc = Gambler->GetActorLocation();
	Pawn->SetActorLocation(GamblerLoc + FVector(250.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
}

void UT66VendorOverlayWidget::RefreshInventory()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FName>& Inv = RunState->GetInventory();

	// Auto-select the first valid item so the details panel is "big" immediately.
	if (SelectedInventoryIndex < 0)
	{
		for (int32 i = 0; i < Inv.Num(); ++i)
		{
			if (!Inv[i].IsNone())
			{
				SelectedInventoryIndex = i;
				break;
			}
		}
	}
	if (SelectedInventoryIndex >= Inv.Num())
	{
		SelectedInventoryIndex = (Inv.Num() > 0) ? (Inv.Num() - 1) : -1;
	}

	for (int32 i = 0; i < InventorySlotTexts.Num(); ++i)
	{
		const bool bHasItem = Inv.IsValidIndex(i) && !Inv[i].IsNone();
		if (InventorySlotTexts[i].IsValid())
		{
			// Keep the strip clean: icons for items, "-" for empty slots.
			InventorySlotTexts[i]->SetText(bHasItem ? FText::GetEmpty() : NSLOCTEXT("T66.Common", "Dash", "-"));
		}
		if (InventorySlotButtons.IsValidIndex(i) && InventorySlotButtons[i].IsValid())
		{
			InventorySlotButtons[i]->SetEnabled(bHasItem && !IsBossActive());
		}
		if (InventorySlotBorders.IsValidIndex(i) && InventorySlotBorders[i].IsValid())
		{
			FLinearColor Fill = FT66Style::Tokens::Panel2;
			FItemData D;
			const bool bHasData = bHasItem && GI && GI->GetItemData(Inv[i], D);
			if (bHasData)
			{
				Fill = D.PlaceholderColor;
			}

			// If selected, tint toward accent for readability.
			if (i == SelectedInventoryIndex)
			{
				Fill = Fill * 0.45f + FT66Style::Tokens::Accent * 0.55f;
			}
			InventorySlotBorders[i]->SetBorderBackgroundColor(Fill);

			if (InventorySlotIconBrushes.IsValidIndex(i) && InventorySlotIconBrushes[i].IsValid())
			{
				if (bHasData && !D.Icon.IsNull() && TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, D.Icon, this, InventorySlotIconBrushes[i], FName(TEXT("VendorInv"), i + 1), /*bClearWhileLoading*/ true);
				}
				else
				{
					InventorySlotIconBrushes[i]->SetResourceObject(nullptr);
				}
			}
			if (InventorySlotIconImages.IsValidIndex(i) && InventorySlotIconImages[i].IsValid())
			{
				const bool bHasIcon = bHasData && !D.Icon.IsNull();
				InventorySlotIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
			}
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
		// Keep visible so the inventory layout doesn't "pop" when selecting an item.
		SellPanelContainer->SetVisibility(EVisibility::Visible);
	}
	if (!bValidSelection)
	{
		if (SellItemNameText.IsValid()) SellItemNameText->SetText(FText::GetEmpty());
		if (SellItemDescText.IsValid()) SellItemDescText->SetText(FText::GetEmpty());
		if (SellItemPriceText.IsValid()) SellItemPriceText->SetText(FText::GetEmpty());
		if (SellItemButton.IsValid()) SellItemButton->SetEnabled(false);
		return;
	}

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	FItemData D;
	const bool bHasData = GI && GI->GetItemData(Inv[SelectedInventoryIndex], D);
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI2 = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI2->GetSubsystem<UT66LocalizationSubsystem>();
	}

	if (SellItemNameText.IsValid())
	{
		SellItemNameText->SetText(Loc ? Loc->GetText_ItemDisplayName(Inv[SelectedInventoryIndex]) : FText::FromName(Inv[SelectedInventoryIndex]));
	}
	if (SellItemDescText.IsValid())
	{
		if (!bHasData)
		{
			SellItemDescText->SetText(FText::GetEmpty());
		}
		else
		{
			auto StatLabel = [&](ET66HeroStatType Type) -> FText
			{
				if (Loc)
				{
					switch (Type)
					{
						case ET66HeroStatType::Damage: return Loc->GetText_Stat_Damage();
						case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
						case ET66HeroStatType::AttackSize: return Loc->GetText_Stat_AttackSize();
						case ET66HeroStatType::Armor: return Loc->GetText_Stat_Armor();
						case ET66HeroStatType::Evasion: return Loc->GetText_Stat_Evasion();
						case ET66HeroStatType::Luck: return Loc->GetText_Stat_Luck();
						default: break;
					}
				}
				switch (Type)
				{
					case ET66HeroStatType::Damage: return NSLOCTEXT("T66.Stats", "Damage", "Damage");
					case ET66HeroStatType::AttackSpeed: return NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed");
					case ET66HeroStatType::AttackSize: return NSLOCTEXT("T66.Stats", "AttackSize", "Attack Size");
					case ET66HeroStatType::Armor: return NSLOCTEXT("T66.Stats", "Armor", "Armor");
					case ET66HeroStatType::Evasion: return NSLOCTEXT("T66.Stats", "Evasion", "Evasion");
					case ET66HeroStatType::Luck: return NSLOCTEXT("T66.Stats", "Luck", "Luck");
					default: return FText::GetEmpty();
				}
			};

			ET66HeroStatType MainType = D.MainStatType;
			int32 MainValue = D.MainStatValue;
			if (MainValue == 0)
			{
				switch (D.EffectType)
				{
					case ET66ItemEffectType::BonusDamagePct: MainType = ET66HeroStatType::Damage; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f); break;
					case ET66ItemEffectType::BonusAttackSpeedPct: MainType = ET66HeroStatType::AttackSpeed; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f); break;
					case ET66ItemEffectType::BonusArmorPctPoints: MainType = ET66HeroStatType::Armor; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f); break;
					case ET66ItemEffectType::BonusEvasionPctPoints: MainType = ET66HeroStatType::Evasion; MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 4.f); break;
					case ET66ItemEffectType::BonusLuckFlat: MainType = ET66HeroStatType::Luck; MainValue = FMath::RoundToInt(FMath::Max(0.f, D.EffectMagnitude)); break;
					case ET66ItemEffectType::BonusMoveSpeedPct:
					case ET66ItemEffectType::DashCooldownReductionPct:
						MainType = ET66HeroStatType::Evasion;
						MainValue = FMath::CeilToInt(FMath::Max(0.f, D.EffectMagnitude) / 10.f);
						break;
					default: break;
				}
			}

			SellItemDescText->SetText((MainValue > 0)
				? FText::Format(NSLOCTEXT("T66.ItemTooltip", "MainStatLineFormat", "{0}: +{1}"), StatLabel(MainType), FText::AsNumber(MainValue))
				: FText::GetEmpty());
		}
	}
	if (SellItemPriceText.IsValid())
	{
		const int32 SellValue = bHasData ? D.SellValueGold : 0;
		SellItemPriceText->SetText(FText::Format(
			NSLOCTEXT("T66.Vendor", "SellForFormat", "SELL FOR: {0}g"),
			FText::AsNumber(SellValue)));
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
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	if (BorrowAmount <= 0)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_BorrowAmountMustBePositive() : NSLOCTEXT("T66.Vendor", "BorrowMustBePositive", "Borrow amount must be > 0."));
		}
		return FReply::Handled();
	}
	RunState->BorrowGold(BorrowAmount);
	if (StatusText.IsValid()) StatusText->SetText(FText::GetEmpty());
	RefreshTopBar();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnPaybackMax()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World))
		{
			PaybackAmount = FMath::Max(0, RunState->GetCurrentDebt());
			if (PaybackAmountSpin.IsValid())
			{
				PaybackAmountSpin->SetValue(PaybackAmount);
			}
			if (StatusText.IsValid()) StatusText->SetText(FText::GetEmpty());
			RefreshTopBar();
		}
	}
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnPaybackClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	if (PaybackAmount <= 0)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_PaybackAmountMustBePositive() : NSLOCTEXT("T66.Vendor", "PaybackMustBePositive", "Payback amount must be > 0."));
		}
		return FReply::Handled();
	}
	const int32 Paid = RunState->PayDebt(PaybackAmount);
	if (StatusText.IsValid()) StatusText->SetText(FText::GetEmpty());
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
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "SelectItemToSell", "Select an item to sell."));
		return FReply::Handled();
	}

	const bool bSold = RunState->SellInventoryItemAt(SelectedInventoryIndex);
	if (bSold)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "SoldStatus", "Sold."));
	}
	else
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "CouldNotSell", "Could not sell."));
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
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "BossIsActive", "Boss is active."));
		return FReply::Handled();
	}

	if (!RunState->HasInventorySpace())
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "InventoryFull", "Inventory full."));
		return FReply::Handled();
	}

	const bool bBought = RunState->TryBuyVendorStockSlot(SlotIndex);
	if (bBought)
	{
		bBoughtSomethingThisVisit = true;
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "Purchased", "Purchased."));
	}
	else
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "CouldNotPurchase", "Could not purchase."));
	}
	RefreshAll();
	return FReply::Handled();
}

FReply UT66VendorOverlayWidget::OnStealSlot(int32 SlotIndex)
{
	if (!bBoughtSomethingThisVisit)
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "BuyOneBeforeStealing", "Buy one item before stealing."));
		return FReply::Handled();
	}
	if (IsBossActive())
	{
		if (StatusText.IsValid()) StatusText->SetText(NSLOCTEXT("T66.Vendor", "BossIsActive", "Boss is active."));
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
	StealLastTickTimeSeconds = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;

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
		// 30Hz is sufficient for this UI and reduces overhead on low-end CPUs.
		World->GetTimerManager().SetTimer(StealTickTimerHandle, this, &UT66VendorOverlayWidget::TickStealBar, 0.033f, true);
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
	UWorld* World = GetWorld();
	if (!World) return;
	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - StealLastTickTimeSeconds;
	StealLastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.05f);
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

	// Backend owns RNG: timing improves odds, Luck can bias outcomes.
	const bool bGranted = RunState->ResolveVendorStealAttempt(PendingStealIndex, bTimingHit, false);
	HideStealPrompt();

	if (StatusText.IsValid())
	{
		const ET66VendorStealOutcome Outcome = RunState->GetLastVendorStealOutcome();
		if (Outcome == ET66VendorStealOutcome::Miss) StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealFailedMiss", "Steal failed (miss). Anger increased."));
		else if (Outcome == ET66VendorStealOutcome::Failed) StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealFailed", "Steal failed. Anger increased."));
		else if (Outcome == ET66VendorStealOutcome::InventoryFull) StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealSucceededButInventoryFull", "Steal succeeded, but inventory is full."));
		else if (Outcome == ET66VendorStealOutcome::Success && bGranted) StatusText->SetText(NSLOCTEXT("T66.Vendor", "Stolen", "Stolen."));
		else StatusText->SetText(NSLOCTEXT("T66.Vendor", "StealFailed", "Steal failed. Anger increased."));
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

