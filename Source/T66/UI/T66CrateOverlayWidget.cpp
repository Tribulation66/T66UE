// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CrateOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RngTuningConfig.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/Style/T66Style.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	static ET66ItemRarity LootRarityToItemRarity(ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Black:  return ET66ItemRarity::Black;
		case ET66Rarity::Red:    return ET66ItemRarity::Red;
		case ET66Rarity::Yellow: return ET66ItemRarity::Yellow;
		case ET66Rarity::White:  return ET66ItemRarity::White;
		default:                 return ET66ItemRarity::Black;
		}
	}

	static FT66RarityWeights ApplyLootCrateBias(const FT66RarityWeights& BaseWeights, float LootCrateMultiplier)
	{
		const float Bias = FMath::Clamp(LootCrateMultiplier, 1.f, 3.f);

		FT66RarityWeights Result;
		Result.Black = BaseWeights.Black / Bias;
		Result.Red = BaseWeights.Red * FMath::Sqrt(Bias);
		Result.Yellow = BaseWeights.Yellow * Bias;
		Result.White = BaseWeights.White * Bias * Bias;
		return Result;
	}
}

void UT66CrateOverlayWidget::GenerateStrip()
{
	StripItems.Empty();

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

	if (RngSub && RunState)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}

	FRandomStream Rng(static_cast<int32>(FPlatformTime::Cycles()));

	FT66RarityWeights CrateWeights;
	CrateWeights.Black = 0.70f;
	CrateWeights.Red = 0.20f;
	CrateWeights.Yellow = 0.08f;
	CrateWeights.White = 0.02f;
	if (RunState)
	{
		CrateWeights = ApplyLootCrateBias(CrateWeights, RunState->GetLootCrateRewardMultiplier());
	}

	const ET66Rarity WinRarity = (RngSub) ? RngSub->RollRarityWeighted(CrateWeights, Rng) : ET66Rarity::Black;
	WinnerItemID = T66GI ? T66GI->GetRandomItemIDForLootRarity(WinRarity) : FName(TEXT("Unknown"));

	for (int32 i = 0; i < StripItemCount; ++i)
	{
		FCrateItemEntry Entry;
		if (i == WinnerPosition)
		{
			Entry.ItemID = WinnerItemID;
			Entry.Rarity = WinRarity;
			WinnerRarity = WinRarity;
			WinnerIndex = i;
		}
		else
		{
			ET66Rarity DecoyRarity = ET66Rarity::Black;
			const float Roll = Rng.GetFraction();
			const float Total = CrateWeights.Black + CrateWeights.Red + CrateWeights.Yellow + CrateWeights.White;
			float Acc = CrateWeights.Black / Total;
			if (Roll < Acc) DecoyRarity = ET66Rarity::Black;
			else { Acc += CrateWeights.Red / Total; if (Roll < Acc) DecoyRarity = ET66Rarity::Red;
			else { Acc += CrateWeights.Yellow / Total; if (Roll < Acc) DecoyRarity = ET66Rarity::Yellow;
			else DecoyRarity = ET66Rarity::White; }}

			Entry.ItemID = T66GI ? T66GI->GetRandomItemIDForLootRarity(DecoyRarity) : FName(TEXT("Decoy"));
			Entry.Rarity = DecoyRarity;
		}
		Entry.Color = FT66RarityUtil::GetRarityColor(Entry.Rarity);
		StripItems.Add(Entry);
	}

	TotalScrollDistance = static_cast<float>(WinnerPosition - 2) * ItemTileWidth;
}

void UT66CrateOverlayWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScrollTickHandle);
		World->GetTimerManager().ClearTimer(StartHandle);
		World->GetTimerManager().ClearTimer(CloseHandle);
	}

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66CrateOverlayWidget::RebuildWidget()
{
	GenerateStrip();

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	TSharedPtr<SHorizontalBox> StripRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < StripItems.Num(); ++i)
	{
		const FCrateItemEntry& Entry = StripItems[i];
		StripRow->AddSlot()
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(ItemTileWidth - 4.f)
				.HeightOverride(ItemTileWidth - 4.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(Entry.Color)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_ItemDisplayName(Entry.ItemID) : FText::FromName(Entry.ItemID))
						.Font(FT66Style::Tokens::FontRegular(9))
						.ColorAndOpacity(FLinearColor::White)
						.Justification(ETextJustify::Center)
					]
				]
			];
	}

	TSharedRef<SWidget> Root = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.9f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.f))
				.Padding(26.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Crate", "Title", "OPEN CRATE"))
						.Font(FT66Style::Tokens::FontBold(28))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(NSLOCTEXT("T66.Crate", "Opening", "Opening..."))
						.Font(FT66Style::Tokens::FontRegular(14))
						.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 12.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBox)
							.WidthOverride(ItemTileWidth * 5.f)
							.HeightOverride(ItemTileWidth + 10.f)
							.Clipping(EWidgetClipping::ClipToBounds)
							[
								SAssignNew(StripContainer, SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
								.Padding(0.f)
								[
									StripRow.ToSharedRef()
								]
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						[
							SNew(SBox)
							.WidthOverride(4.f)
							.HeightOverride(ItemTileWidth + 10.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(1.f, 1.f, 1.f, 0.8f))
							]
						]
					]
				]
			]
		];

	// Auto-start the scrolling animation after a brief delay
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(StartHandle, this, &UT66CrateOverlayWidget::StartScrolling, 0.3f, false);
	}

	return Root;
}

void UT66CrateOverlayWidget::TickScroll()
{
	if (!bScrolling || !StripContainer.IsValid()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - LastTickTimeSeconds;
	LastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.05f);
	ScrollElapsed += Delta;

	const float Alpha = FMath::Clamp(ScrollElapsed / FMath::Max(0.01f, ScrollDuration), 0.f, 1.f);
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 3.f);
	CurrentScrollOffset = TotalScrollDistance * Ease;

	const FMargin Margin(-CurrentScrollOffset, 0.f, 0.f, 0.f);
	StripContainer->SetPadding(Margin);

	if (Alpha >= 1.f)
	{
		ResolveOpen();
	}
}

void UT66CrateOverlayWidget::ResolveOpen()
{
	if (!bScrolling) return;
	bScrolling = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScrollTickHandle);
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	if (RunState && !WinnerItemID.IsNone())
	{
		RunState->RecordLuckQualityRarity(FName(TEXT("CrateRewardRarity")), WinnerRarity);
		RunState->AddItemWithRarity(WinnerItemID, LootRarityToItemRarity(WinnerRarity));
	}

	if (StatusText.IsValid())
	{
		const FText Msg = FText::Format(
			NSLOCTEXT("T66.Crate", "YouGotFormat", "You received: {0}!"),
			Loc ? Loc->GetText_ItemDisplayName(WinnerItemID) : FText::FromName(WinnerItemID));
		StatusText->SetText(Msg);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CloseHandle, this, &UT66CrateOverlayWidget::CloseAfterResolve, 1.2f, false);
	}
}

void UT66CrateOverlayWidget::CloseAfterResolve()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

void UT66CrateOverlayWidget::StartScrolling()
{
	if (bScrolling) return;
	UWorld* World = GetWorld();
	if (!World) return;

	bScrolling = true;
	ScrollElapsed = 0.f;
	CurrentScrollOffset = 0.f;
	LastTickTimeSeconds = static_cast<float>(World->GetTimeSeconds());

	World->GetTimerManager().ClearTimer(ScrollTickHandle);
	World->GetTimerManager().SetTimer(ScrollTickHandle, this, &UT66CrateOverlayWidget::TickScroll, 0.033f, true);
}
