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
	RunState->ScoreChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StageChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->StageTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	RunState->BossChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
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
		RunState->ScoreChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StageChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
		RunState->BossChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
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

	// Stage timer: frozen at 60 until start gate, then countdown (e.g. 1:00, 0:45)
	if (TimerText.IsValid())
	{
		const float Secs = RunState->GetStageTimerSecondsRemaining();
		const int32 M = FMath::FloorToInt(Secs / 60.f);
		const int32 S = FMath::FloorToInt(FMath::Fmod(Secs, 60.f));
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
	static constexpr float BossBarWidth = 600.f;

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
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f, 64.f)
		[
			SAssignNew(StageText, STextBlock)
			.Text(FText::FromString(TEXT("Stage number: 1")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			.ColorAndOpacity(FLinearColor::White)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f, 88.f)
		[
			SAssignNew(TimerText, STextBlock)
			.Text(FText::FromString(TEXT("1:00")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FLinearColor(0.2f, 0.9f, 0.3f, 1.f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f, 132.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Bounty:")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
			.ColorAndOpacity(FLinearColor(0.85f, 0.75f, 0.2f, 1.f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(90.f, 128.f)
		[
			SAssignNew(ScoreText, STextBlock)
			.Text(FText::AsNumber(0))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FLinearColor(0.95f, 0.85f, 0.3f, 1.f))
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
