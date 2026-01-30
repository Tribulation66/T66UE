// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GameplayHUDWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66Rarity.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66LootBagPickup.h"
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
	RunState->IdolsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
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
		RunState->IdolsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHUD);
	}
	Super::NativeDestruct();
}

void UT66GameplayHUDWidget::RefreshHUD()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	// Gold
	if (GoldText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_GoldFormat() : FText::FromString(TEXT("Gold: {0}"));
		GoldText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentGold())));
	}
	// Owe (Debt) in red
	if (DebtText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_OweFormat() : FText::FromString(TEXT("Owe: {0}"));
		DebtText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentDebt())));
	}
	// Bounty (Score)
	if (ScoreText.IsValid())
	{
		ScoreText->SetText(FText::AsNumber(RunState->GetCurrentScore()));
	}

	// Stage number: x
	if (StageText.IsValid())
	{
		const FText Fmt = Loc ? Loc->GetText_StageNumberFormat() : FText::FromString(TEXT("Stage number: {0}"));
		StageText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentStage())));
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

	// Loot prompt: top-of-HUD, non-blocking. Accept with F, Reject with RMB.
	if (LootPromptBox.IsValid())
	{
		AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer());
		AT66LootBagPickup* Bag = PC ? PC->GetNearbyLootBag() : nullptr;
		if (Bag)
		{
			FItemData D;
			UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
			const bool bHasData = T66GI && T66GI->GetItemData(Bag->GetItemID(), D);
			const FString ItemName = bHasData ? D.ItemID.ToString() : Bag->GetItemID().ToString();

			const TCHAR* RarityStr =
				(Bag->GetLootRarity() == ET66Rarity::Black) ? TEXT("BLACK") :
				(Bag->GetLootRarity() == ET66Rarity::Red) ? TEXT("RED") :
				(Bag->GetLootRarity() == ET66Rarity::Yellow) ? TEXT("YELLOW") : TEXT("WHITE");

			if (LootPromptText.IsValid())
			{
				LootPromptText->SetText(FText::FromString(FString::Printf(TEXT("LOOT BAG (%s): %s   [F] Accept   [RMB] Reject"), RarityStr, *ItemName)));
			}
			if (LootPromptBorder.IsValid())
			{
				LootPromptBorder->SetBorderBackgroundColor(FT66RarityUtil::GetRarityColor(Bag->GetLootRarity()) * 0.35f + FLinearColor(0.02f, 0.02f, 0.03f, 0.65f));
			}
			LootPromptBox->SetVisibility(EVisibility::Visible);
		}
		else
		{
			LootPromptBox->SetVisibility(EVisibility::Collapsed);
		}
	}

	// Hearts: 5-slot compression with tier colors (red -> blue -> green -> ...)
	{
		const int32 HeartsNow = RunState->GetCurrentHearts();
		int32 Tier = 0;
		int32 Count = 0;
		FT66RarityUtil::ComputeTierAndCount5(HeartsNow, Tier, Count);
		const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
		const FLinearColor EmptyC(0.25f, 0.25f, 0.28f, 1.f);
		for (int32 i = 0; i < HeartBorders.Num(); ++i)
		{
			if (!HeartBorders[i].IsValid()) continue;
			const bool bFilled = (i < Count);
			HeartBorders[i]->SetBorderBackgroundColor(bFilled ? TierC : EmptyC);
		}
	}

	// Portrait: tier color based on MAX heart quantity (not current health).
	if (PortraitBorder.IsValid())
	{
		int32 Tier = 0;
		int32 Count = 0;
		FT66RarityUtil::ComputeTierAndCount5(RunState->GetMaxHearts(), Tier, Count);
		PortraitBorder->SetBorderBackgroundColor(Count > 0 ? FT66RarityUtil::GetTierColor(Tier) : FLinearColor(0.12f, 0.12f, 0.14f, 1.f));
	}

	// Difficulty (Skulls): 5-slot compression with tier colors + half-step support
	{
		const float Skulls = RunState->GetDifficultySkulls();
		const int32 Tier = FMath::Max(0, FMath::FloorToInt(Skulls / 5.f));
		const float Within = FMath::Clamp(Skulls - (static_cast<float>(Tier) * 5.f), 0.f, 5.f);
		const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
		const FLinearColor EmptyC(0.18f, 0.18f, 0.22f, 1.f);
		for (int32 i = 0; i < DifficultyBorders.Num(); ++i)
		{
			if (!DifficultyBorders[i].IsValid()) continue;
			const float NeedFull = static_cast<float>(i + 1);
			const float NeedHalf = static_cast<float>(i) + 0.5f;
			if (Within >= NeedFull)
			{
				DifficultyBorders[i]->SetBorderBackgroundColor(TierC);
			}
			else if (Within >= NeedHalf)
			{
				DifficultyBorders[i]->SetBorderBackgroundColor(TierC * 0.55f);
			}
			else
			{
				DifficultyBorders[i]->SetBorderBackgroundColor(EmptyC);
			}
		}
	}

	// Idol slots (3): colored if equipped, grey if empty
	const TArray<FName>& Idols = RunState->GetEquippedIdols();
	for (int32 i = 0; i < IdolSlotBorders.Num(); ++i)
	{
		if (!IdolSlotBorders[i].IsValid()) continue;
		FLinearColor C = FLinearColor(0.18f, 0.18f, 0.22f, 1.f);
		if (i < Idols.Num() && !Idols[i].IsNone())
		{
			C = UT66RunStateSubsystem::GetIdolColor(Idols[i]);
		}
		IdolSlotBorders[i]->SetBorderBackgroundColor(C);
	}

	// Inventory slots: item color + hover tooltip, grey when empty
	const TArray<FName>& Inv = RunState->GetInventory();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	for (int32 i = 0; i < InventorySlotBorders.Num(); ++i)
	{
		if (!InventorySlotBorders[i].IsValid()) continue;

		FLinearColor SlotColor = FLinearColor(0.2f, 0.2f, 0.22f, 1.f);
		FText Tooltip = FText::GetEmpty();
		if (i < Inv.Num() && !Inv[i].IsNone())
		{
			const FName ItemID = Inv[i];
			FItemData D;
			if (T66GI && T66GI->GetItemData(ItemID, D))
			{
				SlotColor = D.PlaceholderColor;
				FString Tip = FString::Printf(TEXT("%s"), *ItemID.ToString());
				if (D.PowerGivenPercent != 0.f)
				{
					Tip += FString::Printf(TEXT("\nPower: +%.1f%%"), D.PowerGivenPercent);
				}
				if (!D.EffectLine1.IsEmpty()) Tip += FString::Printf(TEXT("\n%s"), *D.EffectLine1.ToString());
				if (!D.EffectLine2.IsEmpty()) Tip += FString::Printf(TEXT("\n%s"), *D.EffectLine2.ToString());
				if (!D.EffectLine3.IsEmpty()) Tip += FString::Printf(TEXT("\n%s"), *D.EffectLine3.ToString());
				if (D.SellValueGold > 0) Tip += FString::Printf(TEXT("\nSell: %d gold"), D.SellValueGold);
				Tooltip = FText::FromString(Tip);
			}
			else
			{
				SlotColor = FLinearColor(0.95f, 0.15f, 0.15f, 1.f);
				Tooltip = FText::FromString(ItemID.ToString());
			}
		}
		InventorySlotBorders[i]->SetBorderBackgroundColor(SlotColor);
		InventorySlotBorders[i]->SetToolTipText(Tooltip);
	}

	// Panel visibility
	const EVisibility PanelVis = RunState->GetHUDPanelsVisible() ? EVisibility::Visible : EVisibility::Collapsed;
	if (InventoryPanelBox.IsValid()) InventoryPanelBox->SetVisibility(PanelVis);
	if (MinimapPanelBox.IsValid()) MinimapPanelBox->SetVisibility(PanelVis);
}

TSharedRef<SWidget> UT66GameplayHUDWidget::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const FText StageInit = Loc ? FText::Format(Loc->GetText_StageNumberFormat(), FText::AsNumber(1)) : FText::FromString(TEXT("Stage number: 1"));
	const FText GoldInit = Loc ? FText::Format(Loc->GetText_GoldFormat(), FText::AsNumber(0)) : FText::FromString(TEXT("Gold: 0"));
	const FText OweInit = Loc ? FText::Format(Loc->GetText_OweFormat(), FText::AsNumber(0)) : FText::FromString(TEXT("Owe: 0"));
	const FText BountyLabel = Loc ? Loc->GetText_BountyLabel() : FText::FromString(TEXT("Bounty:"));
	const FText PortraitLabel = Loc ? Loc->GetText_PortraitPlaceholder() : FText::FromString(TEXT("PORTRAIT"));
	const FText MinimapLabel = Loc ? Loc->GetText_MinimapPlaceholder() : FText::FromString(TEXT("MINIMAP"));

	HeartBorders.SetNum(UT66RunStateSubsystem::DefaultMaxHearts);
	DifficultyBorders.SetNum(5);
	IdolSlotBorders.SetNum(UT66RunStateSubsystem::MaxEquippedIdolSlots);
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

	// Idol slots: 3 stacked squares (placeholder for idols)
	TSharedRef<SVerticalBox> IdolSlotsRef = SNew(SVerticalBox);
	for (int32 i = 0; i < UT66RunStateSubsystem::MaxEquippedIdolSlots; ++i)
	{
		TSharedPtr<SBorder> IdolBorder;
		IdolSlotsRef->AddSlot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SBox)
				.WidthOverride(22.f)
				.HeightOverride(22.f)
				[
					SAssignNew(IdolBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.18f, 0.18f, 0.22f, 1.f))
				]
			];
		IdolSlotBorders[i] = IdolBorder;
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
		// Top-center loot prompt (non-blocking)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.f, 48.f, 0.f, 0.f)
		[
			SAssignNew(LootPromptBox, SBox)
			.WidthOverride(760.f)
			.HeightOverride(40.f)
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(LootPromptBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.65f))
				.Padding(10.f, 6.f)
				[
					SAssignNew(LootPromptText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
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
				.Text(StageInit)
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
					.Text(GoldInit)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
				[
					SAssignNew(DebtText, STextBlock)
					.Text(OweInit)
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
					.Text(BountyLabel)
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
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
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
							.Text(PortraitLabel)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.f))
							.Justification(ETextJustify::Center)
						]
					]
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(14.f, 0.f, 0.f, 0.f)
			[
				IdolSlotsRef
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
					.Text(MinimapLabel)
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
