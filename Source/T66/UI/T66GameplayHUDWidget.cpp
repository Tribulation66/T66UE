// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GameplayHUDWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UT66GameplayHUDWidget::RebuildWidget()
{
	return BuildSlateUI();
}

UT66RunStateSubsystem* UT66GameplayHUDWidget::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

void UT66GameplayHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	RunState->HeartsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->GoldChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->PanelVisibilityChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RefreshHUD();
}

void UT66GameplayHUDWidget::NativeDestruct()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (RunState)
	{
		RunState->HeartsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->GoldChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->PanelVisibilityChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	}
	Super::NativeDestruct();
}

void UT66GameplayHUDWidget::RefreshHUD()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Gold
	if (GoldText.IsValid())
	{
		GoldText->SetText(FText::AsNumber(RunState->GetCurrentGold()));
	}

	// Hearts: 5 icons, red = full, grey = lost
	const int32 CurrentHearts = RunState->GetCurrentHearts();
	const int32 MaxHearts = RunState->GetMaxHearts();
	for (int32 i = 0; i < HeartBorders.Num() && i < MaxHearts; ++i)
	{
		if (HeartBorders[i].IsValid())
		{
			FLinearColor Color = (i < CurrentHearts) ? FLinearColor(0.9f, 0.2f, 0.2f, 1.f) : FLinearColor(0.35f, 0.35f, 0.35f, 1.f);
			HeartBorders[i]->SetBorderBackgroundColor(Color);
		}
	}

	// Inventory slots: red when item picked up, grey when empty
	const TArray<FName>& Inv = RunState->GetInventory();
	for (int32 i = 0; i < InventorySlotBorders.Num(); ++i)
	{
		if (!InventorySlotBorders[i].IsValid()) continue;
		FLinearColor SlotColor = FLinearColor(0.2f, 0.2f, 0.22f, 1.f);
		if (i < Inv.Num())
		{
			SlotColor = FLinearColor(1.f, 0.f, 0.f, 1.f); // red when item picked up
		}
		InventorySlotBorders[i]->SetBorderBackgroundColor(SlotColor);
	}

	// Panel visibility
	const EVisibility PanelVis = RunState->GetHUDPanelsVisible() ? EVisibility::Visible : EVisibility::Collapsed;
	if (InventoryPanelBox.IsValid()) InventoryPanelBox->SetVisibility(PanelVis);
	if (MinimapPanelBox.IsValid()) MinimapPanelBox->SetVisibility(PanelVis);
}

TSharedRef<SWidget> UT66GameplayHUDWidget::BuildSlateUI()
{
	HeartBorders.SetNum(UT66RunStateSubsystem::DefaultMaxHearts);
	InventorySlotBorders.SetNum(5);

	// Build hearts row first (5 icons)
	TSharedRef<SHorizontalBox> HeartsRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < UT66RunStateSubsystem::DefaultMaxHearts; ++i)
	{
		TSharedPtr<SBorder> HeartBorder;
		HeartsRowRef->AddSlot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(32.f)
				.HeightOverride(32.f)
				[
					SAssignNew(HeartBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.f))
				]
			];
		HeartBorders[i] = HeartBorder;
	}

	// Build inventory row (5 slots)
	TSharedRef<SHorizontalBox> InvRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < 5; ++i)
	{
		TSharedPtr<SBorder> SlotBorder;
		InvRowRef->AddSlot()
			.AutoWidth()
			.Padding(6.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(48.f)
				.HeightOverride(48.f)
				[
					SAssignNew(SlotBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.22f, 1.f))
				]
			];
		InventorySlotBorders[i] = SlotBorder;
	}

	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f, 24.f)
		[ HeartsRowRef ]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f, 100.f)
		[
			SAssignNew(GoldText, STextBlock)
			.Text(FText::AsNumber(0))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
			.ColorAndOpacity(FLinearColor::White)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(24.f, 24.f)
		[
			SAssignNew(MinimapPanelBox, SBox)
			.WidthOverride(200.f)
			.HeightOverride(150.f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.15f, 0.9f))
				.Padding(8.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("MINIMAP")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.f))
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 40.f)
		[
			SAssignNew(InventoryPanelBox, SBox)
			.HeightOverride(56.f)
			[ InvRowRef ]
		];

	return Root;
}
