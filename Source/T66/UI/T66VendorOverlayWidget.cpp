// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66VendorOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66VendorBoss.h"
#include "Data/T66DataTypes.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

static UT66RunStateSubsystem* GetRunStateFromWorld(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

void UT66VendorOverlayWidget::NativeDestruct()
{
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

	UWorld* World = GetWorld();
	UT66LocalizationSubsystem* Loc = nullptr;
	if (World)
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}

	auto Title = [](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
			.ColorAndOpacity(FLinearColor::White);
	};

	auto MakeSmallButton = [](const FText& Text, const FOnClicked& OnClicked, const FLinearColor& Bg) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(160.f)
			.HeightOverride(38.f)
			[
				SNew(SButton)
				.OnClicked(OnClicked)
				.ButtonColorAndOpacity(Bg)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Text)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.ColorAndOpacity(FLinearColor::White)
				]
			];
	};

	// Force stock generation now so we can build tiles.
	if (UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World))
	{
		RunState->EnsureVendorStockForCurrentStage();
		bBoughtSomethingThisVisit = RunState->HasBoughtFromVendorThisStage();
	}

	const FText VendorTitle = FText::FromString(TEXT("VENDOR"));
	const FText ShopTitle = FText::FromString(TEXT("SHOP"));
	const FText BackText = FText::FromString(TEXT("BACK"));
	const FText AngerLabel = FText::FromString(TEXT("ANGER"));
	const FText SellFirstBtn = FText::FromString(TEXT("SELL FIRST"));

	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	const TArray<FName> EmptyStock;
	const TArray<FName>& Stock = RunState ? RunState->GetVendorStockItemIDs() : EmptyStock;

	ItemNameTexts.SetNum(Stock.Num());
	ItemDescTexts.SetNum(Stock.Num());
	ItemPriceTexts.SetNum(Stock.Num());
	ItemTileBorders.SetNum(Stock.Num());
	BuyButtons.SetNum(Stock.Num());
	StealButtons.SetNum(Stock.Num());

	TSharedRef<SVerticalBox> TileList = SNew(SVerticalBox);
	for (int32 i = 0; i < Stock.Num(); ++i)
	{
		TileList->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 10.f)
		[
			SAssignNew(ItemTileBorders[i], SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 1.f))
			.Padding(12.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(ItemNameTexts[i], STextBlock)
						.Text(FText::FromName(Stock[i]))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(ItemDescTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
						.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.9f, 1.f))
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 16.f, 0.f)
				[
					SAssignNew(ItemPriceTexts[i], STextBlock)
					.Text(FText::FromString(TEXT("0g")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.3f, 1.f))
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
				[
					SAssignNew(BuyButtons[i], SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBuySlot, i))
					.ButtonColorAndOpacity(FLinearColor(0.20f, 0.20f, 0.28f, 1.f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("BUY")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SAssignNew(StealButtons[i], SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnStealSlot, i))
					.ButtonColorAndOpacity(FLinearColor(0.35f, 0.18f, 0.18f, 1.f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("STEAL")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor::White)
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
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.06f, 0.95f))
			.Padding(18.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("STEAL (TIMING)")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
					.ColorAndOpacity(FLinearColor::White)
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
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.10f, 0.10f, 0.12f, 1.f))
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
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.20f, 0.95f, 0.35f, 1.f))
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Press STOP near the center.")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					MakeSmallButton(FText::FromString(TEXT("STOP")), FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnStealStop), FLinearColor(0.15f, 0.75f, 0.25f, 1.f))
				]
			]
		];

	TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.82f))
			.Padding(24.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						Title(VendorTitle)
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
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
									.ColorAndOpacity(FLinearColor::White)
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SAssignNew(DebtText, STextBlock)
									.Text(FText::FromString(TEXT("Owe: 0")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
									.ColorAndOpacity(FLinearColor(0.95f, 0.15f, 0.15f, 1.f))
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[
									SNew(STextBlock)
									.Text(Loc ? Loc->GetText_Anger() : AngerLabel)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.10f, 0.10f, 0.12f, 1.f))
									.Padding(2.f)
									[
										SNew(SOverlay)
										+ SOverlay::Slot()
										[
											SAssignNew(AngerFillBox, SBox)
											.WidthOverride(0.f)
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
												.BorderBackgroundColor(FLinearColor(0.9f, 0.2f, 0.2f, 0.95f))
											]
										]
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
								[
									SAssignNew(AngerText, STextBlock)
									.Text(FText::FromString(TEXT("0/100")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
				[
					SAssignNew(StatusText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(FLinearColor(0.95f, 0.95f, 0.95f, 1.f))
				]
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 18.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 20.f, 0.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 1.f))
						.Padding(16.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
							[
								SNew(STextBlock)
								.Text(ShopTitle)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
								.ColorAndOpacity(FLinearColor::White)
							]
							+ SVerticalBox::Slot().FillHeight(1.f)
							[
								TileList
							]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 1.f))
						.Padding(16.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_Bank() : FText::FromString(TEXT("BANK")))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
								.ColorAndOpacity(FLinearColor::White)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[ SNew(STextBlock).Text(Loc ? Loc->GetText_Borrow() : FText::FromString(TEXT("Borrow"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FLinearColor::White) ]
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
									.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
									[ SNew(STextBlock).Text(Loc ? Loc->GetText_Borrow() : FText::FromString(TEXT("BORROW"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(FLinearColor::White) ]
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
								[ SNew(STextBlock).Text(Loc ? Loc->GetText_Payback() : FText::FromString(TEXT("Payback"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(FLinearColor::White) ]
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
									.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
									[ SNew(STextBlock).Text(Loc ? Loc->GetText_Payback() : FText::FromString(TEXT("PAYBACK"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(FLinearColor::White) ]
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
							[
								SNew(SButton)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnSellFirstClicked))
								.ButtonColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.28f, 1.f))
								[
									SNew(STextBlock)
									.Text(SellFirstBtn)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
									.Justification(ETextJustify::Center)
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 16.f, 0.f, 0.f)
				[
					MakeSmallButton(Loc ? Loc->GetText_Back() : BackText, FOnClicked::CreateUObject(this, &UT66VendorOverlayWidget::OnBack), FLinearColor(0.2f, 0.2f, 0.28f, 1.f))
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

void UT66VendorOverlayWidget::RefreshAll()
{
	RefreshTopBar();
	RefreshStock();
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
	for (int32 i = 0; i < Stock.Num(); ++i)
	{
		const bool bSold = RunState->IsVendorStockSlotSold(i);
		FItemData D;
		const bool bHasData = GI && GI->GetItemData(Stock[i], D);

		if (ItemNameTexts.IsValidIndex(i) && ItemNameTexts[i].IsValid())
		{
			ItemNameTexts[i]->SetText(FText::FromName(Stock[i]));
		}
		if (ItemDescTexts.IsValidIndex(i) && ItemDescTexts[i].IsValid())
		{
			const FText Line = bHasData ? D.EffectLine1 : FText::GetEmpty();
			ItemDescTexts[i]->SetText(Line);
		}
		if (ItemPriceTexts.IsValidIndex(i) && ItemPriceTexts[i].IsValid())
		{
			const int32 Price = bHasData ? D.BuyValueGold : 0;
			ItemPriceTexts[i]->SetText(FText::FromString(FString::Printf(TEXT("%dg"), Price)));
		}
		if (ItemTileBorders.IsValidIndex(i) && ItemTileBorders[i].IsValid())
		{
			ItemTileBorders[i]->SetBorderBackgroundColor(bSold ? FLinearColor(0.04f, 0.04f, 0.05f, 1.f) : FLinearColor(0.08f, 0.08f, 0.10f, 1.f));
		}
		if (BuyButtons.IsValidIndex(i) && BuyButtons[i].IsValid())
		{
			BuyButtons[i]->SetEnabled(!bSold && !IsBossActive());
		}
		if (StealButtons.IsValidIndex(i) && StealButtons[i].IsValid())
		{
			StealButtons[i]->SetEnabled(!bSold && bBoughtSomethingThisVisit && !IsBossActive());
		}
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

FReply UT66VendorOverlayWidget::OnSellFirstClicked()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunStateFromWorld(World);
	if (!RunState) return FReply::Handled();

	const bool bSold = RunState->SellFirstItem();
	if (StatusText.IsValid()) StatusText->SetText(bSold ? FText::FromString(TEXT("Sold first item.")) : FText::FromString(TEXT("Nothing to sell.")));
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

