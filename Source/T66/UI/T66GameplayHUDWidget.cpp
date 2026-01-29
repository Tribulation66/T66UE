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
	RunState->DebtChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->PanelVisibilityChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->ScoreChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StageChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StageTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->BossChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->DifficultyChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RefreshHUD();
}

void UT66GameplayHUDWidget::NativeDestruct()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (RunState)
	{
		RunState->HeartsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->GoldChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->DebtChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->PanelVisibilityChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StageChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->BossChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->DifficultyChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
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
		GoldText->SetText(FText::FromString(FString::Printf(TEXT("Gold: %d"), RunState->GetCurrentGold())));
	}
	// Owe (Debt) in red
	if (DebtText.IsValid())
	{
		DebtText->SetText(FText::FromString(FString::Printf(TEXT("Owe: %d"), RunState->GetCurrentDebt())));
	}
	// Bounty (Score)
	if (ScoreText.IsValid())
	{
		ScoreText->SetText(FText::AsNumber(RunState->GetCurrentScore()));
	}

	// Stage number: x
	if (StageText.IsValid())
	{
		StageText->SetText(FText::FromString(FString::Printf(TEXT("Stage number: %d"), RunState->GetCurrentStage())));
	}

	// Stage timer: frozen at full until start gate, then countdown (e.g. 6:00, 0:45)
	if (TimerText.IsValid())
	{
		const float Secs = RunState->GetStageTimerSecondsRemaining();
		const int32 Total = FMath::CeilToInt(FMath::Max(0.f, Secs));
		const int32 M = FMath::FloorToInt(Total / 60.f);
		const int32 S = FMath::FloorToInt(FMath::Fmod(static_cast<float>(Total), 60.f));
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), M, S)));
	}

	// Boss health bar: visible only when boss awakened
	const bool bBossActive = RunState->GetBossActive();
	if (BossBarContainerBox.IsValid())
	{
		BossBarContainerBox->SetVisibility(bBossActive ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (bBossActive)
	{
		const int32 BossHP = RunState->GetBossCurrentHP();
		const int32 BossMax = FMath::Max(1, RunState->GetBossMaxHP());
		const float Pct = static_cast<float>(BossHP) / static_cast<float>(BossMax);
		const float BarWidth = 600.f;
		if (BossBarFillBox.IsValid())
		{
			BossBarFillBox->SetWidthOverride(FMath::Clamp(BarWidth * Pct, 0.f, BarWidth));
		}
		if (BossBarText.IsValid())
		{
			BossBarText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), BossHP, BossMax)));
		}
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

	// Difficulty squares (placeholder for skulls): bright when active
	const int32 DifficultyTier = RunState->GetDifficultyTier();
	for (int32 i = 0; i < DifficultyBorders.Num(); ++i)
	{
		if (!DifficultyBorders[i].IsValid()) continue;
		const bool bActive = i < DifficultyTier;
		const FLinearColor C = bActive ? FLinearColor(0.95f, 0.15f, 0.15f, 1.f) : FLinearColor(0.18f, 0.18f, 0.22f, 1.f);
		DifficultyBorders[i]->SetBorderBackgroundColor(C);
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
	DifficultyBorders.SetNum(5);
	InventorySlotBorders.SetNum(5);
	static constexpr float BossBarWidth = 600.f;

	// Difficulty row (placeholder squares)
	TSharedRef<SHorizontalBox> DifficultyRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < DifficultyBorders.Num(); ++i)
	{
		TSharedPtr<SBorder> DiffBorder;
		DifficultyRowRef->AddSlot()
			.AutoWidth()
			.Padding(3.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(18.f)
				.HeightOverride(18.f)
				[
					SAssignNew(DiffBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.18f, 0.18f, 0.22f, 1.f))
				]
			];
		DifficultyBorders[i] = DiffBorder;
	}

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
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.f, 12.f)
		[
			SAssignNew(BossBarContainerBox, SBox)
			.WidthOverride(BossBarWidth)
			.HeightOverride(28.f)
			.Visibility(EVisibility::Collapsed)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.08f, 0.9f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(BossBarFillBox, SBox)
					.WidthOverride(BossBarWidth)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.9f, 0.1f, 0.1f, 0.95f))
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SAssignNew(BossBarText, STextBlock)
					.Text(FText::FromString(TEXT("100/100")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		]
		// Top-left stats (no overlap with bottom-left portrait stack)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f, 24.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SAssignNew(StageText, STextBlock)
				.Text(FText::FromString(TEXT("Stage number: 1")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				.ColorAndOpacity(FLinearColor::White)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SAssignNew(TimerText, STextBlock)
				.Text(FText::FromString(TEXT("6:00")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
				.ColorAndOpacity(FLinearColor(0.2f, 0.9f, 0.3f, 1.f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SAssignNew(GoldText, STextBlock)
					.Text(FText::FromString(TEXT("Gold: 0")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
				[
					SAssignNew(DebtText, STextBlock)
					.Text(FText::FromString(TEXT("Owe: 0")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(FLinearColor(0.95f, 0.15f, 0.15f, 1.f))
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Bounty: ")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(FLinearColor(0.85f, 0.75f, 0.2f, 1.f))
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SAssignNew(ScoreText, STextBlock)
					.Text(FText::AsNumber(0))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.3f, 1.f))
				]
			]
		]

		// Bottom-left portrait stack: difficulty squares (placeholder skulls) -> hearts -> portrait
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 0.f, 24.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[ DifficultyRowRef ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
			[ HeartsRowRef ]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(84.f)
				.HeightOverride(84.f)
				[
					SAssignNew(PortraitBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.14f, 1.f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("PORTRAIT")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.f))
						.Justification(ETextJustify::Center)
					]
				]
			]
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
